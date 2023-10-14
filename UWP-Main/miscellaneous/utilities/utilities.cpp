#include "utilities.h"

namespace cherry {
	utilities* utilities::singleton{ nullptr };
	auto utilities::getSingleton() -> utilities* {
		if (singleton == nullptr)
			singleton = new utilities();

		return singleton;
	};

	auto utilities::randomString(std::intptr_t len) -> std::string {
		static const char* chars = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZbcdefghijklmnopqrstuvwxyz";
		std::string str; str.reserve(len);

		for (int i = 0; i < len; ++i)
			str += chars[rand() % (strlen(chars) - 1)];

		return str;
	};

	auto utilities::tostring(std::intptr_t num) -> std::string {
		std::stringstream str; std::string result;
		str << num; str >> result;
		return result;
	};

	auto utilities::tostring_double(double num) -> std::string {
		std::stringstream str; std::string result;
		str << num; str >> result;
		return result;
	};

	auto utilities::tostring_float(float num) -> std::string {
		std::stringstream str; std::string result;
		str << num; str >> result;
		return result;
	};

	auto utilities::replace(std::string subject, std::string search, std::string replace) -> std::string {
		size_t pos = 0;

		while ((pos = subject.find(search, pos)) != std::string::npos) {
			subject.replace(pos, search.length(), replace);
			pos += replace.length();
		}

		return subject;
	};

	auto utilities::getProcId() -> std::intptr_t {
		return _getpid();
	};

	auto utilities::rebaseAddress(std::uintptr_t address, std::uintptr_t base) -> std::uintptr_t {
		return ((address - base) + this->robloxBase);
	}

	auto utilities::equalsIgnoreCase(std::string_view a, std::string_view b) -> bool {
		return std::equal(a.begin(), a.end(),
			b.begin(), b.end(), [](char a, char b) {
				return tolower(a) == tolower(b);
			}
		);
	};

	auto utilities::robloxActive() -> bool {
		DWORD fproc_id;
		GetWindowThreadProcessId(GetForegroundWindow(), &fproc_id);
		return (this->getProcId() == fproc_id);
	};

	auto utilities::createFolder(std::string folderName) -> void {
		return;
	};

	auto utilities::createFile(std::string fileName, std::string content) -> void {
		return;
	}
}