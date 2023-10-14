#pragma once
#include "environment.hpp"
#include <luau/vm/src/lapi.h>
#include <luau/vm/src/ludata.h>

auto checkinstance(lua_State* L, int idx) -> bool {
	const char* t = luaL_typename(L, idx);
	return (strcmp(t, "Instance") == 0);
}

/* registering */

auto cache_invalidate(lua_State* L) -> int {
	ARG_CHECK(L, 1, 1, luaL_checktype(L, 1, LUA_TUSERDATA););

	if (!checkinstance(L, 1))
	{
		luaL_argerror(L, 1, "userdata<instance> expected");
		return 0;
	}

	DWORD inst = dereference_pointer((DWORD)lua_touserdata(L, 1));

	lua_pushlightuserdata(L, (void*)cherry::offsets::push_instance);
	lua_gettable(L, LUA_REGISTRYINDEX);
	lua_pushlightuserdata(L, (void*)inst);
	lua_pushnil(L);
	lua_settable(L, -3);

	return 0;
}

auto cache_replace(lua_State* L) -> int {
	ARG_CHECK(L, 2, 2, luaL_checktype(L, 1, LUA_TUSERDATA););
	luaL_checktype(L, 2, LUA_TUSERDATA);

	if (!checkinstance(L, 1))
	{
		luaL_argerror(L, 1, "userdata<instance> expected");
		return 0;
	}

	if (!checkinstance(L, 2))
	{
		luaL_argerror(L, 2, "userdata<instance> expected");
		return 0;
	}

	DWORD inst = dereference_pointer((DWORD)lua_touserdata(L, 1));
	lua_pushlightuserdata(L, (void*)cherry::offsets::push_instance);
	lua_gettable(L, LUA_REGISTRYINDEX);
	lua_pushlightuserdata(L, (void*)inst);
	lua_pushvalue(L, 2);
	lua_settable(L, -3);

	return 0;
}


auto cache_iscached(lua_State* L) -> int {
	ARG_CHECK(L, 1, 1, luaL_checktype(L, 1, LUA_TUSERDATA););

	if (!checkinstance(L, 1))
	{
		luaL_argerror(L, 1, "userdata<instance> expected");
		return 0;
	}

	DWORD inst = dereference_pointer((DWORD)lua_touserdata(L, 1));

	lua_pushlightuserdata(L, (void*)cherry::offsets::push_instance);
	lua_gettable(L, LUA_REGISTRYINDEX);
	lua_pushlightuserdata(L, (void*)inst);
	lua_gettable(L, -2);

	lua_pushboolean(L, !lua_isnil(L, -1));

	return 1;
}

auto compare_instances(lua_State* L) -> int {
	ARG_CHECK(L, 2, 2, luaL_checktype(L, 1, LUA_TUSERDATA););
	luaL_checktype(L, 2, LUA_TUSERDATA);

	if (!checkinstance(L, 1))
	{
		luaL_argerror(L, 1, "userdata<instance> expected");
		return 0;
	}

	if (!checkinstance(L, 2))
	{
		luaL_argerror(L, 1, "userdata<instance> expected");
		return 0;
	}

	int instance_1 = dereference_pointer(lua_touserdata(L, 1));
	int instance_2 = dereference_pointer(lua_touserdata(L, 2));

	lua_pushboolean(L, (instance_1 == instance_2));
	return 1;
}

/*

*/

auto cloneref(lua_State* L) -> int {
	luaL_checktype(L, 1, LUA_TUSERDATA);

	const auto original_userdata = lua_touserdata(L, 1);
	const auto returned_userdata = *reinterpret_cast<std::uintptr_t*>(original_userdata);

	lua_pushlightuserdata(L, (void*)cherry::offsets::push_instance);

	lua_rawget(L, -10000);
	lua_pushlightuserdata(L, reinterpret_cast<void*>(returned_userdata));
	lua_rawget(L, -2);

	lua_pushlightuserdata(L, reinterpret_cast<void*>(returned_userdata));
	lua_pushnil(L);
	lua_rawset(L, -4);

	(reinterpret_cast<void(__stdcall*)(lua_State*, void*)>( (int)cherry::offsets::push_instance))(L, lua_touserdata(L, 1));

	lua_pushlightuserdata(L, reinterpret_cast<void*>(returned_userdata));
	lua_pushvalue(L, -3);
	lua_rawset(L, -5);

	return 1;
}

auto getpropvalue(lua_State* L) -> int {
	ARG_CHECK(L, 2, 2, luaL_checktype(L, 1, LUA_TUSERDATA););
	luaL_checktype(L, 2, LUA_TSTRING);

	if (!checkinstance(L, 1))
	{
		luaL_argerror(L, 1, "userdata<instance> expected");
		return 0;
	}

	lua_getfield(L, 1, lua_tolstring(L, 2, NULL));
	const char* data = lua_tolstring(L, -1, NULL); lua_pop(L, 2);


	lua_pushstring(L, data);
	return 1;
}

auto setpropvalue(lua_State* L) -> int {
	ARG_CHECK(L, 3, 3, luaL_checktype(L, 1, LUA_TUSERDATA););
	luaL_checktype(L, 2, LUA_TSTRING);
	luaL_checkany(L, 3);

	if (!checkinstance(L, 1))
	{
		luaL_argerror(L, 1, "userdata<instance> expected");
		return 0;
	}

	lua_setfield(L, 1, lua_tolstring(L, 2, NULL));
	return 0;
}


static const luaL_Reg env_funcs[] = { // {"NAME", FUNC},
	{"checkinst", compare_instances},
	{"compareinstances", compare_instances},
	{"cloneref", cloneref},
	{"clonereference", cloneref},
	{"getpropvalue", getpropvalue},
	{"setpropvalue", setpropvalue},
	{NULL, NULL},
};

static const luaL_Reg cache_funcs[] = { // {"NAME", FUNC},
	{"invalidate", cache_invalidate},
	{"replace", cache_replace},
	{"iscached", cache_iscached},
	{NULL, NULL},
};

auto cherry::environment::luauopen_cache(lua_State* L) -> void {
	lua_createtable(L, 0, 0);
	lua_setglobal(L, "cache");

	luaL_register(L, "cache", cache_funcs);

	lua_getglobal(L, "cache");
	lua_setreadonly(L, -1, true);

	lua_pushvalue(L, LUA_GLOBALSINDEX);
	luaL_register(L, NULL, env_funcs);

	lua_settop(L, 0);
}