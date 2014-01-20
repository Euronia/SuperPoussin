#include <stdlib.h>
#include <stdio.h>

#include <base/memory.hpp>

#include "exec.hpp"
#include "parse.hpp"


// #define SPB_DEBUG


CElement::CElement(class CDatabase *pDatabase, const class CElementModel *pModel, int Index, CParameters *paParameters)
{
	m_pDatabase = pDatabase;
	m_pModel = pModel;

	m_Removed = false;

	const char *pIndexAttributeName = "$index";
	CAttribute *pIndexAttribute = &m_aAttributes.Insert(pIndexAttributeName);
	pIndexAttribute->m_pName = pIndexAttributeName;
	pIndexAttribute->m_Value = Index;
	pIndexAttribute->m_Const = true;
	pIndexAttribute->m_Defined = true;

	const CArray<CDefaultAttribute> *paDefaultAttributes = m_pDatabase->GetDefaultAttributes(pModel->GetType());
	if(!paDefaultAttributes)
		printf("error: can't add default attributes, type '%s' does not exist (?)\n", pModel->GetType());
	else
	{
		for(CArray<CDefaultAttribute>::CIterator a = paDefaultAttributes->GetIterator(); a.Exists(); a.Next())
		{
			CDefaultAttribute *pDefaultAttribute = &a.Get();
			CAttribute *pAttribute = &m_aAttributes.Insert(pDefaultAttribute->m_Name);
			pAttribute->m_pName = pDefaultAttribute->m_Name;
			if(pDefaultAttribute->m_Parameter)
			{
				if(paParameters->Size() > 0)
				{
					pAttribute->m_Value = paParameters->GetAt(0);
					paParameters->RemoveAt(0);
				}
				else
				{
					pAttribute->m_Value = 0;
					printf("error: missing parameter for default attribute of '%s'\n", pModel->GetType());
				}
				pAttribute->m_Defined = true;
			}
			else
			{
				pAttribute->m_Value = pDefaultAttribute->m_Value;
				pAttribute->m_Defined = true;
			}
			pAttribute->m_Const = pDefaultAttribute->m_Const;
		}
	}

	const CElementModel *pEleModel = m_pModel;
	while(pEleModel)
	{
		const CArray<CAttributeModel> *paAttributes = pEleModel->GetAttributes();
		for(CArray<CAttributeModel>::CIterator a = paAttributes->GetIterator(); a.Exists(); a.Next())
		{
			CAttributeModel *pAttributeModel = &a.Get();
			CAttribute *pAttribute = &m_aAttributes.Insert(pAttributeModel->m_Name);
			pAttribute->m_pName = pAttributeModel->m_Name;
			pAttribute->m_Value = 0;
			pAttribute->m_Const = pAttributeModel->m_Const;
			pAttribute->m_Defined = false;
		}

		pEleModel = pEleModel->GetParentModel();
	}
}

void CElement::SetRemoved()
{
	m_Removed = true;
}

const CAttribute *CElement::FindAttribute(const char *pName) const
{
	return m_aAttributes.Find(pName);
}

CAttribute *CElement::FindAttribute(const char *pName)
{
	return m_aAttributes.Find(pName);
}

int CElement::ComputeValue(const CExpression &Expr, const CParameters *paParameters) const
{
	bool Error;
	int Value = Expr.ComputeValue(&m_aAttributes, paParameters, &Error);
	if(Error)
	{
		printf("error: could not compute expression '%s'\n", Expr.GetString());
		m_pDatabase->PrintCallstack();
	}
	return Value;
}

bool CElement::CheckIf(CIfData *pIfData, const CParameters *paParameters) const
{
	if(pIfData->m_Find)
	{
		CParameters aSearchParameters;
		for(CArray<CExpression>::CIterator p = pIfData->m_aSearchParameters.GetIterator(); p.Exists(); p.Next())
			aSearchParameters.Add(ComputeValue(p.Get(), paParameters));
		CArray<CElement *> apElements;
		m_pDatabase->SearchElements(pIfData->m_ElementType, pIfData->m_ElementModelName, pIfData->m_SearchFunction, &aSearchParameters, &apElements);
		bool True = apElements.Size() > 0;
#ifdef SPB_DEBUG
		printf("success: condition find %d\n", True);
#endif
		return True;
	}
	else
	{
		bool True = ComputeValue(pIfData->m_Expression, paParameters) != 0;
#ifdef SPB_DEBUG
		printf("success: condition %d\n", True);
#endif
		return True;
	}
}

void CElement::ExecInstructions(const CArray<CInstruction *> *papInstructions, const CParameters *paParameters,
								bool *pReturned, int *pReturnValue)
{
	*pReturned = false;
	for(CArray<CInstruction *>::CIterator i = papInstructions->GetIterator(); i.Exists(); i.Next())
	{
		if(m_Removed)
			break;

		CInstruction *pBaseInstr = i.Get();
		m_pDatabase->SetCurrentInstruction(pBaseInstr->m_LineCount, pBaseInstr->m_Filename);

		if(pBaseInstr->m_Type == INSTRUCTION_TYPE_SET_ATTRIBUTE)
		{
			CSetAttributeInstruction *pInstr = (CSetAttributeInstruction *)pBaseInstr;
			CAttribute *pAttribute = FindAttribute(pInstr->m_AttributeName);
			if(!pAttribute)
			{
				printf("error: unknown attribute '%s'\n", pInstr->m_AttributeName.GetString());
				m_pDatabase->PrintCallstack();
				continue;
			}
			if(pAttribute->m_Const && pAttribute->m_Defined)
			{
				printf("error: attribute const '%s' is already defined\n", pAttribute->m_pName);
				m_pDatabase->PrintCallstack();
			}
			else
			{
				pAttribute->m_Value = ComputeValue(pInstr->m_Value, paParameters);
				pAttribute->m_Defined = true;
#ifdef SPB_DEBUG
				printf("success: attribute '%s' set to %d\n", pAttribute->m_pName, pAttribute->m_Value);
#endif
			}
		}
		else if(pBaseInstr->m_Type == INSTRUCTION_TYPE_RETURN)
		{
			CReturnInstruction *pInstr = (CReturnInstruction *)pBaseInstr;
			*pReturned = true;
			*pReturnValue = ComputeValue(pInstr->m_Value, paParameters);
#ifdef SPB_DEBUG
			printf("success: returned %d\n", *pReturnValue);
#endif
			return;
		}
		else if(pBaseInstr->m_Type == INSTRUCTION_TYPE_IF)
		{
			CIfInstruction *pInstr = (CIfInstruction *)pBaseInstr;
			bool True = CheckIf(&pInstr->m_IfData, paParameters);
			if(True)
				ExecInstructions(&pInstr->m_IfData.m_Instructions.m_apList, paParameters, pReturned, pReturnValue);
			else
			{
				for(CArray<CIfData>::CIterator i = pInstr->m_aElsifDatas.GetIterator(); i.Exists(); i.Next())
				{
					CIfData *pElsifData = &i.Get();
					m_pDatabase->SetCurrentInstruction(pElsifData->m_LineCount, pBaseInstr->m_Filename);
					True = CheckIf(pElsifData, paParameters);
					if(True)
					{
						ExecInstructions(&pElsifData->m_Instructions.m_apList, paParameters, pReturned, pReturnValue);
						break;
					}
				}
				if(!True)
					ExecInstructions(&pInstr->m_InstructionsElse.m_apList, paParameters, pReturned, pReturnValue);
			}
			if(*pReturned)
				return;
		}
		else if(pBaseInstr->m_Type == INSTRUCTION_TYPE_FROM)
		{
			CFromInstruction *pInstr = (CFromInstruction *)pBaseInstr;
			int StartVal = ComputeValue(pInstr->m_StartValue, paParameters);
			int EndVal = ComputeValue(pInstr->m_EndValue, paParameters);
#ifdef SPB_DEBUG
			printf("success: from %d to %d (reverse %d)\n", StartVal, EndVal, pInstr->m_Reverse);
#endif
			CParameters aFromParameters;
			paParameters->GetCopy(&aFromParameters);
			int *pNewParameter = &aFromParameters.Add();
			if(pInstr->m_Reverse)
			{
				for(int v = StartVal; v >= EndVal; v--)
				{
					*pNewParameter = v;
					ExecInstructions(&pInstr->m_Instructions.m_apList, &aFromParameters, pReturned, pReturnValue);
					if(*pReturned)
						return;
				}
			}
			else
			{
				for(int v = StartVal; v <= EndVal; v++)
				{
					*pNewParameter = v;
					ExecInstructions(&pInstr->m_Instructions.m_apList, &aFromParameters, pReturned, pReturnValue);
					if(*pReturned)
						return;
				}
			}
		}
		else if(pBaseInstr->m_Type == INSTRUCTION_TYPE_CALL_FUNCTION)
		{
			CCallFunctionInstruction *pInstr = (CCallFunctionInstruction *)pBaseInstr;

			CParameters aFunctionParameters;
			for(CArray<CExpression>::CIterator p = pInstr->m_aFunctionParameters.GetIterator(); p.Exists(); p.Next())
				aFunctionParameters.Add(ComputeValue(p.Get(), paParameters));

			if(pInstr->m_This)
				CallFunction(pInstr->m_FunctionName, &aFunctionParameters);
			else if(pInstr->m_Super)
				CallFunction(pInstr->m_FunctionName, &aFunctionParameters, true, pInstr->m_Superclass);
			else
			{
				CParameters aSearchParameters;
				for(CArray<CExpression>::CIterator p = pInstr->m_aSearchParameters.GetIterator(); p.Exists(); p.Next())
					aSearchParameters.Add(ComputeValue(p.Get(), paParameters));

				CArray<CElement *> apElements;
				m_pDatabase->SearchElements(pInstr->m_ElementType, pInstr->m_ElementModelName, pInstr->m_SearchFunction, &aSearchParameters, &apElements);

				for(CArray<CElement *>::CIterator e = apElements.GetIterator(); e.Exists(); e.Next())
				{
					if(!e.Get()->m_Removed)
						e.Get()->CallFunction(pInstr->m_FunctionName, &aFunctionParameters);
				}
			}
		}
		else if(pBaseInstr->m_Type == INSTRUCTION_TYPE_REMOVE)
		{
			CRemoveInstruction *pInstr = (CRemoveInstruction *)pBaseInstr;
			if(pInstr->m_This)
				m_pDatabase->RemoveElement(this);
			else
			{
				CParameters aSearchParameters;
				for(CArray<CExpression>::CIterator p = pInstr->m_aSearchParameters.GetIterator(); p.Exists(); p.Next())
					aSearchParameters.Add(ComputeValue(p.Get(), paParameters));

				CArray<CElement *> apElements;
				m_pDatabase->SearchElements(pInstr->m_ElementType, pInstr->m_ElementModelName, pInstr->m_SearchFunction, &aSearchParameters, &apElements);

				bool DeleteThis = false;
				for(CArray<CElement *>::CIterator e = apElements.GetIterator(); e.Exists(); e.Next())
				{
					CElement *pElement = e.Get();
					if(pElement->m_Removed)
						continue;
					if(pElement == this)
						DeleteThis = true;
					else
						m_pDatabase->RemoveElement(pElement);
				}
				if(DeleteThis)
					m_pDatabase->RemoveElement(this);
			}
		}
		else if(pBaseInstr->m_Type == INSTRUCTION_TYPE_ADD)
		{
			CAddInstruction *pInstr = (CAddInstruction *)pBaseInstr;
			CParameters aAddParameters;
			for(CArray<CExpression>::CIterator p = pInstr->m_aAddParameters.GetIterator(); p.Exists(); p.Next())
				aAddParameters.Add(ComputeValue(p.Get(), paParameters));
			m_pDatabase->AddElement(pInstr->m_ElementType, pInstr->m_ElementModelName, &aAddParameters);
		}
		else if(pBaseInstr->m_Type == INSTRUCTION_TYPE_CALL_SYS)
		{
			CCallSysInstruction *pInstr = (CCallSysInstruction *)pBaseInstr;

			CParameters aFunctionParameters;
			for(CArray<CExpression>::CIterator p = pInstr->m_aFunctionParameters.GetIterator(); p.Exists(); p.Next())
				aFunctionParameters.Add(ComputeValue(p.Get(), paParameters));

			m_pDatabase->CallSysFunction(pInstr->m_FunctionName, &aFunctionParameters);
		}
	}
}

int CElement::CallFunction(const char *pName, const CParameters *paParameters, bool Super, const char *pSuperclass)
{
	const CElementModel *pModel = m_pModel;
	if(Super)
	{
		pModel = m_pModel->Inherits(pSuperclass);
		if(!pModel)
		{
			printf("error: '%s' does not inherit from '%s'\n", m_pModel->GetName(), pSuperclass);
			m_pDatabase->PrintCallstack();
			return 0;
		}
	}

	const CFunction *pFunction = pModel->FindFunction(pName);
	if(!pFunction)
	{
		if(pName[0] != '$')
		{
			printf("error: unknown function '%s'\n", pName);
			m_pDatabase->PrintCallstack();
		}
		return 0;
	}

	m_pDatabase->CallstackPush(m_pModel->GetType(), m_pModel->GetName(), pFunction->GetName()); // TODO: function model name

	bool Returned;
	int ReturnValue = 0;
	ExecInstructions(pFunction->GetInstructions(), paParameters, &Returned, &ReturnValue);

	m_pDatabase->CallstackPop();

	return ReturnValue;
}

const class CElementModel *CElement::GetModel() const
{
	return m_pModel;
}



CDatabase::CDatabase(const class CModelsDatabase *pModelsDatabase)
{
	m_CurrentInstructionLine = 0;

	m_pModelsDatabase = pModelsDatabase;
	m_ElementsCount = 0;

	const CArray<CString> *paElementTypes = m_pModelsDatabase->GetElementTypes();
	for(CArray<CString>::CIterator t = paElementTypes->GetIterator(); t.Exists(); t.Next())
	{
		CElementDefaultAttributes *pNew = &m_aElementsDefaultAttributes.Add();
		pNew->m_Type = t.Get();
	}
}

CDatabase::~CDatabase()
{
	for(CArray<CElementsGroup>::CIterator g = m_aElements.GetIterator(); g.Exists(); g.Next())
	{
		CElementsGroup *pGroup = &g.Get();
		CArray<CElement *>::CRemoveIterator o = g.Get().m_apList.GetRemoveIterator();
		while(o.Exists())
		{
			MEM_DELETE(o.Get());
			o.RemoveAndNext();
		}
	}
}

void CDatabase::Cleanup()
{
	for(CArray<CElementsGroup>::CIterator g = m_aElements.GetIterator(); g.Exists(); g.Next())
	{
		CElementsGroup *pGroup = &g.Get();
		CArray<CElement *>::CRemoveIterator o = g.Get().m_apList.GetRemoveIterator();
		while(o.Exists())
		{
			if(o.Get()->Removed())
			{
				MEM_DELETE(o.Get());
				o.RemoveAndNext();
			}
			else
				o.Next();
		}
	}
}

CElementsGroup *CDatabase::GetElementsGroup(const char *pType)
{
	for(CArray<CElementsGroup>::CIterator g = m_aElements.GetIterator(); g.Exists(); g.Next())
	{
		if(strcmp(g.Get().m_Type, pType) == 0)
			return &g.Get();
	}
	CElementsGroup *pGroup = &m_aElements.Add();
	pGroup->m_Type = pType;
	return pGroup;
}

const CModelsDatabase *CDatabase::GetModelsDatabase() const
{
	return m_pModelsDatabase;
}

void CDatabase::SetCurrentInstruction(int Line, const char *m_pFilename)
{
	m_CurrentInstructionLine = Line;
	m_pCurrentInstructionFilename = m_pFilename;
}

void CDatabase::CallstackPush(const char *pObjectType, const char *pObjectName, const char *pFunctionName)
{
	CCallstackEntry *pEntry = &m_aCallstack.Add();
	pEntry->m_pObjectType = pObjectType;
	pEntry->m_pObjectName = pObjectName;
	pEntry->m_pFunctionName = pFunctionName;
	pEntry->m_CallLine = m_CurrentInstructionLine;
	pEntry->m_pCallFilename = m_pCurrentInstructionFilename;
}

void CDatabase::CallstackPop()
{
	CCallstackEntry *pEntry = &m_aCallstack.GetAt(m_aCallstack.Size()-1);
	m_CurrentInstructionLine = pEntry->m_CallLine;
	m_pCurrentInstructionFilename = pEntry->m_pCallFilename;

	m_aCallstack.RemoveAt(m_aCallstack.Size()-1);
}

void CDatabase::PrintCallstack() const
{
	if(m_CurrentInstructionLine == 0)
		return;

	printf("at instruction line %d from file %s\n", m_CurrentInstructionLine, m_pCurrentInstructionFilename);
	for(int e = m_aCallstack.Size()-1; e >= 0; e--)
	{
		const CCallstackEntry *pEntry = &m_aCallstack.GetAt(e);
		printf("in function %s in %s %s called ", pEntry->m_pFunctionName, pEntry->m_pObjectType, pEntry->m_pObjectName);
		if(pEntry->m_CallLine == 0)
			printf("from code\n");
		else
			printf("at line %d from file %s\n", pEntry->m_CallLine, pEntry->m_pCallFilename);
	}
}

void CDatabase::AddSysFunction(const char *pName, CSysFunctionCallback pfnCallback)
{
	CSysFunction *pSysFunction = &m_aSysFunctions.Add();
	pSysFunction->m_Name = pName;
	pSysFunction->m_pfnCallback = pfnCallback;
}

void CDatabase::CallSysFunction(const char *pFunctionName, const CParameters *paParameters) const
{
	bool Found = false;
	for(CArray<CSysFunction>::CIterator f = m_aSysFunctions.GetIterator(); f.Exists(); f.Next())
	{
		if(strcmp(pFunctionName, f.Get().m_Name) == 0)
		{
			f.Get().m_pfnCallback(m_CurrentInstructionLine, m_pCurrentInstructionFilename, paParameters);
			Found = true;
			break;
		}
	}
	if(!Found)
	{
		printf("error: couldn't find sys function '%s'\n", pFunctionName);
		PrintCallstack();
	}
}

CArray<CDefaultAttribute> *CDatabase::GetDefaultAttributes(const char *pElementType)
{
	for(CArray<CElementDefaultAttributes>::CIterator e = m_aElementsDefaultAttributes.GetIterator(); e.Exists(); e.Next())
	{
		if(strcmp(e.Get().m_Type, pElementType) == 0)
			return &e.Get().m_aDefaultAttributes;
	}
	return 0;
}

void CDatabase::AddDefaultAttribute(const char *pElementType, const char *pName, int Value, bool Const)
{
	CArray<CDefaultAttribute> *paDefaultAttributes = GetDefaultAttributes(pElementType);
	if(!paDefaultAttributes)
	{
		printf("error: can't add default attribute '%s', type '%s' does not exist\n", pName, pElementType);
		return;
	}
	CDefaultAttribute *pDefaultAttribute = &paDefaultAttributes->Add();
	pDefaultAttribute->m_Name = pName;
	pDefaultAttribute->m_Parameter = false;
	pDefaultAttribute->m_Value = Value;
	pDefaultAttribute->m_Const = Const;
}

void CDatabase::AddDefaultAttributeParameter(const char *pElementType, const char *pName, bool Const)
{
	CArray<CDefaultAttribute> *paDefaultAttributes = GetDefaultAttributes(pElementType);
	if(!paDefaultAttributes)
	{
		printf("error: can't add default attribute '%s', type '%s' does not exist\n", pName, pElementType);
		return;
	}
	CDefaultAttribute *pDefaultAttribute = &paDefaultAttributes->Add();
	pDefaultAttribute->m_Name = pName;
	pDefaultAttribute->m_Parameter = true;
	pDefaultAttribute->m_Value = 0;
	pDefaultAttribute->m_Const = Const;
}

void CDatabase::AddElement(const char *pType, const char *pModelName, CParameters *paParameters)
{
	const CElementModel *pModel = m_pModelsDatabase->FindElementModel(pType, pModelName);
	if(!pModel)
	{
		printf("error: couldn't find model '%s' of type '%s'\n", pModelName, pType);
		PrintCallstack();
		return;
	}

	CElement *pElement = MEM_NEW(CElement(this, pModel, m_ElementsCount++, paParameters));
	CElementsGroup *pGroup = GetElementsGroup(pType);
	pGroup->m_apList.Add(pElement);
	pElement->CallFunction("$on_add", paParameters);

#ifdef SPB_DEBUG
	printf("success: added element %p (type '%s', model '%s')\n", pElement, pType, pModelName);
#endif
}

void CDatabase::RemoveElement(CElement *pElement)
{
	CParameters aParameters;
	pElement->CallFunction("$on_remove", &aParameters);
	CElementsGroup *pGroup = GetElementsGroup(pElement->GetModel()->GetType());
	pElement->SetRemoved();
#ifdef SPB_DEBUG
	printf("success: removed element %p\n", pElement);
#endif
}

CArray<CElement *> *CDatabase::FindAllElements(const char *pElementType)
{
	return &GetElementsGroup(pElementType)->m_apList;
}

void CDatabase::SearchElements(const char *pElementType, const char *pElementName, const char *pSearchFunction,
							   const CParameters *paSearchParameters, CArray<CElement *> *papElements)
{
	CElementsGroup *pGroup = GetElementsGroup(pElementType);
	for(CArray<CElement *>::CIterator e = pGroup->m_apList.GetIterator(); e.Exists(); e.Next())
	{
		CElement *pElement = e.Get();
		if(!pElement->Removed() && pElement->GetModel()->Inherits(pElementName))
		{
			if(pElement->CallFunction(pSearchFunction, paSearchParameters))
			{
				papElements->Add(pElement);
#ifdef SPB_DEBUG
				printf("success: found object %p\n", pElement);
#endif
			}
		}
	}
}
