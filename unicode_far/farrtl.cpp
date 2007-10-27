/*
farrtl.cpp

��������������� ������� ������ � �������: new/delete/malloc/realloc/free
*/
/*
Copyright (c) 1996 Eugene Roshal
Copyright (c) 2000 Far Group
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

#include "headers.hpp"
#pragma hdrstop
#include "fn.hpp"
#include "farconst.hpp"

int _cdecl getdisk(void)
{
  unsigned drive;
  wchar_t  buf[MAX_PATH];

  /* Use GetCurrentDirectory to get the current directory path, then
   * parse the drive name.
   */
  GetCurrentDirectoryW(sizeof(buf)/sizeof(wchar_t), buf);    /* ignore errors */
  drive = Upper(buf[0]) - L'A';
  return (int)drive;
}

#if defined(__BORLANDC__)
//#include <windows.h>
//#include <stdio.h>
//#include <stdlib.h>
//#include <fcntl.h>

#ifdef __cplusplus
extern "C" {
#endif

int  __IOerror    (int  __doserror);   /* returns -1 */
int  __NTerror     (void);              /* returns -1 */

#ifdef __cplusplus
};
#endif

#define RETURN(code)    {rc=(code); goto exit;}

//extern unsigned  _nfile;

/* Array of open file flags.
 */
extern unsigned int _openfd[];

/* Array of open file handles (not used on OS/2).
 */
extern long _handles[];

static int Displacement (FILE *fp)
{
  register    level;
  int         disp;
  register    unsigned char *P;

  if (fp->level < 0)
    disp = level = fp->bsize + fp->level + 1;
  else
    disp = level = fp->level;

  if (fp->flags & _F_BIN)
    return disp;

  P = fp->curp;

  if (fp->level < 0)          /* If writing */
  {
    while (level--)
     if ('\n' == *--P)
       disp++;
  }
  else                        /* Else reading */
  {
    while (level--)
     if ('\n' == *P++)
       disp++;
  }

  return  disp;
}


static __int64 __lseek64(int fd, __int64 offset, int kind)
{
  FAR_INT64 Number;

  DWORD  method;

  if ((unsigned)fd >= _nfile)
    return (__int64)__IOerror(ERROR_INVALID_HANDLE);

  /* Translate the POSIX seek type to the NT method.
   */
  switch (kind)
  {
    case SEEK_SET:
      method = FILE_BEGIN;
      break;
    case SEEK_CUR:
      method = FILE_CURRENT;
      break;
    case SEEK_END:
      method = FILE_END;
      break;
    default:
      return ((__int64)__IOerror(ERROR_INVALID_FUNCTION));
  }

  _openfd[fd] &= ~_O_EOF;      /* forget about ^Z      */

  Number.Part.HighPart=offset>>32;

  Number.Part.LowPart = SetFilePointer((HANDLE)_handles[fd], (DWORD)offset, &Number.Part.HighPart, method);
  if (Number.Part.LowPart == -1 && GetLastError() != NO_ERROR)
  {
    __NTerror();        /* set errno */
    Number.i64=-1;
  }

  return Number.i64;
}

int fseek64 (FILE *fp, __int64 offset, int whence)
{
  if (fflush (fp))
    return (EOF);

  if (SEEK_CUR == whence && fp->level > 0)
    offset -= Displacement (fp);

  fp->flags &= ~(_F_OUT | _F_IN | _F_EOF);
  fp->level = 0;
  fp->curp = fp->buffer;

  return (__lseek64 (fp->fd, offset, whence) == -1L) ? EOF : 0;
}

__int64 ftell64(FILE *fp)
{
  __int64  oldpos, rc;

  if ((rc = __lseek64( fp->fd, 0, SEEK_CUR )) != -1)
  {
    if (fp->level < 0)  /* if writing */
    {
      if (_openfd[fp->fd] & O_APPEND)
      {
        /* In append mode, find out how big the file is,
         * and add the number of buffered bytes to that.
         */
        oldpos = rc;
        if ((rc = __lseek64( fp->fd, 0, SEEK_END )) == -1)
          return rc;
        if (__lseek64( fp->fd, oldpos, SEEK_SET ) == -1)
          return -1;
      }
      rc += Displacement(fp);
    }
    else
      rc -= Displacement(fp);
  }
  return rc;
}

#else

#include <io.h>
#if !defined(FAR_MSVCRT) && defined(_MSC_VER)
extern "C"{
_CRTIMP __int64 __cdecl _ftelli64 (FILE *str);
_CRTIMP int __cdecl _fseeki64 (FILE *stream, __int64 offset, int whence);
};
#endif

__int64 ftell64(FILE *fp)
{
#ifndef FAR_MSVCRT
  #ifdef __GNUC__
    return ftello64(fp);
  #else
    return _ftelli64(fp);
  #endif
#else
  fpos_t pos;
  if(fgetpos(fp,&pos)!=0)return 0;
  return pos;
#endif
}

int fseek64 (FILE *fp, __int64 offset, int whence)
{
#ifndef FAR_MSVCRT
  #ifdef __GNUC__
    return fseeko64(fp,offset,whence);
  #else
    return _fseeki64(fp,offset,whence);
  #endif
#else
  switch(whence)
  {
    case SEEK_SET:return fsetpos(fp,&offset);
    case SEEK_CUR:
    {
      fpos_t pos;
      fgetpos(fp,&pos);
      pos+=offset;
      return fsetpos(fp,&pos);
    }
    case SEEK_END:
    {
      fpos_t pos;
      fseek(fp,0,SEEK_END);
      fgetpos(fp,&pos);
      pos+=offset;
      return fsetpos(fp,&pos);
    }
  }
  return -1;
#endif
}

#endif


wchar_t *WINAPI FarItoa(int value, wchar_t *string, int radix)
{
  if(string)
    return _itow(value,string,radix);
  return NULL;
}

wchar_t *WINAPI FarItoa64(__int64 value, wchar_t *string, int radix)
{
  if(string)
    return _i64tow(value, string, radix);
  return NULL;
}

int WINAPI FarAtoi(const wchar_t *s)
{
  if(s)
    return _wtoi(s);
  return 0;
}
__int64 WINAPI FarAtoi64(const wchar_t *s)
{
  if(s)
    return _wtoi64(s);
  return _i64(0);
}

void WINAPI FarQsort(void *base, size_t nelem, size_t width,
                     int (__cdecl *fcmp)(const void *, const void *))
{
  if(base && fcmp)
    far_qsort(base,nelem,width,fcmp);
}

void WINAPI FarQsortEx(void *base, size_t nelem, size_t width,
                     int (__cdecl *fcmp)(const void *, const void *,void *user),void *user)
{
  if(base && fcmp)
    qsortex((char*)base,nelem,width,fcmp,user);
}

void *WINAPI FarBsearch(const void *key, const void *base, size_t nelem, size_t width, int (__cdecl *fcmp)(const void *, const void *))
{
  if(key && fcmp && base)
    return bsearch(key,base,nelem,width,fcmp);
  return NULL;
}

#if defined(SYSLOG)
extern long CallMallocFree;
#endif

void __cdecl  xf_free(void *__block)
{
#if defined(SYSLOG)
  CallMallocFree--;
#endif
  free(__block);
}

void *__cdecl xf_malloc(size_t __size)
{
  void *Ptr=malloc(__size);
#if defined(SYSLOG)
  CallMallocFree++;
#endif
  return Ptr;
}

void *__cdecl xf_realloc(void *__block, size_t __size)
{
  void *Ptr=realloc(__block,__size);
#if defined(SYSLOG)
  if(!__block)
    CallMallocFree++;
#endif
  return Ptr;
}

#if defined(__BORLANDC__) || defined(_DEBUG)
// || (defined(_MSC_VER) && _MSC_VER < 1300)

#define _UI64_MAX     0xffffffffffffffffui64
#define _I64_MIN      (-9223372036854775807i64 - 1)
#define _I64_MAX      9223372036854775807i64
#define ERANGE        34

#define FL_UNSIGNED   1       /* strtouq called */
#define FL_NEG        2       /* negative sign found */
#define FL_OVERFLOW   4       /* overflow occured */
#define FL_READDIGIT  8       /* we've read at least one correct digit */

static unsigned __int64 strtoxq (
    const char *nptr,
    const char **endptr,
    int ibase,
    int flags
    )
{
    const char *p;
    char c;
    unsigned __int64 number;
    unsigned digval;
    unsigned __int64 maxval;

    p = nptr;            /* p is our scanning pointer */
    number = 0;            /* start with zero */

    c = *p++;            /* read char */
    while ( isspace((int)(unsigned char)c) )
        c = *p++;        /* skip whitespace */

    if (c == '-') {
        flags |= FL_NEG;    /* remember minus sign */
        c = *p++;
    }
    else if (c == '+')
        c = *p++;        /* skip sign */

    if (ibase < 0 || ibase == 1 || ibase > 36) {
        /* bad base! */
        if (endptr)
            /* store beginning of string in endptr */
            *endptr = nptr;
        return 0L;        /* return 0 */
    }
    else if (ibase == 0) {
        /* determine base free-lance, based on first two chars of
           string */
        if (c != '0')
            ibase = 10;
        else if (*p == 'x' || *p == 'X')
            ibase = 16;
        else
            ibase = 8;
    }

    if (ibase == 16) {
        /* we might have 0x in front of number; remove if there */
        if (c == '0' && (*p == 'x' || *p == 'X')) {
            ++p;
            c = *p++;    /* advance past prefix */
        }
    }

    /* if our number exceeds this, we will overflow on multiply */
    maxval = _UI64_MAX / ibase;


    for (;;) {    /* exit in middle of loop */
        /* convert c to value */
        if ( isdigit((int)(unsigned char)c) )
            digval = c - '0';
        else if ( isalpha((int)(unsigned char)c) )
            digval = toupper(c) - 'A' + 10;
        else
            break;
        if (digval >= (unsigned)ibase)
            break;        /* exit loop if bad digit found */

        /* record the fact we have read one digit */
        flags |= FL_READDIGIT;

        /* we now need to compute number = number * base + digval,
           but we need to know if overflow occured.  This requires
           a tricky pre-check. */

        if (number < maxval || (number == maxval &&
        (unsigned __int64)digval <= _UI64_MAX % ibase)) {
            /* we won't overflow, go ahead and multiply */
            number = number * ibase + digval;
        }
        else {
            /* we would have overflowed -- set the overflow flag */
            flags |= FL_OVERFLOW;
        }

        c = *p++;        /* read next digit */
    }

    --p;                /* point to place that stopped scan */

    if (!(flags & FL_READDIGIT)) {
        /* no number there; return 0 and point to beginning of
           string */
        if (endptr)
            /* store beginning of string in endptr later on */
            p = nptr;
        number = 0L;        /* return 0 */
    }
    else if ( (flags & FL_OVERFLOW) ||
              ( !(flags & FL_UNSIGNED) &&
                ( ( (flags & FL_NEG) && (number > -_I64_MIN) ) ||
                  ( !(flags & FL_NEG) && (number > _I64_MAX) ) ) ) )
    {
        /* overflow or signed overflow occurred */
        errno = ERANGE;
        if ( flags & FL_UNSIGNED )
            number = _UI64_MAX;
        else if ( flags & FL_NEG )
            number = _I64_MIN;
        else
            number = _I64_MAX;
    }
    if (endptr != NULL)
        /* store pointer to char that stopped the scan */
        *endptr = p;

    if (flags & FL_NEG)
        /* negate result if there was a neg sign */
        number = (unsigned __int64)(-(__int64)number);

    return number;            /* done. */
}

#ifndef _MSC_VER  // overload in v8
__int64 _cdecl _strtoi64(const char *nptr,char **endptr,int ibase)
{
    return (__int64) strtoxq(nptr, (const char **)endptr, ibase, 0);
}
#endif

#endif

//<SVS>
#if defined(_DEBUG) && defined(_MSC_VER) && !defined(_WIN64)
//</SVS>
// && _MSC_VER < 1300)

#define _UI64_MAX     0xffffffffffffffffui64
#define _I64_MIN      (-9223372036854775807i64 - 1)
#define _I64_MAX      9223372036854775807i64
#define ERANGE        34

#define FL_UNSIGNED   1       /* strtouq called */
#define FL_NEG        2       /* negative sign found */
#define FL_OVERFLOW   4       /* overflow occured */
#define FL_READDIGIT  8       /* we've read at least one correct digit */

#define _wchartodigit(c)    ((c) >= '0' && (c) <= '9' ? (c) - '0' : -1)
#define __ascii_iswalpha(c)     ( ('A' <= (c) && (c) <= 'Z') || ( 'a' <= (c) && (c) <= 'z'))



static unsigned __int64 __cdecl wcstoxq (
        const wchar_t *nptr,
        const wchar_t **endptr,
        int ibase,
        int flags
        )
{
        const wchar_t *p;
        wchar_t c;
        unsigned __int64 number;
        unsigned digval;
        unsigned __int64 maxval;

        p = nptr;                       /* p is our scanning pointer */
        number = 0;                     /* start with zero */

        c = *p++;                       /* read wchar_t */
    while ( iswspace(c) )
                c = *p++;               /* skip whitespace */

        if (c == '-') {
                flags |= FL_NEG;        /* remember minus sign */
                c = *p++;
        }
        else if (c == '+')
                c = *p++;               /* skip sign */

        if (ibase < 0 || ibase == 1 || ibase > 36) {
                /* bad base! */
                if (endptr)
                        /* store beginning of string in endptr */
                        *endptr = nptr;
                return 0L;              /* return 0 */
        }
        else if (ibase == 0) {
                /* determine base free-lance, based on first two chars of
                   string */
                if (_wchartodigit(c) != 0)
                        ibase = 10;
                else if (*p == 'x' || *p == 'X')
                        ibase = 16;
                else
                        ibase = 8;
        }

        if (ibase == 16) {
                /* we might have 0x in front of number; remove if there */
        if (_wchartodigit(c) == 0 && (*p == L'x' || *p == L'X')) {
                        ++p;
                        c = *p++;       /* advance past prefix */
                }
        }

        /* if our number exceeds this, we will overflow on multiply */
        maxval = _UI64_MAX / ibase;


        for (;;) {      /* exit in middle of loop */
                /* convert c to value */
                if ( (digval = _wchartodigit(c)) != -1 )
                        ;
                else if ( __ascii_iswalpha(c) )
                        digval = toupper(c) - 'A' + 10;
                else
                        break;
                if (digval >= (unsigned)ibase)
                        break;          /* exit loop if bad digit found */

                /* record the fact we have read one digit */
                flags |= FL_READDIGIT;

                /* we now need to compute number = number * base + digval,
                   but we need to know if overflow occured.  This requires
                   a tricky pre-check. */

                if (number < maxval || (number == maxval &&
                (unsigned __int64)digval <= _UI64_MAX % ibase)) {
                        /* we won't overflow, go ahead and multiply */
                        number = number * ibase + digval;
                }
                else {
                        /* we would have overflowed -- set the overflow flag */
                        flags |= FL_OVERFLOW;
                }

                c = *p++;               /* read next digit */
        }

        --p;                            /* point to place that stopped scan */

        if (!(flags & FL_READDIGIT)) {
                /* no number there; return 0 and point to beginning of
                   string */
                if (endptr)
                        /* store beginning of string in endptr later on */
                        p = nptr;
                number = 0L;            /* return 0 */
        }
    else if ( (flags & FL_OVERFLOW) ||
              ( !(flags & FL_UNSIGNED) &&
                ( ( (flags & FL_NEG) && (number > -_I64_MIN) ) ||
                  ( !(flags & FL_NEG) && (number > _I64_MAX) ) ) ) )
    {
        /* overflow or signed overflow occurred */
        errno = ERANGE;
        if ( flags & FL_UNSIGNED )
            number = _UI64_MAX;
        else if ( flags & FL_NEG )
            number = (_I64_MIN);
        else
            number = _I64_MAX;
    }

        if (endptr != NULL)
                /* store pointer to wchar_t that stopped the scan */
                *endptr = p;

        if (flags & FL_NEG)
                /* negate result if there was a neg sign */
                number = (unsigned __int64)(-(__int64)number);

        return number;                  /* done. */
}

#ifndef _MSC_VER  // overload in v8
__int64 __cdecl _wcstoi64(const wchar_t *nptr,wchar_t **endptr,int ibase)
{
    return (__int64) wcstoxq(nptr, (const wchar_t **)endptr, ibase, 0);
}

unsigned __int64 __cdecl _wcstoui64 (const wchar_t *nptr,wchar_t **endptr,int ibase)
{
        return wcstoxq(nptr, (const wchar_t **)endptr, ibase, FL_UNSIGNED);
}
#endif

#endif


#if defined(_DEBUG) && defined(_MSC_VER) && (_MSC_VER >= 1300) && !defined(_WIN64)
// && (WINVER < 0x0500)
// ������ � ������� �������� ��������:
// strmix.obj : error LNK2019: unresolved external symbol __ftol2 referenced in function "char * __stdcall FileSizeToStr (char *,unsigned long,unsigned long,int,int)" (?FileSizeToStr@@YGPADPADKKHH@Z)
// ��������: http://q12.org/pipermail/ode/2004-January/010811.html
//VC7 or later, building with pre-VC7 runtime libraries
extern "C" long _ftol( double ); //defined by VC6 C libs
extern "C" long _ftol2( double dblSource ) { return _ftol( dblSource ); }
#endif
