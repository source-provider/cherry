#include "console.hpp"

#pragma section(".text")
__declspec(allocate(".text")) const std::uint8_t ret_stub[] = { 0x90,0xC3 };

auto cherry::console::show(std::string console_name) -> void {
	if (!this->console_bypassed) {
		this->console_bypassed = true;
		this->console_visible = true;

		static auto global_stub_pointer = reinterpret_cast<std::uintptr_t>(ret_stub);

		if (!LoadLibraryA("KERNEL32.dll")) {
			this->console_bypassed = false;
			this->console_visible = false;
			return;
		}

		if (const auto free_console_address = reinterpret_cast<std::uintptr_t>(&FreeConsole)) {
			constexpr const auto size = sizeof(std::uintptr_t) + sizeof(std::uint8_t) * 2;
			VirtualProtect(reinterpret_cast<void*>(&FreeConsole), size, PAGE_EXECUTE_READWRITE, &this->old_protection);
			*reinterpret_cast<void**>(free_console_address + sizeof(std::uint8_t) * 2) = &global_stub_pointer;
			VirtualProtect(reinterpret_cast<void*>(&FreeConsole), size, this->old_protection, &this->old_protection);
		}

		AllocConsole();

		freopen_s(&this->file_ptr, "CONOUT$", "w", stdout);
		freopen_s(&this->file_ptr, "CONOUT$", "w", stderr);
		freopen_s(&this->file_ptr, "CONIN$", "r", stdin);

		SetConsoleTitleA(console_name.c_str());

		this->std_console = GetStdHandle(STD_OUTPUT_HANDLE);
		this->console_window = GetConsoleWindow();

		DeleteMenu(GetSystemMenu(this->console_window, FALSE), SC_CLOSE, MF_BYCOMMAND);
		DeleteMenu(GetSystemMenu(this->console_window, FALSE), SC_MAXIMIZE, MF_BYCOMMAND);
		DrawMenuBar(this->console_window);

		::SetWindowPos(this->console_window, HWND_TOPMOST, 0, 0, 0, 0, SWP_DRAWFRAME | SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);

		return;
	}

	if (this->console_visible)
		return;

	::SetWindowPos(console_window, HWND_TOPMOST, 0, 0, 0, 0, SWP_DRAWFRAME | SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);
	this->console_visible = true;
}

auto cherry::console::hide() -> void {
	if (!this->console_visible)
		return;

	::SetWindowPos(console_window, HWND_TOPMOST, 0, 0, 0, 0, SWP_DRAWFRAME | SWP_NOMOVE | SWP_NOSIZE | SWP_HIDEWINDOW);
	this->console_visible = false;
}

auto cherry::console::write(std::string data) -> void {
	std::cout << data;
}

auto cherry::console::set_color(cherry::console::console_color color_code) -> void {
	SetConsoleTextAttribute(this->std_console, color_code);
}

auto cherry::console::reset_color() -> void {
	this->set_color(cherry::console::console_color::light_gray);
}

auto cherry::console::clear() -> void {
	system("cls");
}

/* custom function */
auto cherry::console::info(std::string data) -> void {
	data += "\n";
	this->show();

	this->reset_color(); this->write("[");
	this->set_color(cherry::console::console_color::light_blue); this->write("*");
	this->reset_color(); this->write("]");
	this->set_color(cherry::console::console_color::light_gray); this->write(": ");
	this->reset_color(); this->write(data);
}

auto cherry::console::warn(std::string data) -> void {
	data += "\n";
	this->show();

	this->reset_color(); this->write("[");
	this->set_color(cherry::console::console_color::orange); this->write("*");
	this->reset_color(); this->write("]");
	this->set_color(cherry::console::console_color::light_gray); this->write(": ");
	this->reset_color(); this->write(data);
}

auto cherry::console::error(std::string data) -> void {
	data += "\n";
	this->show();

	this->reset_color(); this->write("[");
	this->set_color(cherry::console::console_color::red); this->write("*");
	this->reset_color(); this->write("]");
	this->set_color(cherry::console::console_color::light_gray); this->write(": ");
	this->reset_color(); this->write(data);
}

/* others */
auto cherry::console::debug_write(std::string data, std::string side_note) -> void {
	if (!configuration::allow_debug_write)
		return;

	data += "\n";

	this->reset_color(); this->write("[");
	this->set_color(cherry::console::console_color::red); this->write(side_note);
	this->reset_color(); this->write("]");
	this->set_color(cherry::console::console_color::red); this->write(": ");
	this->reset_color(); this->write(data);
}

auto cherry::console::debug_error(std::string data, std::string side_note) -> void {
	if (!configuration::allow_debug_errors)
		return;

	data += "\n";

	this->reset_color(); this->write("[");
	this->set_color(cherry::console::console_color::red); this->write(side_note);
	this->reset_color(); this->write("]");
	this->set_color(cherry::console::console_color::red); this->write(": ");
	this->set_color(cherry::console::console_color::red); this->write(data);

	this->reset_color();
}