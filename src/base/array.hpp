#ifndef BASE_ARRAY_HPP
#define BASE_ARRAY_HPP

#include <typeinfo>

#include "memory.hpp"


template <typename T>
class CArray
{
private:
	struct CArrayElement
	{
		T m_Data;
		CArrayElement *m_pNextElement;
	};

	CArrayElement *m_pFirstElement;
	CArrayElement *m_pLastElement;
	int m_Size;

	void RemoveElement(CArrayElement *pElement, CArrayElement *pPrevElement)
	{
		if(pPrevElement)
			pPrevElement->m_pNextElement = pElement->m_pNextElement;
		else
			m_pFirstElement = pElement->m_pNextElement;
		if(!pElement->m_pNextElement)
			m_pLastElement = pPrevElement;
		MEM_DELETE(pElement);
	}

public:
	CArray()
	{
		m_pFirstElement = 0;
		m_pLastElement = 0;
		m_Size = 0;
	}

	CArray(const CArray &Array); // this is forbidden
	CArray &operator =(const CArray &Array); // this is forbidden

	~CArray()
	{
		Clear();
	}

	T &Add()
	{
		CArrayElement *pElement = MEM_NEW_COMMENT(CArrayElement, typeid(T).name());
		pElement->m_pNextElement = 0;

		if(m_pFirstElement)
			m_pLastElement->m_pNextElement = pElement;
		else
			m_pFirstElement = pElement;

		m_pLastElement = pElement;
		m_Size++;

		return pElement->m_Data;
	}

	T &Add(const T &Data)
	{
		CArrayElement *pElement = MEM_NEW_COMMENT(CArrayElement, typeid(T).name());
		pElement->m_Data = Data;
		pElement->m_pNextElement = 0;

		if(m_pFirstElement)
			m_pLastElement->m_pNextElement = pElement;
		else
			m_pFirstElement = pElement;

		m_pLastElement = pElement;
		m_Size++;

		return pElement->m_Data;
	}

	int Size() const
	{
		return m_Size;
	}

	void Clear()
	{
		CArrayElement *pNextElement;
		for(CArrayElement *pElement = m_pFirstElement; pElement; pElement = pNextElement)
		{
			pNextElement = pElement->m_pNextElement;
			MEM_DELETE(pElement);
		}

		m_pFirstElement = 0;
		m_pLastElement = 0;
		m_Size = 0;
	}

	void RemoveValue(const T &Value)
	{
		CArrayElement *pPrevElement = 0;
		for(CArrayElement *pElement = m_pFirstElement; pElement; pElement = pElement->m_pNextElement)
		{
			if(pElement->m_Data == Value)
			{
				RemoveElement(pElement, pPrevElement);
				break;
			}
			pPrevElement = pElement;
		}
	}

	void Remove(const T *pRef)
	{
		CArrayElement *pPrevElement = 0;
		for(CArrayElement *pElement = m_pFirstElement; pElement; pElement = pElement->m_pNextElement)
		{
			if(&pElement->m_Data == pRef)
			{
				if(pPrevElement)
					pPrevElement->m_pNextElement = pElement->m_pNextElement;
				else
					m_pFirstElement = pElement->m_pNextElement;
				if(!pElement->m_pNextElement)
					m_pLastElement = pPrevElement;
				MEM_DELETE(pElement);
				break;
			}
			pPrevElement = pElement;
		}
	}

	class CIterator
	{
	private:
		CArrayElement *m_pElement;

	public:
		CIterator(CArrayElement *pFirstElement)
		{
			m_pElement = pFirstElement;
		}

		inline bool Exists() const
		{
			return m_pElement != 0;
		}

		inline void Next()
		{
			m_pElement = m_pElement->m_pNextElement;
		}

		inline T &Get()
		{
			return m_pElement->m_Data;
		}
	};

	class CRemoveIterator
	{
	private:
		CArray *m_pArray;
		CArrayElement *m_pElement;
		CArrayElement *m_pPrevElement;

	public:
		CRemoveIterator(CArray *pArray)
		{
			m_pArray = pArray;
			m_pElement = pArray->m_pFirstElement;
			m_pPrevElement = 0;
		}

		inline bool Exists() const
		{
			return m_pElement != 0;
		}

		inline void Next()
		{
			m_pPrevElement = m_pElement;
			m_pElement = m_pElement->m_pNextElement;
		}

		inline void RemoveAndNext()
		{
			CArrayElement *pNextElement = m_pElement->m_pNextElement;
			m_pArray->RemoveElement(m_pElement, m_pPrevElement);
			m_pElement = pNextElement;
		}

		inline T &Get()
		{
			return m_pElement->m_Data;
		}
	};

	CIterator GetIterator() const
	{
		return CIterator(m_pFirstElement);
	}

	CRemoveIterator GetRemoveIterator()
	{
		return CRemoveIterator(this);
	}
};


#endif
