#include "memcheck.hpp"
#include "execution.hpp"
#include <lualib.h>
#include "luau/vm/src/lstate.h"
#include "luau/vm/src/lgc.h"
#include "instance.hpp"

#define CRYPTOPP_ENABLE_NAMESPACE_WEAK 1

#include <aes.h>
#include <rsa.h>
#include <gcm.h>
#include <eax.h>
#include <md2.h>
#include <md5.h>
#include <sha.h>
#include <sha3.h>
#include <osrng.h>
#include <hex.h>
#include <pssr.h>
#include <base64.h>
#include <modes.h>
#include <filters.h>
#include <serpent.h>
#include <pwdbased.h>

#include <blowfish.h>
#include <modes.h>

#define ARG_CHECK(L, MIN, MAX, T) if (lua_gettop(L) < MIN) { T return 0; } else if (lua_gettop(L) > MAX) { lua_settop(L, MAX); } T

namespace cherry {
	namespace global {
		static auto console = cherry::console::get_singleton();
		static auto task_scheduler = cherry::scheduler::get_singleton();
		static auto utilities = cherry::utilities::get_singleton();
		static auto execution = cherry::execution::get_singleton();
		static auto memcheck = cherry::memcheck::get_singleton();
	};

	namespace environment {
		CHERRYRVM_EXTERN Table* get_roblox_env(Table* env = nullptr);
		CHERRYRVM_EXTERN void queue_on_tp(std::string code);

		/* env */
		CHERRYRVM_EXTERN int httpget(lua_State* L);
		CHERRYRVM_EXTERN int httppost(lua_State* L);
		CHERRYRVM_EXTERN int getobjects(lua_State* L);

		/* registering */
		CHERRYRVM_EXTERN void luauopen_environment(lua_State* L);
		CHERRYRVM_EXTERN void luauopen_closure(lua_State* L);
		CHERRYRVM_EXTERN void luauopen_console(lua_State* L);
		CHERRYRVM_EXTERN void luauopen_filesystem(lua_State* L);
		CHERRYRVM_EXTERN void luauopen_input(lua_State* L);
		CHERRYRVM_EXTERN void luauopen_metatable(lua_State* L);
		CHERRYRVM_EXTERN void luauopen_http(lua_State* L);
		CHERRYRVM_EXTERN void luauopen_other(lua_State* L);
		CHERRYRVM_EXTERN void luauopen_initscript(lua_State* L);

		/* librarys */
		CHERRYRVM_EXTERN void luauopen_debug(lua_State* L); /* debug should be easy because I have access to structs */
		CHERRYRVM_EXTERN void luauopen_drawing(lua_State* L);
		CHERRYRVM_EXTERN void luauopen_cache(lua_State* L);
		CHERRYRVM_EXTERN void luauopen_crypt(lua_State* L);
		CHERRYRVM_EXTERN void luauopen_websocket(lua_State* L);
		CHERRYRVM_EXTERN void luauopen_cherry(lua_State* L);

		CHERRYRVM_EXTERN void luauopen_register(lua_State* L);
	}
}

#pragma region classes and functions
using yield_ret = std::function<int(lua_State* L)>;
template<typename T>
static __forceinline std::string hash_with_algo(const std::string& Input)
{
	T Hash;
	std::string Digest;

	CryptoPP::StringSource SS(Input, true,
		new CryptoPP::HashFilter(Hash,
			new CryptoPP::HexEncoder(
				new CryptoPP::StringSink(Digest), false
			)));

	return Digest;
}

class  roblox_yield_t
{
private:
	lua_State* L = (lua_State*)nullptr;
public:
	 roblox_yield_t(lua_State* state)
	{
		this->L = state;
	}

	int execute(const std::function<yield_ret()>& YieldedFunction) const;


};

static auto get_script_bytecode(lua_State* L, bool decrypt = true, bool use_offset = false) -> std::string { // _DWORD *__thiscall sub_8943E0(int this)
	int script = dereference_pointer(lua_touserdata(L, 1));
	instance_t* inst = (instance_t*)script;
	std::string classname = *inst->class_descriptor->class_name;
	int* vftable = *(int**)(script);
	std::string bytecode = "";

	if (classname == "LocalScript")
	{
		int offset = (use_offset == true ? lua_tonumber(L, 2) : cherry::offsets::script::localscript_bytecode);

		if (use_offset)
			std::cout << "[LOCAL SCRIPT]: " << offset << "\n";

		shared_string_t* shared = *(shared_string_t**)(script + offset);
		bytecode = shared->str;

		if (bytecode.size() <= 8)
		{
			//std::cout << "[LOCAL SCRIPT]: Invalid Bytecode\n";
			bytecode = "";
			decrypt = false;
		}
	}
	else if (classname == "ModuleScript") // 148 272 296
	{
		int offset = (use_offset == true ? lua_tonumber(L, 2) : cherry::offsets::script::modulescript_bytecode);
		if (use_offset)
			std::cout << "[MODULE SCRIPT]: " << offset << "\n";

		shared_string_t* shared = *(shared_string_t**)(script + offset);
		bytecode = shared->str;
		//std::cout << protected_str << "\n" << bytecode_int << "\n";

		//bytecode = *reinterpret_cast<std::string*>(bytecode_int);

		if (bytecode.size() <= 8)
		{
			//std::cout << "[MODULE SCRIPT]: Invalid Bytecode\n";
			//std::cout << bytecode << "\n";
			bytecode = "";
			decrypt = false;
		}
		/*BYTE fprot[24];
		int protected_str = ((int(__thiscall*)(int))(vftable[55]))(script);
		std::cout << protected_str << "\n";
		protected_string ps = *(protected_string*)(protected_str);
		bytecode = ps.bytecode->str;
		if (bytecode.size() <= 8)
			bytecode = "";*/
	}
	else if (classname == "Script") {
		bytecode = "";
		decrypt = false;
	}

	if (decrypt)
		bytecode = cherry::encryptions::decompress_bytecode(bytecode);

	return bytecode;
}
#pragma endregion