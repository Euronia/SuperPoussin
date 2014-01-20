#ifndef EXEC_HPP
#define EXEC_HPP

#include <base/array.hpp>
#include <base/real_array.hpp>
#include <base/string.hpp>
#include <base/tree.hpp>

#include "expr.hpp"


typedef CRealArray<int> CParameters;


struct CAttribute
{
	const char *m_pName;
	int m_Value;
	bool m_Const;
	bool m_Defined;
	inline int Compare(const char *pName) const { return strcmp(m_pName, pName); }
};


class CElement
{
private:
	class CDatabase *m_pDatabase;
	const class CElementModel *m_pModel;
	CTree<CAttribute> m_aAttributes;
	bool m_Removed;

	CAttribute *FindAttribute(const char *pName);
	int ComputeValue(const CExpression &Expr, const CParameters *paParameters) const;
	bool CheckIf(struct CIfData *pIfData, const CParameters *paParameters) const;
	void ExecInstructions(const CArray<struct CInstruction *> *papInstructions, const CParameters *paParameters,
						  bool *pReturned, int *pReturnValue);

public:
	CElement(class CDatabase *pDatabase, const class CElementModel *pModel, int Index, CParameters *paParameters);
	inline bool Removed() const { return m_Removed; }
	void SetRemoved();
	const CAttribute *FindAttribute(const char *pName) const;
	int CallFunction(const char *pName, const CParameters *paParameters, bool Super=false, const char *pSuperclass=0);
	const class CElementModel *GetModel() const;
};


struct CCallstackEntry
{
	const char *m_pObjectType;
	const char *m_pObjectName;
	const char *m_pFunctionName;
	int m_CallLine;
	const char *m_pCallFilename;
};

struct CDefaultAttribute
{
	CString m_Name;
	bool m_Parameter;
	int m_Value;
	bool m_Const;
};

struct CElementDefaultAttributes
{
	CString m_Type;
	CArray<CDefaultAttribute> m_aDefaultAttributes;
};

typedef void (*CSysFunctionCallback)(int Line, const char *pFilename, const CParameters *paParameters);

struct CSysFunction
{
	CString m_Name;
	CSysFunctionCallback m_pfnCallback;
};


struct CElementsGroup
{
	CString m_Type;
	CArray<CElement *> m_apList;
};

class CDatabase
{
private:
	int m_CurrentInstructionLine;
	const char *m_pCurrentInstructionFilename;
	CRealArray<CCallstackEntry> m_aCallstack;

	CArray<CSysFunction> m_aSysFunctions;

	const class CModelsDatabase *m_pModelsDatabase;
	int m_ElementsCount;
	CArray<CElementDefaultAttributes> m_aElementsDefaultAttributes;
	CArray<CElementsGroup> m_aElements;

	CElementsGroup *GetElementsGroup(const char *pType);

public:
	CDatabase(const class CModelsDatabase *pModelsDatabase);
	~CDatabase();
	void Cleanup();

	const class CModelsDatabase *GetModelsDatabase() const;

	void SetCurrentInstruction(int Line, const char *m_pFilename);
	void CallstackPush(const char *pObjectType, const char *pObjectName, const char *pFunctionName);
	void CallstackPop();
	void PrintCallstack() const;

	void AddSysFunction(const char *pName, CSysFunctionCallback pfnCallback);
	void CallSysFunction(const char *pFunctionName, const CParameters *paParameters) const;

	CArray<CDefaultAttribute> *GetDefaultAttributes(const char *pElementType);
	void AddDefaultAttribute(const char *pElementType, const char *pName, int Value, bool Const);
	void AddDefaultAttributeParameter(const char *pElementType, const char *pName, bool Const);

	void AddElement(const char *pType, const char *pModelName, CParameters *paParameters);
	void RemoveElement(CElement *pElement);
	CArray<CElement *> *FindAllElements(const char *pElementType);
	void SearchElements(const char *pElementType, const char *pElementName, const char *pSearchFunction,
						const CParameters *paSearchParameters, CArray<CElement *> *papElements);
};


#endif
