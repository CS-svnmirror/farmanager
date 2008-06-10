#include <all_far.h>
#pragma hdrstop

#include "fstdlib.h"

LPVOID DECLSPEC __Alloc_S( SIZE_T sz )              { return malloc(sz); }
void   DECLSPEC __Del_S( LPVOID ptr )               { if (ptr) free(ptr); }
LPVOID DECLSPEC __Realloc_S( LPVOID ptr,SIZE_T sz ) { return (!ptr) ? malloc(sz) : realloc(ptr,sz); }
SIZE_T DECLSPEC __PtrSize_S( LPVOID ptr )           { return 0; }

#if defined(__BORLAND)
BOOL   DECLSPEC __HeapCheck_S( void )               { return heapcheck() == _HEAPOK; }
#else
#if defined(__QNX__)
BOOL   DECLSPEC __HeapCheck_S( void )               { return _heapchk() == _HEAPOK; }
#else
#if defined(__MSOFT) || defined(__GNU)
BOOL   DECLSPEC __HeapCheck_S( void )               { return _heapchk() == _HEAPOK; }
#else
#if defined(__SYMANTEC)
#error "BOOL   DECLSPEC __HeapCheck_S( void )               { return /*_heapchk() == _HEAPOK*/; }"
#endif //__BORLAND
#endif //__QNX
#endif //__MSOFT
#endif //__SYMANTEC
