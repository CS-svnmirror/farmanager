call ../nightly/base_32.bat

SET Path=%Path%;C:\Program Files\Windows Installer XML v3.5\bin

cl -nologo -EHsc -DNIGHTLY genparams.cpp

call ../nightly/base_64.bat

genparams c:\src\outfinalnew64 x64

nmake -nologo RELEASE=1
