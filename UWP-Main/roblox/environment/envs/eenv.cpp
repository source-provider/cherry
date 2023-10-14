#include <dependencies/luau/vm/src/lstate.h>
#include <roblox/environment/environment.h>
#include <dependencies/luau/vm/src/lmem.h>
#include <dependencies/luau/vm/src/lgc.h>
#include <lualib.h>

auto execution{ cherry::execution::getSingleton() };
auto environment{ cherry::environment::getSingleton() };
auto pushInstance{ reinterpret_cast<cherry::addressTypes::pushInstance>(cherry::addresses::pushInstance) };
auto fireClickDetector{ reinterpret_cast<cherry::addressTypes::fireClickDetector>(cherry::addresses::fireClickDetector) };
auto fireProximityPrompt { reinterpret_cast<cherry::addressTypes::fireProximityPrompt>(cherry::addresses::fireProximityPrompt) };

auto loadstring(lua_State* L) -> std::intptr_t {
	ARG_CHECK(L, 1, 2, luaL_checktype(L, 1, LUA_TSTRING);)
		std::string code{ lua_tostring(L, 1) };
	std::string chunk{ luaL_optstring(L, 2, "") };

	return execution->send(code, chunk, L);
};

auto getidentity(lua_State* L) -> std::intptr_t {
	ARG_CHECK(L, 0, 0);
	lua_pushnumber(L, L->extra_space->context_level);
	return 1;
}

auto setidentity(lua_State* L) -> std::intptr_t {
	ARG_CHECK(L, 1, 1, luaL_checktype(L, 1, LUA_TNUMBER););
	L->extra_space->context_level = lua_tonumber(L, 1);

	_TTEB* teb = (_TTEB*)NtCurrentTeb();
	*reinterpret_cast<std::intptr_t*>(*reinterpret_cast<std::intptr_t*>(teb->ThreadLocalStoragePointer) + 200) = L->extra_space->context_level;

	return 1;
};

auto getscriptfromthread(lua_State* L) -> std::intptr_t {
	ARG_CHECK(L, 1, 1, luaL_checktype(L, 1, LUA_TTHREAD););
	lua_State* thread = lua_tothread(L, 1);

	if (thread->extra_space->script_ptr != NULL)
		pushInstance(L, (void*)thread->extra_space->script_ptr);
	else
		lua_pushnil(L);

	return 1;
};

auto gethui(lua_State* L) -> std::intptr_t {
	ARG_CHECK(L, 0, 0);

	uint16_t old_identity = L->extra_space->context_level;
	L->extra_space->context_level = 8;
	_TTEB* teb = (_TTEB*)NtCurrentTeb();
	*reinterpret_cast<std::intptr_t*>(*reinterpret_cast<std::intptr_t*>(teb->ThreadLocalStoragePointer) + 200) = L->extra_space->context_level;

	lua_getglobal(L, "game");
	lua_getfield(L, -1, "GetService");

	lua_getglobal(L, "game");
	lua_pushstring(L, "CoreGui");

	lua_pcall(L, 2, 1, 0);

	L->extra_space->context_level = old_identity;
	teb = (_TTEB*)NtCurrentTeb();
	*reinterpret_cast<std::intptr_t*>(*reinterpret_cast<std::intptr_t*>(teb->ThreadLocalStoragePointer) + 200) = old_identity;

	return 1;
};

auto getgenv(lua_State* L) -> std::intptr_t {
	ARG_CHECK(L, 0, 0);
	lua_pushvalue(L, LUA_GLOBALSINDEX);
	return 1;
};

auto getrenv(lua_State* L) -> std::intptr_t {
	ARG_CHECK(L, 0, 0);
	sethvalue(L, L->top, environment->getRobloxTable());
	L->top++;
	return 1;
};

auto getreg(lua_State* L) -> std::intptr_t {
	ARG_CHECK(L, 0, 0);
	lua_pushvalue(L, LUA_REGISTRYINDEX);
	return 1;
};

struct gc_context {
	lua_State* L;
	std::intptr_t n;
	bool is_table;
};

auto getgc(lua_State* L) -> std::intptr_t {
	ARG_CHECK(L, 0, 1);
	global_State* g = L->global;
	bool tables = luaL_optboolean(L, 1, FALSE);
	lua_createtable(L, 0, 0);

	gc_context context = { L, 0, tables };
	for (lua_Page* page = g->allgcopages; page;) {
		lua_Page* next{ page->gcolistnext }; // block visit might destroy the page

		luaM_visitpage(page, &context,
			[](void* context, lua_Page* page, GCObject* gco) -> bool {
				gc_context* gcContext{ reinterpret_cast<gc_context*>(context) };
				auto type = gco->gch.tt;

				if (type == LUA_TUSERDATA || type == LUA_TFUNCTION || (type == LUA_TTABLE && gcContext->is_table)) {
					TValue* top = gcContext->L->top;
					top->value.p = reinterpret_cast<void*>(gco);
					top->tt = type;
					gcContext->L->top++;

					gcContext->n++;
					lua_rawseti(gcContext->L, -2, gcContext->n);
				}
			}
		);

		page = next;
	}

	return 1;
};

auto getsenv(lua_State* L) -> std::intptr_t {
	ARG_CHECK(L, 1, 1);
	return 1;
};

auto getinstancelist(lua_State* L) -> std::intptr_t { /* this is for getinstances and getnilinstances */
	ARG_CHECK(L, 0, 0);

	lua_pushvalue(L, LUA_REGISTRYINDEX);
	lua_pushlightuserdata(L, (void*)pushInstance);
	lua_gettable(L, -2);

	return 1;
}

struct connection_t { 
	std::intptr_t object; 
	void* orignalObject; 
	std::intptr_t oldState;
};

std::unordered_map<int, connection_t*> connections;
auto getconnections(lua_State* L) -> std::intptr_t {
	ARG_CHECK(L, 1, 1, luaL_checktype(L, 1, LUA_TUSERDATA););
	luaL_argexpected(L, (strcmp(luaL_typename(L, 1), "RBXScriptSignal") == LUA_OK), 1, "RBXScriptSignal");

	lua_getfield(L, 1, "Connect");
	lua_pushvalue(L, 1);

	lua_pushcclosurek(L, [](lua_State* L) -> std::intptr_t {
		return 0;
	}, NULL, NULL, NULL);

	lua_pcall(L, 2, 1, 0);

	const std::intptr_t signal = *reinterpret_cast<std::intptr_t*>(lua_touserdata(L, -1));
	std::intptr_t next = *reinterpret_cast<std::intptr_t*>(signal + cherry::offsets::signalNext);

	lua_getfield(L, -1, "Disconnect");
	lua_pushvalue(L, -2);
	lua_pcall(L, 1, 0, 0);
	lua_pop(L, 1);

	lua_remove(L, 1);
	auto indexFunction = [](lua_State* L) -> std::intptr_t {
		ARG_CHECK(L, 2, 2, luaL_checktype(L, 1, LUA_TUSERDATA););
		luaL_checktype(L, 2, LUA_TSTRING);

		const std::string& key{ lua_tostring(L, 2) };
		auto enableConnection = [](lua_State* L) -> std::intptr_t {
			const auto signal = *(int*)lua_touserdata(L, -10003);

			if (!connections.count(signal)) {
				connection_t newC;
				newC.orignalObject = lua_touserdata(L, -10003);
				newC.object = signal;
				newC.oldState = *reinterpret_cast<std::intptr_t*>(signal + cherry::offsets::signalState);

				*connections[signal] = newC;
			}

			*reinterpret_cast<int*>(signal + cherry::offsets::signalState) = connections[signal]->oldState;
			return 0;
		};

		auto disableConnection = [](lua_State* L) -> std::intptr_t {
			const auto signal = *(int*)lua_touserdata(L, -10003);

			if (!connections.count(signal)) {
				connection_t newC;
				newC.orignalObject = lua_touserdata(L, -10003);
				newC.object = signal;
				newC.oldState = *reinterpret_cast<std::intptr_t*>(signal + cherry::offsets::signalState);

				*connections[signal] = newC;
			}

			*reinterpret_cast<std::intptr_t*>(signal + cherry::offsets::signalState) = 0;
			return 0;
		};

		if (key == "Enable" || key == "enable") {
			lua_pushvalue(L, 1);
			lua_pushcclosure(L, enableConnection, NULL, 1);
		} else if (key == "Disable" || key == "disable" || key == "Disconnect" || key == "disconnect") /* doing Disconnect this way but will chjange it later */ {
			lua_pushvalue(L, 1);
			lua_pushcfunction(L, disableConnection, NULL, 1);
		} else if (key == "Function" || key == "function" || key == "Fire" || key == "fire" || key == "Defer" || key == "defer") {
			const auto connection = *reinterpret_cast<connection_t*>(lua_touserdata(L, 1));
			lua_getref(L, *reinterpret_cast<std::intptr_t*>(*reinterpret_cast<std::intptr_t*>(*reinterpret_cast<std::intptr_t*>(connection.object + cherry::offsets::signalRef0) + cherry::offsets::signalRef1) + cherry::offsets::signalRefIdx));

			if (lua_type(L, -1) <= 0) {
				lua_pushcfunction(L, [](lua_State* L) -> std::intptr_t {
					return 0;
				}, 0);
			};
		} else if (key == "Enabled" || key == "enabled" || key == "State" || key == "state") {
			const auto Signal = *reinterpret_cast<std::intptr_t*>(lua_touserdata(L, 1));
			const auto conn = *reinterpret_cast<std::intptr_t*>(Signal + cherry::offsets::signalState);

			lua_pushboolean(L, conn);
		} else if (key == "LuaConnection" || key == "luaconnection") {
			const auto connection = *reinterpret_cast<connection_t*>(lua_touserdata(L, 1));
			lua_getref(L, *reinterpret_cast<std::intptr_t*>(*reinterpret_cast<std::intptr_t*>(*reinterpret_cast<std::intptr_t*>(connection.object + cherry::offsets::signalRef0) + cherry::offsets::signalRef1) + cherry::offsets::signalRefIdx));

			if (lua_type(L, -1) == LUA_TFUNCTION) {
				lua_pushboolean(L, lua_isLfunction(L, -1));
			}
			else {
				lua_pushboolean(L, false);
			}
		}
		else {
			lua_pushnil(L);
		}

		return 1;
	};

	lua_createtable(L, 0, 0);
	std::intptr_t count = 1;
	while (next != 0) {
		if (connections.count(next)) {
			connection_t* connection = reinterpret_cast<connection_t*>(lua_newuserdata(L, sizeof(connection_t), 0));
			*connection = *connections[next];

			lua_newtable(L);

			lua_pushcfunction(L, indexFunction, 0);
			lua_setfield(L, -2, "__index");

			lua_pushstring(L, "table");
			lua_setfield(L, -2, "__type");

			lua_setmetatable(L, -2);
		} else {
			connection_t* newConnection = reinterpret_cast<connection_t*>(lua_newuserdata(L, sizeof(connection_t), 0));
			*newConnection = connection_t{};
			newConnection->orignalObject = lua_touserdata(L, 1);
			newConnection->object = next;
			newConnection->oldState = *reinterpret_cast<std::intptr_t*>(next + cherry::offsets::signalState);

			lua_newtable(L);

			lua_pushcfunction(L, indexFunction, 0);
			lua_setfield(L, -2, "__index");

			lua_pushstring(L, "table");
			lua_setfield(L, -2, "__type");

			lua_setmetatable(L, -2);
			connections[next] = newConnection;
		}

		lua_rawseti(L, -2, count++);
		next = *reinterpret_cast<std::intptr_t*>(next + cherry::offsets::signalNext);
	}

	return 1;
};

/* registering */
static const luaL_Reg envFuncs[] = {
	{"loadstring", loadstring},

	{"getidentity", getidentity},
	{"getthreadidentity", getidentity},
	{"getthreadcontext", getidentity},
	{"setidentity", setidentity},
	{"setthreadidentity", setidentity},
	{"setthreadcontext", setidentity},
	{"getscriptfromthread", getscriptfromthread},

	{"gethui", gethui},
	{"getgenv", getgenv},
	{"getrenv", getrenv},
	{"getreg", getreg},
	{"getgc", getgc},
	{"getsenv", getsenv},

	{"getinstancelist", getinstancelist},

	{"getconnections", getconnections},
	{"getothersignals", getconnections},
	//{"gethiddenproperty", gethiddenproperty},
	//{"sethiddenproperty", sethiddenproperty},
	//{"isscriptable", isscriptable},
	//{"setscriptable", setscriptable},
	//{"getproperties", getproperties},
	//{"gethiddenproperties", gethiddenproperties},

	//{"getscriptbytecode", getscriptbytecode},
	//{"dumpstring", getscriptbytecode},
	//{"getscripthash", getscripthash},

	//{"getobjects", cherry::environment::getobjects},
	//{"firesignal", firesignal},
	//{"getcustomasset", getcustomasset},
	////{"fireclickdetector", fireclickdetector},
	//{"fireproximityprompt", fireproximityprompt},
	//{"decompile", decompile},
	//{"gettenv", gettenv},
	//{"getstateenv", gettenv},
    {NULL, NULL}
};

auto cherry::environment::createEnv(lua_State* L) -> void {
    lua_pushvalue(L, LUA_GLOBALSINDEX);
    luaL_register(L, NULL, envFuncs);

    lua_pop(L, 1);
}