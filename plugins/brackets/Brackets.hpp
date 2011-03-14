struct Options{
  int IgnoreQuotes;   // �������: "������������ ������, ����������� � �������"
  int IgnoreAfter;    // �������: "������������ �� �������"
  int BracketPrior;   // �������: "��������� ������"
  int JumpToPair;     // �������: "������� �� ������ ������ ��� ������� �����"
  int Beep;
  wchar_t  QuotesType[21]; // ���� �������
  wchar_t  Brackets1[21];  // ��������� ������
  wchar_t  Brackets2[41];  // ������� ������
};

enum{
  BrZERO,                // �� ����������
  BrOne,                 // ������ ���������, ������ �� ������
  BrTwo,                 // ������ �������
  BrRight,               // ������ ���������, ������ ������ �� ������
  BrOneMath,             // "������" == "�������"
};
