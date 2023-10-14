#include <Windows.h>
#pragma comment(lib, "psapi")

#include <Psapi.h>
#include <TlHelp32.h>

#include "io.hpp"
#include "scan.hpp"
#include "MinHook.h"

#define BASE 0xFC0000

/* NEW AC BYPASS */
/*
* REMINDER THE FIRST PROCESS HAS ITS AC TO!
* Suspend the second process
* inject dll
* Unlink module from PEB
* Resume the second process
* (HOOK ANYTHING IF NEEDED TOWARDS THE END)
*/

static __forceinline auto rebase(int x, int base = BASE) -> int {
    return (x - base + (uintptr_t)GetModuleHandleA(NULL));
}
 // 0x101F850

using GetModCloned_t = size_t(__stdcall*)(LPCVOID address, PMEMORY_BASIC_INFORMATION buffer, SIZE_T length);
GetModCloned_t old_GetModA = nullptr;
GetModCloned_t old_GetModATarget = nullptr;

std::thread::id thread_id;

auto init_suspend(HMODULE h_module) -> void {
    utilities::io::create("[ROBLOX] AC Bypass");

    ExitProcess(0);

   /* std::stringstream ss;
    ss << thread_id;
    uint64_t tid = std::stoull(ss.str());

    const std::shared_ptr<void> hThreadSnapshot(CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0), CloseHandle);

    if (hThreadSnapshot.get() == INVALID_HANDLE_VALUE) {
        throw std::runtime_error("GetMainThreadId failed");
    }

    THREADENTRY32 tEntry;
    tEntry.dwSize = sizeof(THREADENTRY32);
    DWORD result = 0;
    HANDLE our_thread_handle = 0;
    DWORD second_process = GetCurrentProcessId();

    std::cout << "[SECOND PROCESS]: " << second_process << std::endl;

    for (BOOL success = Thread32First(hThreadSnapshot.get(), &tEntry); !result && success && GetLastError() != ERROR_NO_MORE_FILES; success = Thread32Next(hThreadSnapshot.get(), &tEntry))
    {
        if (tEntry.th32OwnerProcessID == second_process) {
            if (tEntry.th32ThreadID == tid) {
                std::cout << "[OUR THREAD] " << tEntry.th32ThreadID << std::endl;
                our_thread_handle = OpenThread(THREAD_SUSPEND_RESUME, FALSE, tid);
            }
            else
            {
                HANDLE thr = OpenThread(THREAD_SUSPEND_RESUME, FALSE, tEntry.th32ThreadID);
                if (thr != INVALID_HANDLE_VALUE) {
                    std::cout << "[PROCESS THREAD FOUND]: " << tEntry.th32ThreadID << std::endl;
                    SuspendThread(thr);
                }
                
            }

        }
    }

    CloseHandle(hThreadSnapshot.get());
    SuspendThread(our_thread_handle);*/
}

size_t __stdcall GetModHook(LPCVOID address, PMEMORY_BASIC_INFORMATION buffer, SIZE_T length) {
    auto data = old_GetModA(address, buffer, length);
    
    std::cout << "HOOK WAS CALLED" << std::endl;

    return data;
}

auto init(HMODULE h_module) -> void {
    utilities::io::create("[ROBLOX] AC Bypass");

    MH_STATUS hook_stat = MH_Initialize();
    if (hook_stat != MH_STATUS::MH_OK) {
        std::cout << "[MINHOOK]: " << MH_StatusToString(hook_stat) << std::endl;
        return;
    }

    hook_stat = MH_CreateHookApiEx(L"kernel32", "VirtualQuery", &GetModHook, reinterpret_cast<void**>(&old_GetModA), reinterpret_cast<void**>(&old_GetModATarget));
    if (hook_stat != MH_STATUS::MH_OK) {
        std::cout << "[MINHOOK]: " << MH_StatusToString(hook_stat) << std::endl;
        return;
    }
   
    std::cout << "[OLD]: 0x" << std::hex << (int)old_GetModA << "\n";

    hook_stat = MH_EnableHook(MH_ALL_HOOKS);
    if (hook_stat != MH_STATUS::MH_OK) {
        std::cout << "[MINHOOK]: " << MH_StatusToString(hook_stat) << std::endl;
        return;
    }
}

auto APIENTRY DllMain(HMODULE h_module, DWORD  ul_reason_for_call, LPVOID lpReserved) -> BOOL { 
    if (ul_reason_for_call == DLL_PROCESS_ATTACH) {
        auto th = std::thread(init_suspend, h_module);
        thread_id = th.get_id();
        th.detach();
    }

    return TRUE;
}

