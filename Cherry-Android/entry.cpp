#include "entry.h"
#include "utilities.hpp"
#include "dobby/dobby.h"

auto init() -> void {
	while (!is_library_loaded("libroblox.so")) 
		sleep(1);

	
}

extern "C" {
	/* This trivial function returns the platform ABI for which this dynamic native library is compiled.*/
	const char * Cherry_Android::getPlatformABI()
	{
	#if defined(__arm__)
	#if defined(__ARM_ARCH_7A__)
	#if defined(__ARM_NEON__)
		#define ABI "armeabi-v7a/NEON"
	#else
		#define ABI "armeabi-v7a"
	#endif
	#else
		#define ABI "armeabi"
	#endif
	#elif defined(__i386__)
		#define ABI "x86"
	#else
		#define ABI "unknown"
	#endif
		LOGI("This dynamic shared library is compiled with ABI: %s", ABI);
		return "This native library is compiled with ABI: %s" ABI ".";
	}

	auto Cherry_Android() -> void { /* this is the onloaded I guess */
		std::thread{ init }.detach();
	}

	Cherry_Android::Cherry_Android()
	{
	}

	Cherry_Android::~Cherry_Android()
	{
	}
}
