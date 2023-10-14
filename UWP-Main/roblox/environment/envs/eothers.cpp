#include "dependencies/luau/vm/src/lstate.h"
#include <roblox/environment/environment.h>
#include <lualib.h>


/* registering */
static const luaL_Reg envFuncs[] = {
    {NULL, NULL}
};

auto cherry::environment::createOthers(lua_State* L) -> void {
    lua_pushvalue(L, LUA_GLOBALSINDEX);
    luaL_register(L, NULL, envFuncs);
    lua_pop(L, 1);

    lua_pushvalue(L, LUA_GLOBALSINDEX);
    lua_setglobal(L, "_G");

    lua_newtable(L);
    lua_setglobal(L, "shared");
}