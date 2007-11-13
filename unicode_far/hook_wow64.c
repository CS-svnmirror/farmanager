#if !defined(_MSC_VER) || _MSC_VER < 1400 || defined(_WIN64)
#error
#endif

#pragma optimize("gty", on)

//-----------------------------------------------------------------------------
#include <windows.h>

static PVOID __declspec(thread) saveval;

typedef struct {
  BOOL (WINAPI *disable)(PVOID*);
  BOOL (WINAPI *revert)(PVOID);
}WOW;

static BOOL WINAPI e_disable(PVOID* p) { (void)p; return FALSE; }
static BOOL WINAPI e_revert(PVOID p) { (void)p; return FALSE; }

static const WOW wow = { e_disable, e_revert };

//-----------------------------------------------------------------------------
static void wow_restore(void)
{
    wow.revert(saveval);
}

static PVOID wow_disable(void)
{
    PVOID p;
    wow.disable(&p);
    return p;
}

static void wow_disable_and_save(void)
{
    saveval = wow_disable();
}

//-----------------------------------------------------------------------------
static void init_hook(void);
static void WINAPI HookProc(PVOID h, DWORD dwReason, PVOID u)
{
    (void)h;
    (void)u;
    switch(dwReason) {
      case DLL_PROCESS_ATTACH:
        init_hook();
      case DLL_THREAD_ATTACH:
        wow_disable_and_save();
      default:
        break;
    }
}
#pragma const_seg(".CRT$XLY")
#ifdef __cplusplus
extern "C"
#endif
const PIMAGE_TLS_CALLBACK hook_wow64_tlscb = HookProc;
#pragma const_seg()
// for ulink
#pragma comment(linker, "/include:_hook_wow64_tlscb")

//-----------------------------------------------------------------------------
static void __declspec(naked) hook_ldr(void)
{
  __asm {
        call    wow_restore
        pop     edx             // real call
        pop     eax             // real return
        pop     ecx             // arg1
        xchg    eax, [esp+8]    // real return <=> arg4
        xchg    eax, [esp+4]    // arg4 <=> arg3
        xchg    eax, [esp]      // arg3 <=> arg2
        push    eax             // arg2
        push    ecx             // arg1
        call    _l1
        push    eax     // answer
        call    wow_disable
        pop     eax
        retn
//-----
_l1:    push    240h
        jmp     edx
  }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
static void init_hook(void)
{
   static const wchar_t k32_w[] = L"kernel32", ntd_w[] = L"ntdll";
   static const char dis_c[] = "Wow64DisableWow64FsRedirection",
                     rev_c[] = "Wow64RevertWow64FsRedirection",
                     ldr_c[] = "LdrLoadDll";

    WOW rwow;
#pragma pack(1)
    struct {
      BYTE  cod;
      DWORD off;
    }data = { 0xE8, (DWORD)(SIZE_T)((LPCH)hook_ldr - sizeof(data)) };
#pragma pack()

    register union {
      HMODULE h;
      FARPROC f;
      LPVOID  p;
      DWORD   d;
    }ur;
    
    if(   (ur.h = GetModuleHandleW(k32_w)) == NULL
       || (*(FARPROC*)&rwow.disable = GetProcAddress(ur.h, dis_c)) == NULL
       || (*(FARPROC*)&rwow.revert = GetProcAddress(ur.h, rev_c)) == NULL
       || (ur.h = GetModuleHandleW(ntd_w)) == NULL
       || (ur.f = GetProcAddress(ur.h, ldr_c)) == NULL
       || *(LPDWORD)ur.p == 0x24086 // push 240h
       || ((LPBYTE)ur.p)[sizeof(DWORD)]) return;

    data.off -= ur.d;
    {
      register HANDLE cp = GetCurrentProcess();
      if(   !WriteProcessMemory(cp, ur.p, &data, sizeof(data), &data.off)
         || data.off != sizeof(data)) return;
      WriteProcessMemory(cp, (void*)&wow, &rwow, sizeof(wow), &data.off);
    }
}

//-----------------------------------------------------------------------------
