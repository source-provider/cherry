#pragma once
#include <Windows.h>
#include <cstdint>
#include <string>
#include <sstream> 
#include <iostream> 
#include <cmath>
#include <thread>
#include <queue>
#include <cstdio>
#include <tlhelp32.h>
#include <stdlib.h>
#include <filesystem>
#include <fstream>
#include <d3d11.h>
#include <optional>

#include <shlwapi.h>
#include <unordered_map>
#pragma comment(lib,"shlwapi.lib")

namespace cherry {
	class utilities {
	private:
		std::string loc = "";
	public:
		struct segment_t {
			char name[8];
			size_t size;
			int offset;
			int pad0[6];
		};

		struct section_t {
			int start;
			int clone;
			size_t size;
		};

		struct hasher_t
		{
			int entry;
			int enc;
			std::unordered_map<int, std::size_t> hashes;

			hasher_t() : entry(0), enc(0), hashes({}) {}
		};

		static auto get_singleton() -> utilities* {
			static utilities* _utilities = nullptr;

			if (_utilities == nullptr)
				_utilities = new utilities();

			return _utilities;
		}

		/* strings */
		auto random_string(int len = 16) -> std::string;
		auto tostring(int num) -> std::string;
		auto tostring_hex(int num) -> std::string;
		auto location(HMODULE h_module = NULL) -> std::string;
		auto replace(std::string subject, std::string search, std::string replace) -> std::string;
		auto read_file(std::string file_location) -> std::string;
		auto equals_ignore_case(std::string_view a, std::string_view b) -> bool;

		/* memory */
		auto sig_scan(std::string_view pattern, std::string_view mask, std::pair<std::uint32_t, std::uint32_t> bounds, int size)->std::vector<int>;
		auto get_section(std::string_view name, const bool clone = true) -> section_t;
		auto place_jmp(uintptr_t address, void* to, size_t nop_count = 0) -> void;
		auto get_encryption(int checker, size_t ori, int start, size_t size) -> int;

		/* others */
		auto get_proc_id(const char* name) -> int;
		auto roblox_active() -> bool;
		auto create_workspace() -> void;

	};
}