#pragma once
#include <roblox/execution/execution.h>
#define ARG_CHECK(L, MIN, MAX, T) if (lua_gettop(L) < MIN) { T return 0; } else if (lua_gettop(L) > MAX) { lua_settop(L, MAX); } T

struct Table;
namespace cherry {
	class environment {
	private:
		static environment* singleton;
		Table* robloxEnv = nullptr;
	private:
		/* regular */
		auto createEnv(lua_State* L) -> void;
		auto createClosure(lua_State* L) -> void;
		auto createConsole(lua_State* L) -> void;
		auto createFileSystem(lua_State* L) -> void;
		auto createInput(lua_State* L) -> void;
		auto createMetatable(lua_State* L) -> void;
		auto createHttp(lua_State* L) -> void;
		auto createOthers(lua_State* L) -> void;

		/* libraries */
		auto createCache(lua_State* L) -> void;
		auto createDebug(lua_State* L) -> void;
		auto createDrawing(lua_State* L) -> void;
		auto createCrypt(lua_State* L) -> void;
		auto createWebSockets(lua_State* L) -> void;
		auto createCherry(lua_State* L) -> void;

		/* After register */
		auto createInit(lua_State* L) -> void;
	public:
		static auto getSingleton() -> environment*;
		auto createEnvironment(lua_State* L) -> void;
		auto setRobloxTable(Table* tab) -> void;
		auto getRobloxTable() -> Table*;
	};
};