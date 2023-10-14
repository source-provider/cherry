#include <Windows.h>
#include <string>
#include <iostream>

#ifndef CHERRY_CONSOLE 
#define CHERRY_CONSOLE

#include "configuration.hpp"

namespace cherry
{
	class console {
	private:
		FILE* file_ptr = nullptr;
		HANDLE std_console = nullptr;
		HWND console_window = nullptr;
		DWORD old_protection;
		bool console_visible, console_bypassed = false;
	public:
		enum console_color {
			black = 0,
			dark_blue,
			dark_green,
			light_blue,
			dark_red,
			magenta,
			orange,
			light_gray,
			gray,
			blue,
			green,
			cyan,
			red,
			pink,
			yellow,
			white,
		};

		static auto get_singleton() -> cherry::console* {
			static cherry::console* _console = nullptr;

			if (_console == nullptr)
				_console = new cherry::console();

			return _console;
		}

		auto show(std::string console_name = cherry::configuration::console_name) -> void;
		auto hide() -> void;
		auto write(std::string data) -> void;
		auto set_color(cherry::console::console_color color_code) -> void;
		auto reset_color() -> void;
		auto clear() -> void;

		/* custom function */
		auto info(std::string data) -> void;
		auto warn(std::string data) -> void;
		auto error(std::string data) -> void;

		/* others */
		auto debug_write(std::string data, std::string side_note = cherry::configuration::console_side_note) -> void;
		auto debug_error(std::string data, std::string side_note = cherry::configuration::console_side_note) -> void;
		template<typename ...arg>
		auto dbgprintf(const char* side_note, const char* format, arg... args) -> void {
			this->reset_color(); this->write("[");
			this->set_color(cherry::console::console_color::red); this->write(side_note);
			this->reset_color(); this->write("]");
			this->set_color(cherry::console::console_color::red); this->write(": ");
			this->reset_color();

			printf(format, args...);
		}
	};
}

#endif
