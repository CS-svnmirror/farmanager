struct InitDialogItem
{
  int Type;
  int X1, Y1, X2, Y2;
  int Focus;
  DWORD_PTR Selected;
  unsigned int Flags;
  int DefaultButton;
  const TCHAR *Data;
};

struct Options{
  short IgnoreQuotes;   // �������: "������������ ������, ����������� � �������"
  short IgnoreAfter;    // �������: "������������ �� �������"
  short BracketPrior;   // �������: "��������� ������"
  short JumpToPair;     // �������: "������� �� ������ ������ ��� ������� �����"
  short Beep;
  short Reserved[2];
  TCHAR  QuotesType[21]; // ���� �������
  TCHAR  Brackets1[21];  // ��������� ������
  TCHAR  Brackets2[41];  // ������� ������
  TCHAR  Dummy;
} Opt;

enum{
  BrZERO,
  BrOne,
  BrTwo,
  BrColorer,
  BrOneMath,
};

static struct PluginStartupInfo Info;
static TCHAR PluginRootKey[80];

int Config();
