#include "environment.hpp"
#include "luau/vm/src/lstring.h"
#include <luau/vm/src/lvm.h>
#include <luau/vm/src/ldebug.h>
#include <luau/vm/src/ldo.h>
#include <luau/vm/src/lfunc.h>
#include <luau/vm/src/lgc.h>

Table* _ENV = nullptr;
auto cherry::environment::get_roblox_env(Table* env) -> Table* {
    if (env != nullptr)
        _ENV = env;

    return _ENV;
}

auto cherry::environment::luauopen_register(lua_State* L) -> void {
	
#pragma region undetected environment
	L->extra_space->context_level = 8;
	global::console->debug_write(global::utilities->tostring(L->extra_space->context_level), "IDENTITY");

    cherry::environment::get_roblox_env(L->gt);
    luaL_sandboxthread(L);

    lua_createtable(L, 0, 0);
    lua_setfield(L, -10002, "_G");

    /*lua_createtable(L, 0, 0);
    lua_setfield(L, -10002, "shared");*/
#pragma endregion

	luauopen_environment(L);
	luauopen_closure(L);
	luauopen_console(L);
	luauopen_filesystem(L);
	luauopen_input(L);
	luauopen_metatable(L);
	luauopen_http(L);
	luauopen_other(L);


	luauopen_cache(L);
	luauopen_debug(L);
	luauopen_drawing(L);
	luauopen_crypt(L);
	luauopen_websocket(L);
	//luauopen_cherry(L);

	//luauopen_initscript(L);
    
	global::console->debug_write("REGISTERED!", "ENVIRONMENT");
}

#pragma region classes and functions
struct ylive_thread_ref
{
	int unk_0;
	lua_State* state;
	int thread_id;
};

struct ythread_ref
{
	ylive_thread_ref* ref;
	ythread_ref(lua_State* L)
	{
		ref = new ylive_thread_ref;
		ref->state = L;
		lua_pushthread(L);
		ref->thread_id = lua_ref(L, -1);
		lua_pop(L, 1);
	}
};

static auto yscriptcontext_resume = reinterpret_cast<int(__thiscall*)(int context, ythread_ref thref, int returns)>(cherry::offsets::scriptcontext_resume);
auto THREAD_FUNC(const std::function<yield_ret()>& func, lua_State* state) -> void {
	static auto task_scheduler = cherry::scheduler::get_singleton();
	yield_ret returned_func;

	try
	{
		returned_func = func();
	}
	catch (std::exception& ex)
	{
		task_scheduler->push([state, ex](DWORD) 
			{
				lua_getglobal(state, "warn");
				lua_pushstring(state, ex.what());
				lua_pcall(state, 1, 0, 0);

				lua_settop(state, 0);
			}
		);

		return;
	}

	task_scheduler->push([returned_func, state](DWORD) 
		{
			lua_State* L = state;
			lua_State* l_new = lua_newthread(L);

			const auto returns = returned_func(state);

			//yscriptcontext_resume(state->extra_space->script_context, ythread_ref{ state }, returns);
			lua_getfield(l_new, -10002, "task");
			lua_getfield(l_new, -1, "defer");

			lua_pushthread(L);
			lua_xmove(L, l_new, 1);

			for (int i = returns; i >= 1; i--) {
				lua_pushvalue(L, -i);
				lua_xmove(L, l_new, 1);
			}

			lua_pcall(l_new, returns + 1, 0, 0);
			lua_settop(l_new, 0);
		}
	);
}

int roblox_yield_t::execute(const std::function<yield_ret()>& YieldedFunction) const
{
	static auto utilities = cherry::utilities::get_singleton();
	auto state = L;
	uintptr_t state_ptr = (uintptr_t)L;
	static std::map<uintptr_t, std::string> YieldMap;

	auto& YMS = YieldMap[state_ptr];
	if (YMS.empty())
		YMS = utilities->random_string(16);

	lua_pushthread(L);
	lua_setfield(L, LUA_REGISTRYINDEX, YMS.c_str());

	std::thread(THREAD_FUNC, YieldedFunction, state).detach();

	L->base = L->top; // protect stack slots below
	L->status = LUA_YIELD;

	L->ci->flags |= 1;
	return -1;
}
#pragma endregion