#include "structs.h"
#include "xor.h"
#include <iostream>
#include <fstream>
#include <filesystem>
#include <thread>
#include <chrono>
#include <stdio.h>    
#include <psapi.h>
#include <vector>
#pragma comment( lib, "psapi.lib" )

#pragma warning(suppress : 4996)

#define end_log \
if (std::filesystem::is_regular_file(target_path) == false) { \
    std::ofstream out(target_path);\
    out << output;\
    out.close();\
}\
else {\
    std::ofstream out;\
    out.open(target_path, std::ios_base::app | std::ios_base::binary);\
    out.write(output.c_str(), output.size());\
    out.close();\
}

auto number_to_string_hex(uintptr_t data) -> std::string {
    std::stringstream buffer;
    std::string output;

    buffer << "0x" << std::hex << data;
    buffer >> output;

    return output;
}

auto number_to_string(uintptr_t data) -> std::string {
    std::stringstream buffer;
    std::string output;

    buffer << data;
    buffer >> output;

    return output;
}

auto main_thread() -> int {
    std::string output = XorString("");
    std::string new_line = XorString("\n");
    std::string target_path = XorString("D:\\etc\\Sources\\Cherry\\x64\\Release");
    std::string file_name = XorString("\\log.txt");
    std::string dll_location = XorString("RobloxPlayerBeta.dll");
    std::string log1 = XorString("");

    if (std::filesystem::is_regular_file(target_path) == true) {
        output += new_line;
    }

    uint64_t seconds = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    output += "[ CHERRY ] Process id " + number_to_string(_getpid()) + new_line;
    output += "[ CHERRY ] Unix time " + number_to_string(seconds) + new_line;

    target_path += file_name;

    uintptr_t image_base = reinterpret_cast<uintptr_t>(GetModuleHandle(NULL));
    if (!image_base) {
        std::string err1 = XorString("[ CHERRY ]: Could not find Process Base Address.\n");
        output += err1;
        end_log
        return 0;
    }
    log1 = XorString("[ CHERRY ] RobloxPlayerBeta.exe Base Address: ");
    log1 += number_to_string_hex(image_base);
    log1 += new_line;
    output += log1;

    const char* module_name = dll_location.c_str();
    uintptr_t module_handle = reinterpret_cast<uintptr_t>(GetModuleHandle(module_name));
    if (!module_handle) {
        std::string err1 = XorString("[ CHERRY ] Could not find RobloxPlayerBeta.dll\n");
        output += err1;
        end_log
        return 0;
    }

    log1 = XorString("[ CHERRY ] RobloxPlayerBeta.dll Base Address: ");
    log1 += number_to_string_hex(module_handle);
    log1 += new_line;
    output += log1;

    //0x48, 0x8B, 0x07, 0xC3

    std::vector<uintptr_t> output1{};
    //output1 = sigscan::scan("\x48\x8B\x07", "xxx", module_handle, (module_handle + 0x5000));
    output += "[ CHERRY ] Process Image Size: " + number_to_string_hex(0x5000) + "\n";


    output += "[ CHERRY ] Gadget Size: " + number_to_string(output1.size()) + "\n";

    for (auto i : output1) {
        std::string log2 = XorString("[ CHERRY ] Gadget found: ");
        log2 += number_to_string_hex(i);
        log2 += new_line;
        output += log2;
    }

    output += "[ CHERRY ] eof\n\n";
    end_log
    return 0;
}

auto __stdcall DllMain(HMODULE hmodule, DWORD  call_reason, LPVOID lp_reserved) -> BOOL {
    if (call_reason == DLL_PROCESS_ATTACH) {
        std::thread{ main_thread }.detach();
    }
    return TRUE;
}

