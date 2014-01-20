#include <stdio.h>

#include <base/memory.hpp>

#include "exec.hpp"
#include "expr.hpp"
#include "parse.hpp"


CExpression::CExpression()
{
	m_Type = EXPRESSION_TYPE_VALUE;
	m_Value = 0;
	for(int i = 0; i < 2; i++)
		m_apSubExpressions[i] = 0;
}

CExpression::~CExpression()
{
	for(int i = 0; i < 2; i++)
	{
		if(m_apSubExpressions[i])
			MEM_DELETE(m_apSubExpressions[i]);
	}
}

int CExpression::GetOperator(char Char, char PrevChar, int *pOperatorLen) const
{
	static bool s_Init = true;
	static int s_aSimpleOperators[256];
	static int s_aDoubleOperators[256];
	if(s_Init)
	{
		for(int i = 0; i < 256; i++)
		{
			s_aSimpleOperators[i] = OPERATOR_NONE;
			s_aDoubleOperators[i] = OPERATOR_NONE;
		}
		s_aSimpleOperators['!'] = OPERATOR_NOT;
		s_aSimpleOperators['&'] = OPERATOR_AND;
		s_aSimpleOperators['|'] = OPERATOR_OR;
		s_aSimpleOperators['^'] = OPERATOR_XOR;
		s_aSimpleOperators['+'] = OPERATOR_ADD;
		s_aSimpleOperators['-'] = OPERATOR_SUB;
		s_aSimpleOperators['*'] = OPERATOR_MULT;
		s_aSimpleOperators['/'] = OPERATOR_DIV;
		s_aSimpleOperators['%'] = OPERATOR_MOD;
		s_aSimpleOperators['='] = OPERATOR_EQUAL;
		s_aSimpleOperators['>'] = OPERATOR_HIGHER;
		s_aSimpleOperators['<'] = OPERATOR_LOWER;

		s_aDoubleOperators['!'] = OPERATOR_DIFF;
		s_aDoubleOperators['>'] = OPERATOR_HIGHER_EQUAL;
		s_aDoubleOperators['<'] = OPERATOR_LOWER_EQUAL;

		s_Init = false;
	}

	if(Char == '=')
	{
		int Operator = s_aDoubleOperators[(unsigned char)PrevChar];
		if(Operator != OPERATOR_NONE)
		{
			*pOperatorLen = 2;
			return Operator;
		}
	}

	int Operator = s_aSimpleOperators[(unsigned char)Char];
	if(Operator != OPERATOR_NONE)
	{
		*pOperatorLen = 1;
		return Operator;
	}

	return OPERATOR_NONE;
}

bool CExpression::CheckGlobalParenthesis(const char *pExpr) const
{
	int Len = strlen(pExpr);
	if(Len == 0)
		return false;
	if(pExpr[0] != '(' || pExpr[Len-1] != ')')
		return false;
	int Count = 1;
	for(int i = 1; i < Len-1; i++)
	{
		char Char = pExpr[i];
		if(Char == '(')
			Count++;
		else if(Char == ')')
			Count--;
		if(Count < 1)
			return false;
	}
	return true;
}

int CExpression::FindOperation(const char *pExpr, char *pPart1, char *pPart2) const
{
	int Len = strlen(pExpr);
	int WaitParenthesis = 0;
	for(int i = Len-1; i >= 0; i--)
	{
		char Char = pExpr[i];
		if(Char == ')')
			WaitParenthesis++;
		else if(Char == '(')
			WaitParenthesis--;
		if(WaitParenthesis > 0)
			continue;
		char PrevChar = 0;
		if(i-1 >= 0)
			PrevChar = pExpr[i-1];
		int OperatorLen;
		int Operator = GetOperator(Char, PrevChar, &OperatorLen);
		if(Operator != OPERATOR_NONE)
		{
			int LenPart1 = i+1-OperatorLen;
			memcpy(pPart1, pExpr, LenPart1);
			pPart1[LenPart1] = 0;
			strcpy(pPart2, pExpr+i+1);
			return Operator;
		}
	}
	return OPERATOR_NONE;
}

bool CExpression::ParseImp(const char *pExpr, const CTree<CConstant> *paConstants)
{
	char aNewExpr[256];
	if(CheckGlobalParenthesis(pExpr))
	{
		int Len = strlen(pExpr);
		memcpy(aNewExpr, pExpr+1, Len-2);
		aNewExpr[Len-2] = 0;
		pExpr = aNewExpr;
	}

	char aPart1[256];
	char aPart2[256];
	int Operation = FindOperation(pExpr, aPart1, aPart2);

	if(Operation != OPERATOR_NONE)
	{
		if(!aPart2[0])
			return false;
		if(!aPart1[0])
		{
			if(Operation != OPERATOR_SUB && Operation != OPERATOR_NOT) // TODO: function
				return false;
		}

		if(aPart1[0])
		{
			m_apSubExpressions[0] = MEM_NEW(CExpression);
			bool Success = m_apSubExpressions[0]->ParseImp(aPart1, paConstants);
			if(!Success)
				return false;
		}
		if(aPart2[0])
		{
			m_apSubExpressions[1] = MEM_NEW(CExpression);
			bool Success = m_apSubExpressions[1]->ParseImp(aPart2, paConstants);
			if(!Success)
				return false;
		}

		// TODO: if both parts are values, make this a value instead of an operation

		m_Type = EXPRESSION_TYPE_OPERATION;
		m_Operator = Operation;
		return true;
	}

	if(pExpr[0] == '#')
	{
		int Index = atoi(pExpr+1);
		if(Index < 0)
			return false;

		m_Type = EXPRESSION_TYPE_PARAMETER;
		m_ParameterIndex = Index;
		return true;
	}

	if(pExpr[0] >= '0' && pExpr[0] <= '9')
	{
		m_Type = EXPRESSION_TYPE_VALUE;
		m_Value = atoi(pExpr);
		return true;
	}

	const CConstant *pConstant = paConstants->Find(pExpr);
	if(pConstant)
	{
		m_Type = EXPRESSION_TYPE_VALUE;
		m_Value = pConstant->m_Value;
		return true;
	}

	m_Type = EXPRESSION_TYPE_ATTRIBUTE;
	m_Attribute = pExpr;
	return true;
}

void CExpression::Parse(const char *pExpr, const CTree<CConstant> *paConstants)
{
	m_String = pExpr;
	bool Success = ParseImp(pExpr, paConstants);
	if(!Success)
		printf("error: invalid expression '%s'\n", pExpr);
}

int CExpression::ComputeValue(const CTree<CAttribute> *paAttributes, const CParameters *paParameters, bool *pError) const
{
	*pError = false;
	if(m_Type == EXPRESSION_TYPE_OPERATION)
	{
		if(!m_apSubExpressions[1])
		{
			*pError = true;
			return 0;
		}

		int e1 = m_apSubExpressions[1]->ComputeValue(paAttributes, paParameters, pError);
		if(*pError)
			return 0;

		if(!m_apSubExpressions[0])
		{
			if(m_Operator == OPERATOR_SUB)
				return -e1;
			else if(m_Operator == OPERATOR_NOT)
				return !e1;
		}
		else
		{
			int e0 = m_apSubExpressions[0]->ComputeValue(paAttributes, paParameters, pError);
			if(*pError)
				return 0;

			if(m_Operator == OPERATOR_AND)
				return e0 && e1;
			else if(m_Operator == OPERATOR_OR)
				return e0 || e1;
			else if(m_Operator == OPERATOR_XOR)
				return e0 ^ e1;
			else if(m_Operator == OPERATOR_ADD)
				return e0 + e1;
			else if(m_Operator == OPERATOR_SUB)
				return e0 - e1;
			else if(m_Operator == OPERATOR_MULT)
				return e0 * e1;
			else if(m_Operator == OPERATOR_DIV)
			{
				if(e1 == 0)
				{
					*pError = true;
					return 0;
				}
				else
					return e0 / e1;
			}
			else if(m_Operator == OPERATOR_MOD)
			{
				if(e1 == 0)
				{
					*pError = true;
					return 0;
				}
				else
					return e0 % e1;
			}
			else if(m_Operator == OPERATOR_EQUAL)
				return e0 == e1;
			else if(m_Operator == OPERATOR_DIFF)
				return e0 != e1;
			else if(m_Operator == OPERATOR_HIGHER)
				return e0 > e1;
			else if(m_Operator == OPERATOR_LOWER)
				return e0 < e1;
			else if(m_Operator == OPERATOR_HIGHER_EQUAL)
				return e0 >= e1;
			else if(m_Operator == OPERATOR_LOWER_EQUAL)
				return e0 <= e1;
		}
		*pError = true;
		return 0;
	}
	else if(m_Type == EXPRESSION_TYPE_VALUE)
	{
		return m_Value;
	}
	else if(m_Type == EXPRESSION_TYPE_ATTRIBUTE)
	{
		const CAttribute *pAttribute = paAttributes->Find(m_Attribute);
		if(pAttribute)
			return pAttribute->m_Value;
		else
		{
			*pError = true;
			return 0;
		}
	}
	else if(m_Type == EXPRESSION_TYPE_PARAMETER)
	{
		if(m_ParameterIndex < 0 || m_ParameterIndex >= paParameters->Size())
		{
			*pError = true;
			return 0;
		}
		return paParameters->GetAt(m_ParameterIndex);
	}
	else
	{
		*pError = true;
		return 0;
	}
}

const char *CExpression::GetString() const
{
	return m_String;
}
