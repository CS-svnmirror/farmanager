#pragma once

/*
farrtl.cpp

��������������� ��������� CRT �������
*/

#ifdef _DEBUG
void* xf_malloc(size_t size, const char* Function, const char* File, int Line);
void* xf_realloc(void* block, size_t size, const char* Function, const char* File, int Line);
void* xf_realloc_nomove(void* block, size_t size, const char* Function, const char* File, int Line);
char* xf_strdup(const char* string, const char* Function, const char* File, int Line);
wchar_t* xf_wcsdup(const wchar_t* string, const char* Function, const char* File, int Line);
void* operator new(size_t size, const char* Function, const char* File, int Line);
void* operator new[](size_t size, const char* Function, const char* File, int Line);
void operator delete(void* block, const char* Function, const char* File, int Line);
void operator delete[](void* block, const char* Function, const char* File, int Line);

#define xf_malloc(size) xf_malloc(size, __FUNCTION__, __FILE__, __LINE__)
#define xf_realloc(block, size) xf_realloc(block, size, __FUNCTION__, __FILE__, __LINE__)
#define xf_realloc_nomove(block, size) xf_realloc_nomove(block, size, __FUNCTION__, __FILE__, __LINE__)
#define xf_strdup(string) xf_strdup(string, __FUNCTION__, __FILE__, __LINE__)
#define xf_wcsdup(string) xf_wcsdup(string, __FUNCTION__, __FILE__, __LINE__)
#define new new(__FUNCTION__, __FILE__, __LINE__)
#else
void* xf_malloc(size_t size);
void* xf_realloc_nomove(void* block, size_t size);
void* xf_realloc(void* block, size_t size);
char* xf_strdup(const char* string);
wchar_t* xf_wcsdup(const wchar_t* string);
#endif

void PrintMemory();

void  xf_free(void* block);
char* xstrncpy(char* dest, const char* src, size_t DestSize);
wchar_t* xwcsncpy(wchar_t* dest, const wchar_t* src, size_t DestSize);



extern "C"
{
	void __cdecl qsortex(char *base, size_t nel, size_t width, int (WINAPI *comp_fp)(const void *, const void *,void*), void *user);
	void* WINAPI bsearchex(const void* key,const void* base,size_t nelem,size_t width,int (WINAPI *fcmp)(const void*, const void*,void*),void* userparam);
	void __cdecl far_qsort(void *base, size_t num, size_t width, int (__cdecl *comp)(const void *, const void *));
}

#define ALIGN(value) ((value+(sizeof(void*)-1))&~(sizeof(void*)-1))
