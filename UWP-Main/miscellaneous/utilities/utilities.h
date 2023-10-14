#pragma once
#include <Windows.h>
#include <cstdint>
#include <string>
#include <sstream>
#include <filesystem>

namespace cherry {
	class utilities {
	private:
		static utilities* singleton;
	public:
		std::uintptr_t robloxBase = NULL;
		utilities() {
			this->robloxBase = reinterpret_cast<std::uintptr_t>(GetModuleHandle(NULL));
		}

		static auto getSingleton() -> utilities*;
		auto randomString(std::intptr_t len = 16) -> std::string;
		auto tostring(std::intptr_t num) -> std::string;
		auto tostring_double(double num) -> std::string;
		auto tostring_float(float num) -> std::string;
		auto replace(std::string subject, std::string search, std::string replace) -> std::string;
		auto getProcId() -> std::intptr_t;
		auto rebaseAddress(std::uintptr_t address, std::uintptr_t base = 0x400000) -> std::uintptr_t;
		auto equalsIgnoreCase(std::string_view a, std::string_view b) -> bool;
		auto robloxActive() -> bool;
		auto createFolder(std::string folderName) -> void;
		auto createFile(std::string fileName, std::string content) -> void;
	};
}