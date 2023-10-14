#include "environment.hpp"
using namespace cherry::global;

std::map<std::string, cherry::console::console_color> console_colors = {
	{ "@@BLACK@@", cherry::console::console_color::black },
	{ "@@BLUE@@", cherry::console::console_color::blue },
	{ "@@GREEN@@", cherry::console::console_color::green },
	{ "@@CYAN@@", cherry::console::console_color::cyan },
	{ "@@RED@@", cherry::console::console_color::red },
	{ "@@MAGENTA@@", cherry::console::console_color::magenta },
	{ "@@BROWN@@", cherry::console::console_color::dark_red },
	{ "@@LIGHT_GRAY@@", cherry::console::console_color::light_gray },
	{ "@@DARK_GRAY@@", cherry::console::console_color::gray },
	{ "@@LIGHT_BLUE@@", cherry::console::console_color::light_blue },
	{ "@@LIGHT_GREEN@@", cherry::console::console_color::green },
	{ "@@LIGHT_CYAN@@", cherry::console::console_color::cyan },
	{ "@@LIGHT_RED@@", cherry::console::console_color::red },
	{ "@@LIGHT_RED@@", cherry::console::console_color::red },
	{ "@@LIGHT_MAGENTA@@", cherry::console::console_color::magenta },
	{ "@@YELLOW@@", cherry::console::console_color::yellow },
	{ "@@WHITE@@", cherry::console::console_color::white },
};

auto rconsoleprint(lua_State* L) -> int {
	ARG_CHECK(L, 1, 1, luaL_checktype(L, 1, LUA_TSTRING););
	console->show();
	std::string msg = luaL_tolstring(L, 1, NULL);
	lua_pop(L, 1);

	std::map<std::string, cherry::console::console_color>::iterator it;
	for (it = console_colors.begin(); it != console_colors.end(); it++) {
		if (it->first == msg) {
			console->set_color(it->second);
			return 0;
		}
	}

	std::cout << msg << "\n";

	return 0;
}

auto rconsoleinfo(lua_State* L) -> int {
	ARG_CHECK(L, 1, 1, luaL_checktype(L, 1, LUA_TSTRING););
	console->show();
	std::string msg = luaL_tolstring(L, 1, NULL);
	lua_pop(L, 1);

	console->info(msg);

	return 0;
}

auto rconsolewarn(lua_State* L) -> int {
	ARG_CHECK(L, 1, 1, luaL_checktype(L, 1, LUA_TSTRING););
	console->show();
	std::string msg = luaL_tolstring(L, 1, NULL);
	lua_pop(L, 1);

	console->warn(msg);

	return 0;
}

auto rconsoleerror(lua_State* L) -> int {
	ARG_CHECK(L, 1, 1, luaL_checktype(L, 1, LUA_TSTRING););
	console->show();
	std::string msg = luaL_tolstring(L, 1, NULL);
	lua_pop(L, 1);

	console->error(msg);

	return 0;
}

auto rconsoleclear(lua_State* L) -> int {
	ARG_CHECK(L, 0, 0);
	console->clear();
	return 0;
}

auto rconsolename(lua_State* L) -> int {
	ARG_CHECK(L, 1, 1, luaL_checktype(L, 1, LUA_TSTRING););
	std::string title = lua_tostring(L, 1);

	SetConsoleTitleA(title.c_str());
	return 0;
}

auto rconsolecreate(lua_State* L) -> int {
	ARG_CHECK(L, 0, 0);
	console->show();
	return 0;
}

auto rconsoledestroy(lua_State* L) -> int {
	ARG_CHECK(L, 0, 0);
	console->hide();
	return 0;
}

auto rconsoleinput(lua_State* L) -> int {
	ARG_CHECK(L, 0, 0);
	roblox_yield_t ryield(L);

	return ryield.execute([]()
		{
			std::string out;
			std::getline(std::cin, out);

			return [&](lua_State* L)
			{
				lua_pushstring(L, out.c_str());
				return 1;
			};
		}
	);
}

/* registering */
static const luaL_Reg env_funcs[] = {
	{"rconsoleprint", rconsoleprint},
	{"rconsoleinfo", rconsoleinfo},
	{"rconsolewarn", rconsolewarn},
	{"rconsoleerr", rconsoleerror},
	{"rconsoleclear", rconsoleclear},

	{"consoleprint", rconsoleprint},
	{"consoleinfo", rconsoleinfo},
	{"consolewarn", rconsolewarn},
	{"consoleerr", rconsoleerror},
	{"consoleclear", rconsoleclear},

	{"rconsolename", rconsolename},
	{"consolesettitle", rconsolename},
	{"rconsolesettitle", rconsolename},

	{"rconsoleinput", rconsoleinput},
	{"consoleinput", rconsoleinput},

	{"rconsolecreate", rconsolecreate},
	{"consolecreate", rconsolecreate},
	{"rconsoledestroy", rconsoledestroy},
	{"consoledestroy", rconsoledestroy},

	{NULL, NULL}
};

auto cherry::environment::luauopen_console(lua_State* L) -> void {
	lua_pushvalue(L, LUA_GLOBALSINDEX);
	luaL_register(L, NULL, env_funcs);

	lua_pop(L, 1);
}