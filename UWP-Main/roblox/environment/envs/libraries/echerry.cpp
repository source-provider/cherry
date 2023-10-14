#include "dependencies/luau/vm/src/lstate.h"
#include <roblox/environment/environment.h>
#include <lualib.h>


/* registering */
static const luaL_Reg envFuncs[] = {
    {NULL, NULL}
};

std::vector<std::string> libraryNames = { "cherry", "syn", "fluxus", "krnl" };
auto cherry::environment::createCherry(lua_State* L) -> void {
    for (std::string libName : libraryNames) {
        lua_newtable(L);
        luaL_register(L, NULL, envFuncs);
        lua_setreadonly(L, -1, true);
        lua_setglobal(L, libName.c_str());
    }
}