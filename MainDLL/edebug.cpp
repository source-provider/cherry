#include "environment.hpp"
#include <luau/vm/src/lapi.h>
#include <luau/vm/src/lgc.h>
#include <algorithm>
#include <cctype>
#include <string>
#include <luau/vm/src/lfunc.h>
#include <Luau/Compiler.h>
#include <Luau/BytecodeBuilder.h>

using namespace cherry::global;

auto debug_getconstants(lua_State* L) -> int {
	ARG_CHECK(L, 1, 1);

	if (lua_isfunction(L, 1) == false && lua_isnumber(L, 1) == false) {
		luaL_argerror(L, 1, "function or number expected");
		return 0;
	}


	if (lua_isnumber(L, 1)) {
		lua_Debug ar;

		if (!lua_getinfo(L, lua_tonumber(L, 1), "f", &ar)) {
			luaL_error(L, "level out of range");
			return 0;
		}

		if (lua_iscfunction(L, -1)) {
			luaL_argerror(L, 1, "stack points to a C closure, Lua function expected");
			return 0;
		}
	}
	else {
		lua_pushvalue(L, 1);

		if (lua_iscfunction(L, -1)) {
			luaL_argerror(L, 1, "Lua function expected");
			return 0;
		}
	}

	Closure* cl = (Closure*)lua_topointer(L, -1);
	Proto* p = cl->l.p;
	TValue* k = p->k;

	lua_newtable(L);

	for (int i = 0; i < p->sizek; i++) {
		TValue* tval = &(k[i]);

		if (tval->tt == LUA_TFUNCTION) {
			TValue* i_o = (L->top);
			setnilvalue(i_o);
			L->top++;
		}
		else {
			TValue* i_o = (L->top);
			i_o->value = tval->value;
			i_o->tt = tval->tt;
			L->top++;
		}

		lua_rawseti(L, -2, (i + 1));
	}

	return 1;
}

auto debug_getconstant(lua_State* L) -> int {
	ARG_CHECK(L, 2, 2);

	if (lua_isfunction(L, 1) == false && lua_isnumber(L, 1) == false) {
		luaL_argerror(L, 1, "function or number expected");
		return 0;
	}

	const int index = luaL_checkinteger(L, 2);

	if (lua_isnumber(L, 1)) {
		lua_Debug ar;

		if (!lua_getinfo(L, lua_tonumber(L, 1), "f", &ar)) {
			luaL_error(L, "level out of range");
			return 0;
		}

		if (lua_iscfunction(L, -1)) {
			luaL_argerror(L, 1, "stack points to a C closure, Lua function expected");
			return 0;
		}
	}
	else {
		lua_pushvalue(L, 1);

		if (lua_iscfunction(L, -1)) {
			luaL_argerror(L, 1, "Lua function expected");
			return 0;
		}
	}

	Closure* cl = (Closure*)lua_topointer(L, -1);
	Proto* p = cl->l.p;
	TValue* k = p->k;

	if (!index) {
		luaL_argerror(L, 1, "constant index starts at 1");
		return 0;
	}

	if (index > p->sizek) {
		luaL_argerror(L, 1, "constant index is out of range");
		return 0;
	}

	TValue* tval = &(k[index - 1]);

	if (tval->tt == LUA_TFUNCTION) {
		TValue* i_o = (L->top);
		setnilvalue(i_o);
		L->top++;
	}
	else {
		TValue* i_o = (L->top);
		i_o->value = tval->value;
		i_o->tt = tval->tt;
		L->top++;
	}


	return 1;
}

auto debug_setconstant(lua_State* L)  -> int {
	ARG_CHECK(L, 3, 3);

	if (lua_isfunction(L, 1) == false && lua_isnumber(L, 1) == false) {
		luaL_argerror(L, 1, "function or number expected");
		return 0;
	}

	const int index = luaL_checkinteger(L, 2);

	luaL_checkany(L, 3);

	if (lua_isnumber(L, 1)) {
		lua_Debug ar;

		if (!lua_getinfo(L, lua_tonumber(L, 1), "f", &ar)) {
			luaL_error(L, "level out of range");
			return 0;
		}

		if (lua_iscfunction(L, -1)) {
			luaL_argerror(L, 1, "stack points to a C closure, Lua function expected");
			return 0;
		}
	}
	else {
		lua_pushvalue(L, 1);

		if (lua_iscfunction(L, -1)) {
			luaL_argerror(L, 1, "Lua function expected");
			return 0;
		}
	}

	Closure* cl = (Closure*)lua_topointer(L, -1);
	Proto* p = cl->l.p;
	TValue* k = p->k;

	if (!index) {
		luaL_argerror(L, 1, "constant index starts at 1");
		return 0;
	}

	if (index > p->sizek) {
		luaL_argerror(L, 1, "constant index is out of range");
		return 0;
	}

	auto constant = &k[index - 1]; /* Lua-based indexing */

	if (constant->tt == LUA_TFUNCTION) {
		return 0;
	}

	const TValue* new_t = luaA_toobject(L, 3);
	constant->tt = new_t->tt;
	constant->value = new_t->value;

	return 0;
}

auto debug_getupvalues(lua_State* L) -> int {
	ARG_CHECK(L, 1, 1);
	if (lua_isfunction(L, 1) == false && lua_isnumber(L, 1) == false) {
		luaL_argerror(L, 1, "function or number expected");
		return 0;
	}

	if (lua_isnumber(L, 1)) {
		lua_Debug ar;

		if (!lua_getinfo(L, lua_tonumber(L, 1), "f", &ar)) {
			luaL_error(L, "level out of range");
			return 0;
		}
	}
	else {
		lua_pushvalue(L, 1);
	}

	Closure* closure = (Closure*)lua_topointer(L, -1);
	TValue* upvalue_table = (TValue*)nullptr;

	lua_newtable(L);


	if (!closure->isC)
		upvalue_table = closure->l.uprefs;
	else if (closure->isC)
		upvalue_table = closure->c.upvals;

	for (int i = 0; i < closure->nupvalues; i++) {
		TValue* upval = (&upvalue_table[i]);
		TValue* top = L->top;

		top->value = upval->value;
		top->tt = upval->tt;
		L->top++;

		lua_rawseti(L, -2, (i + 1));
	}

	return 1;
}

auto debug_getupvalue(lua_State* L) -> int {
	ARG_CHECK(L, 2, 2);
	if (lua_isfunction(L, 1) == false && lua_isnumber(L, 1) == false) {
		luaL_argerror(L, 1, "function or number expected");
		return 0;
	}

	if (lua_isnumber(L, 1)) {
		lua_Debug ar;

		if (!lua_getinfo(L, lua_tonumber(L, 1), "f", &ar)) {
			luaL_error(L, "level out of range");
			return 0;
		}
	}
	else
		lua_pushvalue(L, 1);

	const int index = luaL_checkinteger(L, 2);

	Closure* closure = (Closure*)lua_topointer(L, -1);
	TValue* upvalue_table = (TValue*)nullptr;

	if (!closure->isC)
		upvalue_table = closure->l.uprefs;
	else if (closure->isC)
		upvalue_table = closure->c.upvals;

	if (!index) {
		luaL_argerror(L, 1, "upvalue index starts at 1");
		return 0;
	}

	if (index > closure->nupvalues) {
		luaL_argerror(L, 1, "upvalue index is out of range");
		return 0;
	}

	TValue* upval = (&upvalue_table[index - 1]);
	TValue* top = L->top;

	top->value = upval->value;
	top->tt = upval->tt;
	L->top++;

	return 1;
}

auto debug_setupvalue(lua_State* L) -> int {
	ARG_CHECK(L, 3, 3);
	if (lua_isfunction(L, 1) == false && lua_isnumber(L, 1) == false) {
		luaL_argerror(L, 1, "function or number expected");
		return 0;
	}


	const int index = luaL_checkinteger(L, 2);
	luaL_checkany(L, 3);

	if (lua_isnumber(L, 1)) {
		lua_Debug ar;

		if (!lua_getinfo(L, lua_tonumber(L, 1), "f", &ar)) {
			luaL_error(L, "level out of range");
			return 0;
		}
	}
	else
		lua_pushvalue(L, 1);

	Closure* closure = (Closure*)lua_topointer(L, -1);
	const TValue* value = luaA_toobject(L, 3);
	TValue* upvalue_table = (TValue*)nullptr;

	if (!closure->isC)
		upvalue_table = closure->l.uprefs;
	else if (closure->isC)
		upvalue_table = closure->c.upvals;

	if (!index) {
		luaL_argerror(L, 1, "upvalue index starts at 1");
		return 0;
	}

	if (index > closure->nupvalues) {
		luaL_argerror(L, 1, "upvalue index is out of range");
		return 0;
	}

	TValue* upvalue = (&upvalue_table[index - 1]);

	upvalue->value = value->value;
	upvalue->tt = value->tt;

	luaC_barrier(L, closure, value);
	lua_pushboolean(L, true);

	return 1;
}

auto debug_getprotos(lua_State* L) -> int {
	ARG_CHECK(L, 1, 1, luaL_checktype(L, 1, LUA_TFUNCTION););

	if (lua_iscfunction(L, -1)) {
		luaL_argerror(L, 1, "stack points to a C closure, Lua function expected");
		return 0;
	}

	Closure* closure = (Closure*)lua_topointer(L, -1);

	lua_newtable(L);

	Proto* main_proto = closure->l.p;

	for (int i = 0; i < main_proto->sizep; i++) {
		Proto* proto_data = main_proto->p[i];
		Closure* lclosure = luaF_newLclosure(L, proto_data->nups, closure->env, proto_data);

		setclvalue(L, L->top, lclosure);
		L->top++;

		lua_rawseti(L, -2, (i + 1));
	}

	return 1;
}

/*
Closure* closure = (Closure*)lua_topointer(L, 1);
	const int index = lua_tonumber(L, 2);

	lua_newtable(L);

	Proto* main_proto = closure->l.p;

	if (!index)
	{
		luaL_argerror(L, 1, "proto index starts at 1");
		return 0;
	}

	if (index > main_proto->sizep)
	{
		luaL_argerror(L, 1, "proto index is out of range");
		return 0;
	}

	Proto* proto_data = main_proto->p[index - 1];
	Closure* lclosure = luaF_newLclosure(L, proto_data->nups, closure->env, proto_data);

	setclvalue(L, L->top, lclosure);
	L->top++;
*/

auto debug_getproto(lua_State* L) -> int {
	ARG_CHECK(L, 2, 3);
	luaL_checktype(L, 2, LUA_TNUMBER);

	bool active = luaL_optboolean(L, 3, false);

	if (lua_isnumber(L, 1)) {
		int level = lua_tointeger(L, 1);

		if (level >= L->ci - L->base_ci || level < 0) {
			luaL_argerror(L, 1, "stack level out of range");
		}

		lua_Debug ar;
		lua_getinfo(L, level, "f", &ar);

		if (clvalue(reinterpret_cast<CallInfo*>(L->ci - level)->func)->isC) {
			luaL_argerror(L, 1, "stack level to a cclosure, lclosure expected");
		}
	}
	else {
		lua_pushvalue(L, 1);
	}

	if (lua_isnumber(L, -1) == FALSE && lua_isfunction(L, -1) == FALSE) {
		luaL_argerror(L, 1, "function or level expected");
		return 0;
	}

	const auto function = clvalue(luaA_toobject(L, -1));

	if (!function->isC) {
		if (active) {
			lua_newtable(L);
		}

		const auto index = lua_tointeger(L, 2);

		if (index < 1 || index > function->l.p->sizep) {
			luaL_argerror(L, 2, "proto index out of range");
		}

		const auto proto = function->l.p->p[index - 1];

		setclvalue(L, L->top, luaF_newLclosure(L, proto->nups, function->env, proto));
		L->top++;

		if (active) {
			lua_rawseti(L, -2, 1);
		}
	}
	else {
		luaL_argerror(L, 1, "lclosure expected");
	}

	return 1;
}

auto debug_getstack(lua_State* L) -> int {
	ARG_CHECK(L, 1, 2);
	luaL_checktype(L, 1, LUA_TNUMBER);

	const auto level = lua_tointeger(L, 1);
	const auto index = luaL_optinteger(L, 2, -1);

	if (level >= L->ci - L->base_ci || level < 0) {
		luaL_argerror(L, 1, "level out of range");
	}

	const auto frame = reinterpret_cast<CallInfo*>(L->ci - level);
	const auto top = (frame->top - frame->base);

	if (clvalue(frame->func)->isC) {
		luaL_argerror(L, 1, "level points to a cclosure, lclosure expected");
	}

	if (index == -1) {
		lua_newtable(L);

		for (int i = 0; i < top; i++) {
			setobj2s(L, L->top, &frame->base[i]);
			L->top++;

			lua_rawseti(L, -2, i + 1);
		}
	}
	else {
		if (index < 1 || index > top) {
			luaL_argerror(L, 2, "stack index out of range");
		}

		setobj2s(L, L->top, &frame->base[index - 1]);
		L->top++;
	}
	return 1;
}

auto debug_setstack(lua_State* L) -> int {
	ARG_CHECK(L, 3, 3);
	luaL_checktype(L, 1, LUA_TNUMBER);
	luaL_checktype(L, 2, LUA_TNUMBER);
	luaL_checkany(L, 3);

	const auto level = lua_tointeger(L, 1);
	const auto index = lua_tointeger(L, 2);

	if (level >= L->ci - L->base_ci || level < 0) {
		luaL_argerror(L, 1, "level out of range");
	}

	const auto frame = reinterpret_cast<CallInfo*>(L->ci - level);
	const auto top = (frame->top - frame->base);

	if (clvalue(frame->func)->isC) {
		luaL_argerror(L, 1, "level points to a cclosure, lclosure expected");
	}

	if (index < 1 || index > top) {
		luaL_argerror(L, 2, "stack index out of range");
	}

	setobj2s(L, &frame->base[index - 1], luaA_toobject(L, 3));
	return 0;
}

auto debug_getinfo(lua_State* L) -> int {
	ARG_CHECK(L, 1, 1);
	int level{};
	if (lua_isnumber(L, 1)) {
		level = lua_tointeger(L, 1);
		luaL_argcheck(L, level >= 0, 1, "level can't be negative");
	}
	else if (lua_isfunction(L, 1)) {
		level = -lua_gettop(L);
	}
	else {
		luaL_argerror(L, 1, "function or level expected");
		return 0;
	}

	lua_Debug ar;
	if (!lua_getinfo(L, level, "sluanf", &ar))
		luaL_argerror(L, 1, "invalid level");

	lua_newtable(L);

	lua_pushstring(L, ar.source);
	lua_setfield(L, -2, "source");

	lua_pushstring(L, ar.short_src);
	lua_setfield(L, -2, "short_src");

	lua_pushvalue(L, 1);
	lua_setfield(L, -2, "func");

	lua_pushstring(L, ar.what);
	lua_setfield(L, -2, "what");

	lua_pushinteger(L, ar.currentline);
	lua_setfield(L, -2, "currentline");

	lua_pushstring(L, ar.name);
	lua_setfield(L, -2, "name");

	lua_pushinteger(L, ar.nupvals);
	lua_setfield(L, -2, "nups");

	lua_pushinteger(L, ar.nparams);
	lua_setfield(L, -2, "numparams");

	lua_pushinteger(L, ar.isvararg);
	lua_setfield(L, -2, "is_vararg");

	return 1;
}

auto getregistry(lua_State* L) -> int {
	ARG_CHECK(L, 0, 0);
	lua_pushvalue(L, LUA_REGISTRYINDEX);
	return 1;
}

/* OTHERS */
auto getbase(lua_State* L) -> int {
	ARG_CHECK(L, 0, 0);
	lua_pushnumber(L, cherry::encryptions::rebase(0));
	return 1;
}

auto dumpfunction(lua_State* L) -> int {
	ARG_CHECK(L, 1, 1, luaL_checktype(L, 1, LUA_TFUNCTION););
	Closure* cl = (Closure*)lua_topointer(L, 1);
	lua_CFunction closure = cl->c.f;
	uintptr_t addr = (uintptr_t)closure;
	lua_pushnumber(L, cherry::encryptions::unbase(addr));
	return 1;
}

auto dumpcontinuation(lua_State* L) -> int {
	ARG_CHECK(L, 1, 1, luaL_checktype(L, 1, LUA_TFUNCTION););
	Closure* cl = (Closure*)lua_topointer(L, 1);
	lua_Continuation closure = cl->c.cont;
	uintptr_t addr = (uintptr_t)closure;
	lua_pushnumber(L, cherry::encryptions::unbase(addr));
	return 1;
}

auto compile(lua_State* L) -> int {
	ARG_CHECK(L, 1, 1, luaL_checktype(L, 1, LUA_TSTRING););
	std::string code = lua_tostring(L, 1);

	struct : Luau::BytecodeEncoder {
		auto encodeOp(const std::uint8_t op) -> uint8_t override {
			return op * 227;
		}
	} bytecode_encoder;

	std::string bytecode = Luau::compile(code, {}, {}, &bytecode_encoder);

	lua_pushlstring(L, bytecode.c_str(), bytecode.size());
	return 1;
}

auto get_actor_state(lua_State* L) -> int {
	ARG_CHECK(L, 1, 1, luaL_checktype(L, 1, LUA_TUSERDATA););
	int instance = dereference_pointer(lua_topointer(L, 1));

	int actor_state = ((instance + 108) ^ *(int*)(instance + 108));
	std::cout << "state - 0x" << std::hex << actor_state << std::endl;

	lua_State* LL = reinterpret_cast<lua_State*>(actor_state);
	std::cout << "tt - 0x" << std::hex << LL->tt << std::endl;

	lua_pushthread(LL);
	lua_xmove(LL, L, 1);
	return 1;
};

/* registering */
static const luaL_Reg env_funcs[] = { // {"NAME", FUNC},
	{"getbase", getbase},
	{"dumpfunction", dumpfunction},
	{"dumpfunc", dumpfunction},
	{"dumpcontinuation", dumpcontinuation},
	{"dumpcont", dumpcontinuation},
	{"compile", compile},
	//{"get_actor_state", get_actor_state},
	

	{"getconstants", debug_getconstants},
	{"getconstant", debug_getconstant},
	{"setconstant", debug_setconstant},

	{"getupvalues", debug_getupvalues},
	{"getupvalue", debug_getupvalue},
	{"setupvalue", debug_setupvalue},

	{"getprotos", debug_getprotos},
	{"getproto", debug_getproto},

	{"getstack", debug_getstack},
	{"setstack", debug_setstack},

	{"getinfo", debug_getinfo},

	{"getregistry", getregistry},
	{NULL, NULL},
};


void cherry::environment::luauopen_debug(lua_State* L)
{
	lua_pushvalue(L, LUA_GLOBALSINDEX);
	luaL_register(L, NULL, env_funcs);

	lua_getglobal(L, "debug");
	lua_setreadonly(L, -1, false);

	lua_settop(L, 0);

	luaL_register(L, "debug", env_funcs);

	lua_pop(L, 1);
}