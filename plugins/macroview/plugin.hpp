#ifndef __PLUGIN_HPP__
#define __PLUGIN_HPP__

/*
  plugin.hpp

  Plugin API for FAR Manager 1.70

  Copyright (c) 1996-2000 Eugene Roshal
  Copyright (c) 2000-<%YEAR%> FAR group
*/
/* Revision: 1.247 06.07.2004 $ */

#ifdef FAR_USE_INTERNALS
/*
��������!
� ���� ����� ������ ��� ��������� ������ � � ���� �����!!!!

Modify:
  06.07.2004 SVS
    + ActlKeyMacro.Param.MacroResult ��� MCMD_CHECKMACRO (��� Macro II)
  24.05.2004 SVS
    + PFLAGS_NUMERICSORT, FCTL_SETNUMERICSORT,FCTL_SETANOTHERNUMERICSORT - ���������� ���������� � NumericSort
  11.05.2004 SVS
    + DN_LISTHOTKEY - ������ � ������
  01.03.2004 SVS
    + ��� ����������� ������������� SetFileApisTo
    ! SETFILEAPISTO_TYPE - ��� �� ��������� � farconst.hpp
  27.02.2004 SVS
    ! ������� ��� DIF_LISTNOMOUSEREACTION - �� ������� ���� ������
    + LMRT_*
    + DIF_LISTNOCLOSE
    + MCMD_CHECKMACRO - ������������
  19.02.2004 SVS
    + DIF_LISTNOMOUSEREACTION
  09.12.2003 SVS
    + ESPT_GETWORDDIV
  05.11.2003 SVS
    + FMENU_CHANGECONSOLETITLE - �������� ��������� ������� ��� �����
  24.10.2003 SVS
    + DI_MEMOEDIT - ���������� ������ �� ��������������� ���������� ��������� ��������������
      (���� ��� ����������� �������������)
  17.10.2003 SVS
    + ����� ��� ����� � ������� ��������� �������� - FDIS_BSDELETEUNCHANGEDTEXT
  13.10.2003 SVS
    ! ESPT_WORDDIV -> ESPT_SETWORDDIV (NotInternal)
  10.10.2003 SVS
    + ESPT_WORDDIV (Internal)
  04.10.2003 SVS
    + ����� ���� KSFLAGS_NOSENDKEYSTOPLUGINS - �� ���������� ������� �����������
      �������� (��������, �������������� ������� ProcessEditorInput)
    * ��������� ��������� ActlKeyMacro: �������� ���� Param.PlainText -
      ��������� �� ������, ���������� �����������������������.
  08.09.2003 SVS
    + ����� ������� ��� ACTL_KEYMACRO: MCMD_POSTMACROSTRING - ��������� ������
      � ���� plain-text.
    ! � ����� � ����, ��������� ��������� ActlKeyMacro - �������� ����
      ActlKeyMacro.Param.PlainText
  14.07.2003 SVS
    ! ������� ������������ ������������
  15.06.2003 SVS
    + DN_DRAWDIALOGDONE - �������� ����� ��������� �������
    + ACTL_GETDIALOGSETTINGS
    ! FIS_PERSISTENTBLOCKSINEDITCONTROLS -> FDIS_PERSISTENTBLOCKSINEDITCONTROLS
    ! FIS_HISTORYINDIALOGEDITCONTROLS    -> FDIS_HISTORYINDIALOGEDITCONTROLS
    ! FIS_AUTOCOMPLETEININPUTLINES       -> FDIS_AUTOCOMPLETEININPUTLINES
  14.06.2003 SVS
    ! FRS_SCANJUNCTION -> FRS_SCANSYMLINK
  13.06.2003 SVS
    ! ��� FRS_SCANJUNCTION ������ ������� ����!
    + FSS_SCANSYMLINK (����� ��� ����������� ����� :-))
  30.05.2003 SVS
    + ACTL_GETPLUGINMAXREADDATA
    ! ACTL_GETWCHARMODE ��������
    + FRS_SCANJUNCTION
    + FFPOL_MAINMENUDIALOGS
    ! DM_GETSELECTION/DM_SETSELECTION ��������
  27.05.2003 SVS
    + ��������� FARMACRO_KEY_EVENT ���������� ��� �������� ���������
      ��������� INPUT_RECORD, ������� ���������� � ProcessEditorInput
      �� ����� ���������� ������������. ������� Event ��������
      ��������� KEY_EVENT_RECORD � ����������� � ��������� � ����������.
  16.04.2003 SVS
    + DM_GETSELECTION � DM_SETSELECTION - ���� ��� ����������� ������
  17.03.2003 SVS
    + ACTL_GETPOLICIES + FFPOL_* - ���� ��� ����������� ������
  07.01.2003 SVS
    + XLAT_USEKEYBLAYOUTNAME - ���� ��� ���������� �����
  23.12.2002 SVS
    + FARINT64 (���� ��� ����������� ������)
    ! �����, ��� � ��� __int64 ������ ��� ����� FARINT64 �� ��������� ����������
  17.12.2002 SVS
    ! �������� ��������� ViewerSelect, ViewerSetPosition � ViewerInfo
      � ����� � ������ � ����� Viewer64
  27.10.2002 DJ
    ! ����������� FarListColors.ColorItem � ColorCount (����� ���� �������,
      ��� � ����)
    ! ����������� FARColor � FarSetColors (��� ������������ � ���������� �,
      ����� ��, ����� ���� �������, ��� � ����)
  22.10.2002 SVS
    ! ������� CharTableSet.RFCCharset, �� �������������� - ����� ����� �� ������ ���
      ��� ������� ;-)
  30.09.2002 SVS
    + struct FarListColors - �������� �������� ����� ������
  25.09.2002 SVS
    ! ����������� ACTL_SETARRAYCOLOR � ��� � ���.
  23.09.2002 SVS
    + ACTL_SETARRAYCOLOR, FARCOLORFLAGS, FARColor ���� ��� ����������� ������
      (����, ����, �������� �� ���!)
  20.09.2002 SVS
    + ����� FDLG_NODRAWSHADOW � FDLG_NODRAWPANEL
  27.08.2002 SVS
    ! ������� EditorInfo.WindowPos
    + "����� �� ������� �� � DM_SETCHECK ��� ���� ������ ����: BSTATE_TOGGLE"
  19.08.2002 SVS
    + ECTL_DELETEBLOCK - ������� ���� � ���������. ������� ������ TRUE
      � ������ �������� �������� ����� � FALSE, ���� �������� ������������
      (������������ ����� Ctrl-L) ��� ��� ����������� �����.
  21.06.2002 SVS
    + ACTL_GETWCHARMODE ��� FAR_USE_INTERNALS
      "������� ��� ������ � ���� � ������� W-������� ��� ���?"
  14.06.2002 IS
    + VF_DELETEONLYFILEONCLOSE,  EF_DELETEONLYFILEONCLOSE
  10.06.2002 SVS
    + DIF_EDITPATH (FIB_EDITPATH) - ��� ���������� �����
  04.06.2002 SVS
    + DIF_CENTERTEXT
    + DIF_NOTCVTUSERCONTROL
  30.05.2002 SVS
    + FLINK_DONOTUPDATEPANEL
  22.05.2002 SKV
    + ?F_IMMEDIATERETURN
  13.05.2002 VVM
    + EditorInfo.WindowPos - ����� ���� ���������. ����� �������������� � ACTL_*WINDOW*
  11.05.2002 SVS
    ! ������ LIF_UPDATEKEEPUSERDATA �� ��������������� LIF_DELETEUSERDATA,
      ������ ���� ���� ��������� (�������������� �������� ������ -
      ��� ������� ����� ������ ������� ����������� ������ ������ � ������,
      ���� ��� �� ���� ���������)
  10.05.2002 SVS
    + FCTL_CHECKPANELSEXIST - ������ ��������?
  29.04.2002 SVS
    ! WTYPE_COMBOBOX -> for internal
  28.04.2002 KM
    + WTYPE_COMBOBOX
  28.04.2002 IS
    ! ��������� const (SetFindList)
  27.04.2002 SVS
    + MAXSIZE_SHORTCUTDATA ��� "���������������" OpenPluginInfo.ShortcutData
  25.04.2002 SVS
    - BugZ#479 - struct FAR_FIND_DATA and C
  25.04.2002 IS
    ! ��������� const (OpenPluginInfo)
  12.04.2002 SVS
    + FCTL_GET[ANOTHER]PANELSHORTINFO
  08.04.2002 SVS
    + OPM_QUICKVIEW
    ! ������� ������� ;-)
  04.04.2002 SVS
    ! ECTL_TURNOFFMARKINGBLOK -> ECTL_TURNOFFMARKINGBLOCK
  04.04.2002 IS
    + ECTL_TURNOFFMARKINGBLOK
  04.04.2002 SVS
    + DN_ACTIVATEAPP
  25.03.2002 SVS
    ! CONSOLE_* -> FAR_CONSOLE_* (��� � ����������� �� ��������� :-(()
  23.03.2002 IS
    + ESPT_LOCKMODE
  13.02.2002 SVS
    + FIB_NOAMPERSAND
  27.02.2002 SVS
    ! LIFIND_NOPATTERN -> LIFIND_EXACTMATCH
    + LIF_UPDATEKEEPUSERDATA - "�� ������� �������� ��� ����������"
    ! ��������� ���������� ����������� ����� :-))
  23.02.2002 DJ
    ! �������� ��������� LINFO_ALWAYSSCROLLBAR ��� �� ������� ��������
      ���������� (��� ����������� ��� _������_ ������)
  21.02.2002 SVS
    + EJECT_READY - ������ ��� ���������� �����...
  13.02.2002 SVS
    + MIF_USETEXTPTR - ��� ����� FarMenuItemEx.Text.TextPtr
    ! 130 -> 128 - ��� ��� � �����...
  11.02.2002 SVS
    ! ��������� FarListItem, FarListUpdate, FarListInsert, FarMenuItemEx
      - ������ FarMenuItemEx.Text = 130 + AccelKey (� FAR �������)
      - � FarListUpdate � FarListInsert ������ ���������� - �������
      - � FarListItem - ������ � �������
    ! FarDialogItem.Data.ListPos -> FarDialogItem.Param.ListPos
  11.02.2002 SVS
    + DM_LISTGETDATASIZE
  06.02.2002 DJ
    ! _FAR_USE_FARFINDDATA
  30.01.2002 DJ
    ! _FAR_NO_NAMELESS_UNIONS
  30.01.2002 DJ
    + ACTL_GETDESCSETTINGS, FarDescriptionSettings
    ! DM_SETLISTMOUSEREACTION -> DM_LISTSETMOUSEREACTION
  21.01.2002 SVS
    + DM_GETCURSORSIZE, DM_SETCURSORSIZE
  14.01.2002 SVS
    ! "���������" - ��������� ������������ ����������.
  10.01.2002 SVS
    + EEC_* - ���� ��������� ���������
    + EF_NEWIFOPEN - �������������� ����.
  08.01.2002 IS
    ! ������� ������� SVS 28.12.2001 ���� �� �����.
  08.01.2002 SVS
    + DM_SETLISTMOUSEREACTION
  28.12.2001 SVS
    ! ����� (��������) ��������� �-�� Stanislav V. Mekhanoshin ��� ������ C
      (������ � ������ ��������� �������� - ��� ��������� union)
      ��������� FarDialogItem ��� �������� ���� �� ������!!!
      ���� ��� (������� ���� �������� ;-) ���������� �� ������ ��
      �������������� ���.�������� � ���� �����������, �� ����� ������� ����
      ���������.
    ! � ������ IS: #if sizeof(PluginPanelItem) != 366
  26.12.2001 SVS
    + EF_USEEXISTING, EF_BREAKIFOPEN - ��������� ��� �������� ���������
      ��������, � �������� ��������� ��� ��?
  20.12.2001 IS
    ! ��������� �������������� �� ������ ��������� WTYPE_*
  12.12.2001 SVS
    ! BugZ#173. ��� �������� ������������ �������� � FARSTDPOINTTONAME
  12.12.2001 DJ
    ! ��� DM_SETTEXTLENGTH ���� ����� ��������� �������������
  ! ������ ������������ PFLAGS_* � FPS_*
  + PFLAGS_REALNAMES
  10.12.2001 SVS
    ! DM_SETREDRAW=DM_REDRAW, DM_SETTEXTLENGTH -> DM_SETMAXTEXTLENGTH,
      DM_LISTGET -> DM_LISTGETITEM
    + struct FarListGetItem ��� DM_LISTGETITEM
  07.12.2001 IS
    + FIB_CHECKBOX - �������� ���������������� ���-���� � InputBox.
      ������ ��� ���������� ���� ����!
  03.12.2001 SVS
    ! ������ ������� �������� ���� DM_* (�� ������ ������� DM_LISTSET �
      �������� ������)
  01.12.2001 KM
    + DM_LISTSET - ����� ���������, ���������� �� DM_LISTADD ���, ���
      ���� � ������ ���� ������, �� ������� ������ ��, �.�. "������"
      ���������.
  28.11.2001 SVS
    ! DM_EDITCLEARFLAG ������� �� ����� ���������� DM_EDITUNCHANGEDFLAG
    + FIS_SHOWCOPYINGTIMEINFO
  28.11.2001 SVS
    + DM_EDITCLEARFLAG
  24.11.2001 IS
    + ACTL_GETSYSTEMSETTINGS,  ACTL_GETPANELSETTINGS,
      ACTL_GETINTERFACESETTINGS, ACTL_GETCONFIRMATIONS,
      FarSystemSettings, FarPanelSettings,  FarInterfaceSettings,
      FarConfirmationsSettings
  22.11.2001 SVS
    ! ����� ���������� FMSG_COLOURS "��������"
  21.11.2001 SVS
    + DIF_AUTOMATION, DM_GETITEMDATA, DM_SETITEMDATA
  19.11.2001 SVS
    ! FARMANAGERVERSION � ���� MAKEFARVERSION (� ������ JouriM)
  14.11.2001 SVS
    ! FarMenuItemEx.Reserved -> FarMenuItemEx.UserData
  12.11.2001 SVS
    - ����� ������ const � FarDialogItem.ListItems??? ;-(((
  08.11.2001 SVS
    ! FarMenuItemEx � FarListItem ������ ������ 128 + DWORD Reserved
  06.11.2001 SVS
    ! DM_LIST[G|S]ETTITLE -> DM_LIST[G|S]ETTITLES
    ! LINFO_REVERSIHLIGHT -> LINFO_REVERSEHIGHLIGHT
  05.11.2001 SVS
    ! ESPT_SETTABLE -> ESPT_CHARTABLE: ��� ��������� ESPT_* ����
      �������������, �� SET  �  ��  ��������  ����.
  02.11.2001 SVS
    ! ECTL_GETBOOKMARK, EditorBookMark -> ECTL_GETBOOKMARKS, EditorBookMarks
    ! DM_SETNOTIFYMOUSEEVENT -> DM_SETMOUSEEVENTNOTIFY
    ! FCTL_GETCMDLINESELECTION -> FCTL_GETCMDLINESELECTEDTEXT
    + FCTL_GETCMDLINESELECTION - �������� ������� ���������!
  30.10.2001 SVS
    ! FarListUpdate.Items -> FarListUpdate.Item
    ! WTYPE_VIRTUAL - ������� �������������
  29.10.2001 IS
    + ESPT_SAVEFILEPOSITION
  23.10.2001 SVS
    ! FarListTitle -> FarListTitles
  19.10.2001 SVS
    + DIF_SEPARATOR2 - ������� ���������
  17.10.2001 SVS
    + LINFO_* - ���... �������� �� ��� ������ :-(
    ! FARSTDMKLINK - const ���������
  10.10.2001 SVS
    - FAR_USE_INTERNALS ������� �����������!!!!!!!!
  10.10.2001 IS
    + EF_DELETEONCLOSE
    ! ��������� const
  10.10.2001 SVS
    + EditorInfo.CurState - ��������� ��������� (�������������� �����)
    ! EditorGetString.StringText � StringEOL ��� �������� ����� ���� const
  07.10.2001 SVS 1.148
    + �� �������� ���������� - �������� "|| defined(__WATCOMC__)"
  01.10.2001 SVS 1.147
    - GetRepasePointInfo -> GetRepa_R_sePointInfo
  26.09.2001 SVS 1.146
    ! AddEndSlash ����� ������������ ��� BOOL
  24.09.2001 SVS 1.145
    + FSF.GetRepasePointInfo
  20.09.2001 SVS 1.144
    ! � ������� FSF.FarInputRecordToKey �������� ����� �������� "const"!
  15.09.2001 tran 1.143
    + VE_READ, VE_CLOSE
  15.09.2001 tran 1.142
    + ACTL_GETFARHWND
  12.09.2001 SVS
    + FSF.ConvertNameToReal
  09.09.2001 IS
    + VF_DISABLEHISTORY, EF_DISABLEHISTORY
  31.08.2001 IS
    ! ��� ���� ��������� CharTableSet ����� TableName ������ unsigned char.
  17.08.2001 VVM
    + PluginPanelItem.CRC32
  15.08.2001 SVS
    + DN_MOUSE, DM_SETNOTIFYMOUSEEVENT
  13.08.2001 SKV
    + FCTL_GETCMDLINESELECTION, FCTL_SETCMDLINESELECTION, struct CmdLineSelect.
  08.08.2001 SVS
    + DM_GETITEMPOSITION
  07.08.2001 IS
    + ESPT_SETTABLE
    ! FARAPICHARTABLE - ������ �������� ������ �� const, ������ ��� �� �����
      ���������� � FarCharTable.
  07.08.2001 SVS
    + DN_RESIZECONSOLE
  01.08.2001 SVS
    ! ����� FMENU_CUSTOMNAME, FDLG_CUSTOMNAME, FMSG_CUSTOMNAME � ����
  31.07.2001 IS
    + ����� FMENU_CUSTOMNAME, FDLG_CUSTOMNAME, FMSG_CUSTOMNAME
  31.07.2001 SVS
    + ������� ��� FAR_USE_INTERNALS. ������� �� �������! ��� ��� �������,
      ������� ����� �������� ����������� ���� ��� ������������. ���� �����
      ��������� ��� ���� �� �������� ������� - ��������� ��� � ����� "������":
      1 # ifdef FAR_USE_INTERNALS
      2   ��, ��� ������ ���� ������
      3 # else // ELSE FAR_USE_INTERNALS
      4   ������!
      5 # endif // END FAR_USE_INTERNALS
  31.07.2001 IS
    + ��������� const (FARAPIGETMSG)
  27.07.2001 SVS
    + DM_ALLKEYMODE - ��� ���� MacroBrowse (���� ������ ��� ���� :-)
  16.07.2001 SVS
    + FMENU_USEEXT & MENUITEMFLAGS & FarMenuItemEx
    + DM_SETHISTORY - ���������� �������� ������� � DI_EDIT & DI_FIXEDIT
  11.07.2001 OT
    + ����� "�����������" ��������� ������� - DM_KILLSAVESCREEN
  30.06.2001 KM
    ! �������� ���������: LIFIND_NOPATTER -> LIFIND_NOPATTERN
    + ����� ��������� FarListPos.
  29.06.2001 SVS
    ! ��������� FarListFind.
    + LIFIND_NOPATTER - ������ (��� ����� �������� ����) ������������ ���
      ������ � ������
  26.06.2001 SKV
    + ACTL_COMMIT
  26.06.2001 SVS
    ! ��������� DM_GETDROPDOWNOPENED � DM_SETDROPDOWNOPENED � "�������"
      ����� � ����� ��� � ������� :-)
  25.06.2001 IS
    ! ��������� const, ����� ���� ��� ����� ������ ������� �� "������������"
     plugin.hpp
  23.06.2001 KM
    + DM_GETDROPDOWNOPENED - ����������, ������ �� � ������� ��������� ��� �������.
    + DM_SETDROPDOWNOPENED - ������� ��� ������� ����������� ���� ��������� ��� �������.
  21.06.2001 SVS
    ! ACTL_POSTSEQUENCEKEY  -> ACTL_POSTKEYSEQUENCE - (� ����� ������ eng)
    ! SKFLAGS_DISABLEOUTPUT -> KSFLAGS_DISABLEOUTPUT
    ! SequenceKey           -> KeySequence
  20.06.2001 SVS
    ! ACTL_PROCESSSEQUENCEKEY -> ACTL_POSTSEQUENCEKEY
    ! SKEY_NOTMACROS -> SKFLAGS_DISABLEOUTPUT
  19.06.2001 SVS
    + DN_DRAGGED
  14.06.2001 SVS
    + ���������� � ACTL_*WINDOW* - WTYPE_* - ���� ����
      2AT: ���� ���-�� �� ��� - �������.
  06.06.2001 SVS
    + EditorBookMark, ECTL_GETBOOKMARK
    + EditorInfo.BookMarkCount - ���� �� �������� ��������� ��� ECTL_GETBOOKMARK.
  05.06.2001 tran
    + ACTL_GETWINDOWCOUNT,ACTL_GETWINDOWINFO,ACTL_SETCURRENTWINDOW
    + struct WindowInfo
  04.06.2001 SVS
    ! ������� LIF_PTRDATA - ����� ���� ��� ����� :-)
    ! �������������� ���������� ��������� FarListItem
  03.06.2001 KM
    + ��� ����� ���������:
      DM_LISTSETTITLE
      DM_LISTGETTITLE
      ��� ���������/��������� ���������� � DI_LISTBOX.
    + �������� ���� DIF_LISTAUTOHIGHLIGHT.
  03.06.2001 SVS
    ! ��������� ��������� ��������� FarListItemData (�� 16 ���� :-)
    + ���� �������� ��� �����
  03.06.2001 SVS
    + DM_LISTGETDATA, DM_LISTSETDATA, FarListItemData
  30.05.2001 SVS
    + MKLINKOP, FARSTDMKLINK
  29.05.2001 tran
    + ������ - MAKEFARVERSION
  21.05.2001 DJ
    + FDLG_NONMODAL
  21.05.2001 SVS
    + DM_RESIZEDIALOG
    + DM_SETITEMPOSITION
  18.05.2001 SVS
    + DM_LISTINSERT, DM_LISTINFO, DM_LISTFINDSTRING
    + DM_GETCHECK, DM_SETCHECK, DM_SET3STATE
    + BSTATE_*
    + ��������� FarListInsert, FarListInfo, FarListFind
  17.05.2001 SVS
    + DM_LISTUPDATE
    + FMENU_SHOWNOBOX (��� �� �����������, ��� ��� ����������� �������������!)
    + ��������� FarListUpdate ��� DM_LISTUPDATE, �� ���� �� �� ����� ��� �
      FarList - �.�. ��� ������� ��������� :-)
  15.05.2001 KM
    ! ����� ���� DIF_LISTHIGHLIGHT, ��� ��� ��� �������
      ��� �������� DIF_LISTNOAMPERSAND, ������ ��������.
  14.05.2001 SVS
    ! FDLG_SMALLDILAOG -> FDLG_SMALLDIALOG
  13.05.2001 SVS
    + DIF_LISTWRAPMODE, DIF_LISTHIGHLIGHT
    + DM_LISTADDSTR
  12.05.2001 DJ
    + VF_ENABLE_F6, EF_ENABLE_F6
  08.05.2001 SVS
    + FDLG_* - ����� ��� DialogEx
  07.05.2001 SVS
    + DM_LISTADD, DM_LISTDELETE, DM_LISTGET, DM_LISTSORT, DM_LISTGETCURPOS,
      DM_LISTSETCURPOS
    + DIF_LISTNOBOX - ����� ��� DI_LISTBOX �� ��������
    + struct FarListDelete
    + ������� DlgList_*, DlgItem_*, DlgEdit_*, Dlg_* - ���� windowsx.h ;-)
      ��� �������� - ������� � ��������� ��������� :-)))
  04.05.2001 SVS
    ! ������� �� ����� ���� �� DI_LISTBOX ;-) - ����� ���� FarDialogItem.ListPos
  24.04.2001 SVS
    + PanelInfo.Flags, ����� PANELINFOFLAGS.
  22.04.2001 SVS
    + EJECT_LOAD_MEDIA - �������� ������ � NT/2000
  12.04.2001 SVS
    + DM_ADDHISTORY - �������� ������ � �������
    + DIF_MANUALADDHISTORY - ��������� � ������� ������ "�������"
  03.04.2001 IS
    + ESPT_AUTOINDENT, ESPT_CURSORBEYONDEOL, ESPT_CHARCODEBASE
  26.03.2001 SVS
    + FHELP_USECONTENTS - ���� �� ������ ������� �����, �� ���������� "Contents"
  24.03.2001 tran
    + qsortex
  21.03.2001 VVM
    + ���� EF_CREATENEW ��� ��������� - ������� ����� ���� (������ SHIFT+F4)
  20.03.2001 tran 1.89
    + FarRecursiveSearch - �������� void *param
  19.03.2001 SVS
    ! DN_CLOSE=DM_CLOSE, DN_KEY=DM_KEY - ��� �����������. :-)
  16.02.2001 IS
    + ��������� �������� ������������ ������������ �� ������ ����������
      ������� PluginPanelItem - �� ������ ���� ������ 366. ���� ��� �� ���, ��
      ���� ���������� STRICT, �� �������������� ������ �����������, ����� -
      ����� ����� warning
  16.02.2001 IS
    + ������� ECTL_SETPARAM - �������� ����� ��������� ���������
    + EDITOR_SETPARAMETER_TYPES - ��� ���������
    + ��������� EditorSetParameter - ���������� � ���� ��������� � ���
      ���������
  13.02.2001 SVS
    ! � ����� � ��������� DIF_VAREDIT ��� DI_COMBOBOX �������� ���������
      FarListItem � �������� ���� LIF_PTRDATA
    ! �������� �������� ����� LIF_DISABLE
    ! �������� ����� ����� ��������� FarDialogItemData - ��� "���������"
      �� ������.
  11.02.2001 SVS
    ! FarDialogItem - ���������, �������� Ptr
    + DIF_VAREDIT - ����, ����������� �� ��, ��� ����� ��������������
      FarDialogItem.Ptr ������ FarDialogItem.Data
  11.02.2001 SVS
    ! ��������� � LISTITEMFLAGS - ����� ��������� � ������� �����
  28.01.2001 SVS
    ! SequenceKey.Sequence ��! "��������" VK_* - ������ KEY_*
    + FMSG_ALLINONE - � �������� Items ���������� ��������� ��
      ������, � ������� ����������� ����� - ������ '\n'
    + FMSG_MB_* - ������������� �������� ������ (� Items ����� �� ���������)
  25.01.2001 SVS
    ! ��� SequenceKey.Sequence ������� �� DWORD
    + SKEY_VK_KEYS - � SequenceKey.Sequence "��������" VK_* ������ KEY_*
  23.01.2001 SVS
    + SKEY_NOTMACROS - �� ������������ ��������� ������� � SequenceKey
    + ViewerInfo.LeftPos � ViewerInfo.Reserved3;
  21.01.2001 SVS
    + struct SequenceKey
    + ACTL_PROCESSSEQUENCEKEY
  21.01.2001 IS
    ! ��� ����������� � ���������� ������� ���� ��������:
      VCTL_SETPOS -> VCTL_SETPOSITION
      AnsiText -> AnsiMode
  19.01.2001 SVS
    ! ������������ � �������� VIEWER_CONTROL_COMMANDS
    + ��������� ��������� ��� Viewer API: ViewerSelect, ViewerSetPosition �
      ������������ VIEWER_SETPOS_FLAGS
  03.01.2001 SVS
    + DIF_HIDDEN - ������� �� �����
    + DM_SHOWITEM ��������/������ �������
  25.12.2000 SVS
    ! ACTL_KEYMACRO ������������ ������ 2 �������: MCMD_LOADALL, MCMD_SAVEALL
  23.12.2000 SVS
    + MCMD_PLAYSTRING - "���������" ������.
    + MACRO_* - ������� �������� ��������
    ! ActlKeyMacro - ��������� ����������� ��������� ��� ������� MCMD_PLAYSTRING
    + MFLAGS_ - ����� �������
  21.12.2000 SVS
    + ACTL_KEYMACRO
    + ��������� ActlKeyMacro (� ������������������ ������ :-)
    + MacroCommand: MCMD_LOADALL, MCMD_SAVEALL (�� ���� ���� �����������,
      ��������� ����� ������)
  21.12.2000 SVS
    + DM_GETTEXTPTR, DM_SETTEXTPTR
  18.12.2000 SVS
    + FHELP_NOSHOWERROR
  14.12.2000 SVS
    + ACTL_EJECTMEDIA & struct ActlEjectMedia & EJECT_NO_MESSAGE
  08.12.2000 SVS 1.70
    ! ������������ ����� ������� ��������� - 1.70 ;-) - ����������.
      ��� DM_SETTEXT, DM_GETTEXT � Param2 ���������� ���������
      FarDialogItemData.
  07.12.2000 SVS
    ! �������� ��������� FARMANAGERVERSION. ��������� ������� �
      DIFF.DOC\00300.FAR_VERSION.txt
  04.12.2000 SVS
    + DIF_3STATE - 3-� ��������� CheckBox
    + ACTL_GETCOLOR - �������� ������������ ����
    + ACTL_GETARRAYCOLOR - �������� ���� ������ ������
  04.11.2000 SVS
    + XLAT_SWITCHKEYBBEEP - ������ �������� ������ ��� ������������
      ����������
  02.11.2000 OT
    ! �������� �������� �� ����� ������, ����������� ��� ��� �����.
  26.10.2000 SVS
    ! DM_SETEDITPOS/DM_GETEDITPOS -> DM_SETCURSORPOS/DM_GETCURSORPOS
  25.10.2000 IS
    + ������� ��� ��������� � MkTemp � Template �� Prefix
  23.10.2000 SVS
    + DM_SETEDITPOS, DM_GETEDITPOS -
      ���������������� ������� � ������� ��������������.
  20.10.2000 SVS
    ! ProcessName: Flags ������ ���� DWORD, � �� int
  20.10.2000 SVS
    + DM_GETFOCUS - �������� ID �������� �������� ����� �����
  09.10.2000 IS
    + ����� ��� ProcessName (PN_*)
    + ��������� � FARSTANDARDFUNCTIONS �� ProcessName;
  27.09.2000 SVS
    + VCTL_QUIT      - ������� ������
    + VCTL_GETINFO   - ��������� ���������� � Viewer
    + VCTL_SETKEYBAR - ������� ��������� KeyBar Labels �� �������
  27.09.2000 skv
    + DeleteBuffer
  26.09.2000 SVS
    ! FARSTDKEYTOTEXT -> FARSTDKEYTOKEYNAME
  24.09.2000 SVS
    ! ������ ����� �� ������������ - ������ ������ � ���� ����� (Modify)!!!
    ! FarKeyToText -> FarKeyToName
    + FarNameToKey
  21.09.2000 SVS
    + OPEN_FILEPANEL ������ �� �������� ������.
    + ���� PluginInfo.SysID - ��������� ������������� �������
  20.09.2000 SVS
    ! ������ FolderPresent (����, ������ ����� ������� :-(
  19.09.2000 SVS
    + ������������ �� 2 �����
    + ������� FSF.FolderPresent - "���������� �� �������"
  18.09.2000 SVS
    + DIF_READONLY - ���� ��� ����� ��������������
      (����! ��� ����� ��������������).
  18.09.2000 SVS
    ! ������� DialogEx ����� 2 �������������� ��������� (Future)
    ! ��������� � struct PluginStartupInfo!!!!
    ! FarRecurseSearch -> FarRecursiveSearch
    ! FRS_RECURSE -> FRS_RECUR
  14.09.2000 SVS
    ! ������ � �������� XLAT_SWITCHKEYBLAYOUT.
    + FSF.MkTemp
    + ���� DIF_LISTNOAMPERSAND. �� ��������� ��� DI_LISTBOX
      ������������ ���� MENU_SHOWAMPERSAND. ���� ���� ��������� �����
      ����������
  13.09.2000 skv
    + EEREDRAW_XXXXX defines
  12.09.2000 SVS
    + ����� FHELP_* ��� ������� ShowHelp
    ! FSF.ShowHelp ���������� BOOL
  10.09.2000 SVS
    ! KeyToText ���������� BOOL, ���� ��� ����� �������.
  10.09.2000 SVS 1.46
    + typedef struct _CHAR_INFO    CHAR_INFO;
      �� ��� ������, ���� wincon.h �� ��� ��������.
  10.09.2000 tran 1.45
    + FSF/FarRecurseSearch
  10.09.2000 SVS 1.44
    ! �������-�� ������� ���������� ��� ��� QWERTY -> Xlat.
    + DIF_NOFOCUS - ������� �� �������� ������ ����� (�����������)
    + CHAR_INFO *VBuf; � ��������� �������
    + DIF_SELECTONENTRY - ��������� Edit ��� ��������� ������
  08.09.2000 VVM
    + FCTL_SETSORTMODE, FCTL_SETANOTHERSORTMODE
      FCTL_SETSORTORDER, FCTL_SETANOTHERSORTORDER
      ����� ���������� �� ������
  08.09.2000 SVS
    ! QWERTY -> Transliterate
    ! QWED_SWITCHKEYBLAYER -> EDTR_SWITCHKEYBLAYER
  08.09.2000 SVS
    + FARMANAGERVERSION
    ! FarStandardFunctions.Reserved* -> FarStandardFunctions.Reserved[10];
  07.09.2000 skv
    + ECTL_PROCESSKEY
  07.09.2000 VVM 1.39
    + PF_FULLCMDLINE ���� ��� �������� ������� ���� ������ ������ �
      ���������
  07.09.2000 SVS 1.38
    + FSF.bsearch
    + FSF.GetFileOwner
    + FSF.GetNumberOfLinks;
  05.09.2000 SVS 1.37
    + QWERTY - �������������� - StandardFunctions.EDQwerty
  01.09.2000 SVS
    + ����������� (� ������ MY)
      #ifndef _WINCON_
      typedef struct _INPUT_RECORD INPUT_RECORD;
      #endif
  31.08.2000 tran 1.35
    + FSF: int FarInputRecordToKey(INPUT_RECORD*r);
  31.08.2000 SVS
    ! ��������� FSF-�������
      FSF.RemoveLeadingSpaces =FSF.LTrim
      FSF.RemoveTrailingSpaces=FSF.RTrim
      FSF.RemoveExternalSpaces=FSF.Trim
    + DM_ENABLE
    + ���� DIF_DISABLE ����������� ������� ������� � ��������� Disable
    + ���� LIF_DISABLE ����������� ������� ������ � ��������� Disable
  30.08.2000 SVS
    ! ��� ������� ������� ���� FMI_GETFARMSGID
    + DM_MOVEDIALOG - ����������� ������.
  29.08.2000 SVS
    ! ��� � ������ ����� � unsigned char �� ���������� ��������� DialogItem,
      � ��-�� ����� uchar ������� DI_USERCONTROL �� ����� ���� > 255 :-((((((
  29.08.2000 SVS
    + ������ ����� ��������� "�����" �� FAR*.LNG, ��� �����
      ��������� � MsgId (� ������� GetMsg)�������� ���� FMI_GETFARMSGID
  28.08.2000 SVS
    + SFS-������� ��� Local*
    ! ��������� ��� FARSTDQSORT - ����� �������� __cdecl ��� ������� ���������
    ! �� FarStandardFunctions._atoi64, �� FarStandardFunctions.atoi64
    + FARSTDITOA64
  25.08.2000 SVS
    + DM_GETDLGRECT - �������� ���������� ����������� ����
    + DM_USER - �� ��� ���������� ������� :-)
  25.08.2000 SVS
    ! ������� �� FSF �������:
      memset, memcpy, memmove, memcmp,
      strchr, strrchr, strstr, strtok, strpbrk
    + ���� FIB_BUTTONS - � ������� InputBox ���� ����� - ����������
      ������ <Ok> & <Cancel>
  24.08.2000 SVS
    + ACTL_WAITKEY - ������� ������������ (��� �����) �������
    + ������� DI_USERCONTROL - ���������� ���������� ������.
  23.08.2000 SVS
    ! ��������� ��������� DMSG_* -> DM_ (�����) & DN_ (������)
    + DM_KEY        - �������/�������� �������(�)
    + DM_GETDLGDATA - ����� ������ �������.
    + DM_SETDLGDATA - ���������� ������ �������.
    + DM_SHOWDIALOG - ��������/�������� ������
    ! ��� Flags ��������� � ������ ���� -> DWORD.
      ��������������:
        * �������   FarMenuFn, FarMessageFn, FarShowHelp
        * ��������� FarListItem, FarDialogItem
  22.08.2000 SVS
    ! DMSG_PAINT -> DMSG_DRAWDIALOG
    ! DMSG_DRAWITEM -> DMSG_DRAWDLGITEM
    ! DMSG_CHANGELIST -> DMSG_LISTCHANGE
  21.08.2000 SVS 1.23
    ! DMSG_CHANGEITEM -> DMSG_EDITCHANGE
    + DMSG_BTNCLICK
  18.08.2000 tran
    + Flags in ShowHelp
  12.08.2000 KM 1.22
    + DIF_MASKEDIT - ����� ����, ����������� ���������������� �����
      �� ����� � ������� �����.
    ! � ��������� FarDialogItem ����� ����, ���������� � union, char *Mask
  17.08.2000 SVS
    ! struct FarListItems -> struct FarList, � �� ������ ��������� :-)
    + ��������� �������: DMSG_ENABLEREDRAW, DMSG_MOUSECLICK,
    + ���� ��� DI_BUTTON - DIF_BTNNOCLOSE - "������ �� ��� �������� �������"
  17.08.2000 SVS
    ! ��������� ������ ������ :-)
  09.08.2000 SVS
    + FIB_NOUSELASTHISTORY - ���� ��� ������������� ���� �������� ��
      ������� �������� ��������!!!
  09.08.2000 tran
    + #define CONSOLE_*
  04.08.2000 SVS
    + ECTL_SETKEYBAR - ������� ��������� KeyBar Labels � ���������
  04.08.2000 SVS
    + FarListItems.CountItems -> FarListItems.ItemsNumber
  03.08.2000 SVS
    + ������� �� AT: GetMinFarVersion
  03.08.2000 SVS
    + ACTL_GETSYSWORDDIV �������� ������ � ��������� ������������� ����
  02.08.2000 SVS
    + ���������� ��� KeyBarTitles:
        CtrlShiftTitles
        AltShiftTitles,
        CtrlAltTitles
    + ������� � OpenPluginInfo ��� ����, ����� ��������� FAR <= 1.65 � > 1.65
  01.08.2000 SVS
    ! ������� ����� ������ ����� ���� �������� ��� ���� ������
    ! �������������� ��������� � KeyToText - ������ ������
    + ���� DIF_USELASTHISTORY ��� ����� �����.
      ���� � ������ ����� ���� ������� �� ��������� �������� ����� ������
      �� �������
    ! ������ ��������� ��������� ������ � "��������" ������ ������
    + ����� ��� FarListItem.Flags
      LIF_SELECTED, LIF_CHECKED, LIF_SEPARATOR
    + ��������� ��� ��������� �������, �������� ����� ���� :-)
      DMSG_SETDLGITEM, DMSG_CHANGELIST
    ! �������� ������������ ���� ������� ����������� �� �������������
      FARDIALOGPROC -> FARWINDOWPROC
  28.07.2000 SVS
    + ������ ����� ������� DI_LISTBOX (��������������� �����)
    + ��������� ��� ��������� �������, �������� ����� ���� :-)
        DMSG_INITDIALOG, DMSG_ENTERIDLE, DMSG_HELP, DMSG_PAINT,
        DMSG_SETREDRAW, DMSG_DRAWITEM, DMSG_GETDLGITEM, DMSG_KILLFOCUS,
        DMSG_GOTFOCUS, DMSG_SETFOCUS, DMSG_GETTEXTLENGTH, DMSG_GETTEXT,
        DMSG_CTLCOLORDIALOG, DMSG_CTLCOLORDLGITEM, DMSG_CTLCOLORDLGLIST,
        DMSG_SETTEXTLENGTH, DMSG_SETTEXT, DMSG_CHANGEITEM, DMSG_HOTKEY,
        DMSG_CLOSE,
  25.07.2000 SVS
    ! ��������� ������������ � FarStandardFunctions
    + ���������� ������������ FulScreen <-> Windowed (ACTL_CONSOLEMODE)
    + FSF-������� KeyToText
    ! WINAPI ��� ��������� �������������� �������
    + �������-������ ����� �������� ������ InputBox
  23.07.2000 SVS
    + DialogEx, SendDlgMessage, DefDlgProc,
    ! WINAPI ��� ��������� �������������� �������
  18.07.2000 SVS
    + ������ ����� �������: DI_COMBOBOX � ���� DIF_DROPDOWNLIST
      (��� ���������������� DI_COMBOBOX - ���� �� �����������!)
  12.07.2000 IS
    + �����  ���������:
      EF_NONMODAL - �������� ������������ ���������
  11.07.2000 SVS
    ! ��������� ��� ����������� ���������� ��� BC & VC
  10.07.2000 IS
    ! ��������� ��������� � ������ ������ C (�� ������ SVS)
  07.07.2000 IS
    + ��������� �� ������� � FarStandardFunctions:
      atoi, _atoi64, itoa, RemoveLeadingSpaces, RemoveTrailingSpaces,
      RemoveExternalSpaces, TruncStr, TruncPathStr, QuoteSpaceOnly,
      PointToName, GetPathRoot, AddEndSlash
  06.07.2000 IS
    + ������� AdvControl (PluginStartupInfo)
    + ������� ACTL_GETFARVERSION ��� AdvControl
    + ��������� �� ��������� FarStandardFunctions � PluginStartupInfo - ���
      �������� ��������� �� �������� �������. ������ ������ �����������
      ����������� �� ����, ���� ����� ������������ � ����������.
    + ��������� �� ������� � FarStandardFunctions:
      Unquote, ExpandEnvironmentStr,
      sprintf, sscanf, qsort, memcpy, memmove, memcmp, strchr, strrchr, strstr,
      strtok, memset, strpbrk
  05.06.2000 SVS
    + DI_EDIT ����� ���� DIF_EDITEXPAND - ���������� ���������� �����
      � enum FarDialogItemFlags
  03.07.2000 IS
    + ������� ������ ������ � api
  28.06.2000 SVS
    + ��� MSVC ���� ��������� extern "C" ��� ����������
      �������������� ������� + ��������� �� Borland C++ 5.5
  26.06.2000 SVS
    ! ���������� Master Copy
*/
#endif // END FAR_USE_INTERNALS

#define MAKEFARVERSION(major,minor,build) ( ((major)<<8) | (minor) | ((build)<<16))

#define FARMANAGERVERSION  MAKEFARVERSION(1,70,1069)


#ifdef FAR_USE_INTERNALS
#else // ELSE FAR_USE_INTERNALS
#if !defined(_INC_WINDOWS) && !defined(_WINDOWS_)
 #if defined(__GNUC__) || defined(_MSC_VER)
  #if !defined(_WINCON_H) && !defined(_WINCON_)
    #define _WINCON_H
    #define _WINCON_ // to prevent including wincon.h
    #if defined(_MSC_VER)
     #pragma pack(push,2)
    #else
     #pragma pack(2)
    #endif
    #include<windows.h>
    #if defined(_MSC_VER)
     #pragma pack(pop)
    #else
     #pragma pack()
    #endif
    #undef _WINCON_
    #undef  _WINCON_H

    #if defined(_MSC_VER)
     #pragma pack(push,8)
    #else
     #pragma pack(8)
    #endif
    #include<wincon.h>
    #if defined(_MSC_VER)
     #pragma pack(pop)
    #else
     #pragma pack()
    #endif
  #endif
  #define _WINCON_
 #else
   #include<windows.h>
 #endif
#endif
#endif // END FAR_USE_INTERNALS

#if defined(__BORLANDC__)
  #pragma option -a2
#elif defined(__GNUC__) || (defined(__WATCOMC__) && (__WATCOMC__ < 1100)) || defined(__LCC__)
  #pragma pack(2)
  #if defined(__LCC__)
    #define _export __declspec(dllexport)
  #endif
#else
  #pragma pack(push,2)
  #if _MSC_VER
    #define _export
  #endif
#endif

#define NM 260

#define FARMACRO_KEY_EVENT  (KEY_EVENT|0x8000)

#ifdef FAR_USE_INTERNALS
#define _FAR_NO_NAMELESS_UNIONS
#else // ELSE FAR_USE_INTERNALS
// To ensure compatibility of plugin.hpp with compilers not supporting C++,
// you can #define _FAR_NO_NAMELESS_UNIONS. In this case, to access,
// for example, the Data field of the FarDialogItem structure
// you will need to use Data.Data, and the Selected field - Param.Selected
#define _FAR_NO_NAMELESS_UNIONS

// To ensure correct structure packing, you can #define _FAR_USE_FARFINDDATA.
// In this case, the member PluginPanelItem.FindData will have the type
// FAR_FIND_DATA, not WIN32_FIND_DATA. The structure FAR_FIND_DATA has the
// same layout as WIN32_FIND_DATA, but since it is declared in this file,
// it is generated with correct 2-byte alignment.
// This #define is necessary to compile plugins with Borland C++ 5.5.
//#define _FAR_USE_FARFINDDATA
#endif // END FAR_USE_INTERNALS

#ifndef _WINCON_
typedef struct _INPUT_RECORD INPUT_RECORD;
typedef struct _CHAR_INFO    CHAR_INFO;
#endif

enum FARMESSAGEFLAGS{
  FMSG_WARNING             = 0x00000001,
  FMSG_ERRORTYPE           = 0x00000002,
  FMSG_KEEPBACKGROUND      = 0x00000004,
  FMSG_DOWN                = 0x00000008,
  FMSG_LEFTALIGN           = 0x00000010,

  FMSG_ALLINONE            = 0x00000020,
#ifdef FAR_USE_INTERNALS
  FMSG_COLOURS             = 0x00000040,
#endif // END FAR_USE_INTERNALS

  FMSG_MB_OK               = 0x00010000,
  FMSG_MB_OKCANCEL         = 0x00020000,
  FMSG_MB_ABORTRETRYIGNORE = 0x00030000,
  FMSG_MB_YESNO            = 0x00040000,
  FMSG_MB_YESNOCANCEL      = 0x00050000,
  FMSG_MB_RETRYCANCEL      = 0x00060000,
};

typedef int (WINAPI *FARAPIMESSAGE)(
  int PluginNumber,
  DWORD Flags,
  const char *HelpTopic,
  const char * const *Items,
  int ItemsNumber,
  int ButtonsNumber
);


enum DialogItemTypes {
  DI_TEXT,
  DI_VTEXT,
  DI_SINGLEBOX,
  DI_DOUBLEBOX,
  DI_EDIT,
  DI_PSWEDIT,
  DI_FIXEDIT,
  DI_BUTTON,
  DI_CHECKBOX,
  DI_RADIOBUTTON,
  DI_COMBOBOX,
  DI_LISTBOX,
#ifdef FAR_USE_INTERNALS
  DI_MEMOEDIT,
#endif // END FAR_USE_INTERNALS

  DI_USERCONTROL=255,
};

enum FarDialogItemFlags {
  DIF_COLORMASK             = 0x000000ffUL,
  DIF_SETCOLOR              = 0x00000100UL,
  DIF_BOXCOLOR              = 0x00000200UL,
  DIF_GROUP                 = 0x00000400UL,
  DIF_LEFTTEXT              = 0x00000800UL,
  DIF_MOVESELECT            = 0x00001000UL,
  DIF_SHOWAMPERSAND         = 0x00002000UL,
  DIF_CENTERGROUP           = 0x00004000UL,
  DIF_NOBRACKETS            = 0x00008000UL,
  DIF_MANUALADDHISTORY      = 0x00008000UL,
  DIF_SEPARATOR             = 0x00010000UL,
  DIF_VAREDIT               = 0x00010000UL,
  DIF_SEPARATOR2            = 0x00020000UL,
  DIF_EDITOR                = 0x00020000UL,
  DIF_LISTNOAMPERSAND       = 0x00020000UL,
  DIF_LISTNOBOX             = 0x00040000UL,
  DIF_HISTORY               = 0x00040000UL,
  DIF_BTNNOCLOSE            = 0x00040000UL,
  DIF_CENTERTEXT            = 0x00040000UL,
#ifdef FAR_USE_INTERNALS
#if defined(USE_WFUNC)
  DIF_NOTCVTUSERCONTROL     = 0x00040000UL,
#endif
#endif // END FAR_USE_INTERNALS
  DIF_EDITEXPAND            = 0x00080000UL,
  DIF_DROPDOWNLIST          = 0x00100000UL,
  DIF_USELASTHISTORY        = 0x00200000UL,
  DIF_MASKEDIT              = 0x00400000UL,
  DIF_SELECTONENTRY         = 0x00800000UL,
  DIF_3STATE                = 0x00800000UL,
#ifdef FAR_USE_INTERNALS
  DIF_EDITPATH              = 0x01000000UL,
#endif // END FAR_USE_INTERNALS
  DIF_LISTWRAPMODE          = 0x01000000UL,
  DIF_LISTAUTOHIGHLIGHT     = 0x02000000UL,
  DIF_LISTNOCLOSE           = 0x04000000UL,
#ifdef FAR_USE_INTERNALS
  DIF_AUTOMATION            = 0x08000000UL,
#endif // END FAR_USE_INTERNALS
  DIF_HIDDEN                = 0x10000000UL,
  DIF_READONLY              = 0x20000000UL,
  DIF_NOFOCUS               = 0x40000000UL,
  DIF_DISABLE               = 0x80000000UL,
};

enum FarMessagesProc{
  DM_FIRST=0,
  DM_CLOSE,
  DM_ENABLE,
  DM_ENABLEREDRAW,
  DM_GETDLGDATA,
  DM_GETDLGITEM,
  DM_GETDLGRECT,
  DM_GETTEXT,
  DM_GETTEXTLENGTH,
  DM_KEY,
  DM_MOVEDIALOG,
  DM_SETDLGDATA,
  DM_SETDLGITEM,
  DM_SETFOCUS,
  DM_REDRAW,
  DM_SETREDRAW=DM_REDRAW,
  DM_SETTEXT,
  DM_SETMAXTEXTLENGTH,
  DM_SETTEXTLENGTH=DM_SETMAXTEXTLENGTH,
  DM_SHOWDIALOG,
  DM_GETFOCUS,
  DM_GETCURSORPOS,
  DM_SETCURSORPOS,
  DM_GETTEXTPTR,
  DM_SETTEXTPTR,
  DM_SHOWITEM,
  DM_ADDHISTORY,

  DM_GETCHECK,
  DM_SETCHECK,
  DM_SET3STATE,

  DM_LISTSORT,
  DM_LISTGETITEM,
  DM_LISTGETCURPOS,
  DM_LISTSETCURPOS,
  DM_LISTDELETE,
  DM_LISTADD,
  DM_LISTADDSTR,
  DM_LISTUPDATE,
  DM_LISTINSERT,
  DM_LISTFINDSTRING,
  DM_LISTINFO,
  DM_LISTGETDATA,
  DM_LISTSETDATA,
  DM_LISTSETTITLES,
  DM_LISTGETTITLES,

  DM_RESIZEDIALOG,
  DM_SETITEMPOSITION,

  DM_GETDROPDOWNOPENED,
  DM_SETDROPDOWNOPENED,

  DM_SETHISTORY,

  DM_GETITEMPOSITION,
  DM_SETMOUSEEVENTNOTIFY,

  DM_EDITUNCHANGEDFLAG,

  DM_GETITEMDATA,
  DM_SETITEMDATA,

  DM_LISTSET,
  DM_LISTSETMOUSEREACTION,

  DM_GETCURSORSIZE,
  DM_SETCURSORSIZE,

  DM_LISTGETDATASIZE,

  DM_GETSELECTION,
  DM_SETSELECTION,

  DN_LISTHOTKEY,

  DN_FIRST=0x1000,
  DN_BTNCLICK,
  DN_CTLCOLORDIALOG,
  DN_CTLCOLORDLGITEM,
  DN_CTLCOLORDLGLIST,
  DN_DRAWDIALOG,
  DN_DRAWDLGITEM,
  DN_EDITCHANGE,
  DN_ENTERIDLE,
  DN_GOTFOCUS,
  DN_HELP,
  DN_HOTKEY,
  DN_INITDIALOG,
  DN_KILLFOCUS,
  DN_LISTCHANGE,
  DN_MOUSECLICK,
  DN_DRAGGED,
  DN_RESIZECONSOLE,
  DN_MOUSEEVENT,
  DN_DRAWDIALOGDONE,

  DN_CLOSE=DM_CLOSE,
  DN_KEY=DM_KEY,

  DM_USER=0x4000,

#ifdef FAR_USE_INTERNALS
  DM_KILLSAVESCREEN=DN_FIRST-1,
  DM_ALLKEYMODE=DN_FIRST-2,
  DN_ACTIVATEAPP=DM_USER-1,
#endif // END FAR_USE_INTERNALS
};

enum FARCHECKEDSTATE {
  BSTATE_UNCHECKED = 0,
  BSTATE_CHECKED   = 1,
  BSTATE_3STATE    = 2,
  BSTATE_TOGGLE    = 3,
};

enum FARLISTMOUSEREACTIONTYPE{
  LMRT_ONLYFOCUS   = 0,
  LMRT_ALWAYS      = 1,
  LMRT_NEVER       = 2,
};

enum LISTITEMFLAGS {
  LIF_SELECTED           = 0x00010000UL,
  LIF_CHECKED            = 0x00020000UL,
  LIF_SEPARATOR          = 0x00040000UL,
  LIF_DISABLE            = 0x00080000UL,
#ifdef FAR_USE_INTERNALS
  LIF_GRAYED             = 0x00100000UL,
#endif // END FAR_USE_INTERNALS
  LIF_DELETEUSERDATA     = 0x80000000UL,
};

struct FarListItem
{
  DWORD Flags;
  char  Text[128];
  DWORD Reserved[3];
};

struct FarListUpdate
{
  int Index;
  struct FarListItem Item;
};

struct FarListInsert
{
  int Index;
  struct FarListItem Item;
};

struct FarListGetItem
{
  int ItemIndex;
  struct FarListItem Item;
};

struct FarListPos
{
  int SelectPos;
  int TopPos;
};

enum FARLISTFINDFLAGS{
  LIFIND_EXACTMATCH = 0x00000001,
};

struct FarListFind
{
  int StartIndex;
  const char *Pattern;
  DWORD Flags;
  DWORD Reserved;
};

struct FarListDelete
{
  int StartIndex;
  int Count;
};

enum FARLISTINFOFLAGS{
  LINFO_SHOWNOBOX             = 0x00000400,
  LINFO_AUTOHIGHLIGHT         = 0x00000800,
  LINFO_REVERSEHIGHLIGHT      = 0x00001000,
  LINFO_WRAPMODE              = 0x00008000,
  LINFO_SHOWAMPERSAND         = 0x00010000,
};

struct FarListInfo
{
  DWORD Flags;
  int ItemsNumber;
  int SelectPos;
  int TopPos;
  int MaxHeight;
  int MaxLength;
  DWORD Reserved[6];
};

struct FarListItemData
{
  int   Index;
  int   DataSize;
  void *Data;
  DWORD Reserved;
};

struct FarList
{
  int ItemsNumber;
  struct FarListItem *Items;
};

struct FarListTitles
{
  int   TitleLen;
  char *Title;
  int   BottomLen;
  char *Bottom;
};

struct FarListColors{
  DWORD  Flags;
  DWORD  Reserved;
  int    ColorCount;
  LPBYTE Colors;
};

struct FarDialogItem
{
  int Type;
  int X1,Y1,X2,Y2;
  int Focus;
  union
  {
    int Selected;
    const char *History;
    const char *Mask;
    struct FarList *ListItems;
    int  ListPos;
    CHAR_INFO *VBuf;
  }
#ifdef _FAR_NO_NAMELESS_UNIONS
  Param
#endif
  ;
  DWORD Flags;
  int DefaultButton;
  union
  {
    char Data[512];
    struct
    {
      DWORD PtrFlags;
      int   PtrLength;
      char *PtrData;
      char  PtrTail[1];
    } Ptr;
  }
#ifdef _FAR_NO_NAMELESS_UNIONS
  Data
#endif
  ;
};

struct FarDialogItemData
{
  int   PtrLength;
  char *PtrData;
};

#define Dlg_RedrawDialog(Info,hDlg)            Info.SendDlgMessage(hDlg,DM_REDRAW,0,0)

#define Dlg_GetDlgData(Info,hDlg)              Info.SendDlgMessage(hDlg,DM_GETDLGDATA,0,0)
#define Dlg_SetDlgData(Info,hDlg,Data)         Info.SendDlgMessage(hDlg,DM_SETDLGDATA,0,(long)Data)

#define Dlg_GetDlgItemData(Info,hDlg,ID)       Info.SendDlgMessage(hDlg,DM_GETITEMDATA,0,0)
#define Dlg_SetDlgItemData(Info,hDlg,ID,Data)  Info.SendDlgMessage(hDlg,DM_SETITEMDATA,0,(long)Data)

#define DlgItem_GetFocus(Info,hDlg)            Info.SendDlgMessage(hDlg,DM_GETFOCUS,0,0)
#define DlgItem_SetFocus(Info,hDlg,ID)         Info.SendDlgMessage(hDlg,DM_SETFOCUS,ID,0)
#define DlgItem_Enable(Info,hDlg,ID)           Info.SendDlgMessage(hDlg,DM_ENABLE,ID,TRUE)
#define DlgItem_Disable(Info,hDlg,ID)          Info.SendDlgMessage(hDlg,DM_ENABLE,ID,FALSE)
#define DlgItem_IsEnable(Info,hDlg,ID)         Info.SendDlgMessage(hDlg,DM_ENABLE,ID,-1)
#define DlgItem_SetText(Info,hDlg,ID,Str)      Info.SendDlgMessage(hDlg,DM_SETTEXTPTR,ID,(long)Str)

#define DlgItem_GetCheck(Info,hDlg,ID)         Info.SendDlgMessage(hDlg,DM_GETCHECK,ID,0)
#define DlgItem_SetCheck(Info,hDlg,ID,State)   Info.SendDlgMessage(hDlg,DM_SETCHECK,ID,State)

#define DlgEdit_AddHistory(Info,hDlg,ID,Str)   Info.SendDlgMessage(hDlg,DM_ADDHISTORY,ID,(long)Str)

#define DlgList_AddString(Info,hDlg,ID,Str)    Info.SendDlgMessage(hDlg,DM_LISTADDSTR,ID,(long)Str)
#define DlgList_GetCurPos(Info,hDlg,ID)        Info.SendDlgMessage(hDlg,DM_LISTGETCURPOS,ID,0)
#define DlgList_SetCurPos(Info,hDlg,ID,NewPos) {struct FarListPos LPos={NewPos,-1};Info.SendDlgMessage(hDlg,DM_LISTSETCURPOS,ID,&LPos);}
#define DlgList_ClearList(Info,hDlg,ID)        Info.SendDlgMessage(hDlg,DM_LISTDELETE,ID,0)
#define DlgList_DeleteItem(Info,hDlg,ID,Index) {struct FarListDelete FLDItem={Index,1}; Info.SendDlgMessage(hDlg,DM_LISTDELETE,ID,(long)&FLDItem);}
#define DlgList_SortUp(Info,hDlg,ID)           Info.SendDlgMessage(hDlg,DM_LISTSORT,ID,0)
#define DlgList_SortDown(Info,hDlg,ID)         Info.SendDlgMessage(hDlg,DM_LISTSORT,ID,1)
#define DlgList_GetItemData(Info,hDlg,ID,Index)          Info.SendDlgMessage(hDlg,DM_LISTGETDATA,ID,Index)
#define DlgList_SetItemStrAsData(Info,hDlg,ID,Index,Str) {struct FarListItemData FLID{Index,0,Str,0}; Info.SendDlgMessage(hDlg,DM_LISTSETDATA,ID,(long)&FLID);}

enum FARDIALOGFLAGS{
  FDLG_WARNING             = 0x00000001,
  FDLG_SMALLDIALOG         = 0x00000002,
  FDLG_NODRAWSHADOW        = 0x00000004,
  FDLG_NODRAWPANEL         = 0x00000008,
#ifdef FAR_USE_INTERNALS
  FDLG_NONMODAL            = 0x00000010,
#endif // END FAR_USE_INTERNALS
};

typedef long (WINAPI *FARWINDOWPROC)(
  HANDLE hDlg,
  int    Msg,
  int    Param1,
  long   Param2
);

typedef long (WINAPI *FARAPISENDDLGMESSAGE)(
  HANDLE hDlg,
  int    Msg,
  int    Param1,
  long   Param2
);

typedef long (WINAPI *FARAPIDEFDLGPROC)(
  HANDLE hDlg,
  int    Msg,
  int    Param1,
  long   Param2
);

typedef int (WINAPI *FARAPIDIALOG)(
  int                   PluginNumber,
  int                   X1,
  int                   Y1,
  int                   X2,
  int                   Y2,
  const char           *HelpTopic,
  struct FarDialogItem *Item,
  int                   ItemsNumber
);

typedef int (WINAPI *FARAPIDIALOGEX)(
  int                   PluginNumber,
  int                   X1,
  int                   Y1,
  int                   X2,
  int                   Y2,
  const char           *HelpTopic,
  struct FarDialogItem *Item,
  int                   ItemsNumber,
  DWORD                 Reserved,
  DWORD                 Flags,
  FARWINDOWPROC         DlgProc,
  long                  Param
);


struct FarMenuItem
{
  char Text[128];
  int  Selected;
  int  Checked;
  int  Separator;
};

enum MENUITEMFLAGS {
  MIF_SELECTED   = 0x00010000UL,
  MIF_CHECKED    = 0x00020000UL,
  MIF_SEPARATOR  = 0x00040000UL,
  MIF_DISABLE    = 0x00080000UL,
#ifdef FAR_USE_INTERNALS
  MIF_GRAYED     = 0x00100000UL,
#endif // END FAR_USE_INTERNALS
  MIF_USETEXTPTR = 0x80000000UL,
};

struct FarMenuItemEx
{
  DWORD Flags;
  union {
    char  Text[128];
    const char *TextPtr;
  } Text;
  DWORD AccelKey;
  DWORD Reserved;
  DWORD UserData;
};

enum FARMENUFLAGS{
  FMENU_SHOWAMPERSAND        = 0x0001,
  FMENU_WRAPMODE             = 0x0002,
  FMENU_AUTOHIGHLIGHT        = 0x0004,
  FMENU_REVERSEAUTOHIGHLIGHT = 0x0008,
#ifdef FAR_USE_INTERNALS
  FMENU_SHOWNOBOX            = 0x0010,
#endif // END FAR_USE_INTERNALS
  FMENU_USEEXT               = 0x0020,
  FMENU_CHANGECONSOLETITLE   = 0x0040,
};

typedef int (WINAPI *FARAPIMENU)(
  int                 PluginNumber,
  int                 X,
  int                 Y,
  int                 MaxHeight,
  DWORD               Flags,
  const char         *Title,
  const char         *Bottom,
  const char         *HelpTopic,
  const int          *BreakKeys,
  int                *BreakCode,
  const struct FarMenuItem *Item,
  int                 ItemsNumber
);


enum PLUGINPANELITEMFLAGS{
  PPIF_PROCESSDESCR           = 0x80000000,
  PPIF_SELECTED               = 0x40000000,
  PPIF_USERDATA               = 0x20000000,
};

#ifdef _FAR_USE_FARFINDDATA

struct FAR_FIND_DATA
{
  DWORD    dwFileAttributes;
  FILETIME ftCreationTime;
  FILETIME ftLastAccessTime;
  FILETIME ftLastWriteTime;
  DWORD    nFileSizeHigh;
  DWORD    nFileSizeLow;
  DWORD    dwReserved0;
  DWORD    dwReserved1;
  CHAR     cFileName[MAX_PATH];
  CHAR     cAlternateFileName[14];
};

#endif

struct PluginPanelItem
{
#ifdef _FAR_USE_FARFINDDATA
  struct FAR_FIND_DATA   FindData;
#else
  WIN32_FIND_DATA FindData;
#endif
  DWORD           PackSizeHigh;
  DWORD           PackSize;
  DWORD           Flags;
  DWORD           NumberOfLinks;
  char           *Description;
  char           *Owner;
  char          **CustomColumnData;
  int             CustomColumnNumber;
  DWORD           UserData;
  DWORD           CRC32;
  DWORD           Reserved[2];
};

#if defined(__BORLANDC__)
#if sizeof(struct PluginPanelItem) != 366
#if defined(STRICT)
#error Incorrect alignment: sizeof(PluginPanelItem)!=366
#else
#pragma message Incorrect alignment: sizeof(PluginPanelItem)!=366
#endif
#endif
#endif

enum PANELINFOFLAGS {
  PFLAGS_SHOWHIDDEN         = 0x00000001,
  PFLAGS_HIGHLIGHT          = 0x00000002,
  PFLAGS_REVERSESORTORDER   = 0x00000004,
  PFLAGS_USESORTGROUPS      = 0x00000008,
  PFLAGS_SELECTEDFIRST      = 0x00000010,
  PFLAGS_REALNAMES          = 0x00000020,
  PFLAGS_NUMERICSORT        = 0x00000040,
};

enum PANELINFOTYPE{
  PTYPE_FILEPANEL,
  PTYPE_TREEPANEL,
  PTYPE_QVIEWPANEL,
  PTYPE_INFOPANEL
};

struct PanelInfo
{
  int PanelType;
  int Plugin;
  RECT PanelRect;
  struct PluginPanelItem *PanelItems;
  int ItemsNumber;
  struct PluginPanelItem *SelectedItems;
  int SelectedItemsNumber;
  int CurrentItem;
  int TopPanelItem;
  int Visible;
  int Focus;
  int ViewMode;
  char ColumnTypes[80];
  char ColumnWidths[80];
  char CurDir[NM];
  int ShortNames;
  int SortMode;
  DWORD Flags;
  DWORD Reserved;
};


struct PanelRedrawInfo
{
  int CurrentItem;
  int TopPanelItem;
};

struct CmdLineSelect
{
  int SelStart;
  int SelEnd;
};

enum FILE_CONTROL_COMMANDS{
  FCTL_CLOSEPLUGIN,
  FCTL_GETPANELINFO,
  FCTL_GETANOTHERPANELINFO,
  FCTL_UPDATEPANEL,
  FCTL_UPDATEANOTHERPANEL,
  FCTL_REDRAWPANEL,
  FCTL_REDRAWANOTHERPANEL,
  FCTL_SETANOTHERPANELDIR,
  FCTL_GETCMDLINE,
  FCTL_SETCMDLINE,
  FCTL_SETSELECTION,
  FCTL_SETANOTHERSELECTION,
  FCTL_SETVIEWMODE,
  FCTL_SETANOTHERVIEWMODE,
  FCTL_INSERTCMDLINE,
  FCTL_SETUSERSCREEN,
  FCTL_SETPANELDIR,
  FCTL_SETCMDLINEPOS,
  FCTL_GETCMDLINEPOS,
  FCTL_SETSORTMODE,
  FCTL_SETANOTHERSORTMODE,
  FCTL_SETSORTORDER,
  FCTL_SETANOTHERSORTORDER,
  FCTL_GETCMDLINESELECTEDTEXT,
  FCTL_SETCMDLINESELECTION,
  FCTL_GETCMDLINESELECTION,
  FCTL_GETPANELSHORTINFO,
  FCTL_GETANOTHERPANELSHORTINFO,
  FCTL_CHECKPANELSEXIST,
  FCTL_SETNUMERICSORT,
  FCTL_SETANOTHERNUMERICSORT,
};

typedef int (WINAPI *FARAPICONTROL)(
  HANDLE hPlugin,
  int Command,
  void *Param
);

typedef void (WINAPI *FARAPITEXT)(
  int X,
  int Y,
  int Color,
  const char *Str
);

typedef HANDLE (WINAPI *FARAPISAVESCREEN)(int X1, int Y1, int X2, int Y2);

typedef void (WINAPI *FARAPIRESTORESCREEN)(HANDLE hScreen);


typedef int (WINAPI *FARAPIGETDIRLIST)(
  const char *Dir,
  struct PluginPanelItem **pPanelItem,
  int *pItemsNumber
);

typedef int (WINAPI *FARAPIGETPLUGINDIRLIST)(
  int PluginNumber,
  HANDLE hPlugin,
  const char *Dir,
  struct PluginPanelItem **pPanelItem,
  int *pItemsNumber
);

typedef void (WINAPI *FARAPIFREEDIRLIST)(const struct PluginPanelItem *PanelItem);

enum VIEWER_FLAGS {
  VF_NONMODAL              = 0x00000001,
  VF_DELETEONCLOSE         = 0x00000002,
  VF_ENABLE_F6             = 0x00000004,
  VF_DISABLEHISTORY        = 0x00000008,
  VF_IMMEDIATERETURN       = 0x00000100,
  VF_DELETEONLYFILEONCLOSE = 0x00000200,
};

typedef int (WINAPI *FARAPIVIEWER)(
  const char *FileName,
  const char *Title,
  int X1,
  int Y1,
  int X2,
  int Y2,
  DWORD Flags
);

enum EDITOR_FLAGS {
  EF_NONMODAL              = 0x00000001,
  EF_CREATENEW             = 0x00000002,
  EF_ENABLE_F6             = 0x00000004,
  EF_DISABLEHISTORY        = 0x00000008,
  EF_DELETEONCLOSE         = 0x00000010,
#ifdef FAR_USE_INTERNALS
  EF_USEEXISTING           = 0x00000020,
  EF_BREAKIFOPEN           = 0x00000040,
  EF_NEWIFOPEN             = 0x00000080,
#endif // END FAR_USE_INTERNALS
  EF_IMMEDIATERETURN       = 0x00000100,
  EF_DELETEONLYFILEONCLOSE = 0x00000200,
};

enum EDITOR_EXITCODE{
  EEC_OPEN_ERROR          = 0,
  EEC_MODIFIED            = 1,
  EEC_NOT_MODIFIED        = 2,
  EEC_LOADING_INTERRUPTED = 3,
#ifdef FAR_USE_INTERNALS
  EEC_OPENED_EXISTING     = 4,
  EEC_ALREADY_EXISTS      = 5,
  EEC_OPEN_NEWINSTANCE    = 6,
  EEC_RELOAD              = 7,
#endif // END FAR_USE_INTERNALS
};

typedef int (WINAPI *FARAPIEDITOR)(
  const char *FileName,
  const char *Title,
  int X1,
  int Y1,
  int X2,
  int Y2,
  DWORD Flags,
  int StartLine,
  int StartChar
);

typedef int (WINAPI *FARAPICMPNAME)(
  const char *Pattern,
  const char *String,
  int SkipPath
);


enum FARCHARTABLE_COMMAND{
  FCT_DETECT=0x40000000,
};

struct CharTableSet
{
  unsigned char DecodeTable[256];
  unsigned char EncodeTable[256];
  unsigned char UpperTable[256];
  unsigned char LowerTable[256];
  char TableName[128];
#ifdef FAR_USE_INTERNALS
  //char RFCCharset[128];
#endif // END FAR_USE_INTERNALS
};

typedef int (WINAPI *FARAPICHARTABLE)(
  int Command,
  char *Buffer,
  int BufferSize
);

typedef const char* (WINAPI *FARAPIGETMSG)(
  int PluginNumber,
  int MsgId
);


enum FarHelpFlags{
  FHELP_NOSHOWERROR = 0x80000000,
  FHELP_SELFHELP    = 0x00000000,
  FHELP_FARHELP     = 0x00000001,
  FHELP_CUSTOMFILE  = 0x00000002,
  FHELP_CUSTOMPATH  = 0x00000004,
  FHELP_USECONTENTS = 0x40000000,
};

typedef BOOL (WINAPI *FARAPISHOWHELP)(
  const char *ModuleName,
  const char *Topic,
  DWORD Flags
);

enum ADVANCED_CONTROL_COMMANDS{
  ACTL_GETFARVERSION,
  ACTL_CONSOLEMODE,
  ACTL_GETSYSWORDDIV,
  ACTL_WAITKEY,
  ACTL_GETCOLOR,
  ACTL_GETARRAYCOLOR,
  ACTL_EJECTMEDIA,
  ACTL_KEYMACRO,
  ACTL_POSTKEYSEQUENCE,
  ACTL_GETWINDOWINFO,
  ACTL_GETWINDOWCOUNT,
  ACTL_SETCURRENTWINDOW,
  ACTL_COMMIT,
  ACTL_GETFARHWND,
  ACTL_GETSYSTEMSETTINGS,
  ACTL_GETPANELSETTINGS,
  ACTL_GETINTERFACESETTINGS,
  ACTL_GETCONFIRMATIONS,
  ACTL_GETDESCSETTINGS,
  ACTL_SETARRAYCOLOR,
  ACTL_GETWCHARMODE,
  ACTL_GETPLUGINMAXREADDATA,
  ACTL_GETDIALOGSETTINGS,
#ifdef FAR_USE_INTERNALS
  ACTL_GETPOLICIES,
#endif // END FAR_USE_INTERNALS
};

#ifdef FAR_USE_INTERNALS
enum FarPoliciesFlags{
  FFPOL_MAINMENUSYSTEM        = 0x00000001,
  FFPOL_MAINMENUPANEL         = 0x00000002,
  FFPOL_MAINMENUINTERFACE     = 0x00000004,
  FFPOL_MAINMENULANGUAGE      = 0x00000008,
  FFPOL_MAINMENUPLUGINS       = 0x00000010,
  FFPOL_MAINMENUDIALOGS       = 0x00000020,
  FFPOL_MAINMENUCONFIRMATIONS = 0x00000040,
  FFPOL_MAINMENUPANELMODE     = 0x00000080,
  FFPOL_MAINMENUFILEDESCR     = 0x00000100,
  FFPOL_MAINMENUFOLDERDESCR   = 0x00000200,
  FFPOL_MAINMENUVIEWER        = 0x00000800,
  FFPOL_MAINMENUEDITOR        = 0x00001000,
  FFPOL_MAINMENUCOLORS        = 0x00004000,
  FFPOL_MAINMENUHILIGHT       = 0x00008000,
  FFPOL_MAINMENUSAVEPARAMS    = 0x00020000,

  FFPOL_CREATEMACRO           = 0x00040000,
  FFPOL_USEPSWITCH            = 0x00080000,
  FFPOL_PERSONALPATH          = 0x00100000,
  FFPOL_KILLTASK              = 0x00200000,
  FFPOL_SHOWHIDDENDRIVES      = 0x80000000,
};

#endif // END FAR_USE_INTERNALS

enum FarSystemSettings{
  FSS_CLEARROATTRIBUTE               = 0x00000001,
  FSS_DELETETORECYCLEBIN             = 0x00000002,
  FSS_USESYSTEMCOPYROUTINE           = 0x00000004,
  FSS_COPYFILESOPENEDFORWRITING      = 0x00000008,
  FSS_CREATEFOLDERSINUPPERCASE       = 0x00000010,
  FSS_SAVECOMMANDSHISTORY            = 0x00000020,
  FSS_SAVEFOLDERSHISTORY             = 0x00000040,
  FSS_SAVEVIEWANDEDITHISTORY         = 0x00000080,
  FSS_USEWINDOWSREGISTEREDTYPES      = 0x00000100,
  FSS_AUTOSAVESETUP                  = 0x00000200,
  FSS_SCANSYMLINK                    = 0x00000400,
};

enum FarPanelSettings{
  FPS_SHOWHIDDENANDSYSTEMFILES       = 0x00000001,
  FPS_HIGHLIGHTFILES                 = 0x00000002,
  FPS_AUTOCHANGEFOLDER               = 0x00000004,
  FPS_SELECTFOLDERS                  = 0x00000008,
  FPS_ALLOWREVERSESORTMODES          = 0x00000010,
  FPS_SHOWCOLUMNTITLES               = 0x00000020,
  FPS_SHOWSTATUSLINE                 = 0x00000040,
  FPS_SHOWFILESTOTALINFORMATION      = 0x00000080,
  FPS_SHOWFREESIZE                   = 0x00000100,
  FPS_SHOWSCROLLBAR                  = 0x00000200,
  FPS_SHOWBACKGROUNDSCREENSNUMBER    = 0x00000400,
  FPS_SHOWSORTMODELETTER             = 0x00000800,
};

enum FarDialogSettings{
  FDIS_HISTORYINDIALOGEDITCONTROLS    = 0x00000001,
  FDIS_PERSISTENTBLOCKSINEDITCONTROLS = 0x00000002,
  FDIS_AUTOCOMPLETEININPUTLINES       = 0x00000004,
  FDIS_BSDELETEUNCHANGEDTEXT          = 0x00000008,
};

enum FarInterfaceSettings{
  FIS_CLOCKINPANELS                  = 0x00000001,
  FIS_CLOCKINVIEWERANDEDITOR         = 0x00000002,
  FIS_MOUSE                          = 0x00000004,
  FIS_SHOWKEYBAR                     = 0x00000008,
  FIS_ALWAYSSHOWMENUBAR              = 0x00000010,
  FIS_USERIGHTALTASALTGR             = 0x00000080,
  FIS_SHOWTOTALCOPYPROGRESSINDICATOR = 0x00000100,
  FIS_SHOWCOPYINGTIMEINFO            = 0x00000200,
  FIS_USECTRLPGUPTOCHANGEDRIVE       = 0x00000800,
};

enum FarConfirmationsSettings{
  FCS_COPYOVERWRITE                  = 0x00000001,
  FCS_MOVEOVERWRITE                  = 0x00000002,
  FCS_DRAGANDDROP                    = 0x00000004,
  FCS_DELETE                         = 0x00000008,
  FCS_DELETENONEMPTYFOLDERS          = 0x00000010,
  FCS_INTERRUPTOPERATION             = 0x00000020,
  FCS_DISCONNECTNETWORKDRIVE         = 0x00000040,
  FCS_RELOADEDITEDFILE               = 0x00000080,
  FCS_CLEARHISTORYLIST               = 0x00000100,
  FCS_EXIT                           = 0x00000200,
};

enum FarDescriptionSettings {
  FDS_UPDATEALWAYS                   = 0x00000001,
  FDS_UPDATEIFDISPLAYED              = 0x00000002,
  FDS_SETHIDDEN                      = 0x00000004,
  FDS_UPDATEREADONLY                 = 0x00000008,
};

#define FAR_CONSOLE_GET_MODE       (-2)
#define FAR_CONSOLE_TRIGGER        (-1)
#define FAR_CONSOLE_SET_WINDOWED   (0)
#define FAR_CONSOLE_SET_FULLSCREEN (1)
#define FAR_CONSOLE_WINDOWED       (0)
#define FAR_CONSOLE_FULLSCREEN     (1)

enum FAREJECTMEDIAFLAGS{
 EJECT_NO_MESSAGE                    = 0x00000001,
 EJECT_LOAD_MEDIA                    = 0x00000002,
#ifdef FAR_USE_INTERNALS
 EJECT_READY                         = 0x80000000,
#endif // END FAR_USE_INTERNALS
};

struct ActlEjectMedia {
  DWORD Letter;
  DWORD Flags;
};


enum FARKEYSEQUENCEFLAGS {
  KSFLAGS_DISABLEOUTPUT       = 0x00000001,
  KSFLAGS_NOSENDKEYSTOPLUGINS = 0x00000002,
};

struct KeySequence{
  DWORD Flags;
  int Count;
  DWORD *Sequence;
};

enum FARMACROCOMMAND{
  MCMD_LOADALL,
  MCMD_SAVEALL,
  MCMD_POSTMACROSTRING,
#ifdef FAR_USE_INTERNALS
  MCMD_COMPILEMACRO,
  MCMD_CHECKMACRO,
#endif // END FAR_USE_INTERNALS
};

struct ActlKeyMacro{
  int Command;
  union{
    struct {
      char *SequenceText;
      DWORD Flags;
    } PlainText;
#ifdef FAR_USE_INTERNALS
    struct KeySequence Compile;
    struct {
      const char *ErrMsg1;
      const char *ErrMsg2;
      const char *ErrMsg3;
    } MacroResult;
#endif // END FAR_USE_INTERNALS
    DWORD Reserved[3];
  } Param;
};

enum FARCOLORFLAGS{
  FCLR_REDRAW                 = 0x00000001,
};

struct FarSetColors{
  DWORD Flags;
  int StartIndex;
  int ColorCount;
  LPBYTE Colors;
};

enum WINDOWINFO_TYPE{
#ifdef FAR_USE_INTERNALS
  WTYPE_VIRTUAL,
  // ������� �� �������� ���������������� ���������
  // WTYPE_* � MODALTYPE_* (frame.hpp)!!!
  // (� �� ���� ������� ���� �����������, ���� �������� �� ��������� ;)
#endif // END FAR_USE_INTERNALS
  WTYPE_PANELS=1,
  WTYPE_VIEWER,
  WTYPE_EDITOR,
  WTYPE_DIALOG,
  WTYPE_VMENU,
  WTYPE_HELP,
#ifdef FAR_USE_INTERNALS
  WTYPE_COMBOBOX,
  WTYPE_USER,
#endif // END FAR_USE_INTERNALS
};

struct WindowInfo
{
  int  Pos;
  int  Type;
  int  Modified;
  int  Current;
  char TypeName[64];
  char Name[NM];
};

typedef int (WINAPI *FARAPIADVCONTROL)(
  int ModuleNumber,
  int Command,
  void *Param
);


#ifdef FAR_USE_INTERNALS
enum VIEWER_CONTROL_COMMANDS {
  VCTL_GETINFO,
  VCTL_QUIT,
  VCTL_REDRAW,
  VCTL_SETKEYBAR,
  VCTL_SETPOSITION,
  VCTL_SELECT,
};

enum VIEWER_OPTIONS {
  VOPT_SAVEFILEPOSITION=1,
  VOPT_AUTODETECTTABLE=2,
};

typedef union {
  __int64 i64;
  struct {
    DWORD LowPart;
    LONG  HighPart;
  } Part;
} FARINT64;

struct ViewerSelect
{
  FARINT64 BlockStartPos;
  int      BlockLen;
};

enum VIEWER_SETPOS_FLAGS {
  VSP_NOREDRAW    = 0x0001,
  VSP_PERCENT     = 0x0002,
  VSP_RELATIVE    = 0x0004,
  VSP_NORETNEWPOS = 0x0008,
};

struct ViewerSetPosition
{
  DWORD Flags;
  FARINT64 StartPos;
  int   LeftPos;
};

struct ViewerMode{
  int UseDecodeTable;
  int TableNum;
  int AnsiMode;
  int Unicode;
  int Wrap;
  int TypeWrap;
  int Hex;
  DWORD Reserved[4];
};

struct ViewerInfo
{
  int    StructSize;
  int    ViewerID;
  const char *FileName;
  FARINT64 FileSize;
  FARINT64 FilePos;
  int    WindowSizeX;
  int    WindowSizeY;
  DWORD  Options;
  int    TabSize;
  struct ViewerMode CurMode;
  int    LeftPos;
  DWORD  Reserved3;
};

typedef int (WINAPI *FARAPIVIEWERCONTROL)(
  int Command,
  void *Param
);

#define VE_READ     0
#define VE_CLOSE    1

#endif // END FAR_USE_INTERNALS

enum EDITOR_EVENTS {
  EE_READ,
  EE_SAVE,
  EE_REDRAW,
  EE_CLOSE
};

#define EEREDRAW_ALL    (void*)0
#define EEREDRAW_CHANGE (void*)1
#define EEREDRAW_LINE   (void*)2

enum EDITOR_CONTROL_COMMANDS {
  ECTL_GETSTRING,
  ECTL_SETSTRING,
  ECTL_INSERTSTRING,
  ECTL_DELETESTRING,
  ECTL_DELETECHAR,
  ECTL_INSERTTEXT,
  ECTL_GETINFO,
  ECTL_SETPOSITION,
  ECTL_SELECT,
  ECTL_REDRAW,
  ECTL_EDITORTOOEM,
  ECTL_OEMTOEDITOR,
  ECTL_TABTOREAL,
  ECTL_REALTOTAB,
  ECTL_EXPANDTABS,
  ECTL_SETTITLE,
  ECTL_READINPUT,
  ECTL_PROCESSINPUT,
  ECTL_ADDCOLOR,
  ECTL_GETCOLOR,
  ECTL_SAVEFILE,
  ECTL_QUIT,
  ECTL_SETKEYBAR,
  ECTL_PROCESSKEY,
  ECTL_SETPARAM,
  ECTL_GETBOOKMARKS,
  ECTL_TURNOFFMARKINGBLOCK,
  ECTL_DELETEBLOCK,
};

enum EDITOR_SETPARAMETER_TYPES {
  ESPT_TABSIZE,
  ESPT_EXPANDTABS,
  ESPT_AUTOINDENT,
  ESPT_CURSORBEYONDEOL,
  ESPT_CHARCODEBASE,
  ESPT_CHARTABLE,
  ESPT_SAVEFILEPOSITION,
  ESPT_LOCKMODE,
  ESPT_SETWORDDIV,
  ESPT_GETWORDDIV,
};

struct EditorSetParameter
{
  int Type;
  union {
    int iParam;
    char *cParam;
    DWORD Reserved1;
  } Param;
  DWORD Flags;
  DWORD Reserved2;
};

struct EditorGetString
{
  int StringNumber;
#ifdef FAR_USE_INTERNALS
  char *StringText;
  char *StringEOL;
#else // ELSE FAR_USE_INTERNALS
  const char *StringText;
  const char *StringEOL;
#endif // END FAR_USE_INTERNALS
  int StringLength;
  int SelStart;
  int SelEnd;
};


struct EditorSetString
{
  int StringNumber;
#ifdef FAR_USE_INTERNALS
  const char *StringText;
  const char *StringEOL;
#else // ELSE FAR_USE_INTERNALS
  char *StringText;
  char *StringEOL;
#endif // END FAR_USE_INTERNALS
  int StringLength;
};

enum EDITOR_OPTIONS {
  EOPT_EXPANDTABS        = 0x00000001,
  EOPT_PERSISTENTBLOCKS  = 0x00000002,
  EOPT_DELREMOVESBLOCKS  = 0x00000004,
  EOPT_AUTOINDENT        = 0x00000008,
  EOPT_SAVEFILEPOSITION  = 0x00000010,
  EOPT_AUTODETECTTABLE   = 0x00000020,
  EOPT_CURSORBEYONDEOL   = 0x00000040,
};


enum EDITOR_BLOCK_TYPES {
  BTYPE_NONE,
  BTYPE_STREAM,
  BTYPE_COLUMN
};

enum EDITOR_CURRENTSTATE {
  ECSTATE_MODIFIED       = 0x00000001,
  ECSTATE_SAVED          = 0x00000002,
  ECSTATE_LOCKED         = 0x00000004,
};


struct EditorInfo
{
  int EditorID;
  const char *FileName;
  int WindowSizeX;
  int WindowSizeY;
  int TotalLines;
  int CurLine;
  int CurPos;
  int CurTabPos;
  int TopScreenLine;
  int LeftPos;
  int Overtype;
  int BlockType;
  int BlockStartLine;
  int AnsiMode;
  int TableNum;
  DWORD Options;
  int TabSize;
  int BookMarkCount;
  DWORD CurState;
  DWORD Reserved[6];
};

struct EditorBookMarks
{
  long *Line;
  long *Cursor;
  long *ScreenLine;
  long *LeftPos;
  DWORD Reserved[4];
};

struct EditorSetPosition
{
  int CurLine;
  int CurPos;
  int CurTabPos;
  int TopScreenLine;
  int LeftPos;
  int Overtype;
};


struct EditorSelect
{
  int BlockType;
  int BlockStartLine;
  int BlockStartPos;
  int BlockWidth;
  int BlockHeight;
};


struct EditorConvertText
{
  char *Text;
  int TextLength;
};


struct EditorConvertPos
{
  int StringNumber;
  int SrcPos;
  int DestPos;
};


struct EditorColor
{
  int StringNumber;
  int ColorItem;
  int StartPos;
  int EndPos;
  int Color;
};

struct EditorSaveFile
{
  char FileName[NM];
  char *FileEOL;
};

typedef int (WINAPI *FARAPIEDITORCONTROL)(
  int Command,
  void *Param
);

enum INPUTBOXFLAGS{
  FIB_ENABLEEMPTY      = 0x00000001,
  FIB_PASSWORD         = 0x00000002,
  FIB_EXPANDENV        = 0x00000004,
  FIB_NOUSELASTHISTORY = 0x00000008,
  FIB_BUTTONS          = 0x00000010,
  FIB_NOAMPERSAND      = 0x00000020,
#ifdef FAR_USE_INTERNALS
  FIB_CHECKBOX         = 0x00010000,
  FIB_EDITPATH         = 0x01000000,
#endif // END FAR_USE_INTERNALS
};

typedef int (WINAPI *FARAPIINPUTBOX)(
  const char *Title,
  const char *SubTitle,
  const char *HistoryName,
  const char *SrcText,
  char *DestText,
  int   DestLength,
  const char *HelpTopic,
  DWORD Flags
);

// <C&C++>
typedef int     (WINAPIV *FARSTDSPRINTF)(char *Buffer,const char *Format,...);
typedef int     (WINAPIV *FARSTDSSCANF)(const char *Buffer, const char *Format,...);
// </C&C++>
typedef void    (WINAPI *FARSTDQSORT)(void *base, size_t nelem, size_t width, int (__cdecl *fcmp)(const void *, const void *));
typedef void    (WINAPI *FARSTDQSORTEX)(void *base, size_t nelem, size_t width, int (__cdecl *fcmp)(const void *, const void *,void *userparam),void *userparam);
typedef void   *(WINAPI *FARSTDBSEARCH)(const void *key, const void *base, size_t nelem, size_t width, int (__cdecl *fcmp)(const void *, const void *));
typedef int     (WINAPI *FARSTDGETFILEOWNER)(const char *Computer,const char *Name,char *Owner);
typedef int     (WINAPI *FARSTDGETNUMBEROFLINKS)(const char *Name);
typedef int     (WINAPI *FARSTDATOI)(const char *s);
typedef __int64 (WINAPI *FARSTDATOI64)(const char *s);
typedef char   *(WINAPI *FARSTDITOA64)(__int64 value, char *string, int radix);
typedef char   *(WINAPI *FARSTDITOA)(int value, char *string, int radix);
typedef char   *(WINAPI *FARSTDLTRIM)(char *Str);
typedef char   *(WINAPI *FARSTDRTRIM)(char *Str);
typedef char   *(WINAPI *FARSTDTRIM)(char *Str);
typedef char   *(WINAPI *FARSTDTRUNCSTR)(char *Str,int MaxLength);
typedef char   *(WINAPI *FARSTDTRUNCPATHSTR)(char *Str,int MaxLength);
typedef char   *(WINAPI *FARSTDQUOTESPACEONLY)(char *Str);
#ifdef FAR_USE_INTERNALS
typedef char*   (WINAPI *FARSTDPOINTTONAME)(char *Path);
#else // ELSE FAR_USE_INTERNALS
typedef char*   (WINAPI *FARSTDPOINTTONAME)(const char *Path);
#endif // END FAR_USE_INTERNALS
typedef void    (WINAPI *FARSTDGETPATHROOT)(const char *Path,char *Root);
typedef BOOL    (WINAPI *FARSTDADDENDSLASH)(char *Path);
typedef int     (WINAPI *FARSTDCOPYTOCLIPBOARD)(const char *Data);
typedef char   *(WINAPI *FARSTDPASTEFROMCLIPBOARD)(void);
typedef int     (WINAPI *FARSTDINPUTRECORDTOKEY)(const INPUT_RECORD *r);
typedef int     (WINAPI *FARSTDLOCALISLOWER)(unsigned Ch);
typedef int     (WINAPI *FARSTDLOCALISUPPER)(unsigned Ch);
typedef int     (WINAPI *FARSTDLOCALISALPHA)(unsigned Ch);
typedef int     (WINAPI *FARSTDLOCALISALPHANUM)(unsigned Ch);
typedef unsigned (WINAPI *FARSTDLOCALUPPER)(unsigned LowerChar);
typedef unsigned (WINAPI *FARSTDLOCALLOWER)(unsigned UpperChar);
typedef void    (WINAPI *FARSTDLOCALUPPERBUF)(char *Buf,int Length);
typedef void    (WINAPI *FARSTDLOCALLOWERBUF)(char *Buf,int Length);
typedef void    (WINAPI *FARSTDLOCALSTRUPR)(char *s1);
typedef void    (WINAPI *FARSTDLOCALSTRLWR)(char *s1);
typedef int     (WINAPI *FARSTDLOCALSTRICMP)(const char *s1,const char *s2);
typedef int     (WINAPI *FARSTDLOCALSTRNICMP)(const char *s1,const char *s2,int n);

#ifdef FAR_USE_INTERNALS
enum SETFILEAPISTO_TYPE{
  SFAT_APIS2OEM,
  SFAT_APIS2ANSI,
};
typedef void    (WINAPI *FARSETFILEAPISTO)(int Type);
#endif // END FAR_USE_INTERNALS

enum PROCESSNAME_FLAGS{
 PN_CMPNAME      = 0x00000000UL,
 PN_CMPNAMELIST  = 0x00001000UL,
 PN_GENERATENAME = 0x00002000UL,
 PN_SKIPPATH     = 0x00100000UL,
};

typedef int     (WINAPI *FARSTDPROCESSNAME)(const char *param1, char *param2, DWORD flags);

typedef void (WINAPI *FARSTDUNQUOTE)(char *Str);

typedef DWORD (WINAPI *FARSTDEXPANDENVIRONMENTSTR)(
  const char *src,
  char *dst,
  size_t size
);

enum XLATMODE{
  XLAT_SWITCHKEYBLAYOUT = 0x0000001UL,
  XLAT_SWITCHKEYBBEEP   = 0x0000002UL,
#ifdef FAR_USE_INTERNALS
  XLAT_USEKEYBLAYOUTNAME= 0x0000004UL,
#endif // END FAR_USE_INTERNALS
};

typedef char*   (WINAPI *FARSTDXLAT)(char *Line,int StartPos,int EndPos,const struct CharTableSet *TableSet,DWORD Flags);
typedef BOOL    (WINAPI *FARSTDKEYTOKEYNAME)(int Key,char *KeyText,int Size);
typedef int     (WINAPI *FARSTDKEYNAMETOKEY)(const char *Name);

typedef int (WINAPI *FRSUSERFUNC)(
  const WIN32_FIND_DATA *FData,
  const char *FullName,
  void *Param
);

enum FRSMODE{
  FRS_RETUPDIR             = 0x01,
  FRS_RECUR                = 0x02,
  FRS_SCANSYMLINK          = 0x04,
};

typedef void    (WINAPI *FARSTDRECURSIVESEARCH)(const char *InitDir,const char *Mask,FRSUSERFUNC Func,DWORD Flags,void *Param);
typedef char*   (WINAPI *FARSTDMKTEMP)(char *Dest,const char *Prefix);
typedef void    (WINAPI *FARSTDDELETEBUFFER)(char *Buffer);

enum MKLINKOP{
  FLINK_HARDLINK         = 1,
  FLINK_SYMLINK          = 2,
  FLINK_VOLMOUNT         = 3,

  FLINK_SHOWERRMSG       = 0x10000,
  FLINK_DONOTUPDATEPANEL = 0x20000,
};
typedef int     (WINAPI *FARSTDMKLINK)(const char *Src,const char *Dest,DWORD Flags);
typedef int     (WINAPI *FARCONVERTNAMETOREAL)(const char *Src,char *Dest, int DestSize);
typedef int     (WINAPI *FARGETREPARSEPOINTINFO)(const char *Src,char *Dest,int DestSize);

typedef struct FarStandardFunctions
{
  int StructSize;

  FARSTDATOI                 atoi;
  FARSTDATOI64               atoi64;
  FARSTDITOA                 itoa;
  FARSTDITOA64               itoa64;
  // <C&C++>
  FARSTDSPRINTF              sprintf;
  FARSTDSSCANF               sscanf;
  // </C&C++>
  FARSTDQSORT                qsort;
  FARSTDBSEARCH              bsearch;
  FARSTDQSORTEX              qsortex;

#ifdef FAR_USE_INTERNALS
  FARSETFILEAPISTO           SetFileApisTo;
  DWORD                      Reserved[8];
#else // ELSE FAR_USE_INTERNALS
  DWORD                      Reserved[9];
#endif // END FAR_USE_INTERNALS

  FARSTDLOCALISLOWER         LIsLower;
  FARSTDLOCALISUPPER         LIsUpper;
  FARSTDLOCALISALPHA         LIsAlpha;
  FARSTDLOCALISALPHANUM      LIsAlphanum;
  FARSTDLOCALUPPER           LUpper;
  FARSTDLOCALLOWER           LLower;
  FARSTDLOCALUPPERBUF        LUpperBuf;
  FARSTDLOCALLOWERBUF        LLowerBuf;
  FARSTDLOCALSTRUPR          LStrupr;
  FARSTDLOCALSTRLWR          LStrlwr;
  FARSTDLOCALSTRICMP         LStricmp;
  FARSTDLOCALSTRNICMP        LStrnicmp;

  FARSTDUNQUOTE              Unquote;
  FARSTDEXPANDENVIRONMENTSTR ExpandEnvironmentStr;
  FARSTDLTRIM                LTrim;
  FARSTDRTRIM                RTrim;
  FARSTDTRIM                 Trim;
  FARSTDTRUNCSTR             TruncStr;
  FARSTDTRUNCPATHSTR         TruncPathStr;
  FARSTDQUOTESPACEONLY       QuoteSpaceOnly;
  FARSTDPOINTTONAME          PointToName;
  FARSTDGETPATHROOT          GetPathRoot;
  FARSTDADDENDSLASH          AddEndSlash;
  FARSTDCOPYTOCLIPBOARD      CopyToClipboard;
  FARSTDPASTEFROMCLIPBOARD   PasteFromClipboard;
  FARSTDKEYTOKEYNAME         FarKeyToName;
  FARSTDKEYNAMETOKEY         FarNameToKey;
  FARSTDINPUTRECORDTOKEY     FarInputRecordToKey;
  FARSTDXLAT                 XLat;
  FARSTDGETFILEOWNER         GetFileOwner;
  FARSTDGETNUMBEROFLINKS     GetNumberOfLinks;
  FARSTDRECURSIVESEARCH      FarRecursiveSearch;
  FARSTDMKTEMP               MkTemp;
  FARSTDDELETEBUFFER         DeleteBuffer;
  FARSTDPROCESSNAME          ProcessName;
  FARSTDMKLINK               MkLink;
  FARCONVERTNAMETOREAL       ConvertNameToReal;
  FARGETREPARSEPOINTINFO     GetReparsePointInfo;
} FARSTANDARDFUNCTIONS;

struct PluginStartupInfo
{
  int StructSize;
  char ModuleName[NM];
  int ModuleNumber;
  const char *RootKey;
  FARAPIMENU             Menu;
  FARAPIDIALOG           Dialog;
  FARAPIMESSAGE          Message;
  FARAPIGETMSG           GetMsg;
  FARAPICONTROL          Control;
  FARAPISAVESCREEN       SaveScreen;
  FARAPIRESTORESCREEN    RestoreScreen;
  FARAPIGETDIRLIST       GetDirList;
  FARAPIGETPLUGINDIRLIST GetPluginDirList;
  FARAPIFREEDIRLIST      FreeDirList;
  FARAPIVIEWER           Viewer;
  FARAPIEDITOR           Editor;
  FARAPICMPNAME          CmpName;
  FARAPICHARTABLE        CharTable;
  FARAPITEXT             Text;
  FARAPIEDITORCONTROL    EditorControl;

  FARSTANDARDFUNCTIONS  *FSF;

  FARAPISHOWHELP         ShowHelp;
  FARAPIADVCONTROL       AdvControl;
  FARAPIINPUTBOX         InputBox;
  FARAPIDIALOGEX         DialogEx;
  FARAPISENDDLGMESSAGE   SendDlgMessage;
  FARAPIDEFDLGPROC       DefDlgProc;
#ifdef FAR_USE_INTERNALS
  DWORD                  Reserved;
  FARAPIVIEWERCONTROL    ViewerControl;
#else // ELSE FAR_USE_INTERNALS
  DWORD                  Reserved[2];
#endif // END FAR_USE_INTERNALS
};


enum PLUGIN_FLAGS {
  PF_PRELOAD        = 0x0001,
  PF_DISABLEPANELS  = 0x0002,
  PF_EDITOR         = 0x0004,
  PF_VIEWER         = 0x0008,
  PF_FULLCMDLINE    = 0x0010,
};


struct PluginInfo
{
  int StructSize;
  DWORD Flags;
  const char * const *DiskMenuStrings;
  int *DiskMenuNumbers;
  int DiskMenuStringsNumber;
  const char * const *PluginMenuStrings;
  int PluginMenuStringsNumber;
  const char * const *PluginConfigStrings;
  int PluginConfigStringsNumber;
  const char *CommandPrefix;
#ifdef FAR_USE_INTERNALS
  DWORD SysID;
#else // ELSE FAR_USE_INTERNALS
  DWORD Reserved;
#endif // END FAR_USE_INTERNALS
};


struct InfoPanelLine
{
  char Text[80];
  char Data[80];
  int  Separator;
};

struct PanelMode
{
  char  *ColumnTypes;
  char  *ColumnWidths;
  char **ColumnTitles;
  int    FullScreen;
  int    DetailedStatus;
  int    AlignExtensions;
  int    CaseConversion;
  char  *StatusColumnTypes;
  char  *StatusColumnWidths;
  DWORD  Reserved[2];
};


enum OPENPLUGININFO_FLAGS {
  OPIF_USEFILTER               = 0x00000001,
  OPIF_USESORTGROUPS           = 0x00000002,
  OPIF_USEHIGHLIGHTING         = 0x00000004,
  OPIF_ADDDOTS                 = 0x00000008,
  OPIF_RAWSELECTION            = 0x00000010,
  OPIF_REALNAMES               = 0x00000020,
  OPIF_SHOWNAMESONLY           = 0x00000040,
  OPIF_SHOWRIGHTALIGNNAMES     = 0x00000080,
  OPIF_SHOWPRESERVECASE        = 0x00000100,
  OPIF_FINDFOLDERS             = 0x00000200,
  OPIF_COMPAREFATTIME          = 0x00000400,
  OPIF_EXTERNALGET             = 0x00000800,
  OPIF_EXTERNALPUT             = 0x00001000,
  OPIF_EXTERNALDELETE          = 0x00002000,
  OPIF_EXTERNALMKDIR           = 0x00004000,
  OPIF_USEATTRHIGHLIGHTING     = 0x00008000,
};


enum OPENPLUGININFO_SORTMODES {
  SM_DEFAULT,
  SM_UNSORTED,
  SM_NAME,
  SM_EXT,
  SM_MTIME,
  SM_CTIME,
  SM_ATIME,
  SM_SIZE,
  SM_DESCR,
  SM_OWNER,
  SM_COMPRESSEDSIZE,
  SM_NUMLINKS
};


struct KeyBarTitles
{
  char *Titles[12];
  char *CtrlTitles[12];
  char *AltTitles[12];
  char *ShiftTitles[12];

  char *CtrlShiftTitles[12];
  char *AltShiftTitles[12];
  char *CtrlAltTitles[12];
};


enum OPERATION_MODES {
  OPM_SILENT     =0x0001,
  OPM_FIND       =0x0002,
  OPM_VIEW       =0x0004,
  OPM_EDIT       =0x0008,
  OPM_TOPLEVEL   =0x0010,
  OPM_DESCR      =0x0020,
  OPM_QUICKVIEW  =0x0040,
};

#define MAXSIZE_SHORTCUTDATA  8192

struct OpenPluginInfo
{
  int                   StructSize;
  DWORD                 Flags;
  const char           *HostFile;
  const char           *CurDir;
  const char           *Format;
  const char           *PanelTitle;
  const struct InfoPanelLine *InfoLines;
  int                   InfoLinesNumber;
  const char * const   *DescrFiles;
  int                   DescrFilesNumber;
  const struct PanelMode *PanelModesArray;
  int                   PanelModesNumber;
  int                   StartPanelMode;
  int                   StartSortMode;
  int                   StartSortOrder;
  const struct KeyBarTitles *KeyBar;
  const char           *ShortcutData;
  long                  Reserverd;
};

enum OPENPLUGIN_OPENFROM{
  OPEN_DISKMENU,
  OPEN_PLUGINSMENU,
  OPEN_FINDLIST,
  OPEN_SHORTCUT,
  OPEN_COMMANDLINE,
  OPEN_EDITOR,
  OPEN_VIEWER,
#ifdef FAR_USE_INTERNALS
  OPEN_FILEPANEL,
#endif // END FAR_USE_INTERNALS
};

enum FAR_PKF_FLAGS {
  PKF_CONTROL = 0x0001,
  PKF_ALT     = 0x0002,
  PKF_SHIFT   = 0x0004,
};

enum FAR_EVENTS {
  FE_CHANGEVIEWMODE,
  FE_REDRAW,
  FE_IDLE,
  FE_CLOSE,
  FE_BREAK,
  FE_COMMAND
};


#if defined(__BORLANDC__) || defined(_MSC_VER) || defined(__GNUC__) || defined(__WATCOMC__)
#ifdef __cplusplus
extern "C"{
#endif
// Exported Functions

void   WINAPI _export ClosePlugin(HANDLE hPlugin);
int    WINAPI _export Compare(HANDLE hPlugin,const struct PluginPanelItem *Item1,const struct PluginPanelItem *Item2,unsigned int Mode);
int    WINAPI _export Configure(int ItemNumber);
int    WINAPI _export DeleteFiles(HANDLE hPlugin,struct PluginPanelItem *PanelItem,int ItemsNumber,int OpMode);
void   WINAPI _export ExitFAR(void);
void   WINAPI _export FreeFindData(HANDLE hPlugin,struct PluginPanelItem *PanelItem,int ItemsNumber);
void   WINAPI _export FreeVirtualFindData(HANDLE hPlugin,struct PluginPanelItem *PanelItem,int ItemsNumber);
int    WINAPI _export GetFiles(HANDLE hPlugin,struct PluginPanelItem *PanelItem,int ItemsNumber,int Move,char *DestPath,int OpMode);
int    WINAPI _export GetFindData(HANDLE hPlugin,struct PluginPanelItem **pPanelItem,int *pItemsNumber,int OpMode);
int    WINAPI _export GetMinFarVersion(void);
void   WINAPI _export GetOpenPluginInfo(HANDLE hPlugin,struct OpenPluginInfo *Info);
void   WINAPI _export GetPluginInfo(struct PluginInfo *Info);
int    WINAPI _export GetVirtualFindData(HANDLE hPlugin,struct PluginPanelItem **pPanelItem,int *pItemsNumber,const char *Path);
int    WINAPI _export MakeDirectory(HANDLE hPlugin,char *Name,int OpMode);
HANDLE WINAPI _export OpenFilePlugin(char *Name,const unsigned char *Data,int DataSize);
HANDLE WINAPI _export OpenPlugin(int OpenFrom,int Item);
int    WINAPI _export ProcessEditorEvent(int Event,void *Param);
int    WINAPI _export ProcessEditorInput(const INPUT_RECORD *Rec);
int    WINAPI _export ProcessEvent(HANDLE hPlugin,int Event,void *Param);
int    WINAPI _export ProcessHostFile(HANDLE hPlugin,struct PluginPanelItem *PanelItem,int ItemsNumber,int OpMode);
int    WINAPI _export ProcessKey(HANDLE hPlugin,int Key,unsigned int ControlState);
int    WINAPI _export PutFiles(HANDLE hPlugin,struct PluginPanelItem *PanelItem,int ItemsNumber,int Move,int OpMode);
int    WINAPI _export SetDirectory(HANDLE hPlugin,const char *Dir,int OpMode);
int    WINAPI _export SetFindList(HANDLE hPlugin,const struct PluginPanelItem *PanelItem,int ItemsNumber);
void   WINAPI _export SetStartupInfo(const struct PluginStartupInfo *Info);

#ifdef FAR_USE_INTERNALS
int    WINAPI _export ProcessViewerEvent(int Event,void *Param);
#endif // END FAR_USE_INTERNALS

#ifdef __cplusplus
};
#endif
#endif

#if defined(__BORLANDC__)
  #pragma option -a.
#elif defined(__GNUC__) || (defined(__WATCOMC__) && (__WATCOMC__ < 1100)) || defined(__LCC__)
  #pragma pack()
#else
  #pragma pack(pop)
#endif

#endif /* __PLUGIN_HPP__ */
