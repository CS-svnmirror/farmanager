/*
syntax.cpp

���������� ������� ��� MacroDrive II

*/

//---------------------------------------------------------------
// If this code works, it was written by Alexander Nazarenko.
// If not, I don't know who wrote it.
//---------------------------------------------------------------
// ������ � "����������" ���������
//---------------------------------------------------------------


#include "headers.hpp"
#pragma hdrstop

#include "plugin.hpp"
#include "macroopcode.hpp"
#include "lang.hpp"
#include "fn.hpp"
#include "syntax.hpp"
#include "tvar.hpp"

#define EOFCH 65536

static int Size = 0;
static unsigned long FARVar, *exprBuff = NULL;
static int IsProcessFunc=0;

static int _macro_nErr = 0;
static int _macro_ErrCode=err_Success;
static wchar_t nameString[1024];
static wchar_t *sSrcString;
static wchar_t *pSrcString = NULL;
static wchar_t *oSrcString = NULL;

static TToken currTok = tNo;
static TVar currVar;

static void expr(void);
static __int64 _cdecl getInt64();

#ifdef _DEBUG
#ifdef SYSLOG_KEYMACRO
static void printKeyValue(DWORD* k, int& i);
#endif
#endif

static const wchar_t *__GetNextWord(const wchar_t *BufPtr,string &strCurKeyText);
static int parseMacroString(DWORD *&CurMacroBuffer, int &CurMacroBufferSize, const wchar_t *BufPtr);
static void keyMacroParseError(int err, const wchar_t *s, const wchar_t *p, const wchar_t *c=NULL);
static void keyMacroParseError(int err, const wchar_t *c = NULL);

//-----------------------------------------------
static string ErrMessage[3];

static void put(unsigned long code)
{
  exprBuff[Size++] = code;
}

static void put64(unsigned __int64 code)
{
  FARINT64 i64;
  i64.i64=code;
  exprBuff[Size++] = i64.Part.HighPart;   //???
  exprBuff[Size++] = i64.Part.LowPart;    //???
}

static void putstr(const wchar_t *s)
{
  _KEYMACRO(CleverSysLog Clev(L"putstr"));
  _KEYMACRO(SysLog(L"s[%p]='%s'", s,s));

  int Length = (int)(StrLength(s)+1)*sizeof(wchar_t);
  // ������ ������ ���� ��������� �� 4
  int nSize = Length/sizeof(DWORD);
  memmove(&exprBuff[Size],s,Length);
  if ( Length == sizeof(wchar_t) || ( Length % sizeof(DWORD)) != 0 ) // ���������� �� sizeof(DWORD) ������.
    nSize++;
  memset(&exprBuff[Size],0,nSize*sizeof(DWORD));
  memmove(&exprBuff[Size],s,Length);
  Size+=nSize;
}

static void keyMacroParseError(int err, const wchar_t *s, const wchar_t *p, const wchar_t *c)
{
  if ( !_macro_nErr++ )
  {
    _macro_ErrCode=err;
    int oPos = 0, ePos = (int)(s-p);
    ErrMessage[0]=ErrMessage[1]=ErrMessage[2]=L"";
    if ( ePos < 0 )
    {
      ErrMessage[0] = UMSG(MMacroPErrExpr_Expected); // TODO: .Format !
      return;
    }

    ErrMessage[0].Format (UMSG(MMacroPErrUnrecognized_keyword+err-1),c);
    if ( ePos > 61 )
    {
      oPos = ePos-50;
      ErrMessage[1] = L"...";
    }
    ErrMessage[1] += p+oPos;

//    if ( ErrMessage[1][61] ) BUGBUG
//      strncpy(&ErrMessage[1][61], "...",sizeof(ErrMessage[1])-62);

    int lPos = ePos-oPos+(oPos ? 3 : 0);

    InsertQuote(ErrMessage[1]);
    ErrMessage[2].Format (L"%*s%c", lPos+1, L"", L'^');
  }
}

static void keyMacroParseError(int err, const wchar_t *c)
{
  keyMacroParseError(err, oSrcString, pSrcString, c);
  //                      ^ s?
}
//-----------------------------------------------

static int getNextChar()
{
  if ( *sSrcString )
  {
    int ch;
    while ( ( ( ch = *(sSrcString++) ) != 0 ) && iswspace(ch) )
      ;
    return ch ? ch : EOFCH;
  }
  return EOFCH;
}

static inline int getChar()
{
  if ( *sSrcString )
  {
    int ch = *(sSrcString++);
    return ( ch ) ? ch : EOFCH;
  }
  return EOFCH;
}

typedef struct __TMacroFunction{
  const wchar_t *Name;             // ��� �������
  int nParam;                   // ���������� ����������
  TMacroOpCode Code;               // ������� �������
} TMacroFunction;

static TMacroFunction macroFunction[]={
  {L"ABS",            1,    MCODE_F_ABS},                 // N=abs(N)
  {L"AKEY",           0,    MCODE_F_AKEY},                // S=akey()
  {L"ASC",            1,    MCODE_F_ASC},                 // N=asc(N)
  {L"CHECKHOTKEY",    1,    MCODE_F_MENU_CHECKHOTKEY},    // N=checkhotkey(S)
  {L"CHR",            1,    MCODE_F_CHR},                 // S=chr(N)
  {L"CLIP",           2,    MCODE_F_CLIP},                // V=clip(N,S)
  {L"DATE",           1,    MCODE_F_DATE},                // S=date(S)
  {L"DLG.GETVALUE",   2,    MCODE_F_DLG_GETVALUE},        // V=Dlg.GetValue(ID,N)
  {L"EDITOR.SET",     2,    MCODE_F_EDITOR_SET},          // N=Editor.Set(N,Var)
  {L"ENV",            1,    MCODE_F_ENVIRON},             // S=env(S)
  {L"EVAL",           1,    MCODE_F_EVAL},                // N=eval(S)
  {L"FATTR",          1,    MCODE_F_FATTR},               // N=fattr(S)
  {L"FEXIST",         1,    MCODE_F_FEXIST},              // N=fexist(S)
  {L"FLOCK",          2,    MCODE_F_FLOCK},               // N=FLock(N,N)
  {L"FSPLIT",         2,    MCODE_F_FSPLIT},              // S=fsplit(S,N)
  {L"GETHOTKEY",      1,    MCODE_F_MENU_GETHOTKEY},      // S=gethotkey(N)
  {L"IIF",            3,    MCODE_F_IIF},                 // V=iif(Condition,V1,V2)
  {L"INDEX",          2,    MCODE_F_INDEX},               // S=index(S1,S2)
  {L"INT",            1,    MCODE_F_INT},                 // N=int(V)
  {L"ITOA",           2,    MCODE_F_ITOA},                // S=itoa(N,radix)
  {L"LCASE",          1,    MCODE_F_LCASE},               // S=lcase(S1)
  {L"LEN",            1,    MCODE_F_LEN},                 // N=len(S)
  {L"MAX",            2,    MCODE_F_MAX},                 // N=max(N1,N2)
  {L"MSAVE",          1,    MCODE_F_MSAVE},               // N=msave(S)
  {L"MSGBOX",         3,    MCODE_F_MSGBOX},              // N=msgbox("Title","Text",flags)
  {L"MIN",            2,    MCODE_F_MIN},                 // N=min(N1,N2)
  {L"PANEL.FATTR",    2,    MCODE_F_PANEL_FATTR},         // N=Panel.FAttr(panelType,fileMask)
  {L"PANEL.FEXIST",   2,    MCODE_F_PANEL_FEXIST},        // N=Panel.FExist(panelType,fileMask)
  {L"PANEL.SETPOS",   2,    MCODE_F_PANEL_SETPOS},        // N=panel.SetPos(panelType,fileName)
  {L"PANEL.SETPOSIDX",2,    MCODE_F_PANEL_SETPOSIDX},     // N=Panel.SetPosIdx(panelType,Idx)
  {L"PANELITEM",      3,    MCODE_F_PANELITEM},           // V=panelitem(Panel,Index,TypeInfo)
  {L"RINDEX",         2,    MCODE_F_RINDEX},              // S=rindex(S1,S2)
  {L"SLEEP",          1,    MCODE_F_SLEEP},               // N=sleep(N)
  {L"STRING",         1,    MCODE_F_STRING},              // S=string(V)
  {L"SUBSTR",         3,    MCODE_F_SUBSTR},              // S=substr(S,N1,N2)
  {L"UCASE",          1,    MCODE_F_UCASE},               // S=ucase(S1)
  {L"WAITKEY",        1,    MCODE_F_WAITKEY},             // S=waitkey(N)
  {L"XLAT",           1,    MCODE_F_XLAT},                // S=xlat(S)
};

static DWORD funcLook(const wchar_t *s, int& nParam)
{
  nParam=0;
  for(int I=0; I < sizeof(macroFunction)/sizeof(macroFunction[0]); ++I)
    //if(!strnicmp(s, macroFunction[I].Name, strlen(macroFunction[I].Name)))
    if(!StrCmpNI(s, macroFunction[I].Name, (int)Max(StrLength(macroFunction[I].Name),StrLength(s))))
    {
      nParam = macroFunction[I].nParam;
      return (DWORD)macroFunction[I].Code;
    }

  return (DWORD)MCODE_F_NOFUNC;
}

static TToken getToken(void);

static void calcFunc(void)
{
  int nParam;
  TMacroOpCode nFunc = (TMacroOpCode)funcLook(nameString, nParam);
  if ( nFunc != MCODE_F_NOFUNC )
  {
    IsProcessFunc++;
    if ( nParam )
    {
      for ( int i = 0 ; i < nParam ; i++ )
      {
        getToken();
        expr();
        if ( currTok != ( (i == nParam-1) ? tRp : tComma ) )
        {
          if ( i == nParam-1 )
            keyMacroParseError(err_Expected, L")");
          else
            keyMacroParseError(err_Expected, L",");
          currTok = tEnd;
        }
       }
    }
    else
    {
      getToken();
      if ( currTok != tRp )
      {
        keyMacroParseError(err_Expected, L")");
        currTok = tEnd;
      }
    }
    put(nFunc);
    IsProcessFunc--;
  }
  else if(currTok == tFunc)
  {
    keyMacroParseError(err_Unrecognized_function, nameString);
  }
}

static void getVarName(int& ch)
{
  wchar_t* p = nameString;
  *p++ = (wchar_t)ch;
  while ( ( ( ch = getChar() ) != EOFCH ) && ( iswalnum(ch) || ( ch == L'_') ) )
    *p++ = (wchar_t)ch;
  *p = 0;
}

static void getFarName(int& ch)
{
  wchar_t* p = nameString;
  *p++ = (wchar_t)ch;
  while ( ( ( ch = getChar() ) != EOFCH ) && ( iswalnum(ch) || ( ch == L'_') || ( ch == L'.') ) )
    *p++ = (wchar_t)ch;
  *p = 0;
}

static wchar_t *putBack(int ch)
{
  if ( ( ch && ( ch != EOFCH ) ) && ( sSrcString > pSrcString ) )
    sSrcString--;
  return sSrcString;
}

static inline int peekChar()
{
  int c;
  putBack(c = getChar());
  return c;
}

static long getLong()
{
  static wchar_t buffer[32];
  wchar_t *p = buffer;
  int ch;
  while ( ( ( ch = getChar() ) != EOFCH ) && (iswxdigit(ch) || ch == L'x') && ( (p-buffer) < 32 ))
    *p++ = (wchar_t)ch;
  *p = 0;
  putBack(ch);
  wchar_t *endptr;
  return wcstol(buffer,&endptr,0);
}


static __int64 _cdecl getInt64()
{
  static wchar_t buffer[128];
  wchar_t *p = buffer;
  int ch;
  while ( ( ( ch = getChar() ) != EOFCH ) && (iswxdigit(ch) || ch == 'x') && ( (p-buffer) < 32 ))
    *p++ = (char)ch;
  *p = 0;
  putBack(ch);
  wchar_t *endptr;
  __int64 __val=_wcstoi64(buffer,&endptr,0);
  return __val;
}

static wchar_t hex2ch(wchar_t b1, wchar_t b2)
{
  if ( b1 >= L'0' && b1 <= L'9' )
    b1 -= L'0';
  else
  {
    b1 &= ~0x20;
    b1 -= (wchar_t)(L'A'-10);
  }
  if ( b2 >= L'0' && b2 <= L'9')
    b2 -= L'0';
  else
  {
    b2 &= ~0x20;
    b2 -= (wchar_t)(L'A'-10);
  }
  return (wchar_t)( ( ( b1 << 4 ) & 0x00F0 ) | ( b2 & 0x000F ) );
}

static TToken getToken(void)
{
  oSrcString = sSrcString;
  int ch = getNextChar();
  switch ( ch )
  {
    case EOFCH:
    case 0:   return currTok = tEnd;
    case L',': return currTok = tComma;
    case L'+': return currTok = tPlus;
    case L'-': return currTok = tMinus;
    case L'*': return currTok = tMul;
    case L'/': return currTok = tDiv;
    case L'(': return currTok = tLp;
    case L')': return currTok = tRp;
    case L'^': return currTok = tBitXor;
    case L'|':
      if ( ( ch = getChar() ) == L'|')
        return currTok = tBoolOr;
      else
      {
        putBack(ch);
        return currTok = tBitOr;
      }
    case L'&':
      if ( ( ch = getChar() ) == L'&')
        return currTok = tBoolAnd;
      else
      {
        putBack(ch);
        return currTok = tBitAnd;
      }
    case L'=':
      if ( ( ch = getChar() ) == L'=')
        return currTok = tEq;
      else
      {
        putBack(ch);
        return currTok = tLet;
      }
    case L'>':
      switch ( ( ch = getChar() ) )
      {
        case L'=': return currTok = tGe;
        case L'>': return currTok = tBitShr;
        default:
          putBack(ch);
          return currTok = tGt;
      }
    case L'<':
      switch ( ch = getChar() )
      {
        case L'=': return currTok = tLe;
        case L'<': return currTok = tBitShl;
        default:
          putBack(ch);
          return currTok = tLt;
      }
    case L'!':
      if((ch = getChar() ) != L'=')
      {
        putBack(ch);
        return currTok = tNot;
      }
      return currTok = tNe;
    case L'\"':
      //-AN----------------------------------------------
      // ������-�� ��� ����� ������ ������ ParsePlainText
      //-AN----------------------------------------------
      currVar = L"";
      while ( ( ( ch = getChar() ) != EOFCH ) && ( ch != L'\"' ) )
      {
        if ( ch == L'\\' )
        {
          switch ( ch = getChar() )
          {
            case L'a' : ch = L'\a'; break;
            case L'b' : ch = L'\b'; break;
            case L'f' : ch = L'\f'; break;
            case L'n' : ch = L'\n'; break;
            case L'r' : ch = L'\r'; break;
            case L't' : ch = L'\t'; break;
            case L'v' : ch = L'\v'; break;
            case L'\'': ch = L'\''; break;
            case L'\"': ch = L'\"'; break;
            case L'\\': ch = L'\\'; break;
            case L'0': case L'1': case L'2': case L'3': case L'4': case L'5': case L'6': case L'7': // octal: \d \dd \ddd
            {
              BYTE n = ch - L'0';
              if ((ch = getChar()) >= L'0' && ch < L'8')
              {
                n = 8 * n + ch - L'0';
                if ((ch = getChar()) >= L'0' && ch < L'8')
                  n = 8 * n + ch - L'0';
                else
                  putBack(ch);
              }
              else
                putBack(ch);
              ch = n;
              break;
            }
            case L'x':
              if ( iswxdigit(ch = getChar()) )
              {
                wchar_t hBuf[3] = { static_cast<wchar_t>(ch), 0, 0 };
                if ( iswxdigit(ch = getChar()) )
                  hBuf[1] = static_cast<wchar_t>(ch);
                else
                {
                  hBuf[1] = hBuf[0];
                  hBuf[0] = L'0';
                  putBack(ch);
                }
                ch = hex2ch(hBuf[0], hBuf[1]);
              }
              else
              {
                keyMacroParseError(err_Bad_Hex_Control_Char);
                return currTok = tEnd;
              }
              break;
            default:
              keyMacroParseError(err_Bad_Control_Char);
              return currTok = tEnd;
          }
        }
        wchar_t p[] = L" ";
        *p = (wchar_t)ch;
        currVar = currVar+TVar(p);
      }
      return currTok = tStr;
    case L'0': case L'1': case L'2': case L'3': case L'4':
    case L'5': case L'6': case L'7': case L'8': case L'9':
      putBack(ch);
      currVar = getInt64();
      return currTok = tInt;
    case L'%':
      ch = getChar();
      if ( (IsAlphaNum(ch) || ch == L'_') || ( ch == L'%'  && (IsAlphaNum(*sSrcString) || *sSrcString == L'_')))
      {
        getVarName(ch);
        putBack(ch);
        return currTok = tVar;
      }
      else
        keyMacroParseError(err_Var_Expected,L""); // BUG nameString
      break;
    default:
      if ( IsAlpha(ch) ) // || ch == L'_' ????
      {
        getFarName(ch);
        if(ch == L' ')
        {
          while(ch == L' ')
            ch = getNextChar();
        }
        if ( ch == L'(' ) //!!!! � ������� ����������? ��!
          return currTok = tFunc;
        else
        {
          putBack(ch);
          for ( int i = 0 ; i < MKeywordsSize ; i++ )
            if ( !StrCmpI(nameString, MKeywords[i].Name) )
            {
              FARVar = MKeywords[i].Value;
              return currTok = tFARVar;
            }
          if(IsProcessFunc || currTok == tFunc || currTok == tLt) // TODO: ��������
            keyMacroParseError(err_Var_Expected,oSrcString,pSrcString,nameString);
          else
          {
            if(KeyNameMacroToKey(nameString) == -1)
              if(KeyNameToKey(nameString) == -1)
              keyMacroParseError(err_Unrecognized_keyword,nameString);
          }
        }
      }
      break;
  }
  return currTok = tEnd;
}

static void prim(void)
{
  switch ( currTok )
  {
    case tEnd:
      break;
    case tFunc:
      calcFunc();
      getToken();
      break;
    case tVar:
      put(MCODE_OP_PUSHVAR);
      putstr(nameString);
      getToken();
      break;
    case tInt:
      put(MCODE_OP_PUSHINT);
      put64(currVar.i());
      getToken();
      break;
    case tFARVar:
      put(FARVar);
      getToken();
      break;
    case tStr:
      put(MCODE_OP_PUSHSTR);
      putstr(currVar.s());
      getToken();
      break;
    case tMinus:
      getToken();
      prim();
      put(MCODE_OP_NEGATE);
      break;
    case tNot:
      getToken();
      prim();
      put(MCODE_OP_NOT);
      break;
    case tLp:
      getToken();
      expr();
      if ( currTok != tRp )
        keyMacroParseError(err_Expected, L")");
      getToken();
      break;
    default:
      keyMacroParseError(err_Expr_Expected);
      break;
  }
}

static void term(void)
{
  prim();
  for ( ; ; )
    switch ( currTok )
    {
      case tMul: getToken(); prim(); put(MCODE_OP_MUL); break;
      case tDiv: getToken(); prim(); put(MCODE_OP_DIV); break;
      default:
        return;
    }
}

static void mathExpr(void)
{
  term();
  for ( ; ; )
    switch ( currTok )
    {
      case tPlus:    getToken(); term(); put(MCODE_OP_ADD);     break;
      case tMinus:   getToken(); term(); put(MCODE_OP_SUB);     break;
      case tBitShl:  getToken(); term(); put(MCODE_OP_BITSHL);  break;
      case tBitShr:  getToken(); term(); put(MCODE_OP_BITSHR);  break;
      default:
        return;
    }
}

static void booleanPrim(void)
{
  mathExpr();
  for ( ; ; )
    switch ( currTok )
    {
      case tLt: getToken(); mathExpr(); put(MCODE_OP_LT); break;
      case tLe: getToken(); mathExpr(); put(MCODE_OP_LE); break;
      case tGt: getToken(); mathExpr(); put(MCODE_OP_GT); break;
      case tGe: getToken(); mathExpr(); put(MCODE_OP_GE); break;
      case tEq: getToken(); mathExpr(); put(MCODE_OP_EQ); break;
      case tNe: getToken(); mathExpr(); put(MCODE_OP_NE); break;
      default:
        return;
    }
}

static void expr(void)
{
  booleanPrim();
  for ( ; ; )
    switch ( currTok )
    {
      case tBoolAnd: getToken(); booleanPrim(); put(MCODE_OP_AND);    break;
      case tBoolOr:  getToken(); booleanPrim(); put(MCODE_OP_OR);     break;
      case tBitAnd:  getToken(); booleanPrim(); put(MCODE_OP_BITAND); break;
      case tBitOr:   getToken(); booleanPrim(); put(MCODE_OP_BITOR);  break;
      case tBitXor:  getToken(); booleanPrim(); put(MCODE_OP_BITXOR);  break;
      default:
        return;
    }
}

static int parseExpr(const wchar_t*& BufPtr, unsigned long *eBuff, wchar_t bound1, wchar_t bound2)
{
  wchar_t tmp[4];
  IsProcessFunc=0;
  _macro_ErrCode = Size = _macro_nErr = 0;
  while ( *BufPtr && iswspace(*BufPtr) )
    BufPtr++;
  if ( bound1 )
  {
    pSrcString = oSrcString = sSrcString = (wchar_t*)BufPtr+1;
    if ( *BufPtr != bound1 )
    {
      tmp[0] = bound1;
      tmp[1] = 0;
      keyMacroParseError(err_Expected, tmp);
      return 0;
    }
  }
  else
    pSrcString = oSrcString = sSrcString = (wchar_t*)BufPtr;
  exprBuff = eBuff;
#if !defined(TEST000)
  getToken();
  if ( bound2 )
    expr();
  else
    prim();
  BufPtr = oSrcString;
  while ( *BufPtr && iswspace(*BufPtr) )
    BufPtr++;
  if ( bound2 )
  {
    if ( ( *BufPtr != bound2 ) || !( !BufPtr[1] || iswspace(BufPtr[1]) ) )
    {
      tmp[0] = bound2;
      tmp[1] = 0;
      keyMacroParseError(err_Expected, tmp);
      return 0;
    }
    BufPtr++;
  }
#else
  if ( getToken() == tEnd )
    keyMacroParseError(err_Expr_Expected);
  else
  {
    if ( bound2 )
      expr();
    else
      prim();
    BufPtr = oSrcString;
    while ( *BufPtr && iswspace(*BufPtr) )
      BufPtr++;
    if ( bound2 )
    {
      if ( ( *BufPtr != bound2 ) || !( !BufPtr[1] || iswspace(BufPtr[1]) ) )
      {
        tmp[0] = bound2;
        tmp[1] = 0;
        keyMacroParseError(err_Expected, tmp);
        return 0;
      }
      BufPtr++;
    }
  }
#endif
  return Size;
}


static const wchar_t *__GetNextWord(const wchar_t *BufPtr,string &strCurKeyText)
{
   // ���������� ������� ���������� �������
   while (IsSpace(*BufPtr) || IsEol(*BufPtr))
   {
     if(IsEol(*BufPtr))
     {
       //TODO!!!
     }
     BufPtr++;
   }

   if (*BufPtr==0)
     return NULL;

   const wchar_t *CurBufPtr=BufPtr;
   wchar_t Chr=*BufPtr, Chr2=BufPtr[1];
   BOOL SpecMacro=Chr==L'$' && Chr2 && !(IsSpace(Chr2) || IsEol(Chr2));

   // ���� ����� ���������� �������� �������
   while (Chr && !(IsSpace(Chr) || IsEol(Chr)))
   {
     if(SpecMacro && (Chr == L'[' || Chr == L'(' || Chr == L'{'))
       break;
     BufPtr++;
     Chr=*BufPtr;
   }
   int Length=(int)(BufPtr-CurBufPtr);

   wchar_t *CurKeyText = strCurKeyText.GetBuffer (Length+1);

   wcsncpy(CurKeyText,CurBufPtr,Length);
   CurKeyText[Length] = 0;

   strCurKeyText.ReleaseBuffer ();

   return BufPtr;
}

// ������ ��������� ������������ � ���� ������
//- AN ----------------------------------------------
//  ������ ��������� ������������ � �������
//  ��������� ����������� � ���� 15.11.2003
//- AN ----------------------------------------------

#ifdef _DEBUG
#ifdef SYSLOG_KEYMACRO
static wchar_t *printfStr(DWORD* k, int& i)
{
  i++;
  wchar_t *s = (wchar_t *)&k[i];
  while ( StrLength((wchar_t*)&k[i]) > 2 )
    i++;
  return s;
}

static void printKeyValue(DWORD* k, int& i)
{
  DWORD Code=k[i];
  string _mcodename=_MCODE_ToName(Code);
  string cmt=L"";

  static struct {
    DWORD c;
    const wchar_t *n;
  } kmf[]={
    {MCODE_F_ABS,              L"N=abs(N)"},
    {MCODE_F_ASC,              L"N=asc(S)"},
    {MCODE_F_CHR,              L"S=chr(N)"},
    {MCODE_F_AKEY,             L"S=akey()"},
    {MCODE_F_CLIP,             L"V=clip(N,S)"},
    {MCODE_F_DATE,             L"S=date(S)"},
    {MCODE_F_DLG_GETVALUE,     L"V=Dlg.GetValue(ID,N)"},
    {MCODE_F_EDITOR_SET,       L"N=Editor.Set(N,Var)"},
    {MCODE_F_ENVIRON,          L"S=env(S)"},
    {MCODE_F_FATTR,            L"N=fattr(S)"},
    {MCODE_F_FEXIST,           L"S=fexist(S)"},
    {MCODE_F_FLOCK,            L"N=FLock(N,N)"},
    {MCODE_F_FSPLIT,           L"S=fsplit(S,N)"},
    {MCODE_F_IIF,              L"V=iif(Condition,V1,V2)"},
    {MCODE_F_INDEX,            L"S=index(S1,S2)"},
    {MCODE_F_INT,              L"N=int(V)"},
    {MCODE_F_ITOA,             L"S=itoa(N,radix)"},
    {MCODE_F_LCASE,            L"S=lcase(S1)"},
    {MCODE_F_LEN,              L"N=len(S)"},
    {MCODE_F_MAX,              L"N=max(N1,N2)"},
    {MCODE_F_MENU_CHECKHOTKEY, L"N=checkhotkey(S)"},
    {MCODE_F_MENU_GETHOTKEY,   L"S=gethotkey()"},
    {MCODE_F_MIN,              L"N=min(N1,N2)"},
    {MCODE_F_MSAVE,            L"N=msave(S)"},
    {MCODE_F_MSGBOX,           L"N=msgbox(sTitle,sText,flags)"},
    {MCODE_F_PANEL_FATTR,      L"N=panel.fattr(panelType,S)"},
    {MCODE_F_PANEL_FEXIST,     L"S=panel.fexist(panelType,S)"},
    {MCODE_F_PANEL_SETPOS,     L"N=panel.SetPos(panelType,fileName)"},
    {MCODE_F_PANEL_SETPOSIDX,  L"N=panel.SetPosIdx(panelType,Index)"},
    {MCODE_F_PANELITEM,        L"V=panelitem(Panel,Index,TypeInfo)"},
    {MCODE_F_EVAL,             L"N=eval(S)"},
    {MCODE_F_RINDEX,           L"S=rindex(S1,S2)"},
    {MCODE_F_SLEEP,            L"N=Sleep(N)"},
    {MCODE_F_STRING,           L"S=string(V)"},
    {MCODE_F_SUBSTR,           L"S=substr(S1,S2,N)"},
    {MCODE_F_UCASE,            L"S=ucase(S1)"},
    {MCODE_F_WAITKEY,          L"S=waitkey(N)"},
    {MCODE_F_XLAT,             L"S=xlat(S)"},
 };

  if(Code >= MCODE_F_NOFUNC && Code <= KEY_MACRO_C_BASE-1)
  {
    for(int J=0; J <= sizeof(kmf)/sizeof(kmf[0]); ++J)
      if(kmf[J].c == Code)
      {
         cmt=kmf[J].n;
         break;
      }
  }

  if(Code == MCODE_OP_KEYS)
  {
    string strTmp;
    SysLog(L"%08X: %08X | MCODE_OP_KEYS", i,MCODE_OP_KEYS);
    ++i;
    while((Code=k[i]) != MCODE_OP_ENDKEYS)
    {
      if ( KeyToText(Code, strTmp) )
        SysLog(L"%08X: %08X | Key: '%s'", i,Code,(const wchar_t*)strTmp);
      else
        SysLog(L"%08X: %08X | ???", i,Code);
      ++i;
    }
    SysLog(L"%08X: %08X | MCODE_OP_ENDKEYS", i,MCODE_OP_ENDKEYS);
    return;
  }

  if(Code >= KEY_MACRO_BASE && Code <= KEY_MACRO_ENDBASE)
  {
    SysLog(L"%08X: %s  %s%s", i,_mcodename,(!cmt.IsEmpty()?L"# ":L""),(!cmt.IsEmpty()?(const wchar_t*)cmt:L""));
  }

  int ii = i;


  if ( !Code )
  {
    SysLog(L"%08X: %08X | <null>", ii,k[i]);
  }
  else if ( Code == MCODE_OP_REP )
  {
    FARINT64 i64;
    i64.Part.HighPart=k[i+1];
    i64.Part.LowPart=k[i+2];
    SysLog(L"%08X: %08X |   %I64d", ii,k[i+1],i64.i64);
    SysLog(L"%08X: %08X |", ii,k[i+2]);
    i+=2;
  }
  else if ( Code == MCODE_OP_PUSHINT )
  {
    FARINT64 i64;
    ++i;
    i64.Part.HighPart=k[i];
    i64.Part.LowPart=k[i+1];
    SysLog(L"%08X: %08X |   %I64d", ++ii,k[i],i64.i64);
    ++i;
    SysLog(L"%08X: %08X |", ++ii,k[i]);
  }
  else if ( Code == MCODE_OP_PUSHSTR || Code == MCODE_OP_PUSHVAR || Code == MCODE_OP_SAVE)
  {
    int iii=i+1;
    const wchar_t *s=printfStr(k, i);
    if(Code == MCODE_OP_PUSHSTR)
      SysLog(L"%08X: %08X |   \"%s\"", iii,k[iii], s);
    else
      SysLog(L"%08X: %08X |   %%%s", iii,k[iii], s);
    for(iii++; iii <= i; ++iii)
      SysLog(L"%08X: %08X |", iii,k[iii]);
  }
  else if ( Code >= MCODE_OP_JMP && Code <= MCODE_OP_JGE)
  {
    ++i;
    SysLog(L"%08X: %08X |   %08X (%s)", i,k[i],k[i],((DWORD)k[i]<(DWORD)i?L"up":L"down"));
  }
/*
  else if ( Code == MCODE_OP_DATE )
  {
    //sprint(ii, L"$date ''");
  }
  else if ( Code == MCODE_OP_PLAINTEXT )
  {
    //sprint(ii, L"$text ''");
  }
*/
  else if(k[i] < KEY_MACRO_BASE || k[i] > KEY_MACRO_ENDBASE)
  {
    int FARFunc = 0;
    for ( int j = 0 ; j < MKeywordsSize ; j++ )
    {
      if ( k[i] == MKeywords[j].Value)
      {
        FARFunc = 1;
        SysLog(L"%08X: %08X | %s", ii,Code,MKeywords[j].Name);
        break;
      }
      else if ( Code == MKeywordsFlags[j].Value)
      {
        FARFunc = 1;
        SysLog(L"%08X: %08X | %s", ii,Code,MKeywordsFlags[j].Name);
        break;
      }
    }
    if ( !FARFunc )
    {
      string strTmp;
      if ( KeyToText(k[i], strTmp) )
        SysLog(L"%08X: %08X | Key: '%s'", ii,Code,(const wchar_t*)strTmp);
      else if(!cmt.IsEmpty())
        SysLog(L"%08X: %08X | ???", ii,Code);
    }
  }
}
#endif
#endif

// ���� ����������� ����������
enum TExecMode
{
  emmMain, emmWhile, emmThen, emmElse, emmRep
};

struct TExecItem
{
  TExecMode state;
  DWORD pos1, pos2;
};

class TExec
{
  private:
    TExecItem stack[MAXEXEXSTACK];
  public:
    int current;
    void init()
    {
      current = 0;
      stack[current].state = emmMain;
      stack[current].pos1 = stack[current].pos2 = 0;
    }
    TExec() { init(); }
    TExecItem& operator()() { return stack[current]; }
    int add(TExecMode, DWORD, DWORD = 0);
    int del();
};

int TExec::add(TExecMode s, DWORD p1, DWORD p2)
{
  if ( ++current < MAXEXEXSTACK )
  {
    stack[current].state = s;
    stack[current].pos1 = p1;
    stack[current].pos2 = p2;
    return TRUE;
  }
  // Stack Overflow
  return FALSE;
};

int TExec::del()
{
  if ( --current < 0 )
  {
    // Stack Underflow ???
    current = 0;
    return FALSE;
  }
  return TRUE;
};

//- AN ----------------------------------------------
//  ���������� ������ BufPtr � ������� CurMacroBuffer
//- AN ----------------------------------------------
int __parseMacroString(DWORD *&CurMacroBuffer, int &CurMacroBufferSize, const wchar_t *BufPtr)
{
  _KEYMACRO(CleverSysLog Clev(L"parseMacroString"));
  _KEYMACRO(SysLog(L"BufPtr[%p]='%s'", BufPtr,BufPtr));
  _macro_nErr = 0;
  if ( BufPtr == NULL || !*BufPtr)
    return FALSE;

  int SizeCurKeyText = (int)(StrLength(BufPtr)*2)*sizeof (wchar_t);

  string strCurrKeyText;
  //- AN ----------------------------------------------
  //  ����� ��� ������� ���������
  //- AN ----------------------------------------------
  DWORD *exprBuff = (DWORD*)xf_malloc(SizeCurKeyText*sizeof(DWORD));
  if ( exprBuff == NULL )
    return FALSE;

  TExec exec;
  wchar_t varName[256];
  DWORD KeyCode, *CurMacro_Buffer = NULL;

  for (;;)
  {
    int Size = 1;
    int SizeVarName = 0;
    const wchar_t *oldBufPtr = BufPtr;

    if ( ( BufPtr = __GetNextWord(BufPtr, strCurrKeyText) ) == NULL )
       break;

    _SVS(SysLog(L"strCurrKeyText=%s",(const wchar_t*)strCurrKeyText));
    //- AN ----------------------------------------------
    //  �������� �� ��������� �������
    //  ������� $Text ������������
    //- AN ----------------------------------------------
    if ( strCurrKeyText.At(0) == L'\"' && strCurrKeyText.At(1) )
    {
      KeyCode = MCODE_OP_PLAINTEXT;
      BufPtr = oldBufPtr;
    }
    else if ( ( KeyCode = KeyNameMacroToKey(strCurrKeyText) ) == (DWORD)-1 && ( KeyCode = KeyNameToKey(strCurrKeyText) ) == (DWORD)-1)
    {
      int ProcError=0;

      if ( strCurrKeyText.At(0) == L'%' &&
           (
             ( IsAlphaNum(strCurrKeyText.At(1)) || strCurrKeyText.At(1) == L'_' ) ||
             (
               strCurrKeyText.At(1) == L'%' &&
               ( IsAlphaNum(strCurrKeyText.At(2)) || strCurrKeyText.At(2)==L'_' )
             )
           )
         )
      {
        BufPtr = oldBufPtr;
        while ( *BufPtr && (IsSpace(*BufPtr) || IsEol(*BufPtr)) )
          BufPtr++;
        memset(varName, 0, sizeof(varName));
        KeyCode = MCODE_OP_SAVE;
        wchar_t* p = varName;
        const wchar_t* s = (const wchar_t*)strCurrKeyText+1;
        if ( *s == L'%' )
          *p++ = *s++;
        wchar_t ch;
        *p++ = *s++;
        while ( ( iswalnum(ch = *s++) || ( ch == L'_') ) )
          *p++ = ch;
        *p = 0;
        int Length = (int)(StrLength(varName)+1)*sizeof(wchar_t);
        // ������ ������ ���� ��������� �� 4
        SizeVarName = Length/sizeof(DWORD);
        if ( Length == sizeof(wchar_t) || ( Length % sizeof(DWORD)) != 0 ) // ���������� �� sizeof(DWORD) ������.
          SizeVarName++;
        _SVS(SysLog(L"BufPtr=%s",BufPtr));
        BufPtr += Length/sizeof(wchar_t);
        _SVS(SysLog(L"BufPtr=%s",BufPtr));
        Size += parseExpr(BufPtr, exprBuff, L'=', L';');
        if(_macro_nErr)
        {
          ProcError++;
        }
      }
      else
      {
        // �������� �������, ����� ������� �������, �� ��������� �� ���������,
        // ��������, ������� MsgBox(), �� ��������� �������
        // ����� SizeVarName=1 � varName=""
        int __nParam;

        wchar_t *lpwszCurrKeyText = strCurrKeyText.GetBuffer();
        wchar_t *Brack=(wchar_t *)wcspbrk(lpwszCurrKeyText,L"( "), Chr=0;
        if(Brack)
        {
          Chr=*Brack;
          *Brack=0;
        }

        if(funcLook(lpwszCurrKeyText, __nParam) != MCODE_F_NOFUNC)
        {
          if(Brack) *Brack=Chr;
          BufPtr = oldBufPtr;
          while ( *BufPtr && (IsSpace(*BufPtr) || IsEol(*BufPtr)) )
            BufPtr++;
          Size += parseExpr(BufPtr, exprBuff, 0, 0);
          //Size--; //???
          if(_macro_nErr)
          {
            ProcError++;
          }
          else
          {
            KeyCode=MCODE_OP_SAVE;
            SizeVarName=1;
            memset(varName, 0, sizeof(varName));
          }
        }
        else
        {
          if(Brack) *Brack=Chr;
          ProcError++;
        }

        strCurrKeyText.ReleaseBuffer();
      }

      if(ProcError)
      {
        if(!_macro_nErr)
          keyMacroParseError(err_Unrecognized_keyword, strCurrKeyText, strCurrKeyText,strCurrKeyText);

        if ( CurMacro_Buffer != NULL )
        {
          xf_free(CurMacro_Buffer);
          CurMacroBuffer = NULL;
        }
        CurMacroBufferSize = 0;
        xf_free(exprBuff);
        return FALSE;
      }

    }
    else if(!(strCurrKeyText.At(0) == L'$' && strCurrKeyText.At(1)))
    {
      Size=3;
      KeyCode=MCODE_OP_KEYS;
    }

    switch ( KeyCode )
    {
      case MCODE_OP_DATE:
        while ( *BufPtr && IsSpace(*BufPtr) )
          BufPtr++;
        if ( *BufPtr == L'\"' && BufPtr[1] )
          Size += parseExpr(BufPtr, exprBuff, 0, 0);
        else // �������������� ���������
        {
          Size += 2;
          exprBuff[0] = MCODE_OP_PUSHSTR;
          exprBuff[1] = 0;
        }
        break;
      case MCODE_OP_PLAINTEXT:
        Size += parseExpr(BufPtr, exprBuff, 0, 0);
        break;

// $Rep (expr) ... $End
// -------------------------------------
//            <expr>
//            MCODE_OP_SAVEREPCOUNT       1
// +--------> MCODE_OP_REP                    p1=*
// |          <counter>                   3
// |          <counter>                   4
// |          MCODE_OP_JZ  ------------+  5   p2=*+2
// |          ...                      |
// +--------- MCODE_OP_JMP             |
//            MCODE_OP_END <-----------+

      case MCODE_OP_REP:
        Size += parseExpr(BufPtr, exprBuff, L'(', L')');
        if ( !exec.add(emmRep, CurMacroBufferSize+Size, CurMacroBufferSize+Size+4) ) //??? 3
        {
          if ( CurMacro_Buffer != NULL )
          {
            xf_free(CurMacro_Buffer);
            CurMacroBuffer = NULL;
          }
          CurMacroBufferSize = 0;
          xf_free(exprBuff);
          return FALSE;
        }
        Size += 5;  // �����������, ������ ����� ������ = 4
        break;

// $If (expr) ... $End
// -------------------------------------
//            <expr>
//            MCODE_OP_JZ  ------------+      p1=*+0
//            ...                      |
// +--------- MCODE_OP_JMP             |
// |          ...          <-----------+
// +--------> MCODE_OP_END

// ���

//            <expr>
//            MCODE_OP_JZ  ------------+      p1=*+0
//            ...                      |
//            MCODE_OP_END <-----------+

      case MCODE_OP_IF:
        Size += parseExpr(BufPtr, exprBuff, L'(', L')');
        if ( !exec.add(emmThen, CurMacroBufferSize+Size) )
        {
          if ( CurMacro_Buffer != NULL )
          {
            xf_free(CurMacro_Buffer);
            CurMacroBuffer = NULL;
          }
          CurMacroBufferSize = 0;
          xf_free(exprBuff);
          return FALSE;
        }
        Size++;
        break;

      case MCODE_OP_ELSE:
        Size++;
        break;

// $While (expr) ... $End
// -------------------------------------
// +--------> <expr>
// |          MCODE_OP_JZ  ------------+
// |          ...                      |
// +--------- MCODE_OP_JMP             |
//            MCODE_OP_END <-----------+

      case MCODE_OP_WHILE:
        Size += parseExpr(BufPtr, exprBuff, L'(', L')');
        if ( !exec.add(emmWhile, CurMacroBufferSize, CurMacroBufferSize+Size) )
        {
          if ( CurMacro_Buffer != NULL )
          {
            xf_free(CurMacro_Buffer);
            CurMacroBuffer = NULL;
          }
          CurMacroBufferSize = 0;
          xf_free(exprBuff);
          return FALSE;
        }
        Size++;
        break;
      case MCODE_OP_END:
        switch ( exec().state )
        {
          case emmRep:
          case emmWhile:
            Size += 2; // ����� ��� �������������� JMP
            break;
        }
        break;
    }
    if(_macro_nErr)
    {
      if ( CurMacro_Buffer != NULL )
      {
        xf_free(CurMacro_Buffer);
        CurMacroBuffer = NULL;
      }
      CurMacroBufferSize = 0;
      xf_free(exprBuff);
      return FALSE;
    }

    if ( BufPtr == NULL ) // ???
      break;
    // ��� ������, ������� ���� ��� � ����� ������������������.
    CurMacro_Buffer = (DWORD *)xf_realloc(CurMacro_Buffer,sizeof(*CurMacro_Buffer)*(CurMacroBufferSize+Size+SizeVarName));
    if ( CurMacro_Buffer == NULL )
    {
      CurMacroBuffer = NULL;
      CurMacroBufferSize = 0;
      xf_free(exprBuff);
      return FALSE;
    }
    switch ( KeyCode )
    {
      case MCODE_OP_DATE:
      case MCODE_OP_PLAINTEXT:
        _SVS(SysLog(L"[%d] Size=%u",__LINE__,Size));
        memcpy(CurMacro_Buffer+CurMacroBufferSize, exprBuff, Size*sizeof(DWORD));
        CurMacro_Buffer[CurMacroBufferSize+Size-1] = KeyCode;
        break;
      case MCODE_OP_SAVE:
        memcpy(CurMacro_Buffer+CurMacroBufferSize, exprBuff, Size*sizeof(DWORD));
        CurMacro_Buffer[CurMacroBufferSize+Size-1] = KeyCode;
        memcpy(CurMacro_Buffer+CurMacroBufferSize+Size, varName, SizeVarName*sizeof(DWORD));
        break;
      case MCODE_OP_IF:
        memcpy(CurMacro_Buffer+CurMacroBufferSize, exprBuff, Size*sizeof(DWORD));
        CurMacro_Buffer[CurMacroBufferSize+Size-2] = MCODE_OP_JZ;
        break;
      case MCODE_OP_REP:
        memcpy(CurMacro_Buffer+CurMacroBufferSize, exprBuff, Size*sizeof(DWORD));
        CurMacro_Buffer[CurMacroBufferSize+Size-6] = MCODE_OP_SAVEREPCOUNT;
        CurMacro_Buffer[CurMacroBufferSize+Size-5] = KeyCode;
        CurMacro_Buffer[CurMacroBufferSize+Size-4] = 0; // Initilize 0
        CurMacro_Buffer[CurMacroBufferSize+Size-3] = 0;
        CurMacro_Buffer[CurMacroBufferSize+Size-2] = MCODE_OP_JZ;
        break;
      case MCODE_OP_WHILE:
        memcpy(CurMacro_Buffer+CurMacroBufferSize, exprBuff, Size*sizeof(DWORD));
        CurMacro_Buffer[CurMacroBufferSize+Size-2] = MCODE_OP_JZ;
        break;
      case MCODE_OP_ELSE:
        if ( exec().state == emmThen )
        {
          exec().state = emmElse;
          CurMacro_Buffer[exec().pos1] = CurMacroBufferSize+2;
          exec().pos1 = CurMacroBufferSize;
          CurMacro_Buffer[CurMacroBufferSize] = 0;
        }
        else // ��� $else � �� ������������ :-/
        {
          keyMacroParseError(err_Not_expected_ELSE, oldBufPtr+1, oldBufPtr); // strCurrKeyText
          if ( CurMacro_Buffer != NULL )
          {
            xf_free(CurMacro_Buffer);
            CurMacroBuffer = NULL;
          }
          CurMacroBufferSize = 0;
          xf_free(exprBuff);
          return FALSE;
        }
        break;
      case MCODE_OP_END:
        switch ( exec().state )
        {
          case emmMain:
            // ��� $end � �� ������������ :-/
            keyMacroParseError(err_Not_expected_END, oldBufPtr+1, oldBufPtr); // strCurrKeyText
            if ( CurMacro_Buffer != NULL )
            {
              xf_free(CurMacro_Buffer);
              CurMacroBuffer = NULL;
            }
            CurMacroBufferSize = 0;
            xf_free(exprBuff);
            return FALSE;
          case emmThen:
            CurMacro_Buffer[exec().pos1-1] = MCODE_OP_JZ;
            CurMacro_Buffer[exec().pos1+0] = CurMacroBufferSize+Size-1;
            CurMacro_Buffer[CurMacroBufferSize+Size-1] = KeyCode;
            break;
          case emmElse:
            CurMacro_Buffer[exec().pos1-0] = MCODE_OP_JMP; //??
            CurMacro_Buffer[exec().pos1+1] = CurMacroBufferSize+Size-1; //??
            CurMacro_Buffer[CurMacroBufferSize+Size-1] = KeyCode;
            break;
          case emmRep:
            CurMacro_Buffer[exec().pos2] = CurMacroBufferSize+Size-1;   //??????
            CurMacro_Buffer[CurMacroBufferSize+Size-3] = MCODE_OP_JMP;
            CurMacro_Buffer[CurMacroBufferSize+Size-2] = exec().pos1;
            CurMacro_Buffer[CurMacroBufferSize+Size-1] = KeyCode;
            break;
          case emmWhile:
            CurMacro_Buffer[exec().pos2] = CurMacroBufferSize+Size-1;
            CurMacro_Buffer[CurMacroBufferSize+Size-3] = MCODE_OP_JMP;
            CurMacro_Buffer[CurMacroBufferSize+Size-2] = exec().pos1;
            CurMacro_Buffer[CurMacroBufferSize+Size-1] = KeyCode;
            break;
        }
        if ( !exec.del() )  // ������-�� ����� ���� �� ������,  �� �������������
        {
          if ( CurMacro_Buffer != NULL )
          {
            xf_free(CurMacro_Buffer);
            CurMacroBuffer = NULL;
          }
          CurMacroBufferSize = 0;
          xf_free(exprBuff);
          return FALSE;
        }
        break;
      case MCODE_OP_KEYS:
      {
        CurMacro_Buffer[CurMacroBufferSize+Size-3]=MCODE_OP_KEYS;
        CurMacro_Buffer[CurMacroBufferSize+Size-2]=KeyNameToKey(strCurrKeyText);
        CurMacro_Buffer[CurMacroBufferSize+Size-1]=MCODE_OP_ENDKEYS;
        break;
      }
      default:
        CurMacro_Buffer[CurMacroBufferSize]=KeyCode;

    } // end switch(KeyCode)
    CurMacroBufferSize += Size+SizeVarName;
  } // END for (;;)
#ifdef _DEBUG
#ifdef SYSLOG_KEYMACRO
  SysLogDump(L"Macro Buffer",0,(LPBYTE)CurMacro_Buffer,CurMacroBufferSize*sizeof(DWORD),NULL);
  SysLog(L"<ByteCode>{");
  if ( CurMacro_Buffer )
  {
    int ii;
    for ( ii = 0 ; ii < CurMacroBufferSize ; ii++ )
      printKeyValue(CurMacro_Buffer, ii);
  }
  else
    SysLog(L"??? is NULL");
  SysLog(L"}</ByteCode>");
#endif
#endif
  if ( CurMacroBufferSize > 1 )
    CurMacroBuffer = CurMacro_Buffer;
  else if ( CurMacro_Buffer )
  {
    CurMacroBuffer = reinterpret_cast<DWORD*>((DWORD_PTR)(*CurMacro_Buffer));
    xf_free(CurMacro_Buffer);
  }
  xf_free(exprBuff);
  if ( exec().state != emmMain )
  {
    keyMacroParseError(err_Unexpected_EOS, strCurrKeyText, strCurrKeyText);
    return FALSE;
  }
  if ( _macro_nErr )
    return FALSE;
  return TRUE;
}

BOOL __getMacroParseError(string *strErrMsg1,string *strErrMsg2,string *strErrMsg3)
{
  if(_macro_nErr)
  {
    if(strErrMsg1)
      *strErrMsg1 = ErrMessage[0];
    if(strErrMsg2)
      *strErrMsg2 = ErrMessage[1];
    if(strErrMsg3)
      *strErrMsg3 = ErrMessage[2];
    return TRUE;
  }
  return FALSE;
}

int  __getMacroErrorCode(int *nErr)
{
  if(nErr)
    *nErr=_macro_nErr;
  return _macro_ErrCode;
}
