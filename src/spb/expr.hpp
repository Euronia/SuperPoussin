#ifndef EXPR_HPP
#define EXPR_HPP

#include <base/array.hpp>
#include <base/real_array.hpp>
#include <base/string.hpp>
#include <base/tree.hpp>


class CExpression
{
private:
	enum
	{
		EXPRESSION_TYPE_OPERATION=0,
		EXPRESSION_TYPE_VALUE,
		EXPRESSION_TYPE_ATTRIBUTE,
		EXPRESSION_TYPE_PARAMETER
	};
	int m_Type;
	CString m_String;

	// operation
	enum
	{
		OPERATOR_NONE=0,
		OPERATOR_NOT,
		OPERATOR_AND,
		OPERATOR_OR,
		OPERATOR_XOR,
		OPERATOR_ADD,
		OPERATOR_SUB,
		OPERATOR_MULT,
		OPERATOR_DIV,
		OPERATOR_MOD,
		OPERATOR_EQUAL,
		OPERATOR_DIFF,
		OPERATOR_HIGHER,
		OPERATOR_LOWER,
		OPERATOR_HIGHER_EQUAL,
		OPERATOR_LOWER_EQUAL,
		NUM_OPERATORS
	};
	int m_Operator;
	CExpression *m_apSubExpressions[2];

	// value
	int m_Value;

	// attribute
	CString m_Attribute;

	// parameter
	int m_ParameterIndex;

	int GetOperator(char Char, char PrevChar, int *pOperatorLen) const;
	bool CheckGlobalParenthesis(const char *pExpr) const;
	int FindOperation(const char *pExpr, char *pPart1, char *pPart2) const;
	bool ParseImp(const char *pExpr, const CTree<struct CConstant> *paConstants);

public:
	CExpression();
	~CExpression();
	void Parse(const char *pExpr, const CTree<struct CConstant> *paConstants);
	int ComputeValue(const CTree<struct CAttribute> *paAttributes, const class CRealArray<int> *paParameters,
					 bool *pError) const;
	const char *GetString() const;
};


#endif
