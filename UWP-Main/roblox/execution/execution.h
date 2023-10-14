#pragma once
#include <queue>
#include <string>
#include <Windows.h>
#include <functional>

#include <roblox/classes/console/console.h>
#include <roblox/scheduler/scheduler.h>

struct lua_State;
namespace cherry {
	class schedulerTask {
	public:
		std::function<void(lua_State* L)> func;
		std::string script;
		bool isc;
		schedulerTask(std::string script): script(script), isc(false) {}
		schedulerTask(std::function<void(lua_State* L)> func) : func(func), isc(true) {}
	};

	class execution {
	private:
		static execution* singleton;
		std::queue<cherry::schedulerTask> schedulerQueue{};
		bool pipeInitialize = false;
	public:
		static auto getSingleton() -> execution*;
		auto createPipe() -> void;
		auto load() -> void;
	public:
		auto send(std::string script, std::string chunk = "", lua_State* L = nullptr) -> std::intptr_t;
		auto send(std::function<void(lua_State* L)>) -> std::intptr_t;
		auto empty() -> bool;
		auto top() -> schedulerTask;
	};
}