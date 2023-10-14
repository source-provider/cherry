#include "environment.hpp"
#include <luau/vm/src/lmem.h>
#include <filesystem>

using namespace cherry::global;

namespace std {
	namespace filesystem {
		auto get_roblox_path() -> std::string {
			char ro_path[1200];
			GetModuleFileName(GetModuleHandle(NULL), ro_path, sizeof(ro_path));
			std::string path = ro_path;

			path = utilities->replace(path, "RobloxPlayerBeta.exe", "");

			return path;
		};
	}
}

/*
FUNCTIONS TODO:
	getsenv
	getloadedmodules
	gethiddenproperty | sethiddenproperty | gethiddenproperties
	getscriptbytecode/dumpstring | getscripthash (full on getscriptbytecode)
*/

/* getconnections */
struct connection_t {
	int object;
	void* orignal_object;
	int old_state;
};
std::unordered_map<int, connection_t> connections;

auto enable_connection(lua_State* L) -> int {
	const auto signal = *(int*)lua_touserdata(L, 1);

	if (!connections.count(signal)) {
		connection_t new_c;
		new_c.orignal_object = lua_touserdata(L, 1);
		new_c.object = signal;
		new_c.old_state = *reinterpret_cast<int*>(signal + cherry::offsets::signal::state);

		connections[signal] = new_c;
	}
	
	*reinterpret_cast<int*>(signal + cherry::offsets::signal::state) = connections[signal].old_state;
	return 0;
}

auto disable_connection(lua_State* L) -> int {
	const auto signal = *(int*)lua_touserdata(L, 1);

	if (!connections.count(signal)) {
		connection_t new_c;
		new_c.orignal_object = lua_touserdata(L, 1);
		new_c.object = signal;
		new_c.old_state = *reinterpret_cast<int*>(signal + cherry::offsets::signal::state);

		connections[signal] = new_c;
	}

	*reinterpret_cast<int*>(signal + cherry::offsets::signal::state) = 0;
	return 0;
}

auto blank_function(lua_State* L) -> int {
	return 0;
}

auto index_connection(lua_State* L) -> int {
	const std::string key = lua_tolstring(L, 2, nullptr);

	if (key == "Enable" || key == "enable") {
		lua_pushvalue(L, 1);
		lua_pushcfunction(L, enable_connection, 0);
	}
	else if (key == "Function" || key == "function" || key == "Fire" || key == "fire" || key == "Defer" || key == "defer") {
		const auto connection = *reinterpret_cast<connection_t*>(lua_touserdata(L, 1));
		lua_getref(L, *reinterpret_cast<int*>(*reinterpret_cast<int*>(*reinterpret_cast<int*>(connection.object + cherry::offsets::signal::ref) + cherry::offsets::signal::ref1) + cherry::offsets::signal::index));

		if (lua_type(L, -1) <= 0)
			lua_pushcfunction(L, blank_function, 0);
	}
	else if (key == "Enabled" || key == "enabled" || key == "State" || key == "state") {
		const auto Signal = *reinterpret_cast<int*>(lua_touserdata(L, 1));

		const auto conn = *reinterpret_cast<int*>(Signal + cherry::offsets::signal::state);

		lua_pushboolean(L, conn);
		//*reinterpret_cast<int*>(Signal + rbxscriptsignal_state) = 0;
	}
	else if (key == "Disable" || key == "disable" || key == "Disconnect" || key == "disconnect") /* doing Disconnect this way but will chjange it later */ {
		lua_pushvalue(L, 1);
		lua_pushcfunction(L, disable_connection, 0);
	}
	else if (key == "LuaConnection" || key == "luaconnection") {
		const auto connection = *reinterpret_cast<connection_t*>(lua_touserdata(L, 1));
		lua_getref(L, *reinterpret_cast<int*>(*reinterpret_cast<int*>(*reinterpret_cast<int*>(connection.object + cherry::offsets::signal::ref) + cherry::offsets::signal::ref1) + cherry::offsets::signal::index));

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
}


/* custom functions */
auto loadstring(lua_State* L) -> int {
    ARG_CHECK(L, 1, 2, luaL_checktype(L, 1, LUA_TSTRING););
    const char* code = lua_tostring(L, 1);
    const char* chunk_name = luaL_optstring(L, 2, cherry::configuration::source_name.c_str());

    return execution->send((int)L, code, false);
}

auto getidentity(lua_State* L) -> int {
	ARG_CHECK(L, 0, 0);
	lua_pushnumber(L, L->extra_space->context_level);
	return 1;
}

auto setidentity(lua_State* L) -> int {
	ARG_CHECK(L, 1, 2, luaL_checktype(L, 1, LUA_TNUMBER););
	L->extra_space->context_level = lua_tonumber(L, 1);
	return 0;
}

auto getscriptfromthread(lua_State* L) -> int {
	ARG_CHECK(L, 1, 1, luaL_checktype(L, 1, LUA_TTHREAD););
	lua_State* thread = lua_tothread(L, 1);

	if (thread->extra_space->script_ptr != NULL)
		cherry::offsets::push_instance((int)L, (void*)thread->extra_space->script_ptr);
	else
		lua_pushnil(L);

	return 1;
}

auto gethui(lua_State* L) -> int {
	ARG_CHECK(L, 0, 0);

	uint16_t old_identity = L->extra_space->context_level;
	L->extra_space->context_level = 8;

	lua_getglobal(L, "game");
	lua_getfield(L, -1, "GetService");

	lua_getglobal(L, "game");
	lua_pushstring(L, "CoreGui");

	lua_pcall(L, 2, 1, 0);

	L->extra_space->context_level = old_identity;
	
	return 1;
}

auto getgenv(lua_State* L) -> int {
	lua_pushvalue(L, LUA_GLOBALSINDEX);
	return 1;
}

auto getrenv(lua_State* L) -> int {
	ARG_CHECK(L, 0, 0);
	sethvalue(L, L->top, cherry::environment::get_roblox_env());
	L->top++;
	return 1;
}

auto getreg(lua_State* L) -> int {
	ARG_CHECK(L, 0, 0);
	lua_pushvalue(L, LUA_REGISTRYINDEX);
	return 1;
}

auto getgc(lua_State* L) -> int {
	ARG_CHECK(L, 0, 1);
	global_State* g = L->global;
	bool tables = luaL_optboolean(L, 1, FALSE);
	lua_createtable(L, 0, 0);
	int n = 0;

	for (lua_Page* page = g->allgcopages; page;)
	{
		lua_Page* next = page->gcolistnext; // block visit might destroy the page

		char* start;
		char* end;
		int busy_blocks;
		int block_size;
		luaM_getpagewalkinfo(page, &start, &end, &busy_blocks, &block_size);

		for (char* pos = start; pos != end; pos += block_size)
		{
			GCObject* gco = (GCObject*)pos;
			auto type = gco->gch.tt;

			if (type == lua_Type::LUA_TUSERDATA || type == lua_Type::LUA_TFUNCTION || (type == lua_Type::LUA_TTABLE && tables))
			{
				n += 1;
				TValue* top = L->top;
				top->value.p = reinterpret_cast<void*>(gco);
				top->tt = type;
				L->top++;

				lua_rawseti(L, -2, n);
			}
		}

		page = next;
	}

	return 1;
}

auto getsenv(lua_State* L) -> int {
	ARG_CHECK(L, 1, 1);
	return 1;
}

auto getinstancelist(lua_State* L) -> int { /* this is for getinstances and getnilinstances */
	ARG_CHECK(L, 0, 0);

	lua_pushvalue(L, LUA_REGISTRYINDEX);
	lua_pushlightuserdata(L, (void*)cherry::offsets::push_instance);
	lua_gettable(L, -2);

	return 1;
}


auto getconnections(lua_State* L) -> int { 
	ARG_CHECK(L, 1, 1, luaL_checktype(L, 1, LUA_TUSERDATA););

	lua_getfield(L, 1, "Connect");
	lua_pushvalue(L, 1);
	lua_pushcclosurek(L, blank_function, NULL, NULL, NULL);
	lua_pcall(L, 2, 1, 0);

	const auto signal = *reinterpret_cast<int*>(lua_touserdata(L, -1));
	auto next = *reinterpret_cast<int*>(signal + cherry::offsets::signal::next);

	lua_createtable(L, 0, 0);
	auto count = 1;

	while (next != 0) {

		if (connections.count(next)) {
			*reinterpret_cast<connection_t*>(lua_newuserdata(L, sizeof(connection_t), 0)) = connections[next];

			lua_createtable(L, 0, 0);
			lua_pushcfunction(L, index_connection, 0);
			lua_setfield(L, -2, "__index");
			lua_pushstring(L, "table");
			lua_setfield(L, -2, "__type");
			lua_setmetatable(L, -2);
		}
		else {
			connection_t new_connection;
			new_connection.orignal_object = lua_touserdata(L, 1);
			new_connection.object = next;
			new_connection.old_state = *reinterpret_cast<int*>(next + cherry::offsets::signal::state);

			*reinterpret_cast<connection_t*>(lua_newuserdata(L, sizeof(connection_t), 0)) = new_connection;

			lua_createtable(L, 0, 0);
			//r_lua_newtable(rL);
			lua_pushcfunction(L, index_connection, 0);
			lua_setfield(L, -2, "__index");
			lua_pushstring(L, "table");
			lua_setfield(L, -2, "__type");
			lua_setmetatable(L, -2);

			connections[next] = new_connection;
		}

		lua_rawseti(L, -2, count++);
		next = *reinterpret_cast<int*>(next + cherry::offsets::signal::next);
	}

	lua_getfield(L, -2, "Disconnect");
	lua_pushvalue(L, -3);
	lua_pcall(L, 1, 0, 0);

	return 1;
}

auto firesignal(lua_State* L) -> int {
	ARG_CHECK(L, 1, 1, luaL_checktype(L, 1, LUA_TUSERDATA););

	lua_getfield(L, 1, "Connect");
	lua_pushvalue(L, 1);
	lua_pushcclosurek(L, blank_function, NULL, NULL, NULL);
	lua_pcall(L, 2, 1, 0);

	const auto signal = *reinterpret_cast<int*>(lua_touserdata(L, -1));
	int next = *reinterpret_cast<int*>(signal + cherry::offsets::signal::next);

	lua_getfield(L, -1, "Disconnect");
	lua_pushvalue(L, -2);
	lua_pcall(L, 1, 0, 0);
	lua_pop(L, 1);

	lua_remove(L, 1);

	int top = lua_gettop(L);
	while (next != 0) {
		lua_getref(L, *reinterpret_cast<int*>(*reinterpret_cast<int*>(*reinterpret_cast<int*>(next + cherry::offsets::signal::ref) + cherry::offsets::signal::ref1) + cherry::offsets::signal::index));

		if (lua_type(L, -1) <= 0)
			lua_pushcfunction(L, blank_function, 0);

		for (int i = 1; i <= top; i++)
			lua_pushvalue(L, i);

		lua_pcall(L, top, 0, 0);

		next = *reinterpret_cast<int*>(next + cherry::offsets::signal::next);
	}

	return 0;
}

auto gethiddenproperty(lua_State* L) -> int {
	ARG_CHECK(L, 2, 2, luaL_checktype(L, 1, LUA_TUSERDATA););
	luaL_checktype(L, 2, LUA_TSTRING);

	/*instance_t* instance = (instance_t*)dereference_pointer(lua_touserdata(L, 1));
	for (property_descriptor* property : instance->class_descriptor->properties) {
		int is_hidden = (bool)(property->isscriptable < 32);
		std::cout << "	" << *property->name << "	" << (int)property->isscriptable << " - " << (bool)(property->isscriptable < 32) << "\n";
	}*/

	return 0;
}

auto sethiddenproperty(lua_State* L) -> int {
	ARG_CHECK(L, 0, 0);
	return 0;
}

auto setscriptable(lua_State* L) -> int {
	ARG_CHECK(L, 3, 3, luaL_checktype(L, 1, LUA_TUSERDATA););
	luaL_checktype(L, 2, LUA_TSTRING);
	luaL_checktype(L, 3, LUA_TBOOLEAN);

	instance_t* instance = (instance_t*)dereference_pointer(lua_touserdata(L, 1));
	std::string prop_name = lua_tostring(L, 2);
	bool bool_mode = lua_toboolean(L, 3);

	for (property_descriptor* property : instance->class_descriptor->properties) {
		std::string property_name = *property->name;
		int property_ = reinterpret_cast<int>(property);
		if (property_name == prop_name) {
			lua_pushboolean(L, (bool)(*reinterpret_cast<int*>(property_ + cherry::offsets::instance::scriptable) > 32));

			*reinterpret_cast<int*>(property_ + cherry::offsets::instance::scriptable) = (bool_mode ? 33 : 32);

			return 1;
		}
		/*lua_pushlstring(L, property_name.c_str(), property_name.size());
		lua_rawseti(L, -2, ++i);*/
	}

	return 0;
}

auto isscriptable(lua_State* L) -> int {
	ARG_CHECK(L, 2, 2, luaL_checktype(L, 1, LUA_TUSERDATA););
	luaL_checktype(L, 2, LUA_TSTRING);
	instance_t* instance = (instance_t*)dereference_pointer(lua_touserdata(L, 1));
	std::string prop_name = lua_tostring(L, 2);

	for (property_descriptor* property : instance->class_descriptor->properties) {
		std::string property_name = *property->name;
		if (property_name == prop_name) {
			lua_pushboolean(L, (bool)(property->isscriptable > 32));
			return 1;
		}
		/*lua_pushlstring(L, property_name.c_str(), property_name.size());
		lua_rawseti(L, -2, ++i);*/
	}

	return 0;
}

auto getproperties(lua_State* L) -> int {
	ARG_CHECK(L, 1, 1, luaL_checktype(L, 1, LUA_TUSERDATA););
	lua_newtable(L);

	instance_t* instance = (instance_t*)dereference_pointer(lua_touserdata(L, 1));
	int i = 0;

	for (property_descriptor* property : instance->class_descriptor->properties) {
		std::string property_name = *property->name;
		lua_pushlstring(L, property_name.c_str(), property_name.size());
		lua_rawseti(L, -2, ++i);
	}

	return 1;
}

auto gethiddenproperties(lua_State* L) -> int {
	ARG_CHECK(L, 0, 0);
	return 0;
}

auto getscriptbytecode(lua_State* L) -> int {
	ARG_CHECK(L, 1, 1);
	std::string bytecode = get_script_bytecode(L);

	if (bytecode == "") {
		lua_pushstring(L, "Invalid Bytecode");
		return 1;
	}

	lua_pushlstring(L, bytecode.c_str(), bytecode.size());
	return 1;
}

auto getscripthash(lua_State* L) -> int {
	ARG_CHECK(L, 1, 1);
	std::string bytecode = get_script_bytecode(L);
	std::string data = hash_with_algo<CryptoPP::SHA384>(bytecode);

	lua_pushlstring(L, data.c_str(), data.size());
	return 1;
}

auto cherry::environment::getobjects(lua_State* L) -> int {
	ARG_CHECK(L, 2, 2, luaL_checktype(L, 1, LUA_TUSERDATA););
	luaL_checktype(L, 2, LUA_TSTRING);

	//std::string rbx_id = lua_tostring(L, 2);
	//rbx_id = utilities::replace(rbx_id, "rbxassetid://", "");

	lua_pushvalue(L, 1);
	lua_getfield(L, -1, "GetService");
	lua_pushvalue(L, 1);
	lua_pushstring(L, "InsertService");

	lua_pcall(L, 2, 1, 0);

	lua_getfield(L, -1, "LoadLocalAsset");
	lua_pushvalue(L, -2);
	lua_pushvalue(L, 2);

	lua_pcall(L, 2, 1, 0);

	lua_newtable(L);
	lua_pushvalue(L, -2);
	lua_rawseti(L, -2, 1);

	return 1;
}

auto getcustomasset(lua_State* L) -> int {
	ARG_CHECK(L, 1, 1, luaL_checktype(L, 1, LUA_TSTRING););

	if (!std::filesystem::is_directory(std::filesystem::get_roblox_path() + ("\\content\\BA1C3BA115S")))
	{
		std::filesystem::create_directory(std::filesystem::get_roblox_path() + ("\\content\\BA1C3BA115S"));
	}

	auto workspace = std::filesystem::path(utilities->location()) / "workspace" / lua_tostring(L, 1);
	auto custom_asset = std::filesystem::path(std::filesystem::get_roblox_path()) / "content" / "BA1C3BA115S" / workspace.filename();

	std::filesystem::copy_file(workspace, custom_asset, std::filesystem::copy_options::update_existing);
	lua_pushstring(L, std::string("rbxasset://BA1C3BA115S/" + custom_asset.filename().string()).data());
	return 1;
}
//sub_1533190
const auto r_fireclickdetector = reinterpret_cast<void(__cdecl*)(int, float, int)>(cherry::encryptions::rebase(0x1533190, BASE_4));
auto fireclickdetector(lua_State* L) -> int {
	ARG_CHECK(L, 1, 2, luaL_checktype(L, 1, LUA_TUSERDATA););
	int click_detector = *reinterpret_cast<int*>(lua_touserdata(L, 1));

	float distance = 0.0;
	if (lua_gettop(L) == 2) {
		distance = (float)luaL_checknumber(L, 2);
		lua_remove(L, 2);
	}

	lua_getglobal(L, "game");
	lua_getfield(L, -1, "GetService");
	lua_pushvalue(L, -2);
	lua_pushstring(L, "Players");

	lua_pcall(L, 2, 1, 0);
	lua_getfield(L, -1, "LocalPlayer");
	int local_player = *reinterpret_cast<int*>(lua_touserdata(L, -1));

	r_fireclickdetector(local_player, distance, click_detector);
	//std::cout << "COOL RESULT - " << 00 << std::endl;
	return 0;
}

auto fireproximityprompt(lua_State* L) -> int {
	ARG_CHECK(L, 1, 1, luaL_checktype(L, 1, LUA_TUSERDATA););

	int prox = dereference_pointer(lua_touserdata(L, 1));
	cherry::offsets::fireproximityprompt(prox);
	return 0;
}

auto decompile(lua_State* L) -> int {
	ARG_CHECK(L, 1, 1, luaL_checktype(L, 1, LUA_TUSERDATA););
	lua_pushstring(L, "-- decompiler coming soon! ~cherry~");
	return 1;
}

auto gettenv(lua_State* L) -> int {
	ARG_CHECK(L, 1, 1, luaL_checktype(L, 1, LUA_TTHREAD););
	lua_State* ls = reinterpret_cast<lua_State*>( dereference_pointer(lua_topointer(L, 1)) );

	sethvalue(L, L->top, ls->gt);
	L->top++;

	return 1;
}

struct eelive_thread_ref
{
	int unk_0;
	lua_State* state;
	int thread_id;
};

struct eethread_ref
{
	eelive_thread_ref* ref;
	eethread_ref(lua_State* L)
	{
		ref = new eelive_thread_ref;
		ref->state = L;
		lua_pushthread(L);
		ref->thread_id = lua_ref(L, -1);
		lua_pop(L, 1);
	}
};

//const auto scriptcontext_resume = reinterpret_cast<int(__thiscall*)(int context, eethread_ref thref, int returns)>(cherry::offsets::scriptcontext_resume);
//
//auto run_on_corescript(lua_State* L) -> int {
//	ARG_CHECK(L, 1, 1, luaL_checktype(L, 1, LUA_TSTRING););
//	std::string script = lua_tostring(L, 1);
//
//	lua_State* CLS = reinterpret_cast<lua_State*>(cherry::offsets::taskscheduler::get_corescript_lua_state(L->extra_space->script_context));
//	lua_State* LS = lua_newthread(CLS);
//
//	//LS->gt = L->gt;
//
//	lua_ref(CLS, -1);
//	lua_pop(CLS, 1);
//
//	struct : Luau::BytecodeEncoder {
//		auto encodeOp(const std::uint8_t op) -> uint8_t override {
//			return op * 227;
//		}
//	} bytecode_encoder;
//
//	std::string bytecode = Luau::compile(script, {  }, {  }, &bytecode_encoder);
//
//	if (bytecode.at(0) == 0)
//	{
//		bytecode = (bytecode.c_str() + 1);
//		lua_pushnil(L);
//		lua_pushlstring(L, bytecode.c_str(), bytecode.size());
//		return 2;
//	}
//
//	int lua_result = luau_load(LS, "actor", bytecode.c_str(), bytecode.size(), 0);
//
//	if (lua_result == 1) {
//		//std::cout << "error 2\n";
//		const char* err = lua_tostring(L, -1);
//		lua_pop(L, 1);
//
//		lua_getglobal(L, "warn");
//		lua_pushstring(L, err);
//		lua_pcall(L, 1, 0, 0);
//
//		lua_settop(L, 0);
//		return 0;
//	}
//
//	try {
//		scriptcontext_resume(task_scheduler->context(), eethread_ref{ LS }, 0);
//	}
//	catch (std::exception ex) {
//		std::cout << ex.what() << std::endl;
//	}
//
//	lua_settop(LS, 0);
//	return 0; /* to lazy to add returns */
//};

/* registering */
static const luaL_Reg env_funcs[] = {
	{"loadstring", loadstring},
	//{"run_on_corescript", run_on_corescript},

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
	{"gethiddenproperty", gethiddenproperty},
	{"sethiddenproperty", sethiddenproperty},
	{"isscriptable", isscriptable},
	{"setscriptable", setscriptable},
	{"getproperties", getproperties},
	{"gethiddenproperties", gethiddenproperties},

	{"getscriptbytecode", getscriptbytecode},
	{"dumpstring", getscriptbytecode},
	{"getscripthash", getscripthash},

	{"getobjects", cherry::environment::getobjects},
	{"firesignal", firesignal},
	{"getcustomasset", getcustomasset},
	//{"fireclickdetector", fireclickdetector},
	{"fireproximityprompt", fireproximityprompt},
	{"decompile", decompile},
	{"gettenv", gettenv},
	{"getstateenv", gettenv},

	{NULL, NULL}
};

auto cherry::environment::luauopen_environment(lua_State* L) -> void {
	lua_pushvalue(L, LUA_GLOBALSINDEX);
	luaL_register(L, NULL, env_funcs);
	lua_pop(L, 1);
}