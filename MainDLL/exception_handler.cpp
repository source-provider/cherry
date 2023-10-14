#include "exception_handler.hpp"
#include "configuration.hpp"

#include <chrono>
#include <iostream>



uint64_t sec = 0;

auto __stdcall filter_handler(PEXCEPTION_POINTERS pExceptionInfo) -> long {
	double nsec = (double)(duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count() - sec);

	std::cout << "[TIME SINCE INJECTION]: " << nsec << "\n";

	MessageBoxA(0, cherry::configuration::ex_desc.c_str(), cherry::configuration::ex_title.c_str(), MB_ICONERROR | MB_OK | MB_TOPMOST);
	exit(1);

	return EXCEPTION_EXECUTE_HANDLER;
}

cherry::exception_filter::exception_filter() {
	sec = duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();

	const uintptr_t kernel = reinterpret_cast<uintptr_t>(GetModuleHandleA("KERNEL32.dll"));
	const uintptr_t filter = reinterpret_cast<uintptr_t>(GetProcAddress(reinterpret_cast<HMODULE>(kernel), "SetUnhandledExceptionFilter"));

	const uint8_t n_bytes[] = { 0x90, 0x90, 0x90, 0x90, 0x90, 0x90 };
	const uint32_t n_byte_length = sizeof(n_bytes);

	DWORD old_protection;
	VirtualProtect(reinterpret_cast<void*>(filter), n_byte_length, 0x40, &old_protection);
	memmove(reinterpret_cast<void*>(filter), n_bytes, n_byte_length);
	VirtualProtect(reinterpret_cast<void*>(filter), n_byte_length, old_protection, &old_protection);

	SetUnhandledExceptionFilter(filter_handler);
}