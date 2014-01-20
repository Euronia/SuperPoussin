#include <stdlib.h>
#include <stdio.h>

#include <base/memory.hpp>

#include "parse.hpp"


CDataPointer::CDataPointer(const char *pFilename)
{
	m_pFilename = pFilename;
	m_pPointer = 0;
	m_LineCount = 0;

	FILE *pFile = fopen(pFilename, "rb");
	if(!pFile)
		return;
	int Len = fread(m_aData, 1, sizeof(m_aData)-1, pFile);
	fclose(pFile);
	m_aData[Len] = 0;
	m_pPointer = m_aData;
}

void CDataPointer::ReadLine(char *pBuf)
{
	if(!m_pPointer)
		return;

	while(*m_pPointer && *m_pPointer != '\r' && *m_pPointer != '\n')
		*pBuf++ = *m_pPointer++;
	*pBuf = 0;

	while(*m_pPointer == '\r' || *m_pPointer == '\n')
	{
		if(*m_pPointer == '\n')
			m_LineCount++;
		m_pPointer++;
	}
}

bool CDataPointer::EndReached() const
{
	return !m_pPointer || *m_pPointer == 0;
}

int CDataPointer::GetLineCount() const
{
	return m_LineCount;
}

const char *CDataPointer::GetFilename() const
{
	return m_pFilename;
}



void GetLine(CLine *pLine, CDataPointer *pPtr)
{
	char aLineBuf[1024];
	pPtr->ReadLine(aLineBuf);

	char aWordBuf[1024];
	char *pLineBuf = aLineBuf;
	char *pWordBuf = aWordBuf;

	while(*pLineBuf == ' ' || *pLineBuf == '\t')
		*pLineBuf++;

	while(*pLineBuf)
	{
		if(*pLineBuf == '/' && *(pLineBuf+1) == '/')
			break;
		*pWordBuf++ = *pLineBuf++;
		if(*pLineBuf == ' ' || *pLineBuf == '\t' || *pLineBuf == 0)
		{
			*pWordBuf = 0;
			pLine->Add(aWordBuf);
			pWordBuf = aWordBuf;

			while(*pLineBuf == ' ' || *pLineBuf == '\t')
				*pLineBuf++;
		}
	}
}


bool CheckValidIf(const CLine *pLine)
{
	if(pLine->Size() < 2)
		return false;
	bool Find = (strcmp(pLine->GetAt(1), "find") == 0);
	if(Find && pLine->Size() < 5)
		return false;
	else if(!Find && pLine->Size() != 2)
		return false;
	return true;
}

void ParseIfData(const CTree<CConstant> *paConstants, const CLine *pLine, CIfData *pOut)
{
	if(strcmp(pLine->GetAt(1), "find") == 0)
	{
		pOut->m_Find = true;
		pOut->m_ElementType = pLine->GetAt(2);
		pOut->m_ElementModelName = pLine->GetAt(3);
		pOut->m_SearchFunction = pLine->GetAt(4);
		for(int w = 5; w < pLine->Size(); w++)
			pOut->m_aSearchParameters.Add().Parse(pLine->GetAt(w), paConstants);
	}
	else
	{
		pOut->m_Find = false;
		pOut->m_Expression.Parse(pLine->GetAt(1), paConstants);
	}
}

bool ParseInstructions(const CTree<CConstant> *paConstants, CDataPointer *pPtr, bool ElsePossible,
					   CArray<CInstruction *> *papInstructions, CLine *pEndLine)
{
	while(!pPtr->EndReached())
	{
		CLine Line;
		GetLine(&Line, pPtr);
		if(Line.Size() == 0)
			continue;

		if(strcmp(Line.GetAt(0), "end") == 0)
		{
			if(Line.Size() == 1)
			{
				if(pEndLine)
					Line.GetCopy(pEndLine);
				return true;
			}
			else
				return false;
		}
		else if(strcmp(Line.GetAt(0), "else") == 0)
		{
			if(ElsePossible && Line.Size() == 1)
			{
				if(pEndLine)
					Line.GetCopy(pEndLine);
				return true;
			}
			else
				return false;
		}
		else if(strcmp(Line.GetAt(0), "elsif") == 0)
		{
			if(ElsePossible && Line.Size() >= 1)
			{
				if(pEndLine)
					Line.GetCopy(pEndLine);
				return true;
			}
			else
				return false;
		}
		else if(strcmp(Line.GetAt(0), "return") == 0)
		{
			if(Line.Size() != 2)
				return false;
			CReturnInstruction *pInstr = MEM_NEW(CReturnInstruction(pPtr->GetLineCount(), pPtr->GetFilename()));
			pInstr->m_Value.Parse(Line.GetAt(1), paConstants);
			papInstructions->Add(pInstr);
		}
		else if(strcmp(Line.GetAt(0), "if") == 0)
		{
			if(!CheckValidIf(&Line))
				return false;

			CIfInstruction *pInstr = MEM_NEW(CIfInstruction(pPtr->GetLineCount(), pPtr->GetFilename()));
			ParseIfData(paConstants, &Line, &pInstr->m_IfData);
			papInstructions->Add(pInstr);

			CLine EndLine;
			bool Success = ParseInstructions(paConstants, pPtr, true, &pInstr->m_IfData.m_Instructions.m_apList, &EndLine);
			if(!Success)
				return false;

			while(strcmp(EndLine.GetAt(0), "end") != 0)
			{
				if(strcmp(EndLine.GetAt(0), "elsif") == 0)
				{
					if(!CheckValidIf(&EndLine))
						return false;
					CIfData *pElsifData = &pInstr->m_aElsifDatas.Add();
					pElsifData->m_LineCount = pPtr->GetLineCount();
					ParseIfData(paConstants, &EndLine, pElsifData);

					EndLine.Clear();
					Success = ParseInstructions(paConstants, pPtr, true, &pElsifData->m_Instructions.m_apList, &EndLine);
					if(!Success)
						return false;
				}
				if(strcmp(EndLine.GetAt(0), "else") == 0)
				{
					EndLine.Clear();
					Success = ParseInstructions(paConstants, pPtr, false, &pInstr->m_InstructionsElse.m_apList, &EndLine);
					if(!Success)
						return false;
				}
			}
		}
		else if(strcmp(Line.GetAt(0), "from") == 0)
		{
			if(Line.Size() != 4)
				return false;
			bool Reverse;
			if(strcmp(Line.GetAt(2), "to") == 0)
				Reverse = false;
			else if(strcmp(Line.GetAt(2), "downto") == 0)
				Reverse = true;
			else
				return false;

			CFromInstruction *pInstr = MEM_NEW(CFromInstruction(pPtr->GetLineCount(), pPtr->GetFilename()));
			pInstr->m_StartValue.Parse(Line.GetAt(1), paConstants);
			pInstr->m_EndValue.Parse(Line.GetAt(3), paConstants);
			pInstr->m_Reverse = Reverse;
			papInstructions->Add(pInstr);

			bool Success = ParseInstructions(paConstants, pPtr, false, &pInstr->m_Instructions.m_apList, 0);
			if(!Success)
				return false;
		}
		else if(strcmp(Line.GetAt(0), "set_attribute") == 0)
		{
			if(Line.Size() != 3)
				return false;
			CSetAttributeInstruction *pInstr = MEM_NEW(CSetAttributeInstruction(pPtr->GetLineCount(), pPtr->GetFilename()));
			pInstr->m_AttributeName = Line.GetAt(1);
			pInstr->m_Value.Parse(Line.GetAt(2), paConstants);
			papInstructions->Add(pInstr);
		}
		else if(strcmp(Line.GetAt(0), "call_function") == 0)
		{
			if(Line.Size() < 4)
				return false;
			CCallFunctionInstruction *pInstr = MEM_NEW(CCallFunctionInstruction(pPtr->GetLineCount(), pPtr->GetFilename()));
			pInstr->m_FunctionName = Line.GetAt(1);
			int OfIndex = -1;
			for(int w = 2; w < Line.Size(); w++)
			{
				if(strcmp(Line.GetAt(w), "of") == 0)
				{
					OfIndex = w;
					break;
				}
			}
			if(OfIndex == -1)
				return false;
			for(int w = 2; w < OfIndex; w++)
				pInstr->m_aFunctionParameters.Add().Parse(Line.GetAt(w), paConstants);

			bool This = (strcmp(Line.GetAt(OfIndex+1), "this") == 0);
			bool Super = (strcmp(Line.GetAt(OfIndex+1), "super") == 0);
			if(This)
			{
				if(Line.Size() != OfIndex+2)
					return false;
			}
			else if(Super)
			{
				if(Line.Size() != OfIndex+3)
					return false;
			}
			else if(Line.Size() < OfIndex+4)
				return false;

			pInstr->m_This = false;
			pInstr->m_Super = false;
			if(This)
				pInstr->m_This = true;
			else if(Super)
			{
				pInstr->m_Super = true;
				pInstr->m_Superclass = Line.GetAt(OfIndex+2);
			}
			else
			{
				pInstr->m_ElementType = Line.GetAt(OfIndex+1);
				pInstr->m_ElementModelName = Line.GetAt(OfIndex+2);
				pInstr->m_SearchFunction = Line.GetAt(OfIndex+3);
				for(int w = OfIndex+4; w < Line.Size(); w++)
					pInstr->m_aSearchParameters.Add().Parse(Line.GetAt(w), paConstants);
			}
			papInstructions->Add(pInstr);
		}
		else if(strcmp(Line.GetAt(0), "remove") == 0)
		{
			if(Line.Size() < 2)
				return false;
			bool This = (strcmp(Line.GetAt(1), "this") == 0);
			if(This && Line.Size() != 2)
				return false;
			else if(!This && Line.Size() < 4)
				return false;

			CRemoveInstruction *pInstr = MEM_NEW(CRemoveInstruction(pPtr->GetLineCount(), pPtr->GetFilename()));
			if(This)
				pInstr->m_This = true;
			else
			{
				pInstr->m_This = false;
				pInstr->m_ElementType = Line.GetAt(1);
				pInstr->m_ElementModelName = Line.GetAt(2);
				pInstr->m_SearchFunction = Line.GetAt(3);
				for(int w = 4; w < Line.Size(); w++)
					pInstr->m_aSearchParameters.Add().Parse(Line.GetAt(w), paConstants);
			}
			papInstructions->Add(pInstr);
		}
		else if(strcmp(Line.GetAt(0), "add") == 0)
		{
			if(Line.Size() < 3)
				return false;
			if(strcmp(Line.GetAt(1), "this") == 0)
				return false;

			CAddInstruction *pInstr = MEM_NEW(CAddInstruction(pPtr->GetLineCount(), pPtr->GetFilename()));
			pInstr->m_ElementType = Line.GetAt(1);
			pInstr->m_ElementModelName = Line.GetAt(2);
			for(int w = 3; w < Line.Size(); w++)
				pInstr->m_aAddParameters.Add().Parse(Line.GetAt(w), paConstants);
			papInstructions->Add(pInstr);
		}
		else if(strcmp(Line.GetAt(0), "call_sys") == 0)
		{
			if(Line.Size() < 2)
				return false;

			CCallSysInstruction *pInstr = MEM_NEW(CCallSysInstruction(pPtr->GetLineCount(), pPtr->GetFilename()));
			pInstr->m_FunctionName = Line.GetAt(1);
			for(int w = 2; w < Line.Size(); w++)
				pInstr->m_aFunctionParameters.Add().Parse(Line.GetAt(w), paConstants);
			papInstructions->Add(pInstr);
		}
		else
		{
			printf("Unknown instruction %s\n", Line.GetAt(0).GetString());
			return false;
		}
	}

	return false;
}



CFunction::CFunction(const char *pName)
{
	m_Name = pName;
}

bool CFunction::Parse(const CTree<CConstant> *paConstants, CDataPointer *pPtr)
{
	return ParseInstructions(paConstants, pPtr, false, &m_Instructions.m_apList, 0);
}

const CArray<CInstruction *> *CFunction::GetInstructions() const
{
	return &m_Instructions.m_apList;
}



CElementModel::CElementModel(class CModelsDatabase *pModelsDatabase, const char *pType, const char *pName, const char *pParent)
{
	m_pModelsDatabase = pModelsDatabase;
	m_Type = pType;
	m_Name = pName;
	m_Parent = pParent;
	m_pParentModel = 0;
}

bool CElementModel::Parse(CDataPointer *pPtr)
{
	while(!pPtr->EndReached())
	{
		CLine Line;
		GetLine(&Line, pPtr);
		if(Line.Size() == 0)
			continue;

		if(strcmp(Line.GetAt(0), "end") == 0)
		{
			if(Line.Size() == 1)
				return true;
			else
				return false;
		}
		else if(strcmp(Line.GetAt(0), "attribute") == 0)
		{
			if(Line.Size() != 2 && Line.Size() != 3)
				return false;

			if(Line.Size() == 2)
			{
				CAttributeModel *pAttribute = &m_aAttributes.Add();
				pAttribute->m_Name = Line.GetAt(1);
				pAttribute->m_Const = false;
			}
			else
			{
				bool Const = (strcmp(Line.GetAt(1), "const") == 0);
				if(!Const)
					return false;
				CAttributeModel *pAttribute = &m_aAttributes.Add();
				pAttribute->m_Name = Line.GetAt(2);
				pAttribute->m_Const = true;
			}
		}
		else if(strcmp(Line.GetAt(0), "function") == 0)
		{
			if(Line.Size() != 2)
				return false;

			CFunction *pFunction = MEM_NEW(CFunction(Line.GetAt(1)));
			bool Success = pFunction->Parse(m_pModelsDatabase->GetConstants(), pPtr);
			if(!Success)
				return false;
			CFunctionPtr *pFunctionPtr = &m_aFunctions.Insert(pFunction->GetName());
			pFunctionPtr->m_pName = pFunction->GetName();
			pFunctionPtr->m_pFunction = pFunction;
		}
		else
			return false;
	}

	return false;
}

void CElementModel::Finish()
{
	if(m_Parent)
	{
		m_pParentModel = m_pModelsDatabase->FindElementModel(m_Type, m_Parent);
		if(!m_pParentModel)
		{
			printf("error: unknown %s '%s' inherited by '%s'\n", m_Type.GetString(), m_Parent.GetString(), m_Name.GetString());
			m_pParentModel = m_pModelsDatabase->FindElementModel(m_Type, "any");
		}
	}
}

const char *CElementModel::GetType() const
{
	return m_Type;
}

const char *CElementModel::GetName() const
{
	return m_Name;
}

const CElementModel *CElementModel::GetParentModel() const
{
	return m_pParentModel;
}

const CElementModel *CElementModel::Inherits(const char *pElementName) const
{
	if(strcmp(m_Name, pElementName) == 0)
		return this;
	else if(m_pParentModel)
		return m_pParentModel->Inherits(pElementName);
	else
		return 0;
}

const CFunction *CElementModel::FindFunction(const char *pName) const
{
	const CFunctionPtr *pFunctionPtr = m_aFunctions.Find(pName);
	if(pFunctionPtr)
		return pFunctionPtr->m_pFunction;
	if(!m_pParentModel)
		return 0;
	return m_pParentModel->FindFunction(pName);
}

const CArray<CAttributeModel> *CElementModel::GetAttributes() const
{
	return &m_aAttributes;
}



CElementModelsGroup *CModelsDatabase::GetElementModelsGroup(const char *pType)
{
	for(CArray<CElementModelsGroup>::CIterator g = m_aElementModels.GetIterator(); g.Exists(); g.Next())
	{
		if(strcmp(g.Get().m_Type, pType) == 0)
			return &g.Get();
	}
	CElementModelsGroup *pGroup = &m_aElementModels.Add();
	pGroup->m_Type = pType;
	return pGroup;
}

void CModelsDatabase::AddConstant(const char *pName, int Value)
{
	CConstant *pConstant = &m_aConstants.Insert(pName);
	pConstant->m_Name = pName;
	pConstant->m_Value = Value;
}

void CModelsDatabase::AddElementType(const char *pType)
{
	m_aElementTypes.Add(pType);
}

bool CModelsDatabase::Parse(CDataPointer *pPtr)
{
	while(!pPtr->EndReached())
	{
		CLine Line;
		GetLine(&Line, pPtr);
		if(Line.Size() == 0)
			continue;

		if(Line.Size() != 2 && Line.Size() != 4)
			return false;

		bool ValidType = false;
		for(CArray<CString>::CIterator t = m_aElementTypes.GetIterator(); t.Exists(); t.Next())
		{
			if(strcmp(Line.GetAt(0), t.Get()) == 0)
			{
				ValidType = true;
				break;
			}
		}

		if(ValidType)
		{
			CString Parent = "any";
			if(Line.Size() == 4)
			{
				if(strcmp(Line.GetAt(2), "is") != 0)
					return false;
				Parent = Line.GetAt(3);
			}
			else if(strcmp(Line.GetAt(1), "any") == 0)
				Parent = 0;
			CElementModel *pElementModel = MEM_NEW(CElementModel(this, Line.GetAt(0), Line.GetAt(1), Parent));
			bool Success = pElementModel->Parse(pPtr);
			if(!Success)
				return false;
			CElementModelsGroup *pGroup = GetElementModelsGroup(Line.GetAt(0));
			pGroup->m_apList.Add(pElementModel);
		}
		else
		{
			printf("error: unknown element type '%s'\n", Line.GetAt(0).GetString());
			return false;
		}
	}

	return true;
}

void CModelsDatabase::Finish()
{
	for(CArray<CElementModelsGroup>::CIterator g = m_aElementModels.GetIterator(); g.Exists(); g.Next())
	{
		CElementModelsGroup *pGroup = &g.Get();
		for(CArray<CElementModel *>::CIterator m = pGroup->m_apList.GetIterator(); m.Exists(); m.Next())
		{
			m.Get()->Finish();
		}
	}
}

const CTree<CConstant> *CModelsDatabase::GetConstants() const
{
	return &m_aConstants;
}

const CArray<CString> *CModelsDatabase::GetElementTypes() const
{
	return &m_aElementTypes;
}

const CElementModel *CModelsDatabase::FindElementModel(const char *pType, const char *pName) const
{
	CElementModelsGroup *pGroup = 0;
	for(CArray<CElementModelsGroup>::CIterator g = m_aElementModels.GetIterator(); g.Exists(); g.Next())
	{
		if(strcmp(g.Get().m_Type, pType) == 0)
		{
			pGroup = &g.Get();
			break;
		}
	}
	if(!pGroup)
		return 0;
	for(CArray<CElementModel *>::CIterator m = pGroup->m_apList.GetIterator(); m.Exists(); m.Next())
	{
		if(strcmp(m.Get()->GetName(), pName) == 0)
			return m.Get();
	}
	return 0;
}
