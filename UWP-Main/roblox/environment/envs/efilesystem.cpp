#include "dependencies/luau/vm/src/lstate.h"
#include <roblox/environment/environment.h>
#include <lualib.h>


/* registering */
static const luaL_Reg envFuncs[] = {
    {NULL, NULL}
};

auto cherry::environment::createFileSystem(lua_State* L) -> void {
    lua_pushvalue(L, LUA_GLOBALSINDEX);
    luaL_register(L, NULL, envFuncs);

    lua_pop(L, 1);
}