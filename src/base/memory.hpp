#ifndef BASE_MEMORY_HPP
#define BASE_MEMORY_HPP

#define MEMORY_ADVANCED
// #define MEMORY_THREAD_SAFE


#ifdef MEMORY_ADVANCED

#ifdef MEM_FREE
#undef MEM_FREE
#endif

#include <new>

template<typename T>
inline void MEM_DESTROY(T *pObject)
{
	pObject->~T();
}

void *MEM_ALLOC_IMP(const char *pFilename, int Line, unsigned Size, const char *pComment);
#define MEM_ALLOC_COMMENT(s, c) MEM_ALLOC_IMP(__FILE__, __LINE__, (s), (c))
#define MEM_ALLOC(s) MEM_ALLOC_COMMENT((s), "")

void MEM_FREE(void *pBlock);

#define MEM_NEW_COMMENT(o, c) new(MEM_ALLOC_COMMENT(sizeof(o), c)) o
#define MEM_NEW(o) MEM_NEW_COMMENT(o, "")
#define MEM_DELETE(b) do { MEM_DESTROY((b)); MEM_FREE((b)); } while(0)

void MEM_DUMP(const char *pFilename);
void MEM_CHECK();

#else

#define MEM_ALLOC_COMMENT(s, c) malloc((s))
#define MEM_ALLOC(s) malloc((s))

#define MEM_FREE_COMMENT(b, c) free((b))
#define MEM_FREE(b) free((b))

#define MEM_NEW_COMMENT(o, c) new o
#define MEM_NEW(o) new o

#define MEM_DELETE_COMMENT(b, c) delete b
#define MEM_DELETE(b) delete b

#define MEM_DUMP(f)
#define MEM_CHECK()

#endif


#endif
