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

#ifdef MEMCHECK
#undef xf_malloc
#undef xf_realloc
#undef xf_realloc_nomove
#undef DuplicateString
#undef new
#endif

static void* Expander(void* block, size_t size)
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
	return malloc(size);
}

void xf_free(void* block)
{
	return free(block);
}

void* xf_realloc(void* block, size_t size)
{
	return realloc(block, size);
}

void* xf_realloc_nomove(void * block, size_t size)
{
	if (!block)
	{
		return xf_malloc(size);
	}
	else if (Expander(block, size))
	{
		return block;
	}
	else
	{
		xf_free(block);
		return xf_malloc(size);
	}
}

char* DuplicateString(const char * str)
{
	return str? strcpy(new char[strlen(str) + 1], str) : nullptr;
}

wchar_t* DuplicateString(const wchar_t * str)
{
	return str? wcscpy(new wchar_t[wcslen(str) + 1], str) : nullptr;
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
static bool MonitoringEnabled = true;

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

static const int EndMarker = 0xDEADBEEF;

inline static int& GetMarker(MEMINFO* Info)
{
	return *reinterpret_cast<int*>(reinterpret_cast<char*>(Info)+Info->Size-sizeof(EndMarker));
}

static void RegisterBlock(MEMINFO *block)
{
	if (!MonitoringEnabled)
		return;

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
	if (!MonitoringEnabled)
		return;

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

static std::string FormatLine(const char* File, int Line, const char* Function, ALLOCATION_TYPE Type, size_t Size)
{
	const char* sType = nullptr;
	switch (Type)
	{
	case AT_RAW:
		sType = "malloc";
		break;
	case AT_SCALAR:
		sType = "operator new";
		break;
	case AT_VECTOR:
		sType = "operator new[]";
		break;
	};

	return std::string(File) + ':' + std::to_string(Line) + " -> " + Function + ':' + sType + " (" + std::to_string(Size) + " bytes)";
}

thread_local bool inside_far_bad_alloc = false;

class far_bad_alloc: public std::bad_alloc
{
public:
	far_bad_alloc(const char* File, int Line, const char* Function, ALLOCATION_TYPE Type, size_t Size) noexcept
	{
		if (!inside_far_bad_alloc)
		{
			inside_far_bad_alloc = true;
			try
			{
				m_What = "bad allocation at " + FormatLine(File, Line, Function, Type, Size);
			}
			catch (...)
			{
			}
			inside_far_bad_alloc = false;
		}
	}

	far_bad_alloc(const far_bad_alloc& rhs):
		std::bad_alloc(rhs),
		m_What(rhs.m_What)
	{
	}

	COPY_OPERATOR_BY_SWAP(far_bad_alloc);

	far_bad_alloc(far_bad_alloc&& rhs) noexcept { *this = std::move(rhs); }
	MOVE_OPERATOR_BY_SWAP(far_bad_alloc);

	virtual const char* what() const noexcept override { return m_What.empty() ? std::bad_alloc::what() : m_What.data(); }

	void swap(far_bad_alloc& rhs) noexcept
	{
		m_What.swap(rhs.m_What);
	}

	FREE_SWAP(far_bad_alloc);

private:
	std::string m_What;
};

inline static size_t GetRequiredSize(size_t RequestedSize)
{
	return sizeof(MEMINFO) + RequestedSize + sizeof(EndMarker);
}

static void* DebugAllocator(size_t size, bool Noexcept, ALLOCATION_TYPE type,const char* Function,  const char* File, int Line)
{
	size_t realSize = GetRequiredSize(size);
	auto Info = static_cast<MEMINFO*>(malloc(realSize));

	if (!Info)
	{
		if (Noexcept)
			return nullptr;
		else
			throw far_bad_alloc(File, Line, Function, type, size);
	}

	Info->AllocationType = type;
	Info->Size = realSize;
	Info->Function = Function;
	Info->File = File;
	Info->Line = Line;

	GetMarker(Info) = EndMarker;

	RegisterBlock(Info);
	return ToUser(Info);
}

static void DebugDeallocator(void* block, ALLOCATION_TYPE type)
{
	void* realBlock = block? ToReal(block) : nullptr;
	if (realBlock)
	{
		auto Info = static_cast<MEMINFO*>(realBlock);
		assert(Info->AllocationType == type);
		assert(GetMarker(Info) == EndMarker);
		UnregisterBlock(Info);
	}
	free(realBlock);
}

static void* DebugReallocator(void* block, size_t size, const char* Function, const char* File, int Line)
{
	if(!block)
		return DebugAllocator(size, true, AT_RAW, Function, File, Line);

	MEMINFO* Info = ToReal(block);
	assert(Info->AllocationType == AT_RAW);
	assert(GetMarker(Info) == EndMarker);
	UnregisterBlock(Info);
	size_t realSize = GetRequiredSize(size);

	Info = static_cast<MEMINFO*>(realloc(Info, realSize));

	if (!Info)
		return nullptr;

	Info->AllocationType = AT_RAW;
	Info->Size = realSize;

	GetMarker(Info) = EndMarker;

	RegisterBlock(Info);
	return ToUser(Info);
}

static void* DebugExpander(void* block, size_t size)
{
	MEMINFO* Info = ToReal(block);
	assert(Info->AllocationType == AT_RAW);
	size_t realSize = GetRequiredSize(size);

	// _expand() calls HeapReAlloc which can change the status code, it's bad for us
	NTSTATUS status = Imports().RtlGetLastNtStatus();
	Info = static_cast<MEMINFO*>(Expander(Info, realSize));
	//RtlNtStatusToDosError also remembers the status code value in the TEB:
	Imports().RtlNtStatusToDosError(status);

	if(Info)
	{
		AllocatedMemorySize-=Info->Size;
		Info->Size = realSize;
		AllocatedMemorySize+=Info->Size;
		GetMarker(Info) = EndMarker;
	}

	return Info? ToUser(Info) : nullptr;
}

void PrintMemory()
{
	bool MonitoringState = MonitoringEnabled;
	MonitoringEnabled = false;

	if (CallNewDeleteVector || CallNewDeleteScalar || CallMallocFree || AllocatedMemoryBlocks || AllocatedMemorySize)
	{
		std::wostringstream oss;
		oss << L"Memory leaks detected:" << std::endl;
		if (CallNewDeleteVector)
			oss << L"  delete[]:   " << CallNewDeleteVector << std::endl;
		if (CallNewDeleteScalar)
			oss << L"  delete:     " << CallNewDeleteScalar << std::endl;
		if (CallMallocFree)
			oss << L"  free():     " << CallMallocFree << std::endl;
		if (AllocatedMemoryBlocks)
			oss << L"Total blocks: " << AllocatedMemoryBlocks << std::endl;
		if (AllocatedMemorySize)
			oss << L"Total bytes:  " << AllocatedMemorySize - AllocatedMemoryBlocks * sizeof(MEMINFO) <<  L" payload, " << AllocatedMemoryBlocks * sizeof(MEMINFO) << L" overhead" << std::endl;
		oss << std::endl;

		oss << "Not freed blocks:" << std::endl;

		std::wcerr << oss.str();
		OutputDebugString(oss.str().data());
		oss.str(string());

		for(auto i = FirstMemBlock.next; i; i = i->next)
		{
			oss << FormatLine(i->File, i->Line, i->Function, i->AllocationType, i->Size - sizeof(MEMINFO)).data() << std::endl;
			std::wcerr << oss.str();
			OutputDebugString(oss.str().data());
			oss.str(string());
		}
	}
	MonitoringEnabled = MonitoringState;
}

};

void* xf_malloc(size_t size, const char* Function, const char* File, int Line)
{
	return memcheck::DebugAllocator(size, true, memcheck::AT_RAW, Function, File, Line);
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
	return memcheck::DebugAllocator(size, false, memcheck::AT_SCALAR, __FUNCTION__, __FILE__, __LINE__);
}

void* operator new(size_t size, const std::nothrow_t& nothrow_value) noexcept
{
	return memcheck::DebugAllocator(size, true, memcheck::AT_SCALAR, __FUNCTION__, __FILE__, __LINE__);
}

void* operator new[](size_t size)
{
	return memcheck::DebugAllocator(size, false, memcheck::AT_VECTOR, __FUNCTION__, __FILE__, __LINE__);
}

void* operator new[](size_t size, const std::nothrow_t& nothrow_value) noexcept
{
	return memcheck::DebugAllocator(size, true, memcheck::AT_VECTOR, __FUNCTION__, __FILE__, __LINE__);
}

void* operator new(size_t size, const char* Function, const char* File, int Line)
{
	return memcheck::DebugAllocator(size, false, memcheck::AT_SCALAR, Function, File, Line);
}

void* operator new(size_t size, const std::nothrow_t& nothrow_value, const char* Function, const char* File, int Line) noexcept
{
	return memcheck::DebugAllocator(size, true, memcheck::AT_SCALAR, Function, File, Line);
}

void* operator new[](size_t size, const char* Function, const char* File, int Line)
{
	return memcheck::DebugAllocator(size, false, memcheck::AT_VECTOR, Function, File, Line);
}

void* operator new[](size_t size, const std::nothrow_t& nothrow_value, const char* Function, const char* File, int Line) noexcept
{
	return memcheck::DebugAllocator(size, true, memcheck::AT_VECTOR, Function, File, Line);
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

char* DuplicateString(const char * str, const char* Function, const char* File, int Line)
{
	return str? strcpy(new(Function, File, Line) char[strlen(str) + 1], str) : nullptr;
}

wchar_t* DuplicateString(const wchar_t * str, const char* Function, const char* File, int Line)
{
	return str? wcscpy(new(Function, File, Line) wchar_t[wcslen(str) + 1], str) : nullptr;
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
