#ifndef BASE_TREE_H
#define BASE_TREE_H

#include <typeinfo>

#include "memory.hpp"

#define maxi(a, b) (((a) > (b)) ? (a) : (b))


template<typename T>
class CTree
{
private:
	struct CNode
	{
		T m_Data;
		CNode *m_Left;
		CNode *m_Right;
		int m_Height;
	};
	typedef struct CNode *CNodePtr;

	CNodePtr m_Root;

	template<typename T2>
	T &Insert(const T2 &x, CNodePtr &p)
	{
		if(p == 0)
		{
			p = MEM_NEW_COMMENT(CNode, typeid(T).name());
			p->m_Left = 0;
			p->m_Right = 0;
			p->m_Height = 0;
			return p->m_Data;
		}
		else
		{
			T *pData;
			int Comp = p->m_Data.Compare(x);
			if(Comp < 0)
			{
				pData = &Insert(x, p->m_Left);
				if(NodeHeight(p->m_Left) - NodeHeight(p->m_Right) == 2)
				{
					Comp = p->m_Left->m_Data.Compare(x);
					if(Comp < 0)
						p = srl(p);
					else
						p = drl(p);
				}
			}
			else
			{
				pData = &Insert(x, p->m_Right);
				if(NodeHeight(p->m_Right) - NodeHeight(p->m_Left) == 2)
				{
					Comp = p->m_Right->m_Data.Compare(x);
					if(Comp > 0)
						p = srr(p);
					else
						p = drr(p);
				}
			}
			p->m_Height = maxi(NodeHeight(p->m_Left), NodeHeight(p->m_Right)) + 1;
			return *pData;
		}
	}

	int NodeHeight(CNodePtr p)
	{
		if(p == 0)
			return 0;
		else
			return p->m_Height;
	}

	template<typename T2>
	T *Find(const T2 &c, const CNodePtr &p) const
	{
		if(p == 0)
			return 0;
		else
		{
			int Comp = p->m_Data.Compare(c);
			if(Comp < 0)
				return Find(c, p->m_Left);
			else if(Comp > 0)
				return Find(c, p->m_Right);
			else
				return &p->m_Data;
		}
	}

	void Clear(CNodePtr &p)
	{
		if(p == 0)
			return;
		Clear(p->m_Left);
		Clear(p->m_Right);
		MEM_DELETE(p);
		p = 0;
	}

	CNodePtr srl(CNodePtr &p1)
	{
		CNodePtr p2;
		p2 = p1->m_Left;
		p1->m_Left = p2->m_Right;
		p2->m_Right = p1;
		p1->m_Height = maxi(NodeHeight(p1->m_Left), NodeHeight(p1->m_Right)) + 1;
		p2->m_Height = maxi(NodeHeight(p2->m_Left), NodeHeight(p1)) + 1;
		return p2;
	}
	CNodePtr srr(CNodePtr &p1)
	{
		CNodePtr p2;
		p2 = p1->m_Right;
		p1->m_Right = p2->m_Left;
		p2->m_Left = p1;
		p1->m_Height = maxi(NodeHeight(p1->m_Left), NodeHeight(p1->m_Right)) + 1;
		p2->m_Height = maxi(NodeHeight(p1), NodeHeight(p2->m_Right)) + 1;
		return p2;
	}
	CNodePtr drl(CNodePtr &p1)
	{
		p1->m_Left = srr(p1->m_Left);
		return srl(p1);
	}
	CNodePtr drr(CNodePtr &p1)
	{
		p1->m_Right = srl(p1->m_Right);
		return srr(p1);
	}

	int NumNodes(CNodePtr p)
	{
		if(p == 0)
			return 0;
		else
			return NumNodes(p->m_Left) + NumNodes(p->m_Right) + 1;
	}

public:
	CTree()
	{
		m_Root = 0;
	}

	~CTree()
	{
		Clear();
	}

	template<typename T2>
	T &Insert(const T2 &x)
	{
		return Insert(x, m_Root);
	}

	template<typename T2>
	const T *Find(const T2 &c) const
	{
		return Find(c, m_Root);
	}

	template<typename T2>
	T *Find(const T2 &c)
	{
		return Find(c, m_Root);
	}

	void Clear()
	{
		Clear(m_Root);
	}
};


#endif
