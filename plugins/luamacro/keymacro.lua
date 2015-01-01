-- coding: utf-8

local Shared = ...
local Msg, ErrMsg = Shared.Msg, Shared.ErrMsg
local MacroInit, MacroStep = Shared.MacroInit, Shared.MacroStep
local pack, loadmacro, utils = Shared.pack, Shared.loadmacro, Shared.utils
Shared = nil

local F = far.Flags

-- enum MACROPLUGINRETURNTYPE
local MPRT_NORMALFINISH, MPRT_ERRORFINISH, MPRT_HASNOMACRO =
      F.MPRT_NORMALFINISH, F.MPRT_ERRORFINISH, F.MPRT_HASNOMACRO

-- enum FARMACROSTATE
local MACROSTATE_NOMACRO, MACROSTATE_EXECUTING, MACROSTATE_EXECUTING_COMMON =
      F.MACROSTATE_NOMACRO, F.MACROSTATE_EXECUTING, F.MACROSTATE_EXECUTING_COMMON

-- unsigned __int64 MACROFLAGS_MFLAGS
local MFLAGS_ENABLEOUTPUT, MFLAGS_NOSENDKEYSTOPLUGINS, MFLAGS_POSTFROMPLUGIN =
      0x1, 0x2, 0x10000000

local type, setmetatable = type, setmetatable
local bit = bit or bit64
local band, bor, bxor, lshift = bit.band, bit.bor, bit.bxor, bit.lshift
local far_MacroCallFar = far.MacroCallFar

local far_FarMacroCallToLua = far.FarMacroCallToLua
far.FarMacroCallToLua = nil
--------------------------------------------------------------------------------

local MCODE_F_KEYMACRO = 0x80C68
local Import = {
  RestoreMacroChar        = function()  return far_MacroCallFar(MCODE_F_KEYMACRO, 1) end,
  ScrBufLock              = function()  return far_MacroCallFar(MCODE_F_KEYMACRO, 2) end,
  ScrBufUnlock            = function()  return far_MacroCallFar(MCODE_F_KEYMACRO, 3) end,
  ScrBufResetLockCount    = function()  return far_MacroCallFar(MCODE_F_KEYMACRO, 4) end,
  ScrBufGetLockCount      = function()  return far_MacroCallFar(MCODE_F_KEYMACRO, 5) end,
  ScrBufSetLockCount      = function(v) return far_MacroCallFar(MCODE_F_KEYMACRO, 6, v) end,
  GetUseInternalClipboard = function()  return far_MacroCallFar(MCODE_F_KEYMACRO, 7) end,
  SetUseInternalClipboard = function(v) return far_MacroCallFar(MCODE_F_KEYMACRO, 8, v) end,
  KeyNameToKey            = function(v) return far_MacroCallFar(MCODE_F_KEYMACRO, 9, v) end,
}
--------------------------------------------------------------------------------

local NewMacroRecord do
  local MacroRecord = {
    m_id     = 0,   -- идентификатор загруженного макроса в плагине LuaMacro; 0 для макроса, запускаемого посредством MSSC_POST.
    m_flags  = 0,   -- флаги макропоследовательности
    m_key    = -1,  -- назначенная клавиша
    m_value  = nil, -- значение, хранимое исполняющимся макросом
    m_handle = nil  -- хэндл исполняющегося макроса
  }
  local meta = { __index=MacroRecord }

  function MacroRecord:GetFlags() return self.m_flags end
  function MacroRecord:SetFlags(f) self.m_flags=f end
  function MacroRecord:GetHandle() return self.m_handle end
  function MacroRecord:SetHandle(handle) self.m_handle=handle end
  function MacroRecord:GetValue() return self.m_value end
  function MacroRecord:SetValue(val) self.m_value=val end

  NewMacroRecord = function (MacroId, Flags, Key)
    return setmetatable({m_id=MacroId, m_flags=Flags, m_key=Key}, meta)
  end
end
--------------------------------------------------------------------------------

-- Специализированная очередь, оптимизированная для очередей макросов.
-- Оптимизация по скорости и памяти, имеет смысл при добавлении в цикле сотен тысяч элементов.
-- Данная оптимизация накладывает следующие ограничения (соблюдаемые в нашем случае):
--   Элементы должны быть таблицами.
--   Очередь добавляет в элементы поле _next.
--   Нельзя добавлять элемент в очередь, если он уже имеется в этой очереди.
local NewQueue do
  local queue = {}
  local meta = { __index=queue }
  function queue:add(v)
    if self.last ~= nil then
      self.last._next = v
      self.last = v
    else
      self.first, self.last = v, v
    end
  end
  function queue:addqueue(q)
    if q.last ~= nil then
      if self.last ~= nil then
        self.last._next = q.first
        self.last = q.last
      else
        self.first, self.last = q.first, q.last
      end
    end
  end
  function queue:remove()
    if self.first ~= nil then
      self.first = self.first._next
      if self.first==nil then self.last=nil end
    end
  end
  function queue:empty() return self.first==nil end
  NewQueue = function() return setmetatable({}, meta) end
end
--------------------------------------------------------------------------------

local NewMacroState do
  local MacroState = {
    IntKey = 0, -- "описание реально нажатой клавиши"
    HistoryDisableMask = 0,
    UseInternalClipboard = false,
    Executing = MACROSTATE_NOMACRO,
    MacroQueue = nil
  }
  local meta = { __index=MacroState }
  function MacroState:GetCurMacro() return self.MacroQueue.first end
  function MacroState:RemoveCurMacro() self.MacroQueue:remove() end
  NewMacroState = function() return setmetatable({ MacroQueue=NewQueue() }, meta) end
end
--------------------------------------------------------------------------------

local NewStack do
  local stack = {}
  local meta = { __index=stack }
  function stack:top() return self[#self] end
  function stack:pop() local v=self[#self]; self[#self]=nil; return v; end
  function stack:push(v) self[#self+1]=v end
  function stack:empty() return self[1]==nil end
  NewStack = function() return setmetatable({}, meta) end
end
--------------------------------------------------------------------------------

local KeyMacro = {}
local CurState = NewMacroState()
local StateStack = NewStack()
local LastMessage = {}
local MacroIsRunning = 0
--------------------------------------------------------------------------------

local function GetCurMacro() return CurState:GetCurMacro() end
local function GetTopMacro() return StateStack[1] and StateStack:top():GetCurMacro() end
local function RemoveCurMacro() CurState:RemoveCurMacro() end

local function IsExecuting()
  local m = GetCurMacro()
  if m and m:GetHandle() then
    return band(m:GetFlags(),MFLAGS_NOSENDKEYSTOPLUGINS)~=0 and MACROSTATE_EXECUTING or MACROSTATE_EXECUTING_COMMON
  else
    return StateStack[1] and MACROSTATE_EXECUTING_COMMON or MACROSTATE_NOMACRO
  end
end

local function IsHistoryDisable (TypeHistory)
  local State = StateStack:top() or CurState
  return State:GetCurMacro() and band(State.HistoryDisableMask, lshift(1,TypeHistory))~=0 and 1 or 0
end

local function IsDisableOutput()
  local m = GetCurMacro()
  return m and band(m:GetFlags(),MFLAGS_ENABLEOUTPUT)==0 and 1 or 0
end

local function PushState (withClip)
  if withClip then
    CurState.UseInternalClipboard = Import.GetUseInternalClipboard()
  end
  StateStack:push(CurState)
  CurState = NewMacroState()
end

local function PopState (withClip)
  if StateStack[1] then
    StateStack:top().MacroQueue:addqueue(CurState.MacroQueue)
    CurState = StateStack:pop()
    if withClip then
      Import.SetUseInternalClipboard(CurState.UseInternalClipboard)
    end
  end
end

function KeyMacro.InitInternalVars (InitedRAM)
  if InitedRAM then
    CurState.MacroQueue = NewQueue()
    CurState.Executing = MACROSTATE_NOMACRO
  end
  CurState.HistoryDisableMask = 0
end

function KeyMacro.mmode (Action, Value)     -- N=MMode(Action[,Value])
  local TopMacro = GetTopMacro()
  if not TopMacro then return false end

  local result = 0
  local flags = TopMacro:GetFlags()
  if Action==1 then -- enable/disable output
    result = band(flags, MFLAGS_ENABLEOUTPUT)==1 and 0 or 1
    if (Value==0 or Value==1 or Value==2) and Value~=result then
      TopMacro:SetFlags(bxor(flags, MFLAGS_ENABLEOUTPUT))
      far.Text() -- M#2389: mmode(1,x): вывод на экран включается/отключается не вовремя
    end
  elseif Action==2 then -- get MacroRecord flags
    result = bor(lshift(flags,8), 0xFF)
  end
  return result
end

local function CheckCurMacro()
  local macro = GetCurMacro()
  if macro then
    if not macro:GetHandle() then
      PushState(false)
      macro:SetHandle(MacroInit(macro.m_id))
      PopState(false)
    end
    if macro:GetHandle() then
      return macro
    else
      RemoveCurMacro()
      Import.RestoreMacroChar()
    end
  end
end

local function ACall (macro, param)
  local EntryStackSize = #StateStack
  macro:SetValue(true)
  PushState(true)

  local lockCount = Import.ScrBufGetLockCount()
  Import.ScrBufSetLockCount(0)

  local Result = pack(param[1](unpack(param,2,param.n)))

  Import.ScrBufSetLockCount(lockCount)

  if #StateStack > EntryStackSize then
    PopState(true)
    macro:SetValue(Result)
  end
end

local function CallStep()
  while true do
    local macro = CheckCurMacro()
    if not macro then return end

    Import.ScrBufResetLockCount()

    PushState(false)
    local r1,r2
    local value, handle = macro:GetValue(), macro:GetHandle()
    if type(value) == "userdata" then
      r1,r2 = MacroStep(handle, far_FarMacroCallToLua(value))
    elseif type(value) == "table" then
      r1,r2 = MacroStep(handle, unpack(value,1,value.n))
    elseif value ~= nil then
      r1,r2 = MacroStep(handle, value)
    else
      r1,r2 = MacroStep(handle)
    end
    PopState(false)
    macro:SetValue(nil)

    if r1 == MPRT_NORMALFINISH or r1 == MPRT_ERRORFINISH then
      if band(macro:GetFlags(),MFLAGS_ENABLEOUTPUT) == 0 then
        Import.ScrBufUnlock()
      end
      RemoveCurMacro()
      if not GetCurMacro() then
        Import.RestoreMacroChar()
      end
    else
      if band(macro:GetFlags(),MFLAGS_ENABLEOUTPUT) == 0 then
        Import.ScrBufLock()
      end
      return r1, r2
    end
  end
end

local function GetInputFromMacro()
  if utils.LoadingInProgress() then
    return false
  end
  if MacroIsRunning==0 and not GetCurMacro() then
    if StateStack[1] then
      PopState(true)
      return false
    else
      return MPRT_HASNOMACRO
    end
  end
  MacroIsRunning = MacroIsRunning + 1
  local r1,r2 = CallStep()
  MacroIsRunning = MacroIsRunning - 1

  if r1 == "acall" then
    ACall(GetCurMacro(), r2)
    return GetInputFromMacro() -- tail recursion
  end

  return r1,r2
end

function KeyMacro.PostNewMacro (macroId, flags, key, postFromPlugin)
  flags = flags or 0
  flags = postFromPlugin and bor(flags,MFLAGS_POSTFROMPLUGIN) or flags
  local AKey = 0
  if key then
    local dKey = Import.KeyNameToKey(key)
    if dKey ~= -1 then
      AKey = dKey
    end
  end
  CurState.MacroQueue:add(NewMacroRecord(macroId,flags,AKey))
  return true
end

local function TryToPostMacro (Mode, TextKey, IntKey)
  local m = utils.GetMacro(Mode, TextKey, true, false)
  if m then
    if m.id then
      KeyMacro.PostNewMacro(m.id, m.flags, TextKey, false)
      CurState.HistoryDisableMask = 0
      CurState.IntKey = IntKey
    end
    return true
  end
end

function KeyMacro.DisableHistory (Mask)
  Mask = type(Mask)=="number" and math.floor(Mask)
  local t = StateStack:top()
  local oldHistoryDisable = t and t.HistoryDisableMask or 0
  if t and Mask then t.HistoryDisableMask = Mask end
  return oldHistoryDisable
end

local OP_PUSHSTATE          =  1
local OP_POPSTATE           =  2
local OP_ISEXECUTING        =  3
local OP_ISDISABLEOUTPUT    =  4
local OP_HISTORYDISABLEMASK =  5
local OP_ISHISTORYDISABLE   =  6
local OP_GETCURMACRO        =  7
local OP_GETTOPMACRO        =  8
local OP_ISMACROQUEUEEMPTY  =  9
local OP_GETSTATESTACKSIZE  = 10
local OP_GETINTKEY          = 11
local OP_POSTNEWMACRO       = 12
local OP_GETTOPINTKEY       = 13
local OP_SETMACROVALUE      = 14
local OP_GETINPUTFROMMACRO  = 15
local OP_TRYTOPOSTMACRO     = 16

function KeyMacro.Dispatch (opcode, ...)
  local p1 = (...)
  if opcode == OP_ISEXECUTING then
    return IsExecuting()
  elseif opcode == OP_GETINPUTFROMMACRO then
    return GetInputFromMacro()
  elseif opcode == OP_PUSHSTATE then
    PushState(p1)
  elseif opcode == OP_POPSTATE then
    PopState(p1)
  elseif opcode == OP_ISDISABLEOUTPUT then
    return IsDisableOutput()
  elseif opcode == OP_HISTORYDISABLEMASK then
    local OldMask = CurState.HistoryDisableMask
    if p1 then CurState.HistoryDisableMask = p1 end
    return OldMask
  elseif opcode == OP_ISHISTORYDISABLE then
    return IsHistoryDisable(p1)
  elseif opcode == OP_GETCURMACRO or opcode==OP_GETTOPMACRO then
    local mr
    if opcode==OP_GETCURMACRO then mr=GetCurMacro() else mr=GetTopMacro() end
    if mr then
      LastMessage = pack(mr:GetFlags(), mr.m_key)
      return MPRT_NORMALFINISH, LastMessage
    end
  elseif opcode == OP_ISMACROQUEUEEMPTY then
    return GetCurMacro() and 0 or 1
  elseif opcode == OP_GETSTATESTACKSIZE then
    return #StateStack
  elseif opcode == OP_GETINTKEY then
    return CurState.IntKey
  elseif opcode == OP_POSTNEWMACRO then
    local Lang,Code,Flags,AKey = ...
    local f1,f2 = loadmacro(Lang,Code)
    if f1 then
      CurState.MacroQueue:add(NewMacroRecord({ f1,f2,HasFunction=true },Flags,AKey))
      return true
    else
      ErrMsg(f2, Msg.MMacroParseErrorTitle)
    end
  elseif opcode == OP_GETTOPINTKEY then
    local t=StateStack:top()
    return t and t.IntKey or 0
  elseif opcode == OP_SETMACROVALUE then
    local m = GetCurMacro()
    if m then m:SetValue(p1) end
  elseif opcode == OP_TRYTOPOSTMACRO then
    return TryToPostMacro(...)
  end
end

return KeyMacro
