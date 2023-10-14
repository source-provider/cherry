#include "environment.hpp"
#include <luau/vm/src/lmem.h>
#include <luau/vm/src/lapi.h>
using namespace cherry::global;
std::map<int, int> CLOSURE_MAP = {};

/* custom functions */
auto getscriptclosure(lua_State* L) -> int {
    ARG_CHECK(L, 1, 1);
    instance_t* instance = (instance_t*)dereference_pointer(lua_touserdata(L, 1));
    std::string class_name = *instance->class_descriptor->class_name;

    if (class_name == "ModuleScript") {
        static cherry::offsets::execution::roblox_function_t debug_loadmodule = NULL;
        if (debug_loadmodule == NULL) {
            lua_State* thread1 = lua_newthread(L);
            lua_pop(L, 1); /* pops the lua state pushed onto the stack */

            lua_getfield(thread1, -10002, "debug");
            lua_getfield(thread1, -1, "loadmodule");

            Closure* cl = (Closure*)lua_topointer(thread1, -1);
            lua_CFunction func = cl->c.f;

            //std::cout << "[DEBUG.LOADMODULE]: 0x" << std::hex << cherry::encryptions::unbase((int)func) << "\n";

            debug_loadmodule = (cherry::offsets::execution::roblox_function_t)((int)func);

            lua_settop(thread1, 0);
            //lua_close(thread1);
        }
        *reinterpret_cast<bool*>(cherry::offsets::loadmodule_flag) = true;
        debug_loadmodule((int)L);
        *reinterpret_cast<bool*>(cherry::offsets::loadmodule_flag) = false;

        lua_pop(L, 2);
    }
    else {
        std::string compressed_bytecode = get_script_bytecode(L, false);
        cherry::offsets::execution::luavm_load((int)L, &compressed_bytecode, ("=" + *instance->name).c_str(), 0);
    }

    return 1;
}

std::map<Closure*, Closure*> newcclosure_map;
auto newcclosure_handler(lua_State* L) -> int {
    const auto nargs = lua_gettop(L);

    void* real_closure = reinterpret_cast<void*>(newcclosure_map.find(clvalue(L->ci->func))->second);
        
    if (real_closure == nullptr)
        return 0;

    L->top->value.p = real_closure;
    L->top->tt = LUA_TFUNCTION;
    L->top++;



    lua_insert(L, 1);

    const char* error;
    const auto res = lua_pcall(L, nargs, LUA_MULTRET, 0);

    if (res && res != LUA_YIELD && (error = lua_tostring(L, -1), !std::strcmp(error, "attempt to yield across metamethod/C-call boundary")))
    {
        return lua_yield(L, 0);
    }

    return lua_gettop(L);
}

int closure_count = 0;
auto newcclosure(lua_State* L) -> int { 
    ARG_CHECK(L, 1, 1, luaL_checktype(L, 1, LUA_TFUNCTION););

    if (lua_iscfunction(L, 1)) {
        lua_pushvalue(L, 1);
        return 1;
    }

    lua_ref(L, 1);
    lua_pushcclosure(L, newcclosure_handler, 0, 0);
    newcclosure_map[&luaA_toobject(L, -1)->value.gc->cl] = &luaA_toobject(L, 1)->value.gc->cl;

    return 1;
}

auto blank_func(lua_State* L) -> int {
    return 0;
}

auto hookfunction(lua_State* L) -> int {
    ARG_CHECK(L, 2, 2, luaL_checktype(L, 1, LUA_TFUNCTION););
    luaL_checktype(L, 2, LUA_TFUNCTION);

    Closure* hook_to = (Closure*)lua_topointer(L, 1);
    Closure* hook_with = (Closure*)lua_topointer(L, 2);

    if (hook_to->isC == TRUE) {
        lua_pushvalue(L, 2);
        lua_setfield(L, -10000, utilities->random_string(16).c_str());

        lua_CFunction func1 = hook_with->c.f;

        lua_clonefunction(L, 1); /* clone the cclosure */

        hook_to->c.f = blank_func; /* we don't wanna break while we set upvalues */

        for (auto i = 0; i < hook_with->nupvalues; i++) {
            TValue* old_tval = &hook_to->c.upvals[i];
            TValue* hook_tval = &hook_with->c.upvals[i];

            old_tval->value = hook_tval->value;
            old_tval->tt = hook_tval->tt;
        }

        hook_to->nupvalues = hook_with->nupvalues;
        hook_to->c.f = func1;

        return 1;
    }
    else {
        lua_pushvalue(L, 2);
        lua_setfield(L, -10000, utilities->random_string(16).c_str());

        Proto* n_p = hook_with->l.p;

        lua_clonefunction(L, 1); /* clones the lclosure */


        hook_to->l.p = n_p;

        hook_to->nupvalues = hook_with->nupvalues;
        //hook_to->env = hook_with->env; /* not sure if this could be a vuln because you could hook a game lclosure and be able to access our env (KEEPING FOR DEBUG)*/

        hook_to->stacksize = hook_with->stacksize;
        hook_to->preload = hook_with->preload;

        for (int i = 0; i < hook_with->nupvalues; ++i)
            setobj2n(L, &hook_to->l.uprefs[i], &hook_with->l.uprefs[i]);

        return 1;
    }

    return 0;
}

auto checkcaller(lua_State* L) -> int {
    ARG_CHECK(L, 0 ,0);
    lua_pushboolean(L, (L->extra_space->context_level > 3));
    return 1;
}

auto getcallingscript(lua_State* L) -> int { /* this is for testing and it's temp */
    ARG_CHECK(L, 0, 0);

    lua_pushvalue(L, -10002);
    lua_getfield(L, -1, "script");

    return 1;
}

auto islclosure(lua_State* L) -> int { 
    ARG_CHECK(L, 1, 1, luaL_checktype(L, 1, LUA_TFUNCTION););
    lua_pushboolean(L, !lua_iscfunction(L, 1));
    return 1;
}

auto iscclosure(lua_State* L) -> int {
    ARG_CHECK(L, 1, 1, luaL_checktype(L, 1, LUA_TFUNCTION););
    lua_pushboolean(L, lua_iscfunction(L, 1));
    return 1;
}

auto checkclosure(lua_State* L) -> int { 
    ARG_CHECK(L, 1, 1, luaL_checktype(L, 1, LUA_TFUNCTION););
    Closure* cl = (Closure*)lua_topointer(L, 1);

    if (lua_iscfunction(L, 1)) {
        lua_CFunction cf = cl->c.f;
        auto closures = get_closures_made();
        lua_CFunction func = cl->c.f;
        bool is_closure = closures[reinterpret_cast<uintptr_t>(func)] == true ? true : false;

        lua_pushboolean(L, is_closure);
        return 1;
    }
    else {
        Proto* p = cl->l.p;
        TString* source = p->source;
        lua_pushboolean(L, (strcmp(source->data, cherry::configuration::source_name.c_str()) == 0));
        return 1;
    }
}

auto clonefunction(lua_State* L) -> int {
    ARG_CHECK(L, 1, 1, luaL_checktype(L, 1, LUA_TFUNCTION););
    Closure* cl = (Closure*)lua_topointer(L, 1);

    lua_clonefunction(L, 1);

    return 1;
}

/* registering */
static const luaL_Reg env_funcs[] = {
    {"getscriptclosure", getscriptclosure},
    {"getscriptfunction", getscriptclosure},
    {"newcclosure", newcclosure},
    {"hookfunction", hookfunction}, /* make it more stable (atleast try to) */
    {"replaceclosure", hookfunction},
    {"checkcaller", checkcaller},
    {"getcallingscript", getcallingscript},
    {"islclosure", islclosure},
    {"iscclosure", iscclosure},

    {"checkclosure", checkclosure},
    {"isourclosure", checkclosure},
    {"isexecutorclosure", checkclosure},

    {"clonefunction", clonefunction},
    {"clonefunc", clonefunction},

    {NULL, NULL}
};

auto cherry::environment::luauopen_closure(lua_State* L) -> void {
    lua_pushvalue(L, LUA_GLOBALSINDEX);
    luaL_register(L, NULL, env_funcs);

    lua_pop(L, 1);
}