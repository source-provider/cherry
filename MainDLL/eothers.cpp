#include "environment.hpp"
using namespace cherry::global;

auto setclipboard(lua_State* L) -> int {
    ARG_CHECK(L, 1, 1, luaL_checkany(L, 1););
    const char* data = luaL_tolstring(L, 1, NULL);
    lua_pop(L, 1);

    HWND hwnd = GetDesktopWindow();

    OpenClipboard(hwnd);
    EmptyClipboard();

    HGLOBAL hg = GlobalAlloc(GMEM_MOVEABLE, strlen(data) + 1);

    if (!hg)
    {
        CloseClipboard();
        return 0;
    }

    memcpy(GlobalLock(hg), data, strlen(data) + 1);
    GlobalUnlock(hg);

    SetClipboardData(CF_TEXT, hg);
    CloseClipboard();

    GlobalFree(hg);

    return 0;
}

int lz4compress(lua_State* L) {
    ARG_CHECK(L, 1, 1, luaL_checktype(L, 1, LUA_TSTRING););
    const char* data = lua_tostring(L, 1);
    int nMaxCompressedSize = LZ4_compressBound(strlen(data));
    char* out_buffer = new char[nMaxCompressedSize];

    LZ4_compress(data, out_buffer, strlen(data));
    lua_pushlstring(L, out_buffer, nMaxCompressedSize);
    return 1;
}

int lz4decompress(lua_State* L) {
    ARG_CHECK(L, 2, 2, luaL_checktype(L, 1, LUA_TSTRING););
    luaL_checktype(L, 2, LUA_TNUMBER);
    const char* data = lua_tostring(L, 1);
    int data_size = lua_tointeger(L, 2);

    char* pszUnCompressedFile = new char[data_size];

    LZ4_uncompress(data, pszUnCompressedFile, data_size);
    lua_pushlstring(L, pszUnCompressedFile, data_size);
    return 1;
}

int identifyexecutor(lua_State* L)  {
    ARG_CHECK(L, 0, 0);

    lua_pushstring(L, cherry::configuration::exploit_name.c_str());
    lua_pushstring(L, cherry::configuration::exploit_version.c_str());

    return 2;
}

auto setfpscap(lua_State* L) -> int {
    ARG_CHECK(L, 1, 1, luaL_checktype(L, 1, LUA_TNUMBER););
    double fps = lua_tonumber(L, 1);

    task_scheduler->setfps(fps);

    return 0;
}

auto getfpscap(lua_State* L) -> int {
    ARG_CHECK(L, 0, 0);
    lua_pushnumber(L, task_scheduler->getfps());
    return 1;
}

auto queue_on_teleport(lua_State* L) -> int { 
    ARG_CHECK(L, 1, 1, luaL_checktype(L, 1, LUA_TSTRING););
    std::string code = lua_tostring(L, 1);
    cherry::environment::queue_on_tp(code);
    return 0;
}

/* registering */
static const luaL_Reg env_funcs[] = {
	{"setclipboard", setclipboard},
	{"toclipboard", setclipboard},

    {"lz4compress", lz4compress},
    {"lz4decompress", lz4decompress},

    {"identifyexecutor", identifyexecutor},
    {"getexecutorname", identifyexecutor},

    {"setfpscap", setfpscap},
    {"getfpscap", getfpscap},

    {"queue_on_teleport", queue_on_teleport},
    {"queueonteleport", queue_on_teleport},

	{NULL, NULL}
};

auto cherry::environment::luauopen_other(lua_State* L) -> void {
	lua_pushvalue(L, LUA_GLOBALSINDEX);
	luaL_register(L, NULL, env_funcs);

	lua_pop(L, 1);
}