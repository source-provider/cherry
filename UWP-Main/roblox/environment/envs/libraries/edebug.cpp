#include "dependencies/luau/vm/src/lstate.h"
#include <roblox/environment/environment.h>
#include <lualib.h>


/* registering */
static const luaL_Reg envFuncs[] = {
    {NULL, NULL}
};

auto cherry::environment::createDebug(lua_State* L) -> void {
    lua_newtable(L);
    luaL_register(L, NULL, envFuncs);
    lua_setreadonly(L, -1, true);
    lua_setglobal(L, "debug");
}