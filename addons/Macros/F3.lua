Macro {
  area="Shell"; key="F3"; flags="DisableOutput|NoPluginPanels|NoFolders"; description="Use internal editor as viewer"; action = function()

Keys('F4')
if Area.Dialog then Keys('Enter') end
if Area.Editor and bit64.band(Editor.State,0x200)~=0x200 then Keys('CtrlL') end

  end;
}

