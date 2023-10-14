#pragma once
#include <Windows.h>
#include <string_view>

namespace cherry {
	class utilities;
	class console {
	public:
		enum consoleMode: std::intptr_t {
			normal = 0,
			info,
			warn,
			error
		};
	private:
		static console* singleton;
		consoleMode currentMode = consoleMode::normal;
		utilities* utils = nullptr;
	public:
		explicit console();
		static auto getSingleton() -> console*;
		auto getOutput() -> console&;
		auto write(std::string_view str) -> void;
		auto writeMode(std::string_view str, consoleMode mode) -> void;
		auto changeMode(consoleMode mode) -> void;

		const console& operator<<(std::string_view str) const;
		const console& operator<<(const char* str) const;
		const console& operator<<(consoleMode mode) const;
		const console& operator<<(uintptr_t num) const;
	};
}