#include "execution.hpp"
#include <lualib.h>
#include <Luau/Compiler.h>
#include <Luau/BytecodeBuilder.h>
#include <luau/vm/src/lvm.h>
#include <luau/vm/src/lgc.h>
#include <aclapi.h>

struct elive_thread_ref
{
	int unk_0;
	lua_State* state;
	int thread_id;
};

struct ethread_ref
{
	elive_thread_ref* ref;
	ethread_ref(lua_State* L)
	{
		ref = new elive_thread_ref;
		ref->state = L;
		lua_pushthread(L);
		ref->thread_id = lua_ref(L, -1);
		lua_pop(L, 1);
	}
};


const auto scriptcontext_resume = reinterpret_cast<int(__thiscall*)(int context, ethread_ref thref, int returns)>(cherry::offsets::scriptcontext_resume);
void cherry::scheduler::step_scheduler() /* not going to bother calling cherry::execution::send */
{
	static auto task_scheduler = cherry::scheduler::get_singleton();
	static auto execution = cherry::execution::get_singleton();
	static auto utilities = cherry::utilities::get_singleton();
	//std::unique_lock<std::mutex> guard{ task_scheduler->mutex };

	if (!task_scheduler->empty())
	{
		scheduler_task task = task_scheduler->top();
		//guard.unlock();

		if (!task.isc)
		{
			lua_State* state = (lua_State*)task_scheduler->state();
			lua_State* L = lua_newthread((lua_State*)task_scheduler->state());

			ethread_ref th_ref{ L };
			//lua_ref(state, -1);
			//lua_pop(state, 1);
			

			struct : Luau::BytecodeEncoder {
				auto encodeOp(const std::uint8_t op) -> uint8_t override {
					return op * 227;
				}
			} bytecode_encoder;

			std::string code = "task.defer(function()\tscript=Instance.new(\"LocalScript\")\t" + task.script + "\nend)";
			//std::string code = "script=Instance.new(\"LocalScript\")\t" + task.script;
			std::string bytecode = Luau::compile(code, { 2, 1, 0 }, {  }, &bytecode_encoder);

			if (bytecode.at(0) == 0) {
				//std::cout << "error 1\n";
				lua_getglobal(L, "warn");
				std::string err = (bytecode.c_str() + 1);
				lua_pushstring(L, err.c_str());
				lua_pcall(L, 1, 0, 0);

				lua_settop(L, 0);
				return;
			}

			int lua_result = luau_load(L, cherry::configuration::source_name.c_str(), bytecode.c_str(), bytecode.size(), 0);

			/* compressing
			bytecode = cherry::encryptions::compress_bytecode(bytecode);
			int lua_result = cherry::offsets::execution::luavm_load((int)L, &bytecode, cherry::configuration::source_name.c_str(), 0);
			*/

			if (lua_result == 1) {
				//std::cout << "error 2\n";
				const char* err = lua_tostring(L, -1);
				lua_pop(L, 1);

				lua_getglobal(L, "warn");
				lua_pushstring(L, err);
				lua_pcall(L, 1, 0, 0);

				lua_settop(L, 0);
				return;
			}

			//lua_pcall(L, 0, 0, 0);

			try {
				scriptcontext_resume(task_scheduler->context(), th_ref, 0);
			}
			catch (std::exception ex) {
				std::cout << ex.what() << std::endl;
			}

			lua_settop(L, 0);
		}
	}
}

auto cherry::execution::send(int state, std::string script, bool push_to_scheduler) -> int {
	if (push_to_scheduler)
	{
		static auto task_scheduler_t = cherry::scheduler::get_singleton();
		task_scheduler_t->push(script);
		return 1;
	}

	lua_State* L = (lua_State*)state;

	struct : Luau::BytecodeEncoder {
		auto encodeOp(const std::uint8_t op) -> uint8_t override {
			return op * 227;
		}
	} bytecode_encoder;

	std::string bytecode = Luau::compile(script, {  }, {  }, &bytecode_encoder);

	if (bytecode.at(0) == 0)
	{
		bytecode = (bytecode.c_str() + 1);
		lua_pushnil(L);
		lua_pushlstring(L, bytecode.c_str(), bytecode.size());
		return 2;
	}

	if (luau_load(L, cherry::configuration::source_name.c_str(), bytecode.c_str(), bytecode.size(), 0))
	{
		const char* err = lua_tostring(L, -1);
		lua_pop(L, 1);

		lua_pushnil(L);
		lua_pushstring(L, err);
		return 2;
	}

	return 1;
}

auto cherry::execution::launch_pipe() -> void {
	auto utils = cherry::utilities::get_singleton();
	
	EXPLICIT_ACCESS ea[2];
	SID_IDENTIFIER_AUTHORITY SIDAuthWorld = SECURITY_WORLD_SID_AUTHORITY;
	SID_IDENTIFIER_AUTHORITY SIDAuthNT = SECURITY_NT_AUTHORITY;
	SECURITY_ATTRIBUTES sa;
	PSID pEveryoneSID;
	PSID pAdminSID;
	PACL pACL;

	if (!AllocateAndInitializeSid(&SIDAuthWorld, 1, SECURITY_WORLD_RID, 0, 0, 0, 0, 0, 0, 0, &pEveryoneSID)) 
		return;

	SecureZeroMemory(&ea, 2 * sizeof(EXPLICIT_ACCESS));
	ea[0].grfAccessPermissions = FILE_ALL_ACCESS | GENERIC_WRITE | GENERIC_READ;
	ea[0].grfAccessMode = SET_ACCESS;
	ea[0].grfInheritance = NO_INHERITANCE;
	ea[0].Trustee.TrusteeForm = TRUSTEE_IS_SID;
	ea[0].Trustee.TrusteeType = TRUSTEE_IS_WELL_KNOWN_GROUP;
	ea[0].Trustee.ptstrName = (LPTSTR)pEveryoneSID;

	
	if (!AllocateAndInitializeSid(&SIDAuthNT, 2, SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0, &pAdminSID)) 
		return;

	ea[1].grfAccessPermissions = FILE_ALL_ACCESS | GENERIC_WRITE | GENERIC_READ;
	ea[1].grfAccessMode = SET_ACCESS;
	ea[1].grfInheritance = NO_INHERITANCE;
	ea[1].Trustee.TrusteeForm = TRUSTEE_IS_SID;
	ea[1].Trustee.TrusteeType = TRUSTEE_IS_GROUP;
	ea[1].Trustee.ptstrName = (LPTSTR)pAdminSID;

	
	DWORD dwRes = SetEntriesInAcl(2, ea, NULL, &pACL);
	if (ERROR_SUCCESS != dwRes) 
		return;

	auto secDesc = std::vector<unsigned char>(SECURITY_DESCRIPTOR_MIN_LENGTH);
	PSECURITY_DESCRIPTOR pSD = (PSECURITY_DESCRIPTOR)(&secDesc[0]);
	if (nullptr == pSD) 
		return;

	if (!InitializeSecurityDescriptor(pSD, SECURITY_DESCRIPTOR_REVISION))
		return;

	if (!SetSecurityDescriptorDacl(pSD, TRUE, pACL, FALSE)) 
		return;

	sa.nLength = sizeof(SECURITY_ATTRIBUTES);
	sa.lpSecurityDescriptor = pSD;
	sa.bInheritHandle = FALSE;
	
	std::string name = ("\\\\.\\pipe\\" + configuration::pipe_name + (configuration::pipe_pid == true ? utils->tostring(GetProcessId(GetCurrentProcess())) : "")); // utils->get_proc_id("RobloxPlayerBeta.exe")
	HANDLE pipe = CreateNamedPipeA(
		name.c_str(), 
		PIPE_ACCESS_DUPLEX,
		PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT, 
		1, 
		9999999, 
		9999999, 
		NMPWAIT_USE_DEFAULT_WAIT, 
		&sa
	);

	std::thread([utils, pipe, this]() {
		char buffer[999999];
		std::string script = "";
		DWORD read = 0;

		while (pipe != INVALID_HANDLE_VALUE) {
			if (ConnectNamedPipe(pipe, NULL) != FALSE) {
				while (ReadFile(pipe, buffer, sizeof(buffer) - 1, &read, NULL) != FALSE) {
					buffer[read] = '\0';
					script = script + buffer;
				}

				this->send(NULL, script, true);
				script = "";
			}

			DisconnectNamedPipe(pipe);
		}
	}).detach();
}