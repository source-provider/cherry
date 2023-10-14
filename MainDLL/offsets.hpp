#include "encryptions.hpp"

/*
	Folders we use:
		- AST
		- Common
		- Compiler
		- VM
	Files used:
		- lobject.h
		- lstate.h
		- ltm.cpp
		- ltm.h
		- lua.h
*/

/* MAIN UPDATE */
#define LUAVM_SHUFFLE2(s, a1, a2) a1 s a2
#define LUAVM_SHUFFLE3(s, a1, a2, a3) a1 s a3 s a2
#define LUAVM_SHUFFLE4(s, a1, a2, a3, a4) a1 s a2 s a4 s a3
#define LUAVM_SHUFFLE5(s, a1, a2, a3, a4, a5) a1 s a2 s a5 s a4 s a3
#define LUAVM_SHUFFLE6(s, a1, a2, a3, a4, a5, a6) a2 s a6 s a4 s a1 s a5 s a3
#define LUAVM_SHUFFLE7(s, a1, a2, a3, a4, a5, a6, a7) a7 s a4 s a2 s a5 s a6 s a1 s a3
#define LUAVM_SHUFFLE8(s, a1, a2, a3, a4, a5, a6, a7, a8) a1 s a4 s a5 s a3 s a8 s a2 s a7 s a6 //a8 s a7 s a1 s a3 s a4 s a6 s a5 s a2
#define LUAVM_SHUFFLE9(s, a1, a2, a3, a4, a5, a6, a7, a8, a9) a1 s a8 s a5 s a3 s a4 s a6 s a7 s a9 s a2 // a2 s a1 s a9 s a3 s a5 s a4 s a7 a6 s a8

/*
vmvalue1_t - ADD
vmvalue2_t - SUB2
vmvalue3_t - XOR
vmvalue4_t - SUB1
*/

/* MAIN UPDATE */
// lobject.h

/* from what it looks like len is always -1 or +1 from hash */
#define tstring_hash vm_value3
#define tstring_len vm_value4

/* all 4 share the same encs */
#define userdata_metatable vm_value4
#define table_metatable vm_value4
#define table_array vm_value4
#define table_node vm_value4 

/* proto */
#define member_enc vm_value2
#define debugname_enc vm_value3
#define debuginsn_enc vm_value4


/* cclosure (cclosure_f and lclosure_p share the same enc since same offset) */
#define cclosure_f vm_value4
#define lclosure_p vm_value4
#define cclosure_cont vm_value2
#define cclosure_debugname vm_value1


// lstate.h (they all share the same enc)
#define gs_ttname vm_value3
#define gs_tmname vm_value3
#define global_enc vm_value3
#define stacksize_enc vm_value3

/* offsets */
namespace cherry {
	namespace offsets {
		const uintptr_t lua_nilobject = cherry::encryptions::rebase(0x2AA5D78, BASE_4); /* index2addr or psuedo2addr (located in luau_load) */
		const uintptr_t dummynode = cherry::encryptions::rebase(0x2AA5D58, BASE_4); /* "\"table\"" */
		const uintptr_t loadmodule_flag = cherry::encryptions::rebase(0x0, BASE_4); /* "debug.loadmodule is not enabled." */
		const uintptr_t scriptcontext_resume = cherry::encryptions::rebase(0x85E8C0, BASE_4); /* "Maximum re-entrancy depth (%i) exceeded" */

		/* possible fireclickdetector */
		const auto push_instance = reinterpret_cast<int(__stdcall*)(int, void*)>(cherry::encryptions::rebase(0x0, BASE_4)); /* "Unable to create an Instance of type \"%s\"" -> second to last call || type 2 - "InvalidInstance" -> scroll all the way up and copy address */
		const auto fireclickdetector = reinterpret_cast<void(__thiscall*)(int click_detector, float distance, int player)>(cherry::encryptions::rebase(0x0, BASE_4)); /* (NOT DIRECTLY FIRECLICKDETECTOR) E8 ? ? ? ? 83 C4 08 84 C0 74 5F 8B 07 */
		const auto fireproximityprompt = reinterpret_cast<void(__thiscall*)(int)>(cherry::encryptions::rebase(0x0, BASE_4));

		namespace taskscheduler {
			const auto get_taskscheduler = reinterpret_cast<int(__cdecl*)()>(cherry::encryptions::rebase(0xB72120, BASE_4)); /* E8 ? ? ? ? 8D 0C 24 51 */

			static auto get_lua_state(int context) -> int {
				return ((context + 244) + *(int*)(context + 244));
			};

			static auto get_corescript_lua_state(int context) -> int {
				return ((context + 396) + *(int*)(context + 396));
			};

			static cherry::encryptions::offset_t<uintptr_t> start{ 308 };
			static cherry::encryptions::offset_t<uintptr_t> end{ 312 };
			static cherry::encryptions::offset_t<double> fps{ 280 };
			static cherry::encryptions::offset_t<uintptr_t> datamodel{ 40 };
			static cherry::encryptions::offset_t<uintptr_t> script_context{ 304 };
		};

		namespace instance {
			constexpr int scriptable = 32;
		}

		namespace script {
			constexpr int localscript_bytecode = 304; /* keep in mind that this has changed a lot */
			constexpr int modulescript_bytecode = 276;
		}

		namespace signal {
			constexpr int next = 16;
			constexpr int state = 20;

			constexpr int ref = 28;
			constexpr int ref1 = 60;
			constexpr int index = 12;
		}

		namespace execution { /* task.spawn is automatically updated inside execution */
			using roblox_function_t = int(__cdecl*)(int L);
			const auto luau_execute = reinterpret_cast<int(__thiscall*)(uintptr_t rl)>(cherry::encryptions::rebase(0x18800E0, BASE_4)); /* located any function calling lua_call */
			//const auto luaD_throw = reinterpret_cast<void(__fastcall*)(uintptr_t rl, int errcode)>(cherry::encryptions::rebase(0x1942050, BASE_4)); /* "GetEnums" -> last function called in if statement | REF - sub_XXXXXXX(a1, 4); */
			const auto luavm_load = reinterpret_cast<uintptr_t(__fastcall*)(uintptr_t rl, std::string * source, const char* chunk, int env)>(cherry::encryptions::rebase(0x91FF10, BASE_4)); /* "moduleRef" -> should be 3rd function */
		};

		namespace memcheck {
			const int hasher = cherry::encryptions::rebase(0x0, BASE_4); /* 55 8B EC 83 EC 48 53 56 57 8B D9 6A ? 89 5D E4 E9 */
			const int core_start = cherry::encryptions::rebase(0x0, BASE_4); /* 8B 03 03 C3 69 C0 2D FE 94 15 03 C7 C1 C0 13 */
			const int core_end = (offsets::memcheck::core_start + 107); // 0x75F2AB

			static auto deciper_region(size_t region) -> size_t {
				return ((362085932 * region - 854064575) ^ (-759031019 - 877351945 * region));
			}

			namespace sigs { /* majority are opcode searches so they really don't need to be updated */
				const std::string_view secondary_hashers_sig = "\x0F\xBE\x00\xFE";
				const std::string_view secondary_hashers_pattern = "xx?x";
				const std::string_view secondary_hashers_pattern1 = "x?x";

				/* 2 possible varients left to search */
				const std::string_view secondary_hashers_b1_sig = "\x0F\xBE\x00\xFF";
				const std::string_view secondary_hashers_b2_sig = "\x0F\xBE\x00\xEF";

				/* last 3 instructon we need to search */
				const std::string_view secondary_hashers_b3_sig = "\x8D\x00\x02";
				const std::string_view secondary_hashers_b4_sig = "\x8B\x00\x0C";
				const std::string_view secondary_hashers_b5_sig = "\x8B\x00\x08";

				/* regions */
				const std::string_view regions_sig = "\x8B\x34\xBD\x00\x00\x00\x00";
				const std::string_view regions_pattern = "xxx";

				const std::string_view regions_sig1 = "\x8B\x14\xBD\x00\x00\x00\x00";
			};
		};
	};
};