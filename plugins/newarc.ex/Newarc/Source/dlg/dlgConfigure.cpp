enum enumConfigure {
	ID_CFG_TITLE,
	ID_CFG_ARCHIVERSHOWALWAYS,
	ID_CFG_ARCHIVERSHOWEDITVIEW,
	ID_CFG_ARCHIVERSHOWNEVER,
	ID_CFG_SEPARATOR1,
	ID_CFG_OK,
	ID_CFG_CANCEL
};

void dlgConfigure(Configuration& cfg)
{
	FarDialog D (-1, -1, 78, 12);

	D.DoubleBox (3, 1, 74, 10, _M(MConfigCommonTitle));

	D.RadioButton (5, 2, (cfg.uArchiverOutput == ARCHIVER_OUTPUT_SHOW_ALWAYS), _M(MConfigCommonAlwaysShowOutput));
	D.RadioButton (5, 3, (cfg.uArchiverOutput == ARCHIVER_OUTPUT_SHOW_EDIT_VIEW), _M(MConfigCommonDontShowOutputWhenViewingEditing));
	D.RadioButton (5, 4, (cfg.uArchiverOutput == ARCHIVER_OUTPUT_SHOW_NEVER), _M(MConfigCommonNeverShowOutput));

	D.Separator (5);

	D.Text(5, 6, _T("Archivers path:"));
	D.Edit(5, 7, 20);

	D.Separator(8);

	D.Button (-1, 9, _M(MSG_cmn_B_OK));
	D.DefaultButton ();

	D.Button (-1, 9, _M(MSG_cmn_B_CANCEL));

	if ( D.Run() == D.FirstButton() )
	{
		if ( D.GetResultCheck(ID_CFG_ARCHIVERSHOWALWAYS) )
			cfg.uArchiverOutput = ARCHIVER_OUTPUT_SHOW_ALWAYS;

		if ( D.GetResultCheck(ID_CFG_ARCHIVERSHOWEDITVIEW) )
			cfg.uArchiverOutput = ARCHIVER_OUTPUT_SHOW_EDIT_VIEW;

		if ( D.GetResultCheck(ID_CFG_ARCHIVERSHOWNEVER) )
			cfg.uArchiverOutput = ARCHIVER_OUTPUT_SHOW_NEVER;
	}
}
