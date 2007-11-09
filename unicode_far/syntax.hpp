/*
syntax.hpp

���������� ������� ��� MacroDrive II

*/

//---------------------------------------------------------------
// If this code works, it was written by Alexander Nazarenko.
// If not, I don't know who wrote it.
//---------------------------------------------------------------

//---------------------------------------------------------------
// ���������� ������ TVar ("�������������" ������� - ������ �����
// � ��������� ��������)
//---------------------------------------------------------------

#ifndef __SYNTAX_H
#define __SYNTAX_H

//---------------------------------------------------------------
// ������ ���������
//---------------------------------------------------------------

#define MAXEXEXSTACK 256

enum TToken
{
  tNo, tEnd,  tLet,
  tVar, tStr, tInt, tFunc, tFARVar,
  tPlus, tMinus, tMul, tDiv, tLp, tRp, tComma,
  tBoolAnd, tBoolOr,
  tBitAnd, tBitOr, tBitXor, tNot, tBitShl, tBitShr,
  tEq, tNe, tLt, tLe, tGt, tGe,
};


struct TMacroKeywords {
  int Type;                    // ���: 0=Area, 1=Flags, 2=Condition
  const wchar_t *Name;                  // ������������
  DWORD Value;         // ��������
  DWORD Reserved;
};

enum errParseCode
{
  err_Success,
  err_Unrecognized_keyword,
  err_Unrecognized_function,
  err_Not_expected_ELSE,
  err_Not_expected_END,
  err_Unexpected_EOS,
  err_Expected,
  err_Bad_Hex_Control_Char,
  err_Bad_Control_Char,
  err_Var_Expected,
  err_Expr_Expected,
};

extern int MKeywordsSize;
extern struct TMacroKeywords MKeywords[];
extern int MKeywordsFlagsSize;
extern struct TMacroKeywords MKeywordsFlags[];

#endif
