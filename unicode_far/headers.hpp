#pragma once

/*
headers.hpp

����������� ���������
*/
/*
Copyright � 1996 Eugene Roshal
Copyright � 2000 Far Group
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:
1. Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.
3. The name of the authors may not be used to endorse or promote products
   derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <algorithm>
#include <bitset>
#include <queue>
#include <list>
#include <map>
#include <memory>
#include <stack>
#include <vector>
#include <cassert>
#include <ctime>

#ifdef __GNUC__
#include <climits>
#include <cfloat>
#include <malloc.h>
#endif //__GNUC__

#include <process.h>

#undef _W32API_OLD

#ifdef _MSC_VER
# include <sdkddkver.h>
# if _WIN32_WINNT < 0x0601
#  error Windows SDK v7.0 (or higher) required
# endif
#endif //_MSC_VER

#ifdef __GNUC__
# define GCC_VER_(gcc_major,gcc_minor,gcc_patch) (100*(gcc_major) + 10*(gcc_minor) + (gcc_patch))
# define _GCC_VER GCC_VER_(__GNUC__, __GNUC_MINOR__, __GNUC_PATCHLEVEL__)
# include <w32api.h>
# define _W32API_VER (100*(__W32API_MAJOR_VERSION) + (__W32API_MINOR_VERSION))
# if _W32API_VER < 314
#  error w32api-3.14 (or higher) required
# endif
# if _W32API_VER < 317
#  define _W32API_OLD 1
# endif
# undef WINVER
# undef _WIN32_WINNT
# undef _WIN32_IE
# define WINVER       0x0601
# define _WIN32_WINNT 0x0601
# define _WIN32_IE    0x0700
#endif // __GNUC__

#define WIN32_LEAN_AND_MEAN
#define VC_EXTRALEAN
#ifndef NOMINMAX
#define NOMINMAX
#endif

#define WIN32_NO_STATUS //exclude ntstatus.h macros from winnt.h
#include <windows.h>
#undef WIN32_NO_STATUS
#include <winioctl.h>
#include <mmsystem.h>
#include <wininet.h>
#include <winspool.h>
#include <setupapi.h>
#include <aclapi.h>
#include <sddl.h>
#include <dbt.h>
#include <lm.h>
#define SECURITY_WIN32
#include <security.h>
#define PSAPI_VERSION 1
#include <psapi.h>
#include <shlobj.h>
#include <shellapi.h>

#ifdef _MSC_VER
# include <ntstatus.h>
# include <shobjidl.h>
# include <winternl.h>
# include <cfgmgr32.h>
# include <ntddscsi.h>
# include <virtdisk.h>
# include <RestartManager.h>
#endif // _MSC_VER

#ifdef __GNUC__
# define __NTDDK_H
  struct _ADAPTER_OBJECT;
  typedef struct _ADAPTER_OBJECT ADAPTER_OBJECT,*PADAPTER_OBJECT;
# ifdef _W32API_OLD
#  include <ntstatus.h>
#  include <cfgmgr32.h>
#  include <ntddscsi.h>
# else
#  include <ddk/ntstatus.h>
#  include <ddk/cfgmgr32.h>
#  include <ddk/ntddscsi.h>
#  include <ntdef.h>
# endif
# ifndef offsetof
#  define offsetof(Type, Field) __builtin_offsetof(Type, Field)
# endif
#endif // __GNUC__

#include "SDK/sdk.common.h"

#ifdef _MSC_VER
# include "SDK/sdk.vc.h"
# define SEH_TRY __try
# define SEH_EXCEPT __except
#endif // _MSC_VER

#ifdef __GNUC__
# include "SDK/sdk.gcc.h"
# define _wcstoi64 wcstoll
# define SEH_TRY
# define SEH_EXCEPT(x) if(false)
#endif // __GNUC__

inline const wchar_t* NullToEmpty(const wchar_t* Str) { return Str? Str : L"";} 
inline const wchar_t* EmptyToNull(const wchar_t* Str) { return (Str && !*Str)? nullptr : Str;} 

template <class T>
inline const T Round(const T &a, const T &b) { return a/b+(a%b*2>b?1:0); }

inline void* ToPtr(intptr_t T){ return reinterpret_cast<void*>(T); }

template<typename T>
inline void ClearStruct(T& s)
{
	static_assert(std::is_pod<T>::value, "ClearStruct template requires a POD type");
	static_assert(!std::is_pointer<T>::value, "ClearStruct template requires a reference to an object");
	memset(&s, 0, sizeof(s));
}

template<typename T>
inline void ClearStructUnsafe(T& s)
{
	static_assert(!std::is_pointer<T>::value, "ClearStruct template requires a reference to an object");
	memset(&s, 0, sizeof(s));
}

template<typename T>
inline void ClearArray(T& a)
{
	static_assert(std::is_array<T>::value, "ClearArray template requires an array");
	memset(a, 0, sizeof(a));
}

#define SIGN_UNICODE    0xFEFF
#define SIGN_REVERSEBOM 0xFFFE
#define SIGN_UTF8       0xBFBBEF

#ifdef __GNUC__
# if _GCC_VER < GCC_VER_(4,6,1)
#  error gcc 4.6.1 (or higher) required
# endif
# define ENUM(ENUM_NAME) enum ENUM_NAME:int
#endif

#ifdef _MSC_VER
# if _MSC_VER>1600
#  define ENUM(ENUM_NAME) enum ENUM_NAME:int
# else
#  define ENUM(ENUM_NAME) enum ENUM_NAME
#  if _MSC_VER<1600
#   define nullptr NULL
#  endif
# endif
#endif

// trick to allow usage of operator :: with decltype(T)
#define DECLTYPE(T) std::enable_if<true, decltype(T)>::type
#define PTRTYPE(T) std::remove_pointer<decltype(T)>::type

#define DECLTYPEOF(T, subtype) std::remove_reference<std::remove_pointer<DECLTYPE(T)>::type>::type::subtype
#define VALUE_TYPE(T) DECLTYPEOF(T, value_type)
#define ITERATOR(T) DECLTYPEOF(T, iterator)
#define CONST_ITERATOR(T) DECLTYPEOF(T, const_iterator)
#define REVERSE_ITERATOR(T) DECLTYPEOF(T, reverse_iterator)
#define CONST_REVERSE_ITERATOR(T) DECLTYPEOF(T, const_reverse_iterator)

#define RANGE(T, i) (T).begin(), (T).end(), [&](VALUE_TYPE(T)& i)
#define FOR_RANGE(T, i) for(auto i = (T).begin(); i != (T).end(); ++i)

template<typename T>
inline void DeleteValues(T& std_container)
{
	std::for_each(std_container.begin(), std_container.end(), std::default_delete<typename std::remove_pointer<typename T::value_type>::type>());
}

template <typename T>
bool CheckNullOrStructSize(const T* s) {return !s || (s->StructSize >= sizeof(T));}
template <typename T>
bool CheckStructSize(const T* s) {return s && (s->StructSize >= sizeof(T));}


#include "noncopyable.hpp"
#include "farrtl.hpp"
#include "UnicodeString.hpp"
#include "format.hpp"
#include "local.hpp"
#include "farwinapi.hpp"
#include "cvtname.hpp"

#include "colors.hpp"
#include "palette.hpp"

#ifdef _DEBUG
#define SELF_TEST(code) \
	namespace { \
		struct SelfTest { \
			SelfTest() { \
				code; \
			} \
		} _SelfTest; \
	}
#else
#define SELF_TEST(code)
#endif

#include "global.hpp"
