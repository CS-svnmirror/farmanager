@echo --------------------------------------------------------------------
@echo Continue only if you are sure that you have set the correct
@echo build and commited the changes.
@echo This command will tag the trunk under tags/VERSION_bBUILD, if such
@echo tag already exists this command might cause problems.
@echo --------------------------------------------------------------------
@echo If you're not sure press CtrlC.
@echo --------------------------------------------------------------------
@echo --------------------------------------------------------------------
@echo �த������ ⮫쪥 �᫨ �� 㢥७� �� �� ���⠢��� �ࠢ����
@echo ����� ����� � ������⨫� ���������.
@echo �� ������� ������ ⥪�騩 trunk � tags/VERSION_bBUILD, �᫨ ⠪��
@echo ����⪠ 㦥 �������, � �� ����� �ਢ��� � �஡�����.
@echo --------------------------------------------------------------------
@echo �᫨ �� �� 㢥७�, � ������ CtrlC
@echo --------------------------------------------------------------------
@pause
@tools\m4 -P svn_tag_build.m4
