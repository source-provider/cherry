#pragma once
#include <Windows.h>
#include <string>

namespace cherry
{
	namespace configuration
	{
		/* regular */
		const std::string exploit_name = "Cherry";
		const std::string exploit_version = "1.0.0";
		const std::string roblox_exploit_version = "version-40b6a27c6c4d46ef";

		/* console */
		const std::string console_name = "[Roblox] Cherry";
		const std::string console_side_note = "CHERRY";

		/* exception filter */
		const std::string ex_title = "[Cherry] Roblox Crash";
		const std::string ex_desc = "An unexpected error occurred and Roblox needs to quit. We're sorry!";

		/* debug */
		const bool allow_debug_write = true;
		const bool allow_debug_errors = true;

		/* exploit settings */
		const std::string source_name = "CherryRVM";
#define USE_CUSTOM_VM FALSE
#define USE_MEMCHECK TRUE

		/* pipe settings */
		const std::string pipe_name = "Cherry";
		const bool pipe_pid = true;
	}
}