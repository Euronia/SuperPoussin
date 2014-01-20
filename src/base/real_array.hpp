#ifndef BASE_REALARRAY_HPP
#define BASE_REALARRAY_HPP


template <typename T>
class CRealArray
{
private:
	T *m_pList;
	int m_ListSize;
	int m_NumElements;

	void Init()
	{
		m_pList = 0;
		m_ListSize = 0;
		m_NumElements = 0;
	}

	void IncrementSize()
	{
		if(m_NumElements == m_ListSize)
		{
			if(m_ListSize < 2)
				Alloc(m_ListSize+1);
			else
				Alloc(m_ListSize+m_ListSize/2);
		}
	}

	void Alloc(int NewSize)
	{
		T *pNewList = new T[NewSize];

		for(int e = 0; e < m_NumElements; e++)
			pNewList[e] = m_pList[e];

		if(m_pList)
			delete[] m_pList;

		m_pList = pNewList;
		m_ListSize = NewSize;
	}

public:
	CRealArray()
	{
		Init();
	}

	CRealArray(const CRealArray &Array); // this is forbidden
	CRealArray &operator =(const CRealArray &Array); // this is forbidden

	~CRealArray()
	{
		Clear();
	}

	void Clear()
	{
		if(m_pList)
			delete[] m_pList;
		Init();
	}

	int Size() const
	{
		return m_NumElements;
	}

	T &Add(const T &Item)
	{
		IncrementSize();
		SetSize(m_NumElements+1);
		m_pList[m_NumElements-1] = Item;
		return m_pList[m_NumElements-1];
	}

	T &Add()
	{
		IncrementSize();
		SetSize(m_NumElements+1);
		return m_pList[m_NumElements-1];
	}

	T &GetAt(int Index)
	{
		return m_pList[Index];
	}

	const T &GetAt(int Index) const
	{
		return m_pList[Index];
	}

	void RemoveAt(int Index)
	{
		for(int i = Index; i < m_NumElements-1; i++)
			m_pList[i] = m_pList[i+1];
		m_NumElements--;
	}

	void SetSize(int NewSize)
	{
		if(m_ListSize < NewSize)
			Alloc(NewSize);
		m_NumElements = NewSize;
	}

	void Optimize()
	{
		Alloc(m_NumElements);
	}

	void GetCopy(CRealArray *pOut) const
	{
		pOut->SetSize(m_NumElements);
		for(int i = 0; i < m_NumElements; i++)
			pOut->m_pList[i] = m_pList[i];
	}
};


#endif
