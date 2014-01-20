#include "memory.hpp"

#ifdef MEMORY_ADVANCED

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifdef MEMORY_THREAD_SAFE
	#include <QMutex>
#endif


struct CMemStats
{
	int m_ActiveAllocated;
	int m_TotalAllocated;
	int m_MaxAllocated;
	int m_ActiveAllocations;
	int m_TotalAllocations;
	int m_MaxAllocations;
};

struct CMemHeader
{
	const char *m_pFilename;
	int m_Line;
	int m_Size;
	const char *m_pComment;
	CMemHeader *m_pPrev;
	CMemHeader *m_pNext;
};

struct CMemTail
{
	int m_Guard;
};

static const int MEM_GUARD_VAL = 0xbaadc0de;

static CMemStats g_MemoryStats = {0};
static CMemHeader *g_pFirstHeader = 0;

#ifdef MEMORY_THREAD_SAFE
	static QMutex g_MemoryMutex;
#endif


void *MEM_ALLOC_IMP(const char *pFilename, int Line, unsigned Size, const char *pComment)
{
#ifdef MEMORY_THREAD_SAFE
	g_MemoryMutex.lock();
#endif

	CMemHeader *pHeader = (CMemHeader *)malloc(Size+sizeof(CMemHeader)+sizeof(CMemTail));
	if(!pHeader)
		return 0;

	pHeader->m_pFilename = pFilename;
	pHeader->m_Line = Line;
	pHeader->m_Size = Size;
	pHeader->m_pComment = pComment;
	pHeader->m_pPrev = 0;
	pHeader->m_pNext = g_pFirstHeader;

	CMemTail *pTail = (CMemTail *)(((char *)(pHeader+1))+pHeader->m_Size);
	pTail->m_Guard = MEM_GUARD_VAL;

	if(g_pFirstHeader)
		g_pFirstHeader->m_pPrev = pHeader;
	g_pFirstHeader = pHeader;

	g_MemoryStats.m_ActiveAllocated += Size;
	g_MemoryStats.m_TotalAllocated += Size;
	if(g_MemoryStats.m_ActiveAllocated > g_MemoryStats.m_MaxAllocated)
		g_MemoryStats.m_MaxAllocated = g_MemoryStats.m_ActiveAllocated;

	g_MemoryStats.m_ActiveAllocations++;
	g_MemoryStats.m_TotalAllocations++;
	if(g_MemoryStats.m_ActiveAllocations > g_MemoryStats.m_MaxAllocations)
		g_MemoryStats.m_MaxAllocations = g_MemoryStats.m_ActiveAllocations;

#ifdef MEMORY_THREAD_SAFE
	g_MemoryMutex.unlock();
#endif

	return pHeader+1;
}

void MEM_FREE(void *pBlock)
{
	if(!pBlock)
		return;

#ifdef MEMORY_THREAD_SAFE
	g_MemoryMutex.lock();
#endif

	CMemHeader *pHeader = ((CMemHeader *)pBlock) - 1;
	CMemTail *pTail = (CMemTail *)(((char *)(pHeader+1))+pHeader->m_Size);

	if(pTail->m_Guard != MEM_GUARD_VAL)
		printf("memory check failed while freeing %p\n", pBlock);

	if(pHeader->m_pPrev)
		pHeader->m_pPrev->m_pNext = pHeader->m_pNext;
	else
		g_pFirstHeader = pHeader->m_pNext;
	if(pHeader->m_pNext)
		pHeader->m_pNext->m_pPrev = pHeader->m_pPrev;

	g_MemoryStats.m_ActiveAllocated -= pHeader->m_Size;
	g_MemoryStats.m_ActiveAllocations--;

	free(pHeader);

#ifdef MEMORY_THREAD_SAFE
	g_MemoryMutex.unlock();
#endif
}

void MEM_DUMP(const char *pFilename)
{
#ifdef MEMORY_THREAD_SAFE
	g_MemoryMutex.lock();
#endif

	FILE *pFile = fopen(pFilename, "wb");
	if(!pFile)
		return;

	char aBuf[1024];

	sprintf(aBuf, "Active allocated memory: %d bytes\r\n", g_MemoryStats.m_ActiveAllocated);
	fwrite(aBuf, strlen(aBuf), 1, pFile);

	sprintf(aBuf, "Total allocated memory: %d bytes\r\n", g_MemoryStats.m_TotalAllocated);
	fwrite(aBuf, strlen(aBuf), 1, pFile);

	sprintf(aBuf, "Max allocated memory: %d bytes\r\n", g_MemoryStats.m_MaxAllocated);
	fwrite(aBuf, strlen(aBuf), 1, pFile);

	sprintf(aBuf, "Active allocations: %d\r\n", g_MemoryStats.m_ActiveAllocations);
	fwrite(aBuf, strlen(aBuf), 1, pFile);

	sprintf(aBuf, "Total allocations: %d\r\n", g_MemoryStats.m_TotalAllocations);
	fwrite(aBuf, strlen(aBuf), 1, pFile);

	sprintf(aBuf, "Max allocations: %d\r\n", g_MemoryStats.m_MaxAllocations);
	fwrite(aBuf, strlen(aBuf), 1, pFile);

	CMemHeader *pHeader = g_pFirstHeader;
	while(pHeader)
	{
		sprintf(aBuf, "%s(%d): %d bytes - '%s'\r\n", pHeader->m_pFilename, pHeader->m_Line, pHeader->m_Size, pHeader->m_pComment);
		fwrite(aBuf, strlen(aBuf), 1, pFile);
		pHeader = pHeader->m_pNext;
	}

	fclose(pFile);

#ifdef MEMORY_THREAD_SAFE
	g_MemoryMutex.unlock();
#endif
}

void MEM_CHECK()
{
#ifdef MEMORY_THREAD_SAFE
	g_MemoryMutex.lock();
#endif

	CMemHeader *pHeader = g_pFirstHeader;
	while(pHeader)
	{
		CMemTail *pTail = (CMemTail *)(((char *)(pHeader+1))+pHeader->m_Size);
		if(pTail->m_Guard != MEM_GUARD_VAL)
		{
			printf("memory check failed at %s(%d): %d - '%s'\n", pHeader->m_pFilename, pHeader->m_Line, pHeader->m_Size, pHeader->m_pComment);
			return;
		}
		pHeader = pHeader->m_pNext;
	}

	printf("memory check succeeded\n");

#ifdef MEMORY_THREAD_SAFE
	g_MemoryMutex.unlock();
#endif
}

#endif
