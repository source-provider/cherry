#include "console.h"
#include <roblox/offsets/addresses.h>
#include <miscellaneous/utilities/utilities.h>

auto stdOut = reinterpret_cast<cherry::addressTypes::stdOut>(cherry::addresses::stdOut);
namespace cherry {
	console* console::singleton{ nullptr };
	console::console() {
		this->utils = utilities::getSingleton();
	};

	auto console::getSingleton() -> console* {
		if (singleton == nullptr)
			singleton = new console();

		return singleton;
	};

	#pragma region console
	auto console::getOutput() -> console& {
		return *this;
	};

	auto console::write(std::string_view str) -> void {
		stdOut(this->currentMode, str.data());
	};

	auto console::writeMode(std::string_view str, consoleMode mode) -> void {
		stdOut(mode, str.data());
	}

	auto console::changeMode(consoleMode mode) -> void {
		this->currentMode = mode;
	};
	#pragma endregion

	#pragma region operators
	const console& console::operator<<(std::string_view str) const {
		singleton->write(str);
		return *this;
	};

	const console& console::operator<<(const char* str) const {
		singleton->write(str);
		return *this;
	};

	const console& console::operator<<(consoleMode mode) const {
		singleton->changeMode(mode);
		return *this;
	};

	const console& console::operator<<(uintptr_t num) const {
		singleton->write(utils->tostring(num));
		return *this;
	};
	#pragma endregion
}