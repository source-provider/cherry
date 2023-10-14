#pragma once
#include <Windows.h>
#include <cstdint>
#include <vector>
#include <string_view>

struct lua_State;
namespace cherry {
	class vfhook;
	class scheduler {
	private:
		static scheduler* singleton;
		std::intptr_t taskScheduler{ 0 };
		vfhook* schedulerHook{ nullptr };
		std::vector<std::intptr_t> jobVector{};
	public:
		using jobType = std::uintptr_t(__thiscall*)(std::uintptr_t, std::uintptr_t);
	public:
		explicit scheduler();
		static auto getSingleton() -> scheduler*;
		auto reInitialize() -> void; /* used in teleport handler */
	public:
		auto getJobName(std::intptr_t job) -> std::string;
		auto getJob(std::string_view name) -> std::intptr_t;
		auto jobHook(void* hookFunc) -> std::intptr_t;
	public:
		auto getScriptContext() -> std::intptr_t;
		auto getDataModel() -> std::intptr_t;
		auto getLuaState() -> lua_State*;
	};

}