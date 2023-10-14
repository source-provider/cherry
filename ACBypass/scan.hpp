#pragma once

#include <Windows.h>
#include <thread>
#include <format>
#include <array>
#include <vector>
#include <sstream>
#include <vector>
#include <winternl.h>

#include <string>
#include <functional>
#include <iostream>
#include <optional>

namespace utilities {
	namespace scan {
		inline auto scan(const char* const pattern, const char* const mask, std::uintptr_t start, std::uintptr_t end) -> std::vector<std::uintptr_t> {
			std::vector<std::uintptr_t> result;

			for (auto at = start; at < end; ++at) {
				const auto is_same = [&]() -> bool {
					for (auto i = 0u; i < std::strlen(mask); ++i) {
						if (*reinterpret_cast<std::uint8_t*>(at + i) != static_cast<std::uint8_t>(pattern[i]) && mask[i] != '?')
							return false;
					}

					return true;
				};

				if (is_same())
					result.push_back(at);
			}

			return result;
		}

		auto get_allocations() -> std::vector< MEMORY_BASIC_INFORMATION> {
			std::vector< MEMORY_BASIC_INFORMATION > allocations;

			std::uintptr_t addr = 0;

			MEMORY_BASIC_INFORMATION mbi;

			while (VirtualQuery(reinterpret_cast<std::uintptr_t*>(addr), &mbi, sizeof(mbi))) {
				if (mbi.State == MEM_COMMIT && mbi.Protect == PAGE_EXECUTE_READ)
					allocations.push_back(mbi);

				addr += mbi.RegionSize;
			}

			return allocations;
		}

		/*inline auto find_matching_funcs() -> std::optional<std::vector<std::uintptr_t>> {
			for (const auto& alloc : get_allocations())
				if (const auto result = scan("\xFF\x70\x08\x8B\x83\x00\x00\x00\x00\x03\x4C", "xxxxx????xx", reinterpret_cast<std::uintptr_t>(alloc.BaseAddress), reinterpret_cast<std::uintptr_t>(alloc.BaseAddress) + alloc.RegionSize); !result.empty())
					return result;

			return std::nullopt;
		}

		inline auto find_scan_reporter() -> std::uintptr_t {
			for (const auto& alloc : get_allocations()) {
				if (const auto result = scan("\x8B\x4D\x30\x85\xC9\x74\x00\x8B\x31", "xxxxxx?xx", reinterpret_cast<std::uintptr_t>(alloc.BaseAddress), reinterpret_cast<std::uintptr_t>(alloc.BaseAddress) + alloc.RegionSize); !result.empty())
					return result.front();
			}

			return 0;
		}*/
	}
}