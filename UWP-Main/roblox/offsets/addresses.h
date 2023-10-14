#pragma once
#ifndef CHERRY_ADDRESS
#define CHERRY_ADDRESS
#include "offsets.h"

struct lua_State;	
struct threadRef;
#define rebase(addy) (addy - 0x400000 + (uintptr_t)GetModuleHandle(NULL))

namespace cherry {
	namespace addresses {
		const uintptr_t nilObject			= rebase(0x2C1AB50);
		const uintptr_t dummyNode			= rebase(0x2C1AB30);
		const uintptr_t taskScheduler		= rebase(0xBB8590);
		const uintptr_t luauExecute			= rebase(0x19B0770);
		const uintptr_t fireProximityPrompt = rebase(0x136E390);
		const uintptr_t pushInstance		= rebase(0x7C6900);
		const uintptr_t stdOut				= rebase(0xF568A0);
		const uintptr_t fireClickDetector	= rebase(0x0);
		const uintptr_t vmLoad				= rebase(0x95D430);
		const uintptr_t scriptContextResume = rebase(0x89A6E0);
		const uintptr_t getState			= rebase(0x854B20);
	};

	namespace addressTypes {
		using taskScheduler		  = std::intptr_t(__cdecl*)();
		using luauExecute		  = void(__thiscall*)(lua_State* L);
		using fireProximityPrompt = void(__thiscall*)(std::intptr_t proximityPrompt);
		using pushInstance		  = std::intptr_t(__stdcall*)(lua_State* L, void* instance);
		using stdOut			  = void(__cdecl*)(std::intptr_t color, const char* fmt, ...);
		using fireClickDetector   = void(__thiscall*)(std::intptr_t clickDetector, float distance, std::intptr_t player);
		using vmLoad			  = std::intptr_t(__fastcall*)(lua_State* L, std::string* source, const char* chunk, int env);
		using scriptContextResume = std::intptr_t(__thiscall*)(std::intptr_t scriptContext, threadRef threadRef, std::intptr_t results);
		using getState			  = lua_State*(__thiscall*)(std::intptr_t scriptContext, std::intptr_t* contextLevel, std::intptr_t* scriptPtr);
	};
};
#endif