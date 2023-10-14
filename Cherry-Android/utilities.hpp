#pragma once
#include <jni.h>
#include <unistd.h>
#include <cstdio>
#include <cstring>
#include <string>
#include <cstdlib>

typedef unsigned long DWORD;
static uintptr_t lib_base;

static DWORD get_lib_base(const char* library) {
    char filename[0xFF] = { 0 }, buffer[1024] = { 0 };

    FILE* fp = NULL;
    DWORD address = 0;

    sprintf(filename, "/proc/self/maps");

    fp = fopen(filename, "rt");
    if (fp == NULL) {
        perror("fopen");
        goto done;
    }

    while (fgets(buffer, sizeof(buffer), fp)) {
        if (strstr(buffer, library)) {
            address = (DWORD)strtoul(buffer, NULL, 16);
            goto done;
        }
    }

done:
    if (fp) {
        fclose(fp);
    }

    return address;
}

static DWORD rebase(DWORD relative_addr) {
    lib_base = get_lib_base(("libroblox.so"));

    if (lib_base == 0)
        return 0;

    return (reinterpret_cast<DWORD>(lib_base + relative_addr));
}

static DWORD get_lib_address(const char* lib, DWORD relative_addr)
{
    lib_base = get_lib_base(lib);

    if (lib_base == 0)
        return 0;

    return (reinterpret_cast<DWORD>(lib_base + relative_addr));
}

static bool is_library_loaded(const char* libraryName) {
    char line[512] = { 0 };
    FILE* fp = fopen(("/proc/self/maps"), ("rt"));
    if (fp != NULL) {
        while (fgets(line, sizeof(line), fp)) {
            std::string a = line;
            if (strstr(line, libraryName)) {
                return true;
            }
        }
        fclose(fp);
    }
    return false;
}


static jboolean is_game_lib_loaded(JNIEnv* env, jobject thiz) {
    return is_library_loaded(("libroblox.so"));
}