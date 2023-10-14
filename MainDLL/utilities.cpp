#include "utilities.hpp"

/* strings */
auto cherry::utilities::random_string(int len) -> std::string {
    static const char* chars = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZbcdefghijklmnopqrstuvwxyz";

    std::string str;
    str.reserve(len);

    for (int i = 0; i < len; ++i) {
        str += chars[rand() % (strlen(chars) - 1)];
    }

    return str;
}

auto cherry::utilities::tostring(int num) -> std::string {
    std::stringstream str;
    std::string result;

    str << num;
    str >> result;

    return result;
}

auto cherry::utilities::tostring_hex(int num) -> std::string {
    std::stringstream str;
    std::string result;

    str << std::hex << num;
    str >> result;

    return ("0x" + result);
}

auto cherry::utilities::location(HMODULE h_module) -> std::string {
    if (h_module == NULL)
        return this->loc;

    char buffer[MAX_PATH + 1];

    GetModuleFileNameA(h_module, buffer, MAX_PATH);

    std::string::size_type pos = std::string(buffer).find_last_of("\\/");
    this->loc = std::string(buffer).substr(0, pos).append("");

    return this->loc;
}

auto cherry::utilities::replace(std::string subject, std::string search, std::string replace) -> std::string {
    size_t pos = 0;

    while ((pos = subject.find(search, pos)) != std::string::npos)
    {
        subject.replace(pos, search.length(), replace);
        pos += replace.length();
    }

    return subject;
}

auto cherry::utilities::read_file(std::string file_location) -> std::string {
    auto close_file = [](FILE* f) { fclose(f); };
    auto holder = std::unique_ptr<FILE, decltype(close_file)>(fopen(file_location.c_str(), "rb"), close_file);

    if (!holder)
        return "";

    FILE* f = holder.get();

    if (fseek(f, 0, SEEK_END) < 0)
        return "";

    const long size = ftell(f);

    if (size < 0)
        return "";

    if (fseek(f, 0, SEEK_SET) < 0)
        return "";

    std::string res;
    res.resize(size);
    fread(const_cast<char*>(res.data()), 1, size, f);

    return res;
}

auto cherry::utilities::equals_ignore_case(std::string_view a, std::string_view b) -> bool {
    return std::equal(a.begin(), a.end(),
        b.begin(), b.end(),
        [](char a, char b) {
            return tolower(a) == tolower(b);
        }
    );
}


/* memory */
auto cherry::utilities::sig_scan(std::string_view pattern, std::string_view mask, std::pair<std::uint32_t, std::uint32_t> bounds, int size) -> std::vector<int> {
    std::vector<int> results = {};
    auto& [start, end] = bounds;

    while (start < end)
    {
        auto matching = true;

        for (auto iter = 0; iter < mask.length(); iter++)
        {
            if (*reinterpret_cast<std::uint8_t*>(start + iter) != static_cast<std::uint8_t>(pattern[iter]) && mask[iter] == 'x')
            {
                matching = false;
                break;
            }
        }

        if (matching)
        {
            results.emplace_back(start);
            if (results.size() == size) break;
        }

        ++start;
    }

    return results;
}

auto cherry::utilities::get_section(std::string_view name, const bool clone) -> cherry::utilities::section_t {
    section_t result = { 0, 0, 0 };

    int start = 0;
    const int base = reinterpret_cast<int>(GetModuleHandle(NULL));

    while (*reinterpret_cast<std::uint64_t*>(base + start) != 0x747865742E)
        start += 4;

    for (auto at = reinterpret_cast<segment_t*>(base + start); (at->offset != 0 && at->size != 0); at++)
    {
        if (!std::strncmp(at->name, name.data(), name.length() + 1))
        {
            const auto offset = (base + at->offset);
            const auto _clone = VirtualAlloc(nullptr, at->size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

            if (!_clone)
                throw std::runtime_error("VirtualAlloc for clone failed");

            result.start = offset;
            result.size = at->size;

            if (clone)
            {
                result.clone = reinterpret_cast<int>(_clone);
                std::memcpy(reinterpret_cast<void*>(result.clone), reinterpret_cast<void*>(result.start), at->size);
            }

            break;
        }
    }

    return result;
}

auto cherry::utilities::place_jmp(uintptr_t address, void* to, size_t nop_count) -> void {
    DWORD protection{ 0u };
    VirtualProtect(reinterpret_cast<void*>(address), 5 + nop_count, PAGE_EXECUTE_READWRITE, &protection);

    *(uint8_t*)(address) = 0xE9;
    *(uintptr_t*)(address + 1) = (uintptr_t)(to)-address - 5;

    for (size_t i = 0; i < nop_count; i++)
        *(uint8_t*)(address + 5 + i) = 0x90;

    VirtualProtect(reinterpret_cast<void*>(address), 5 + nop_count, protection, &protection);
}

auto cherry::utilities::get_encryption(int checker, size_t ori, int start, size_t size) -> int {
    uint8_t enc = 7;

    for (size_t i = 0; i < 16; i++) {
        int res = (i * 1236467);
        size_t hash_enc = reinterpret_cast<size_t(__cdecl*)(int, int, int, int)>(checker)(start, size, 0, res);

        if ((enc & 1) && hash_enc - res != ori)
            enc &= ~1; // add

        if ((enc & 2) && hash_enc + res != ori)
            enc &= ~2; // sub

        if ((enc & 4) && (hash_enc ^ res) != ori)
            enc &= ~4; // xor
    }

    switch (enc)
    {
        case 1:
            return 1;
        case 2:
            return 2;
        case 4:
            return 4;
    }

    return 0;
}

/* others */
auto cherry::utilities::get_proc_id(const char* name) -> int {
    PROCESSENTRY32 entry;
    entry.dwSize = sizeof(PROCESSENTRY32);

    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);

    if (Process32First(snapshot, &entry) == TRUE)
    {
        while (Process32Next(snapshot, &entry) == TRUE)
        {
            if (_stricmp(entry.szExeFile, name) == 0)
            {
                CloseHandle(snapshot);
                return entry.th32ProcessID;
            }
        }
    }

    CloseHandle(snapshot);
    return 0;
}

auto cherry::utilities::roblox_active() -> bool {
    int proc_id = this->get_proc_id("RobloxPlayerBeta.exe");
    HWND foreground = GetForegroundWindow();

    DWORD fproc_id;
    GetWindowThreadProcessId(foreground, &fproc_id);

    return (fproc_id == proc_id);
}

auto cherry::utilities::create_workspace() -> void {
    if (!std::filesystem::exists(this->location() + ("\\workspace")))
        std::filesystem::create_directory(this->location() + ("\\workspace"));
}