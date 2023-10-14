#include "environment.hpp"
using namespace cherry::global;

std::vector<const char*> lib_names = { "cherry", "Nihon", "fluxus", "nihon", "syn", "Syn" };

/* instances protection */
auto protect_gui(lua_State* L) -> int {
	return 0;
}

auto unprotect_gui(lua_State* L) -> int {
	return 0;
}

/* secure */

auto secure_call(lua_State* L) -> int {
	return 0;
}

auto create_secure_function(lua_State* L) -> int {
	return 0;
}

auto run_secure_function(lua_State* L) -> int {
	return 0;
}

/* actors */


/* others */
auto is_beta(lua_State* L) -> int {
	lua_pushboolean(L, false);
	return 1;
}

/* regisering */

static const luaL_Reg env_funcs[] = {
	{"protect_gui", protect_gui},
	{"unprotect_gui", unprotect_gui},
	{"is_beta", is_beta},

	{"create_secure_function", create_secure_function},
	{"run_secure_function", run_secure_function},

	{NULL, NULL},
};

auto cherry::environment::luauopen_cherry(lua_State* L) -> void {
	lua_createtable(L, 0, 0);

	/* cache */

	lua_getglobal(L, "cache");

	lua_getfield(L, -1, "replace");
	lua_setfield(L, -3, "cache_replace");

	lua_getfield(L, -1, "invalidate");
	lua_setfield(L, -3, "cache_invalidate");

	lua_getfield(L, -1, "iscached");
	lua_setfield(L, -3, "is_cached");

	lua_pop(L, 1); /* pops the cache table */

	/* identity */

	lua_getglobal(L, "setidentity");
	lua_setfield(L, -2, "set_thread_identity");

	lua_getglobal(L, "getidentity");
	lua_setfield(L, -2, "get_thread_identity");

	/* misc */
	lua_getglobal(L, "setclipboard");
	lua_setfield(L, -2, "write_clipboard");

	lua_getglobal(L, "queue_on_teleport");
	lua_setfield(L, -2, "queue_on_teleport");

	lua_getglobal(L, "crypt");
	lua_setfield(L, -2, "crypto");

	lua_getglobal(L, "crypt");
	lua_setfield(L, -2, "crypt");

	lua_getglobal(L, "request");
	lua_setfield(L, -2, "request");

	/* others */


	luaL_register(L, NULL, env_funcs);

	


	lua_setreadonly(L, -1, true);
	lua_setglobal(L, "Cherry");

	lua_pop(L, 1);

	for (const char* library_name : lib_names) {
		lua_getglobal(L, "Cherry");
		lua_setglobal(L, library_name);
	}
}