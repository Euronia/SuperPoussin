#ifndef BASE_STRING_HPP
#define BASE_STRING_HPP

#include <stdlib.h>
#include <string.h>

#include "memory.hpp"


class CString
{
private:
	char *m_pString;
	bool m_Allocated;

	void Init()
	{
		m_pString = 0;
		m_Allocated = false;
	}

	void Allocate(const char *pStr, unsigned Len)
	{
		if(!pStr)
		{
			Init();
			return;
		}
		m_pString = (char *)MEM_ALLOC(Len+1);
		memcpy(m_pString, pStr, Len);
		m_pString[Len] = 0;
		m_Allocated = true;
	}

	void Allocate(const char *pStr)
	{
		if(!pStr)
		{
			Init();
			return;
		}
		Allocate(pStr, strlen(pStr));
	}

	void Deallocate()
	{
		if(m_Allocated)
		{
			MEM_FREE(m_pString);
			Init();
		}
	}

public:
	CString()
	{
		Init();
	}

	CString(const CString &Str); // this is forbidden

	CString(const char *pStr)
	{
		Init();
		Allocate(pStr);
	}

	CString(const char *pStr, unsigned Len)
	{
		Init();
		Allocate(pStr, Len);
	}

	~CString()
	{
		Deallocate();
	}

	CString &operator =(const CString &Str)
	{
		if(Str.GetString() != m_pString)
		{
			Deallocate();
			Allocate(Str);
		}
		return *this;
	}

	CString &operator =(const char *pStr)
	{
		if(pStr != m_pString)
		{
			Deallocate();
			Allocate(pStr);
		}
		return *this;
	}

	void SetString(const char *pStr)
	{
		if(pStr != m_pString)
		{
			Deallocate();
			Allocate(pStr);
		}
	}

	void SetString(const char *pStr, unsigned Len)
	{
		if(pStr != m_pString)
		{
			Deallocate();
			Allocate(pStr, Len);
		}
		else
			m_pString[Len] = 0;
	}

	inline const char *GetString() const
	{
		return m_pString;
	}

	inline operator const char *() const
	{
		return m_pString;
	}

	inline operator bool() const
	{
		return m_pString != 0;
	}
};


#endif
