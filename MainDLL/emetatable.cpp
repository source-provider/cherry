#include "environment.hpp"
#include <luau/vm/src/lapi.h>
using namespace cherry::global;

/* functions */
int getrawmetatable(lua_State* L)
{
	ARG_CHECK(L, 1, 1, luaL_checkany(L, 1););

	if (!lua_getmetatable(L, 1))
		lua_pushnil(L);

	return 1;
}

int setrawmetatable(lua_State* L)
{
	ARG_CHECK(L, 2, 2, luaL_checkany(L, 1););

	int t = lua_type(L, 2);
	luaL_argcheck(L, t == LUA_TNIL || t == LUA_TTABLE, 2, "nil or table expected");

	lua_settop(L, 2);
	lua_pushboolean(L, lua_setmetatable(L, 1));

	return 1;
}

int setreadonly(lua_State* L)
{
	ARG_CHECK(L, 2, 2, luaL_checktype(L, 1, LUA_TTABLE););
	luaL_checktype(L, 2, LUA_TBOOLEAN);

	Table* t = (Table*)lua_topointer(L, 1);
	t->readonly = lua_toboolean(L, 2);

	return 0;
}

int isreadonly(lua_State* L)
{
	ARG_CHECK(L, 1, 1, luaL_checktype(L, 1, LUA_TTABLE););

	Table* t = (Table*)lua_topointer(L, 1);
	lua_pushboolean(L, t->readonly);

	return 1;
}

int iswriteable(lua_State* L)
{
	ARG_CHECK(L, 1, 1, luaL_checktype(L, 1, LUA_TTABLE););

	Table* t = (Table*)lua_topointer(L, 1);
	lua_pushboolean(L, !t->readonly);

	return 1;
}

int makereadonly(lua_State* L)
{
	ARG_CHECK(L, 1, 1, luaL_checktype(L, 1, LUA_TTABLE););

	Table* t = (Table*)lua_topointer(L, 1);
	t->readonly = TRUE;

	return 0;
}

int makewriteable(lua_State* L)
{
	ARG_CHECK(L, 1, 1, luaL_checktype(L, 1, LUA_TTABLE););

	Table* t = (Table*)lua_topointer(L, 1);
	t->readonly = FALSE;

	return 0;
}

int getnamecallmethod(lua_State* L)
{
	ARG_CHECK(L, 0, 0);
	if (L->namecall != NULL)
	{
		lua_pushstring(L, L->namecall->data);
		return 1;
	}

	return 0;
}

int setnamecallmethod(lua_State* L)
{
	ARG_CHECK(L, 1, 1, luaL_checktype(L, 1, LUA_TSTRING););
	L->namecall = tsvalue(luaA_toobject(L, 1));
	return 0;
}

/* registering */

static const luaL_Reg env_funcs[] = { // {"NAME", FUNC},
	{"getrawmetatable", getrawmetatable},
	{"setrawmetatable", setrawmetatable},
	{"setreadonly", setreadonly},
	{"isreadonly", isreadonly},
	{"iswriteable", iswriteable},
	{"makereadonly", makereadonly},
	{"makewriteable", makewriteable},
	{"getnamecallmethod", getnamecallmethod},
	{"setnamecallmethod", setnamecallmethod},
	{NULL, NULL},
};

auto cherry::environment::luauopen_metatable(lua_State* L) -> void {
	lua_pushvalue(L, LUA_GLOBALSINDEX);
	luaL_register(L, NULL, env_funcs);

	lua_pop(L, 1);
}