#ifndef PARSE_HPP
#define PARSE_HPP

#include <base/array.hpp>
#include <base/real_array.hpp>
#include <base/string.hpp>
#include <base/tree.hpp>

#include "expr.hpp"


class CDataPointer
{
private:
	const char *m_pFilename;
	char m_aData[16*1024];
	char *m_pPointer;
	int m_LineCount;

public:
	CDataPointer(const char *pFilename);
	void ReadLine(char *pBuf);
	bool EndReached() const;
	int GetLineCount() const;
	const char *GetFilename() const;
};



typedef CRealArray<CString> CLine;



enum
{
	INSTRUCTION_TYPE_INVALID=0,
	INSTRUCTION_TYPE_RETURN,
	INSTRUCTION_TYPE_IF,
	INSTRUCTION_TYPE_FROM,
	INSTRUCTION_TYPE_SET_ATTRIBUTE,
	INSTRUCTION_TYPE_CALL_FUNCTION,
	INSTRUCTION_TYPE_REMOVE,
	INSTRUCTION_TYPE_ADD,
	INSTRUCTION_TYPE_CALL_SYS
};

struct CInstruction
{
	int m_Type;
	CString m_Filename;
	int m_LineCount;
	CInstruction(int LineCount, const char *pFilename) { m_Type = INSTRUCTION_TYPE_INVALID; m_LineCount = LineCount; m_Filename = pFilename; }
	virtual ~CInstruction() {}
};

struct CInstructions
{
	CArray<CInstruction *> m_apList;
	~CInstructions()
	{
		for(CArray<CInstruction *>::CIterator i = m_apList.GetIterator(); i.Exists(); i.Next())
			MEM_DELETE(i.Get());
	}
};

struct CReturnInstruction : CInstruction
{
	CExpression m_Value;
	CReturnInstruction(int LineCount, const char *pFilename) : CInstruction(LineCount, pFilename) { m_Type = INSTRUCTION_TYPE_RETURN; }
};

struct CIfData
{
	int m_LineCount;
	bool m_Find;
	CString m_ElementType;
	CString m_ElementModelName;
	CString m_SearchFunction;
	CArray<CExpression> m_aSearchParameters;
	CExpression m_Expression;
	CInstructions m_Instructions;
};
struct CIfInstruction : CInstruction
{
	CIfData m_IfData;
	CArray<CIfData> m_aElsifDatas;
	CInstructions m_InstructionsElse;
	CIfInstruction(int LineCount, const char *pFilename) : CInstruction(LineCount, pFilename) { m_Type = INSTRUCTION_TYPE_IF; }
};

struct CFromInstruction : CInstruction
{
	CExpression m_StartValue;
	CExpression m_EndValue;
	bool m_Reverse;
	CInstructions m_Instructions;
	CFromInstruction(int LineCount, const char *pFilename) : CInstruction(LineCount, pFilename) { m_Type = INSTRUCTION_TYPE_FROM; }
};

struct CSetAttributeInstruction : CInstruction
{
	CString m_AttributeName;
	CExpression m_Value;
	CSetAttributeInstruction(int LineCount, const char *pFilename) : CInstruction(LineCount, pFilename) { m_Type = INSTRUCTION_TYPE_SET_ATTRIBUTE; }
};

struct CCallFunctionInstruction : CInstruction
{
	CString m_FunctionName;
	CArray<CExpression> m_aFunctionParameters;

	bool m_This;
	bool m_Super;
	CString m_Superclass;

	CString m_ElementType;
	CString m_ElementModelName;
	CString m_SearchFunction;
	CArray<CExpression> m_aSearchParameters;

	CCallFunctionInstruction(int LineCount, const char *pFilename) : CInstruction(LineCount, pFilename) { m_Type = INSTRUCTION_TYPE_CALL_FUNCTION; }
};

struct CRemoveInstruction : CInstruction
{
	bool m_This;
	CString m_ElementType;
	CString m_ElementModelName;
	CString m_SearchFunction;
	CArray<CExpression> m_aSearchParameters;

	CRemoveInstruction(int LineCount, const char *pFilename) : CInstruction(LineCount, pFilename) { m_Type = INSTRUCTION_TYPE_REMOVE; }
};

struct CAddInstruction : CInstruction
{
	CString m_ElementType;
	CString m_ElementModelName;
	CArray<CExpression> m_aAddParameters;

	CAddInstruction(int LineCount, const char *pFilename) : CInstruction(LineCount, pFilename) { m_Type = INSTRUCTION_TYPE_ADD; }
};

struct CCallSysInstruction : CInstruction
{
	CString m_FunctionName;
	CArray<CExpression> m_aFunctionParameters;
	CCallSysInstruction(int LineCount, const char *pFilename) : CInstruction(LineCount, pFilename) { m_Type = INSTRUCTION_TYPE_CALL_SYS; }
};



class CFunction
{
private:
	CString m_Name;
	int m_LineCount;
	CInstructions m_Instructions;

public:
	CFunction(const char *pName);
	bool Parse(const CTree<CConstant> *paConstants, CDataPointer *pPtr);
	inline const char *GetName() const { return m_Name; }
	const CArray<CInstruction *> *GetInstructions() const;
};



struct CAttributeModel
{
	CString m_Name;
	bool m_Const;
};

struct CFunctionPtr
{
	const char *m_pName;
	CFunction *m_pFunction;
	inline int Compare(const char *pName) const { return strcmp(m_pName, pName); }
	~CFunctionPtr()
	{
		MEM_DELETE(m_pFunction);
	}
};



class CElementModel
{
private:
	class CModelsDatabase *m_pModelsDatabase;
	CString m_Type;
	CString m_Name;
	CString m_Parent;
	const CElementModel *m_pParentModel;
	CArray<CAttributeModel> m_aAttributes;
	CTree<CFunctionPtr> m_aFunctions;

public:
	CElementModel(class CModelsDatabase *pModelsDatabase, const char *pType, const char *pName, const char *pParent);
	bool Parse(CDataPointer *pPtr);
	void Finish();

	const char *GetType() const;
	const char *GetName() const;
	const CElementModel *GetParentModel() const;
	const CElementModel *Inherits(const char *pElementName) const;
	const CFunction *FindFunction(const char *pName) const;
	const CArray<CAttributeModel> *GetAttributes() const;
};



struct CConstant
{
	CString m_Name;
	int m_Value;
	inline int Compare(const char *pName) const { return strcmp(m_Name, pName); }
};

struct CElementModelsGroup
{
	CString m_Type;
	CArray<CElementModel *> m_apList;
	~CElementModelsGroup()
	{
		for(CArray<CElementModel *>::CIterator e = m_apList.GetIterator(); e.Exists(); e.Next())
			MEM_DELETE(e.Get());
	}
};

class CModelsDatabase
{
private:
	CTree<CConstant> m_aConstants;
	CArray<CString> m_aElementTypes;
	CArray<CElementModelsGroup> m_aElementModels;

	CElementModelsGroup *GetElementModelsGroup(const char *pType);

public:
	void AddConstant(const char *pName, int Value);
	void AddElementType(const char *pType);

	bool Parse(CDataPointer *pPtr);
	void Finish();

	const CTree<CConstant> *GetConstants() const;
	const CArray<CString> *GetElementTypes() const;
	const CElementModel *FindElementModel(const char *pType, const char *pName) const; // warning: slow
};


#endif
