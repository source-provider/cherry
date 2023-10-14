#include "environment.hpp"
using namespace cherry::global;

auto isrbxactive(lua_State* L) -> int {
	ARG_CHECK(L, 0, 0);
	lua_pushboolean(L, utilities->roblox_active());
	return 1;
}

auto mouse1press(lua_State* L) -> int {
	ARG_CHECK(L, 0, 0);
	if (utilities->roblox_active())
		mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);

	return 0;
}

auto mouse1release(lua_State* L) -> int {
	ARG_CHECK(L, 0, 0);
	if (utilities->roblox_active())
		mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);

	return 0;
}

auto mouse1click(lua_State* L) -> int {
	ARG_CHECK(L, 0, 0);
	if (utilities->roblox_active())
		mouse_event(MOUSEEVENTF_LEFTDOWN | MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);

	return 0;
}

auto mouse2press(lua_State* L)
{
	ARG_CHECK(L, 0, 0);
	if (utilities->roblox_active())
		mouse_event(MOUSEEVENTF_RIGHTDOWN, 0, 0, 0, 0);

	return 0;
}

auto mouse2release(lua_State* L) -> int {
	ARG_CHECK(L, 0, 0);
	if (utilities->roblox_active())
		mouse_event(MOUSEEVENTF_RIGHTUP, 0, 0, 0, 0);

	return 0;
}

auto mouse2click(lua_State* L) -> int {
	ARG_CHECK(L, 0, 0);
	if (utilities->roblox_active())
		mouse_event(MOUSEEVENTF_RIGHTDOWN | MOUSEEVENTF_RIGHTUP, 0, 0, 0, 0);

	return 0;
}

auto keypress(lua_State* L) -> int {
	ARG_CHECK(L, 1, 1, luaL_checktype(L, 1, LUA_TNUMBER);)
		UINT key = lua_tointeger(L, 1);

	if (utilities->roblox_active())
		keybd_event(0, (BYTE)MapVirtualKeyA(key, MAPVK_VK_TO_VSC), KEYEVENTF_SCANCODE, 0);

	return 0;
}

auto keyrelease(lua_State* L) -> int {
	ARG_CHECK(L, 1, 1, luaL_checktype(L, 1, LUA_TNUMBER););
	UINT key = lua_tointeger(L, 1);

	if (utilities->roblox_active())
		keybd_event(0, (BYTE)MapVirtualKeyA(key, MAPVK_VK_TO_VSC), KEYEVENTF_SCANCODE | KEYEVENTF_KEYUP, 0);

	return 0;
}

auto mousemoverel(lua_State* L) -> int {
	ARG_CHECK(L, 2, 2, luaL_checktype(L, 1, LUA_TNUMBER););
	luaL_checktype(L, 2, LUA_TNUMBER);

	int x = lua_tointeger(L, 1);
	int y = lua_tointeger(L, 2);

	if (utilities->roblox_active())
		mouse_event(MOUSEEVENTF_MOVE, x, y, 0, 0);

	return 0;
}

auto mousemoveabs(lua_State* L) -> int {
	ARG_CHECK(L, 2, 2, luaL_checktype(L, 1, LUA_TNUMBER););
	luaL_checktype(L, 2, LUA_TNUMBER);

	int x = lua_tointeger(L, 1);
	int y = lua_tointeger(L, 2);

	if (!utilities->roblox_active())
		return 0;

	int width = GetSystemMetrics(SM_CXSCREEN) - 1;
	int height = GetSystemMetrics(SM_CYSCREEN) - 1;

	RECT CRect;
	GetClientRect(GetForegroundWindow(), &CRect);

	POINT Point{ CRect.left, CRect.top };
	ClientToScreen(GetForegroundWindow(), &Point);

	x = (x + Point.x) * (65535 / width);
	y = (y + Point.y) * (65535 / height);

	mouse_event(MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_MOVE, x, y, 0, 0);
	return 0;
}

auto mousescroll(lua_State* L) -> int {
	ARG_CHECK(L, 1, 1, luaL_checktype(L, 1, LUA_TNUMBER););
	int amt = lua_tointeger(L, 1);

	if (utilities->roblox_active())
		mouse_event(MOUSEEVENTF_WHEEL, 0, 0, amt, 0);

	return 0;
}

/* registering */

static const luaL_Reg env_funcs[] = { // {"NAME", FUNC},
	{"isrbxactive", isrbxactive},
	{"iswindowactive", isrbxactive},
	{"isgameactive", isrbxactive},

	{"keypress", keypress},
	{"keyrelease", keyrelease},

	{"mouse1click", mouse1click},
	{"mouse1press", mouse1press},
	{"mouse1release", mouse1release},

	{"mouse2click", mouse2click},
	{"mouse2press", mouse2press},
	{"mouse2release", mouse2release},

	{"mousescroll", mousescroll},
	{"mousemoverel", mousemoverel},
	{"mousemoveabs", mousemoveabs},

	{NULL, NULL},
};

auto cherry::environment::luauopen_input(lua_State* L) -> void {
	lua_pushvalue(L, LUA_GLOBALSINDEX);
	luaL_register(L, NULL, env_funcs);

	lua_pop(L, 1);
}