/*
farrtl.cpp

��������������� ��������� CRT �������
*/

#include "headers.hpp"
#pragma hdrstop

#include "console.hpp"
#include "colormix.hpp"
#include "imports.hpp"
#include "synchro.hpp"

#ifdef _MSC_VER
#pragma intrinsic (memcpy)
#endif

#ifdef MEMCHECK
#undef xf_malloc
#undef xf_realloc
#undef xf_realloc_nomove
#undef DuplicateString
#undef new
#endif

static bool InsufficientMemoryHandler()
{
	if (!Global)
		return false;
	static FarColor ErrColor;
	Colors::ConsoleColorToFarColor(FOREGROUND_RED|FOREGROUND_INTENSITY, ErrColor);
	Global->Console->SetTextAttributes(ErrColor);
	COORD OldPos,Pos={};
	Global->Console->GetCursorPosition(OldPos);
	Global->Console->SetCursorPosition(Pos);
	static WCHAR ErrorMessage[] = L"Not enough memory is available to complete this operation.\nPress Enter to retry or Esc to continue...";
	Global->Console->Write(ErrorMessage, ARRAYSIZE(ErrorMessage)-1);
	Global->Console->Commit();
	Global->Console->SetCursorPosition(OldPos);
	INPUT_RECORD ir={};
	size_t Read;
	do
	{
		Global->Console->ReadInput(&ir, 1, Read);
	}
	while(!(ir.EventType == KEY_EVENT && !ir.Event.KeyEvent.bKeyDown && (ir.Event.KeyEvent.wVirtualKeyCode == VK_RETURN || ir.Event.KeyEvent.wVirtualKeyCode == VK_ESCAPE)));
	return ir.Event.KeyEvent.wVirtualKeyCode == VK_RETURN;
}

static void* ReleaseAllocator(size_t size)
{
	void* newBlock;
	do newBlock = malloc(size);
	while (!newBlock && InsufficientMemoryHandler());
	return newBlock;
}

static void ReleaseDeallocator(void* block)
{
	return free(block);
}

static void* ReleaseReallocator(void* block, size_t size)
{
	void* newBlock;
	do newBlock = realloc(block, size);
	while (!newBlock && InsufficientMemoryHandler());
	return newBlock;
}

static void* ReleaseExpander(void* block, size_t size)
{
#ifdef _MSC_VER
	return _expand(block, size);
#else
	return nullptr;
#endif
}

#ifndef MEMCHECK

void* xf_malloc(size_t size)
{
	return ReleaseAllocator(size);
}

void xf_free(void* block)
{
	return ReleaseDeallocator(block);
}

void* xf_realloc(void* block, size_t size)
{
	return ReleaseReallocator(block, size);
}

void* xf_realloc_nomove(void * block, size_t size)
{
	if (!block)
	{
		return xf_malloc(size);
	}
	else if (ReleaseExpander(block, size))
	{
		return block;
	}
	else
	{
		xf_free(block);
		return xf_malloc(size);
	}
}

void* operator new(size_t size) throw()
{
	return ReleaseAllocator(size);
}

void* operator new[](size_t size) throw()
{
	return ReleaseAllocator(size);
}

void operator delete(void* block)
{
	return ReleaseDeallocator(block);
}

void operator delete[](void* block)
{
	return ReleaseDeallocator(block);
}

char* DuplicateString(const char * string)
{
	return string? strcpy(new char[strlen(string) + 1], string) : nullptr;
}

wchar_t* DuplicateString(const wchar_t * string)
{
	return string? wcscpy(new wchar_t[wcslen(string) + 1], string) : nullptr;
}

#else
namespace memcheck
{
static intptr_t CallNewDeleteVector = 0;
static intptr_t CallNewDeleteScalar = 0;
static intptr_t CallMallocFree = 0;
static size_t AllocatedMemoryBlocks = 0;
static size_t AllocatedMemorySize = 0;
static size_t TotalAllocationCalls = 0;

enum ALLOCATION_TYPE
{
	AT_RAW    = 0xa7000ea8,
	AT_SCALAR = 0xa75ca1ae,
	AT_VECTOR = 0xa77ec10e,
};

static CRITICAL_SECTION CS;

struct MEMINFO
{
	union
	{
		struct
		{
			ALLOCATION_TYPE AllocationType;
			int Line;
			const char* File;
			const char* Function;
			size_t Size;
			MEMINFO* prev;
			MEMINFO* next;
		};
		char c[MEMORY_ALLOCATION_ALIGNMENT*4];
	};
};

static MEMINFO FirstMemBlock = {};
static MEMINFO* LastMemBlock = &FirstMemBlock;

static_assert(sizeof(MEMINFO) == MEMORY_ALLOCATION_ALIGNMENT*4, "MEMINFO not aligned");
inline MEMINFO* ToReal(void* address) { return static_cast<MEMINFO*>(address) - 1; }
inline void* ToUser(MEMINFO* address) { return address + 1; }

static void CheckChain()
{
#if 0
	auto p = &FirstMemBlock;

	while(p->next)
		p = p->next;
	assert(p==LastMemBlock);

	while(p->prev)
		p = p->prev;
	assert(p==&FirstMemBlock);
#endif
}

static inline void updateCallCount(ALLOCATION_TYPE type, bool increment)
{
	int op = increment? 1 : -1;
	switch(type)
	{
	case AT_RAW:    CallMallocFree += op;      break;
	case AT_SCALAR: CallNewDeleteScalar += op; break;
	case AT_VECTOR: CallNewDeleteVector += op; break;
	}
}

static void RegisterBlock(MEMINFO *block)
{
	if (!AllocatedMemoryBlocks)
		InitializeCriticalSection(&CS);
	EnterCriticalSection(&CS);

	block->prev = LastMemBlock;
	block->next = nullptr;

	LastMemBlock->next = block;
	LastMemBlock = block;

	CheckChain();

	updateCallCount(block->AllocationType, true);
	++AllocatedMemoryBlocks;
	++TotalAllocationCalls;
	AllocatedMemorySize+=block->Size;

	LeaveCriticalSection(&CS);
}

static void UnregisterBlock(MEMINFO *block)
{
	EnterCriticalSection(&CS);

	if (block->prev)
		block->prev->next = block->next;
	if (block->next)
		block->next->prev = block->prev;
	if(block == LastMemBlock)
		LastMemBlock = LastMemBlock->prev;

	CheckChain();

	updateCallCount(block->AllocationType, false);
	--AllocatedMemoryBlocks;
	AllocatedMemorySize-=block->Size;

	LeaveCriticalSection(&CS);

	if (!AllocatedMemoryBlocks)
		DeleteCriticalSection(&CS);
}

static void* DebugAllocator(size_t size, ALLOCATION_TYPE type,const char* Function,  const char* File, int Line)
{
	size_t realSize = size + sizeof(MEMINFO);
	MEMINFO* Info = static_cast<MEMINFO*>(ReleaseAllocator(realSize));
	Info->AllocationType = type;
	Info->Size = realSize;
	Info->Function = Function;
	Info->File = File;
	Info->Line = Line;
	RegisterBlock(Info);
	return ToUser(Info);
}

static void DebugDeallocator(void* block, ALLOCATION_TYPE type)
{
	void* realBlock = block? ToReal(block) : nullptr;
	if (realBlock)
	{
		MEMINFO* Info = static_cast<MEMINFO*>(realBlock);
		assert(Info->AllocationType == type);
		UnregisterBlock(Info);
	}
	ReleaseDeallocator(realBlock);
}

static void* DebugReallocator(void* block, size_t size, const char* Function, const char* File, int Line)
{
	if(!block)
		return DebugAllocator(size, AT_RAW, Function, File, Line);

	MEMINFO* Info = ToReal(block);
	assert(Info->AllocationType == AT_RAW);
	UnregisterBlock(Info);
	size_t realSize = size + sizeof(MEMINFO);

	Info = static_cast<MEMINFO*>(ReleaseReallocator(Info, realSize));

	Info->AllocationType = AT_RAW;
	Info->Size = realSize;
	RegisterBlock(Info);
	return ToUser(Info);
}

static void* DebugExpander(void* block, size_t size)
{
	MEMINFO* Info = ToReal(block);
	assert(Info->AllocationType == AT_RAW);
	size_t realSize = size + sizeof(MEMINFO);

	// _expand() calls HeapReAlloc which can change the status code, it's bad for us
	NTSTATUS status = Global->ifn->RtlGetLastNtStatus();
	Info = static_cast<MEMINFO*>(ReleaseExpander(Info, realSize));
	//RtlNtStatusToDosError also remembers the status code value in the TEB:
	Global->ifn->RtlNtStatusToDosError(status);

	if(Info)
	{
		AllocatedMemorySize-=Info->Size;
		Info->Size = realSize;
		AllocatedMemorySize+=Info->Size;
	}

	return Info? ToUser(Info) : nullptr;
}

static inline const char* getAllocationTypeString(ALLOCATION_TYPE type)
{
	switch(type)
	{
	case AT_RAW: return "malloc";
	case AT_SCALAR: return "operator new";
	case AT_VECTOR: return "operator new[]";
	}
	return "unknown";
}

void PrintMemory()
{
	if (CallNewDeleteVector || CallNewDeleteScalar || CallMallocFree || AllocatedMemoryBlocks || AllocatedMemorySize)
	{
		std::wcout << L"Memory leaks detected:" << std::endl;
		if (CallNewDeleteVector)
			std::wcout << L"  delete[]:   " << CallNewDeleteVector << std::endl;
		if (CallNewDeleteScalar)
			std::wcout << L"  delete:     " << CallNewDeleteScalar << std::endl;
		if (CallMallocFree)
			std::wcout << L"  free():     " << CallMallocFree << std::endl;
		if (AllocatedMemoryBlocks)
			std::wcout << L"Total blocks: " << AllocatedMemoryBlocks << std::endl;
		if (AllocatedMemorySize)
			std::wcout << L"Total bytes:  " << AllocatedMemorySize - AllocatedMemoryBlocks * sizeof(MEMINFO) <<  L" payload, " << AllocatedMemoryBlocks * sizeof(MEMINFO) << L" overhead" << std::endl;
		std::wcout << std::endl;

		std::wcout << "Not freed blocks:" << std::endl;
		for(auto i = FirstMemBlock.next; i; i = i->next)
		{
			std::wcout << i->File << L':' << i->Line << L" -> " << i->Function << L':' << getAllocationTypeString(i->AllocationType) << L" (" << i->Size - sizeof(MEMINFO) << L" bytes)" << std::endl;
		}
	}
}

};

void* xf_malloc(size_t size, const char* Function, const char* File, int Line)
{
	return memcheck::DebugAllocator(size, memcheck::AT_RAW, Function, File, Line);
}

void xf_free(void* block)
{
	return memcheck::DebugDeallocator(block, memcheck::AT_RAW);
}

void* xf_realloc(void* block, size_t size, const char* Function, const char* File, int Line)
{
	return memcheck::DebugReallocator(block, size, Function, File, Line);
}

void* xf_realloc_nomove(void * block, size_t size, const char* Function, const char* File, int Line)
{
	if (!block)
	{
		return xf_malloc(size, Function, File, Line);
	}
	else if (memcheck::DebugExpander(block, size))
	{
		return block;
	}
	else
	{
		xf_free(block);
		return xf_malloc(size, File, Function, Line);
	}
}

void* operator new(size_t size)
{
	return memcheck::DebugAllocator(size, memcheck::AT_SCALAR, __FUNCTION__, __FILE__, __LINE__);
}

void* operator new[](size_t size)
{
	return memcheck::DebugAllocator(size, memcheck::AT_VECTOR, __FUNCTION__, __FILE__, __LINE__);
}

void* operator new(size_t size, const char* Function, const char* File, int Line)
{
	return memcheck::DebugAllocator(size, memcheck::AT_SCALAR, Function, File, Line);
}

void* operator new[](size_t size, const char* Function, const char* File, int Line)
{
	return memcheck::DebugAllocator(size, memcheck::AT_VECTOR, Function, File, Line);
}

void operator delete(void* block)
{
	return memcheck::DebugDeallocator(block, memcheck::AT_SCALAR);
}

void operator delete[](void* block)
{
	return memcheck::DebugDeallocator(block, memcheck::AT_VECTOR);
}

void operator delete(void* block, const char* Function, const char* File, int Line)
{
	return memcheck::DebugDeallocator(block, memcheck::AT_SCALAR);
}

void operator delete[](void* block, const char* Function, const char* File, int Line)
{
	return memcheck::DebugDeallocator(block, memcheck::AT_VECTOR);
}

char* DuplicateString(const char * string, const char* Function, const char* File, int Line)
{
	return string ? strcpy(new(Function, File, Line) char[strlen(string) + 1], string) : nullptr;
}

wchar_t* DuplicateString(const wchar_t * string, const char* Function, const char* File, int Line)
{
	return string ? wcscpy(new(Function, File, Line) wchar_t[wcslen(string) + 1], string) : nullptr;
}

#endif

void PrintMemory()
{
#ifdef MEMCHECK
	memcheck::PrintMemory();
#endif
}


// dest � src �� ������ ������������
char * xstrncpy(char * dest,const char * src,size_t DestSize)
{
	char *tmpsrc = dest;

	while (DestSize>1 && (*dest++ = *src++))
	{
		DestSize--;
	}

	*dest = 0;
	return tmpsrc;
}

wchar_t * xwcsncpy(wchar_t * dest,const wchar_t * src,size_t DestSize)
{
	wchar_t *tmpsrc = dest;

	while (DestSize>1 && (*dest++ = *src++))
		DestSize--;

	*dest = 0;
	return tmpsrc;
}

namespace cfunctions
{

void* bsearchex(const void* key,const void* base,size_t nelem,size_t width,int (WINAPI *fcmp)(const void*, const void*,void*),void* userparam)
{
	if(width)
	{
		size_t low=0,high=nelem;
		while(low<high)
		{
			size_t curr=(low+high)/2;
			void* ptr=(void*)(((char*)base)+curr*width);
			int cmp=fcmp(key,ptr,userparam);
			if(0==cmp)
			{
				return ptr;
			}
			else if(cmp<0)
			{
				high=curr;
			}
			else
			{
				low=curr+1;
			}
		}
	}
	return nullptr;
}

/* start qsortex */

/*
Copyright Prototronics, 1987
Totem Lake P.O. 8117
Kirkland, Washington 98034

(206) 820-1972

Licensed to Zortech. */
/*
Modified by Joe Huffman (d.b.a Prototronics) June 11, 1987 from Ray Gardner's,
(Denver, Colorado) public domain version. */

/*    qsortex()  --  Quicksort function
**
**    Usage:   qsortex(base, nbr_elements, width_bytes, compare_function);
**                char *base;
**                unsigned int nbr_elements, width_bytes;
**                int (*compare_function)();
**
**    Sorts an array starting at base, of length nbr_elements, each
**    element of size width_bytes, ordered via compare_function; which
**    is called as  (*compare_function)(ptr_to_element1, ptr_to_element2)
**    and returns < 0 if element1 < element2, 0 if element1 = element2,
**    > 0 if element1 > element2.  Most of the refinements are due to
**    R. Sedgewick.  See "Implementing Quicksort Programs", Comm. ACM,
**    Oct. 1978, and Corrigendum, Comm. ACM, June 1979.
*/

static void iswap(int *a, int *b, size_t n_to_swap);       /* swap ints */
static void cswap(char *a, char *b, size_t n_to_swap);     /* swap chars */

//static unsigned int n_to_swap;  /* nbr of chars or ints to swap */
int _maxspan = 7;               /* subfiles of _maxspan or fewer elements */
/* will be sorted by a simple insertion sort */

/* Adjust _maxspan according to relative cost of a swap and a compare.  Reduce
_maxspan (not less than 1) if a swap is very expensive such as when you have
an array of large structures to be sorted, rather than an array of pointers to
structures.  The default value is optimized for a high cost for compares. */

#define SWAP(a,b) (*swap_fp)(a,b,n_to_swap)
#define COMPEX(a,b,u) (*comp_fp)(a,b,u)
#define COMP(a,b) (*comp_fp)(a,b)

typedef void (__cdecl *SWAP_FP)(void *, void *, size_t);

void __cdecl qsortex(char *base, size_t nel, size_t width,
                     int (WINAPI *comp_fp)(const void *, const void *,void*), void *user)
{
	char *stack[40], **sp;                 /* stack and stack pointer        */
	char *i, *j, *limit;                   /* scan and limit pointers        */
	size_t thresh;                         /* size of _maxspan elements in   */
	void (__cdecl  *swap_fp)(void *, void *, size_t);               /* bytes */
	size_t n_to_swap;

	if ((width % sizeof(int)) )
	{
		swap_fp = (SWAP_FP)cswap;
		n_to_swap = width;
	}
	else
	{
		swap_fp = (SWAP_FP)iswap;
		n_to_swap = width / sizeof(int);
	}

	thresh = _maxspan * width;             /* init threshold                 */
	sp = stack;                            /* init stack pointer             */
	limit = base + nel * width;            /* pointer past end of array      */

	for (;;)                               /* repeat until done then return  */
	{
		while ((size_t)(limit - base) > thresh) /* if more than _maxspan elements */
		{
			/*swap middle, base*/
			SWAP(((size_t)(limit - base) >> 1) -
			     ((((size_t)(limit - base) >> 1)) % width) + base, base);
			i = base + width;                /* i scans from left to right     */
			j = limit - width;               /* j scans from right to left     */

			if (COMPEX(i, j,user) > 0)              /* Sedgewick's                    */
				SWAP(i, j);                    /*    three-element sort          */

			if (COMPEX(base, j,user) > 0)           /*        sets things up          */
				SWAP(base, j);                 /*            so that             */

			if (COMPEX(i, base,user) > 0)           /*              *i <= *base <= *j */
				SWAP(i, base);                 /* *base is the pivot element     */

			for (;;)
			{
				do                            /* move i right until *i >= pivot */
					i += width;

				while (COMPEX(i, base,user) < 0);

				do                            /* move j left until *j <= pivot  */
					j -= width;

				while (COMPEX(j, base,user) > 0);

				if (i > j)                    /* break loop if pointers crossed */
					break;

				SWAP(i, j);                   /* else swap elements, keep scanning */
			}

			SWAP(base, j);                   /* move pivot into correct place  */

			if (j - base > limit - i)        /* if left subfile is larger...   */
			{
				sp[0] = base;                 /* stack left subfile base        */
				sp[1] = j;                    /*    and limit                   */
				base = i;                     /* sort the right subfile         */
			}
			else                             /* else right subfile is larger   */
			{
				sp[0] = i;                    /* stack right subfile base       */
				sp[1] = limit;                /*    and limit                   */
				limit = j;                    /* sort the left subfile          */
			}

			sp += 2;                        /* increment stack pointer        */
		}

		/* Insertion sort on remaining subfile. */
		i = base + width;

		while (i < limit)
		{
			j = i;

			while (j > base && COMPEX(j - width, j,user) > 0)
			{
				SWAP(j - width, j);
				j -= width;
			}

			i += width;
		}

		if (sp > stack)    /* if any entries on stack...     */
		{
			sp -= 2;         /* pop the base and limit         */
			base = sp[0];
			limit = sp[1];
		}
		else              /* else stack empty, all done     */
			break;          /* Return. */
	}
}

static void iswap(int *a, int *b, size_t n_to_swap)   /* swap ints */
{
	do
	{
		int tmp = *a;
		*a = *b;
		*b = tmp;
		a++; b++;
	}
	while (--n_to_swap);
}

static void cswap(char *a, char *b, size_t n_to_swap)  /* swap chars */
{
	do
	{
		char tmp = *a;
		*a = *b;
		*b = tmp;
		a++; b++;
	}
	while (--n_to_swap);
}

/* end qsortex */

/* start qsort */
void __cdecl qsort_b(
    void *base,
    size_t num,
    size_t width,
    int (__cdecl *comp)(const void *, const void *));

void __cdecl qsort_m(
    void *base,
    size_t num,
    size_t width,
    int (__cdecl *comp)(const void *, const void *));

void __cdecl far_qsort(
    void *base,
    size_t num,
    size_t width,
    int (__cdecl *comp)(const void *, const void *)
)
{
	if (width >=32) qsort_m(base, num, width, comp);
	else qsort_b(base, num, width, comp);
}

/* prototypes for local routines */
static void  shortsort(char *lo, char *hi, size_t width,
                       int (__cdecl *comp)(const void *, const void *));
static void  swap(char *p, char *q, size_t width);

/* this parameter defines the cutoff between using quick sort and
   insertion sort for arrays; arrays with lengths shorter or equal to the
   below value use insertion sort */

#define CUTOFF 8            /* testing shows that this is good value */

/***
*qsort(base, num, wid, comp) - quicksort function for sorting arrays
*
*Purpose:
*       quicksort the array of elements
*       side effects:  sorts in place
*       maximum array size is number of elements times size of elements,
*       but is limited by the virtual address space of the processor
*
*Entry:
*       char *base = pointer to base of array
*       size_t num  = number of elements in the array
*       size_t width = width in bytes of each array element
*       int (*comp)() = pointer to function returning analog of strcmp for
*               strings, but supplied by user for comparing the array elements.
*               it accepts 2 pointers to elements and returns neg if 1<2, 0 if
*               1=2, pos if 1>2.
*
*Exit:
*       returns void
*
*Exceptions:
*
*******************************************************************************/

/* sort the array between lo and hi (inclusive) */

#define STKSIZ (8*sizeof(void*) - 2)

void __cdecl qsort_b(
    void *base,
    size_t num,
    size_t width,
    int (__cdecl *comp)(const void *, const void *)
)
{
	/* Note: the number of stack entries required is no more than
	   1 + log2(num), so 30 is sufficient for any array */
	char *lo, *hi;              /* ends of sub-array currently sorting */
	char *mid;                  /* points to middle of subarray */
	char *loguy, *higuy;        /* traveling pointers for partition step */
	size_t size;                /* size of the sub-array */
	char *lostk[STKSIZ], *histk[STKSIZ];
	int stkptr;                 /* stack for saving sub-array to be processed */

	if (num < 2 || !width)
		return;                 /* nothing to do */

	stkptr = 0;                 /* initialize stack */
	lo = (char *)base;
	hi = (char *)base + width * (num-1);        /* initialize limits */
	/* this entry point is for pseudo-recursion calling: setting
	   lo and hi and jumping to here is like recursion, but stkptr is
	   preserved, locals aren't, so we preserve stuff on the stack */
recurse:
	size = (hi - lo) / width + 1;        /* number of el's to sort */

	/* below a certain size, it is faster to use a O(n^2) sorting method */
	if (size <= CUTOFF)
	{
		shortsort(lo, hi, width, comp);
	}
	else
	{
		/* First we pick a partitioning element.  The efficiency of the
		   algorithm demands that we find one that is approximately the median
		   of the values, but also that we select one fast.  We choose the
		   median of the first, middle, and last elements, to avoid bad
		   performance in the face of already sorted data, or data that is made
		   up of multiple sorted runs appended together.  Testing shows that a
		   median-of-three algorithm provides better performance than simply
		   picking the middle element for the latter case. */
		mid = lo + (size / 2) * width;      /* find middle element */

		/* Sort the first, middle, last elements into order */
		if (comp(lo, mid) > 0)
		{
			swap(lo, mid, width);
		}

		if (comp(lo, hi) > 0)
		{
			swap(lo, hi, width);
		}

		if (comp(mid, hi) > 0)
		{
			swap(mid, hi, width);
		}

		/* We now wish to partition the array into three pieces, one consisting
		   of elements <= partition element, one of elements equal to the
		   partition element, and one of elements > than it.  This is done
		   below; comments indicate conditions established at every step. */
		loguy = lo;
		higuy = hi;

		/* Note that higuy decreases and loguy increases on every iteration,
		   so loop must terminate. */
		for (;;)
		{
			/* lo <= loguy < hi, lo < higuy <= hi,
			   A[i] <= A[mid] for lo <= i <= loguy,
			   A[i] > A[mid] for higuy <= i < hi,
			   A[hi] >= A[mid] */
			/* The doubled loop is to avoid calling comp(mid,mid), since some
			   existing comparison funcs don't work when passed the same
			   value for both pointers. */
			if (mid > loguy)
			{
				do
				{
					loguy += width;
				}
				while (loguy < mid && comp(loguy, mid) <= 0);
			}

			if (mid <= loguy)
			{
				do
				{
					loguy += width;
				}
				while (loguy <= hi && comp(loguy, mid) <= 0);
			}

			/* lo < loguy <= hi+1, A[i] <= A[mid] for lo <= i < loguy,
			   either loguy > hi or A[loguy] > A[mid] */

			do
			{
				higuy -= width;
			}
			while (higuy > mid && comp(higuy, mid) > 0);

			/* lo <= higuy < hi, A[i] > A[mid] for higuy < i < hi,
			   either higuy == lo or A[higuy] <= A[mid] */

			if (higuy < loguy)
				break;

			/* if loguy > hi or higuy == lo, then we would have exited, so
			   A[loguy] > A[mid], A[higuy] <= A[mid],
			   loguy <= hi, higuy > lo */
			swap(loguy, higuy, width);
			/* If the partition element was moved, follow it.  Only need
			   to check for mid == higuy, since before the swap,
			   A[loguy] > A[mid] implies loguy != mid. */

			if (mid == higuy)
				mid = loguy;

			/* A[loguy] <= A[mid], A[higuy] > A[mid]; so condition at top
			   of loop is re-established */
		}

		/*     A[i] <= A[mid] for lo <= i < loguy,
		       A[i] > A[mid] for higuy < i < hi,
		       A[hi] >= A[mid]
		       higuy < loguy
		   implying:
		       higuy == loguy-1
		       or higuy == hi - 1, loguy == hi + 1, A[hi] == A[mid] */
		/* Find adjacent elements equal to the partition element.  The
		   doubled loop is to avoid calling comp(mid,mid), since some
		   existing comparison funcs don't work when passed the same value
		   for both pointers. */
		higuy += width;

		if (mid < higuy)
		{
			do
			{
				higuy -= width;
			}
			while (higuy > mid && !comp(higuy, mid));
		}

		if (mid >= higuy)
		{
			do
			{
				higuy -= width;
			}
			while (higuy > lo && !comp(higuy, mid));
		}

		/* OK, now we have the following:
		      higuy < loguy
		      lo <= higuy <= hi
		      A[i]  <= A[mid] for lo <= i <= higuy
		      A[i]  == A[mid] for higuy < i < loguy
		      A[i]  >  A[mid] for loguy <= i < hi
		      A[hi] >= A[mid] */
		/* We've finished the partition, now we want to sort the subarrays
		   [lo, higuy] and [loguy, hi].
		   We do the smaller one first to minimize stack usage.
		   We only sort arrays of length 2 or more.*/

		if (higuy - lo >= hi - loguy)
		{
			if (lo < higuy)
			{
				lostk[stkptr] = lo;
				histk[stkptr] = higuy;
				++stkptr;
			}                           /* save big recursion for later */

			if (loguy < hi)
			{
				lo = loguy;
				goto recurse;           /* do small recursion */
			}
		}
		else
		{
			if (loguy < hi)
			{
				lostk[stkptr] = loguy;
				histk[stkptr] = hi;
				++stkptr;               /* save big recursion for later */
			}

			if (lo < higuy)
			{
				hi = higuy;
				goto recurse;           /* do small recursion */
			}
		}
	}

	/* We have sorted the array, except for any pending sorts on the stack.
	   Check if there are any, and do them. */
	--stkptr;

	if (stkptr >= 0)
	{
		lo = lostk[stkptr];
		hi = histk[stkptr];
		goto recurse;           /* pop subarray from stack */
	}
	else
		return;                 /* all subarrays done */
}


/***
*shortsort(hi, lo, width, comp) - insertion sort for sorting short arrays
*
*Purpose:
*       sorts the sub-array of elements between lo and hi (inclusive)
*       side effects:  sorts in place
*       assumes that lo < hi
*
*Entry:
*       char *lo = pointer to low element to sort
*       char *hi = pointer to high element to sort
*       size_t width = width in bytes of each array element
*       int (*comp)() = pointer to function returning analog of strcmp for
*               strings, but supplied by user for comparing the array elements.
*               it accepts 2 pointers to elements and returns neg if 1<2, 0 if
*               1=2, pos if 1>2.
*
*Exit:
*       returns void
*
*Exceptions:
*
*******************************************************************************/

static void  shortsort(
    char *lo,
    char *hi,
    size_t width,
    int (__cdecl *comp)(const void *, const void *)
)
{
	char *p;
	/* Note: in assertions below, i and j are alway inside original bound of
	   array to sort. */

	while (hi > lo)
	{
		/* A[i] <= A[j] for i <= j, j > hi */
		char* max = lo;

		for (p = lo+width; p <= hi; p += width)
		{
			/* A[i] <= A[max] for lo <= i < p */
			if (comp(p, max) > 0)
			{
				max = p;
			}

			/* A[i] <= A[max] for lo <= i <= p */
		}

		/* A[i] <= A[max] for lo <= i <= hi */
		swap(max, hi, width);
		/* A[i] <= A[hi] for i <= hi, so A[i] <= A[j] for i <= j, j >= hi */
		hi -= width;
		/* A[i] <= A[j] for i <= j, j > hi, loop top condition established */
	}

	/* A[i] <= A[j] for i <= j, j > lo, which implies A[i] <= A[j] for i < j,
	   so array is sorted */
}


/***
*swap(a, b, width) - swap two elements
*
*Purpose:
*       swaps the two array elements of size width
*
*Entry:
*       char *a, *b = pointer to two elements to swap
*       size_t width = width in bytes of each array element
*
*Exit:
*       returns void
*
*Exceptions:
*
*******************************************************************************/

static void  swap(
    char *a,
    char *b,
    size_t width
)
{
	if (a != b)
	{
		while (width--)
		{
			if (*a != *b)
			{
				*a ^= *b;
				*b ^= *a;
				*a ^= *b;
			}
			++a;
			++b;
		}
	}
}

/* Always compile this module for speed, not size */
/* prototypes for local routines */
static void  shortsort_m(char *lo, char *hi, size_t width,
                         int (__cdecl *comp)(const void *, const void *), void* t);
static void  swap_m(char *p, char *q, size_t width, void* t);

/* this parameter defines the cutoff between using quick sort and
   insertion sort for arrays; arrays with lengths shorter or equal to the
   below value use insertion sort */

void __cdecl qsort_m(
    void *base,
    size_t num,
    size_t width,
    int (__cdecl *comp)(const void *, const void *)
)
{
	/* Note: the number of stack entries required is no more than
	   1 + log2(num), so 30 is sufficient for any array */
	char *lo, *hi;              /* ends of sub-array currently sorting */
	char *mid;                  /* points to middle of subarray */
	char *loguy, *higuy;        /* traveling pointers for partition step */
	size_t size;                /* size of the sub-array */
	char *lostk[STKSIZ], *histk[STKSIZ];
	char* t = (char*)alloca(width);
	int stkptr;                 /* stack for saving sub-array to be processed */

	if (num < 2 || !width)
		return;                 /* nothing to do */

	stkptr = 0;                 /* initialize stack */
	lo = (char *)base;
	hi = (char *)base + width * (num-1);        /* initialize limits */
	/* this entry point is for pseudo-recursion calling: setting
	   lo and hi and jumping to here is like recursion, but stkptr is
	   preserved, locals aren't, so we preserve stuff on the stack */
recurse:
	size = (hi - lo) / width + 1;        /* number of el's to sort */

	/* below a certain size, it is faster to use a O(n^2) sorting method */
	if (size <= CUTOFF)
	{
		shortsort_m(lo, hi, width, comp, t);
	}
	else
	{
		/* First we pick a partitioning element.  The efficiency of the
		   algorithm demands that we find one that is approximately the median
		   of the values, but also that we select one fast.  We choose the
		   median of the first, middle, and last elements, to avoid bad
		   performance in the face of already sorted data, or data that is made
		   up of multiple sorted runs appended together.  Testing shows that a
		   median-of-three algorithm provides better performance than simply
		   picking the middle element for the latter case. */
		mid = lo + (size / 2) * width;      /* find middle element */

		/* Sort the first, middle, last elements into order */
		if (comp(lo, mid) > 0)
		{
			swap_m(lo, mid, width, t);
		}

		if (comp(lo, hi) > 0)
		{
			swap_m(lo, hi, width, t);
		}

		if (comp(mid, hi) > 0)
		{
			swap_m(mid, hi, width, t);
		}

		/* We now wish to partition the array into three pieces, one consisting
		   of elements <= partition element, one of elements equal to the
		   partition element, and one of elements > than it.  This is done
		   below; comments indicate conditions established at every step. */
		loguy = lo;
		higuy = hi;

		/* Note that higuy decreases and loguy increases on every iteration,
		   so loop must terminate. */
		for (;;)
		{
			/* lo <= loguy < hi, lo < higuy <= hi,
			   A[i] <= A[mid] for lo <= i <= loguy,
			   A[i] > A[mid] for higuy <= i < hi,
			   A[hi] >= A[mid] */
			/* The doubled loop is to avoid calling comp(mid,mid), since some
			   existing comparison funcs don't work when passed the same
			   value for both pointers. */
			if (mid > loguy)
			{
				do
				{
					loguy += width;
				}
				while (loguy < mid && comp(loguy, mid) <= 0);
			}

			if (mid <= loguy)
			{
				do
				{
					loguy += width;
				}
				while (loguy <= hi && comp(loguy, mid) <= 0);
			}

			/* lo < loguy <= hi+1, A[i] <= A[mid] for lo <= i < loguy,
			   either loguy > hi or A[loguy] > A[mid] */

			do
			{
				higuy -= width;
			}
			while (higuy > mid && comp(higuy, mid) > 0);

			/* lo <= higuy < hi, A[i] > A[mid] for higuy < i < hi,
			   either higuy == lo or A[higuy] <= A[mid] */

			if (higuy < loguy)
				break;

			/* if loguy > hi or higuy == lo, then we would have exited, so
			   A[loguy] > A[mid], A[higuy] <= A[mid],
			   loguy <= hi, higuy > lo */
			swap_m(loguy, higuy, width, t);
			/* If the partition element was moved, follow it.  Only need
			   to check for mid == higuy, since before the swap,
			   A[loguy] > A[mid] implies loguy != mid. */

			if (mid == higuy)
				mid = loguy;

			/* A[loguy] <= A[mid], A[higuy] > A[mid]; so condition at top
			   of loop is re-established */
		}

		/*     A[i] <= A[mid] for lo <= i < loguy,
		       A[i] > A[mid] for higuy < i < hi,
		       A[hi] >= A[mid]
		       higuy < loguy
		   implying:
		       higuy == loguy-1
		       or higuy == hi - 1, loguy == hi + 1, A[hi] == A[mid] */
		/* Find adjacent elements equal to the partition element.  The
		   doubled loop is to avoid calling comp(mid,mid), since some
		   existing comparison funcs don't work when passed the same value
		   for both pointers. */
		higuy += width;

		if (mid < higuy)
		{
			do
			{
				higuy -= width;
			}
			while (higuy > mid && !comp(higuy, mid));
		}

		if (mid >= higuy)
		{
			do
			{
				higuy -= width;
			}
			while (higuy > lo && !comp(higuy, mid));
		}

		/* OK, now we have the following:
		      higuy < loguy
		      lo <= higuy <= hi
		      A[i]  <= A[mid] for lo <= i <= higuy
		      A[i]  == A[mid] for higuy < i < loguy
		      A[i]  >  A[mid] for loguy <= i < hi
		      A[hi] >= A[mid] */
		/* We've finished the partition, now we want to sort the subarrays
		   [lo, higuy] and [loguy, hi].
		   We do the smaller one first to minimize stack usage.
		   We only sort arrays of length 2 or more.*/

		if (higuy - lo >= hi - loguy)
		{
			if (lo < higuy)
			{
				lostk[stkptr] = lo;
				histk[stkptr] = higuy;
				++stkptr;
			}                           /* save big recursion for later */

			if (loguy < hi)
			{
				lo = loguy;
				goto recurse;           /* do small recursion */
			}
		}
		else
		{
			if (loguy < hi)
			{
				lostk[stkptr] = loguy;
				histk[stkptr] = hi;
				++stkptr;               /* save big recursion for later */
			}

			if (lo < higuy)
			{
				hi = higuy;
				goto recurse;           /* do small recursion */
			}
		}
	}

	/* We have sorted the array, except for any pending sorts on the stack.
	   Check if there are any, and do them. */
	--stkptr;

	if (stkptr >= 0)
	{
		lo = lostk[stkptr];
		hi = histk[stkptr];
		goto recurse;           /* pop subarray from stack */
	}
	else
	{
		return;                 /* all subarrays done */
	}
}


/***
*shortsort_m(hi, lo, width, comp) - insertion sort for sorting short arrays
*
*Purpose:
*       sorts the sub-array of elements between lo and hi (inclusive)
*       side effects:  sorts in place
*       assumes that lo < hi
*
*Entry:
*       char *lo = pointer to low element to sort
*       char *hi = pointer to high element to sort
*       size_t width = width in bytes of each array element
*       int (*comp)() = pointer to function returning analog of strcmp for
*               strings, but supplied by user for comparing the array elements.
*               it accepts 2 pointers to elements and returns neg if 1<2, 0 if
*               1=2, pos if 1>2.
*
*Exit:
*       returns void
*
*Exceptions:
*
*******************************************************************************/

static void  shortsort_m(
    char *lo,
    char *hi,
    size_t width,
    int (__cdecl *comp)(const void *, const void *),
    void* t
)
{
	char *p;
	/* Note: in assertions below, i and j are alway inside original bound of
	   array to sort. */

	while (hi > lo)
	{
		/* A[i] <= A[j] for i <= j, j > hi */
		char* ptrmax = lo;

		for (p = lo+width; p <= hi; p += width)
		{
			/* A[i] <= A[ptrmax] for lo <= i < p */
			if (comp(p, ptrmax) > 0)
			{
				ptrmax = p;
			}

			/* A[i] <= A[ptrmax] for lo <= i <= p */
		}

		/* A[i] <= A[ptrmax] for lo <= i <= hi */
		swap_m(ptrmax, hi, width, t);
		/* A[i] <= A[hi] for i <= hi, so A[i] <= A[j] for i <= j, j >= hi */
		hi -= width;
		/* A[i] <= A[j] for i <= j, j > hi, loop top condition established */
	}

	/* A[i] <= A[j] for i <= j, j > lo, which implies A[i] <= A[j] for i < j,
	   so array is sorted */
}


/***
*swap(a, b, width) - swap two elements
*
*Purpose:
*       swaps the two array elements of size width
*
*Entry:
*       char *a, *b = pointer to two elements to swap
*       size_t width = width in bytes of each array element
*
*Exit:
*       returns void
*
*Exceptions:
*
*******************************************************************************/

static void  swap_m(
    char *a,
    char *b,
    size_t width,
    void* t
)
{
	memcpy(t, a, width);
	memcpy(a, b, width);
	memcpy(b, t, width);
}

/* end qsort */

};
