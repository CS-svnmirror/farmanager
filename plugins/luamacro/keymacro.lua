-- coding: utf-8

local Shared = ...
local pack, utils = Shared.pack, Shared.utils
local MacroInit, MacroStep = Shared.MacroInit, Shared.MacroStep

local F = far.Flags
local bit = bit or bit64
local band,bor,bxor,lshift = bit.band,bit.bor,bit.bxor,bit.lshift
local FarMacroCallToLua = far.FarMacroCallToLua
far.FarMacroCallToLua = nil
local LastMessage = {}
local MacroPluginIsRunning = 0

local MACROMODE_NOMACRO          =0  -- не в режиме макро
local MACROMODE_EXECUTING        =1  -- исполнение: без передачи плагину пимп
local MACROMODE_EXECUTING_COMMON =2  -- исполнение: с передачей плагину пимп

local MFLAGS_ENABLEOUTPUT        = 0x00000001 -- не подавлять обновление экрана во время выполнения макроса
local MFLAGS_NOSENDKEYSTOPLUGINS = 0x00000002 -- не передавать плагинам клавиши во время записи/воспроизведения макроса
local MFLAGS_POSTFROMPLUGIN      = 0x10000000 -- последовательность пришла от АПИ

local MCODE_F_KEYMACRO = 0x80C68
local Import = {
  RestoreMacroChar        = function()    far.MacroCallFar(MCODE_F_KEYMACRO, 1) end,
  ScrBufLock              = function()    far.MacroCallFar(MCODE_F_KEYMACRO, 2) end,
  ScrBufUnlock            = function()    far.MacroCallFar(MCODE_F_KEYMACRO, 3) end,
  ScrBufResetLockCount    = function()    far.MacroCallFar(MCODE_F_KEYMACRO, 4) end,
  GetUseInternalClipboard = function()    return far.MacroCallFar(MCODE_F_KEYMACRO, 5) end,
  SetUseInternalClipboard = function(v)   far.MacroCallFar(MCODE_F_KEYMACRO, 6, v) end,
  KeyNameToKey            = function(v)   return far.MacroCallFar(MCODE_F_KEYMACRO, 7, v) end,
}

local MacroRecord = {
  m_lang="lua",     -- Язык макропоследовательности
  m_flags=0,        -- Флаги макропоследовательности
  m_key=-1,         -- Назначенная клавиша
  m_code="",        -- оригинальный "текст" макроса
  m_macroId=0,      -- Идентификатор загруженного макроса в плагине LuaMacro; 0 для макроса, запускаемого посредством MSSC_POST.
  m_macrovalue=nil, -- Значение, хранимое исполняющимся макросом
  m_handle=0        -- Хэндл исполняющегося макроса
}
local meta_MacroRecord = { __index=MacroRecord }

local function NewMacroRecord(Lang, Flags, MacroId, Key, Code)
  local mr = {
    m_lang=Lang,
    m_flags=Flags,
    m_key=Key,
    m_code=Code,
    m_macroId=MacroId,
    m_macrovalue=nil,
    m_handle=0
  }
  return setmetatable(mr, meta_MacroRecord)
end

function MacroRecord:Flags() return self.m_flags end
function MacroRecord:GetHandle() return self.m_handle end
function MacroRecord:SetHandle(handle) self.m_handle=handle end
function MacroRecord:GetValue() return self.m_macrovalue end
function MacroRecord:SetValue(val) self.m_macrovalue=val end
--------------------------------------------------------------------------------

local MacroState = {
  IntKey=0, -- "описание реально нажатой клавиши"
  Executing=0,
  m_MacroQueue=nil,
  HistoryDisable=0,
  UseInternalClipboard=false
}
local meta_MacroState = { __index=MacroState }
local function NewMacroState()
  return setmetatable({ m_MacroQueue={} }, meta_MacroState)
end
function MacroState:GetCurMacro() return self.m_MacroQueue[1] end
function MacroState:RemoveCurMacro() table.remove(self.m_MacroQueue, 1) end
--------------------------------------------------------------------------------

local stack = {}
local meta_stack = { __index=stack }
local function NewStack() return setmetatable({}, meta_stack) end
function stack:top() return self[#self] end
function stack:pop() local v=self[#self]; self[#self]=nil; return v; end
function stack:push(v) self[#self+1]=v end
function stack:empty() return self[1]==nil end
--------------------------------------------------------------------------------

local KeyMacro = {
  m_StateStack = NewStack(),
  m_CurState = NewMacroState(),
}

function KeyMacro:GetCurMacro() return self.m_CurState:GetCurMacro() end
function KeyMacro:GetTopMacro() return self.m_StateStack[1] and self.m_StateStack:top():GetCurMacro() end
function KeyMacro:RemoveCurMacro() self.m_CurState:RemoveCurMacro() end

function KeyMacro:IsExecuting()
  local m = self:GetCurMacro()
  if m and m:GetHandle()~=0 then
    return band(m:Flags(),MFLAGS_NOSENDKEYSTOPLUGINS)~=0 and MACROMODE_EXECUTING or MACROMODE_EXECUTING_COMMON
  else
    return self.m_StateStack[1] and MACROMODE_EXECUTING_COMMON or MACROMODE_NOMACRO
  end
end

function KeyMacro:GetHistoryDisableMask()
  return self.m_CurState.HistoryDisable
end

function KeyMacro:SetHistoryDisableMask (Mask)
  local OldHistoryDisable = self.m_CurState.HistoryDisable
  self.m_CurState.HistoryDisable = Mask
  return OldHistoryDisable
end

function KeyMacro:IsHistoryDisable (TypeHistory)
  return self:GetCurMacro() and
    band(self.m_CurState.HistoryDisable, lshift(1,TypeHistory))~=0 and 1 or 0
end

function KeyMacro:IsDisableOutput()
  local m = self:GetCurMacro()
  return m and band(m:Flags(),MFLAGS_ENABLEOUTPUT)==0 and 1 or 0
end

function KeyMacro:PushState (withClip)
  if withClip then
    self.m_CurState.UseInternalClipboard = Import.GetUseInternalClipboard()
  end
  self.m_StateStack:push(self.m_CurState)
  self.m_CurState = NewMacroState()
end

function KeyMacro:PopState (withClip)
  if self.m_StateStack[1] then
    local q = self.m_StateStack:top().m_MacroQueue
    for i,v in ipairs(self.m_CurState.m_MacroQueue) do
      q[#q+1] = v
    end
    self.m_CurState = self.m_StateStack:pop()
    if withClip then
      Import.SetUseInternalClipboard(self.m_CurState.UseInternalClipboard)
    end
  end
end

-- инициализация всех переменных
function KeyMacro:InitInternalVars (InitedRAM)
  if InitedRAM then
    self.m_CurState.m_MacroQueue = {}
    self.m_CurState.Executing = MACROMODE_NOMACRO
  end
  self.m_CurState.HistoryDisable = 0
end

function KeyMacro:mmode (Action, nValue)     -- N=MMode(Action[,Value])
  local TopMacro = self:GetTopMacro()
  if not TopMacro then return false end

  if Action==1 then -- DisableOutput
    local Result = band(TopMacro:Flags(),MFLAGS_ENABLEOUTPUT)==1 and 0 or 1
    nValue = type(nValue)=="number" and math.floor(nValue)
    if nValue and nValue>=0 and nValue<=2 and nValue~=Result then
      TopMacro.m_flags = bxor(TopMacro.m_flags, MFLAGS_ENABLEOUTPUT)
    end
    return Result

  elseif Action==2 then -- Get MacroRecord Flags
    return bor(lshift(TopMacro:Flags(),8), 0xFF)
  end

  return 0
end

function KeyMacro:CheckCurMacro()
  local macro = self:GetCurMacro()
  if macro then
    local handle = macro:GetHandle()
    if handle == 0 then
      handle = MacroInit(macro.m_macroId, macro.m_lang, macro.m_code)
      if handle then macro:SetHandle(handle) end
    end
    if handle and handle ~= 0 then
      return macro
    end
    self:RemoveCurMacro()
    Import.RestoreMacroChar()
  end
end

function KeyMacro:CallStep()
  while true do
    local macro = self:CheckCurMacro()
    if not macro then return end

    Import.ScrBufResetLockCount()

    self:PushState(false)
    local r1,r2
    local value, handle = macro:GetValue(), macro:GetHandle()
    if type(value) == "userdata" then
      r1,r2 = MacroStep(handle, FarMacroCallToLua(value))
    elseif value ~= nil then
      r1,r2 = MacroStep(handle, value)
    else
      r1,r2 = MacroStep(handle)
    end
    self:PopState(false)
    macro:SetValue(nil)

    if not (r1==F.MPRT_NORMALFINISH or r1==F.MPRT_ERRORFINISH) then
      if band(macro:Flags(),MFLAGS_ENABLEOUTPUT)==0 then Import.ScrBufLock() end
      return r1, r2
    end

    if band(macro:Flags(),MFLAGS_ENABLEOUTPUT)==0 then Import.ScrBufUnlock() end

    self:RemoveCurMacro()

    if not self:GetCurMacro() then
      Import.RestoreMacroChar()
    end
  end
end

function KeyMacro:PostNewMacro (macroId, code, flags, key, postFromPlugin)
  if macroId and macroId~=0 then
    flags = flags or 0
    flags = postFromPlugin and bor(flags,MFLAGS_POSTFROMPLUGIN) or flags
    local AKey = 0
    if key then
      local dKey = Import.KeyNameToKey(key)
      if dKey ~= -1 then
        AKey = dKey
      end
    end
    table.insert(self.m_CurState.m_MacroQueue, NewMacroRecord("lua",flags,macroId,AKey,code))
    return true
  end
  return false
end

function KeyMacro:TryToPostMacro (Mode, TextKey, IntKey)
  local m = utils.GetMacro(Mode, TextKey, true, false)
  if m then
    if m.id ~= 0  then
      self:PostNewMacro(m.id, m.code, m.flags, TextKey, false)
      self:SetHistoryDisableMask(0)
      self.m_CurState.IntKey = IntKey
    end
    return true
  end
end

function KeyMacro:Dispatch (opcode, ...)
  local p1 = (...)
  if     opcode==1  then self:PushState(p1)
  elseif opcode==2  then self:PopState(p1)
  elseif opcode==3  then self:InitInternalVars(p1)
  elseif opcode==4  then return self:IsExecuting()
  elseif opcode==5  then return self:IsDisableOutput()
  elseif opcode==6  then return self:SetHistoryDisableMask(p1)
  elseif opcode==7  then return self:GetHistoryDisableMask()
  elseif opcode==8  then return self:IsHistoryDisable(p1)
  elseif opcode==9 or opcode==10 then
    local mr
    if opcode==9 then mr=self:GetCurMacro() else mr=self:GetTopMacro() end
    if mr then
      LastMessage = pack(mr.m_flags, mr.m_key)
      return F.MPRT_NORMALFINISH, LastMessage
    end
  elseif opcode==11 then return self:GetCurMacro()==nil
  elseif opcode==12 then return #self.m_StateStack
  elseif opcode==13 then return self.m_CurState.IntKey
  elseif opcode==14 then
    table.insert(self.m_CurState.m_MacroQueue, NewMacroRecord(...))
    return true
  elseif opcode==15 then local t=self.m_StateStack:top() return t and t.IntKey or 0
  elseif opcode==16 then
    local t = self.m_StateStack:top()
    local oldHistoryDisable = t and t.HistoryDisable or 0
    if t and p1 then t.HistoryDisable = p1 end
    return oldHistoryDisable
  elseif opcode==17 then
    local m=self:GetCurMacro()
    if m then m:SetValue(p1) end

  elseif opcode==18 then
    if MacroPluginIsRunning==0 and not self:GetCurMacro() then
      if self.m_StateStack[1] then
        self:PopState(true)
        return false
      else
        return F.MPRT_HASNOMACRO
      end
    end
    MacroPluginIsRunning = MacroPluginIsRunning + 1
    local r1,r2 = self:CallStep()
    MacroPluginIsRunning = MacroPluginIsRunning - 1
    return r1,r2

  elseif opcode==19 then return self:TryToPostMacro(...)
  end
end

return KeyMacro
