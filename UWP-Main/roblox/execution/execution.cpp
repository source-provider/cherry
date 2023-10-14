#include <miscellaneous/utilities/utilities.h>
#include "dependencies/luau/vm/src/lstate.h"
#include <roblox/classes/console/console.h>
#include <miscellaneous/configuration.hpp>
#include <Luau/BytecodeBuilder.h>
#include <Luau/Compiler.h>
#include "execution.h"
#include <lualib.h>
#include <thread>
#include <mutex>

std::mutex mutex; 
cherry::scheduler::jobType schedulerRet{ NULL };
auto utils{ cherry::utilities::getSingleton() };
cherry::execution* executionSingleton{ nullptr };

#pragma region threadRef
struct liveThreadRef
{
	int unk_0;
	lua_State* state;
	int threadId;
};

struct threadRef
{
	liveThreadRef* ref;
	threadRef(lua_State* L) {
		ref = new liveThreadRef;
		ref->state = L;
		lua_pushthread(L);
		ref->threadId = lua_ref(L, -1);
		lua_pop(L, 1);
	}
};
#pragma endregion
auto __fastcall schedulerHook(std::intptr_t self, std::intptr_t _, std::intptr_t a2) -> std::uintptr_t {
	std::unique_lock<std::mutex> guard{ mutex };

	if (!executionSingleton->empty()) {
		cherry::schedulerTask task{ executionSingleton->top()}; 
		guard.unlock();

		auto taskScheduler{ cherry::scheduler::getSingleton() };
		auto luaState{ taskScheduler->getLuaState() };
		auto console{ cherry::console::getSingleton() };

		if (task.isc) {
			try {
				task.func(luaState);
			}
			catch (std::exception ex) {
				console->writeMode(ex.what(), cherry::console::error);
			}
		}
		else {
			struct : Luau::BytecodeEncoder {
				auto encodeOp(const std::uint8_t op) -> uint8_t override {
					return op * 227;
				}
			} bytecodeEncoder;

			luaState = lua_newthread(luaState);

			/*static std::intptr_t deferRef{ NULL };
			if (deferRef == NULL) {
				lua_getglobal(luaState, "task");
				lua_getfield(luaState, -1, "defer");

				lua_clonecfunction(luaState, -1);
				deferRef = lua_ref(luaState, -1);

				lua_settop(luaState, 0);
			}*/

			const std::string& luaScript { ("script=Instance.new(\"LocalScript\");\t" + task.script) };
			const std::string& bytecode { Luau::compile(luaScript, { 2, 1, 0 }, {  }, &bytecodeEncoder) };

			_TTEB* teb = (_TTEB*)NtCurrentTeb();
			*reinterpret_cast<std::intptr_t*>(*reinterpret_cast<std::intptr_t*>(teb->ThreadLocalStoragePointer) + 200) = luaState->extra_space->context_level;

			if (bytecode[0] == 0) {
				std::string err{ (bytecode.c_str() + 1) };
				console->writeMode(err, cherry::console::error);
			} else {
				//lua_getref(luaState, deferRef);
				if (luau_load(luaState, utils->randomString(16).c_str(), bytecode.c_str(), bytecode.size(), 0) == LUA_OK) {					
					if (lua_pcall(luaState, 0, 0, 0) != LUA_OK) {
						console->writeMode(lua_tostring(luaState, -1), cherry::console::error);
						lua_pop(luaState, 1);
					}
				}
				else {
					console->writeMode(lua_tostring(luaState, -1), cherry::console::error);
					lua_pop(luaState, 1);
				}
			}

			lua_settop(luaState, 0);
		}
	}

	return schedulerRet(self, a2);
};

namespace cherry {
	execution* execution::singleton{ nullptr };
	auto execution::getSingleton() -> execution* {
		if (singleton == nullptr)
			singleton = new execution();

		return singleton;
	};

	auto execution::load() -> void {
		executionSingleton = this;
		auto taskScheduler{ cherry::scheduler::getSingleton() };
		schedulerRet = reinterpret_cast<scheduler::jobType>(taskScheduler->jobHook(schedulerHook));
	}

	auto execution::createPipe() -> void {
		if (this->pipeInitialize)
			return;

		this->pipeInitialize = true;
		std::thread(
			[this]() -> void {
				DWORD read{};
				char bufferSize[999999];

				std::string pipeName{ "\\\\.\\pipe\\local\\" + configuration::pipeName + (configuration::usePipePid ? utils->tostring(utils->getProcId()) : "") };
				std::string luaCode{ "" };

				HANDLE pipe{ CreateNamedPipeA(pipeName.c_str(), PIPE_ACCESS_DUPLEX | PIPE_TYPE_BYTE | PIPE_READMODE_BYTE, PIPE_WAIT, 1, 9999999, 9999999, NMPWAIT_USE_DEFAULT_WAIT, nullptr) };
				while (pipe != INVALID_HANDLE_VALUE) {
					if (ConnectNamedPipe(pipe, nullptr) != FALSE) {
						while (ReadFile(pipe, bufferSize, sizeof(bufferSize) - 1, &read, nullptr) != FALSE) {
							bufferSize[read] = '\0';
							luaCode += bufferSize;
						}

						this->send(luaCode);
						luaCode.clear();
					}

					DisconnectNamedPipe(pipe);
				}
			}
		).detach();
	}

	auto execution::send(std::string script, std::string chunk, lua_State* L) -> std::intptr_t {
		std::unique_lock<std::mutex> guard{ mutex };
		if (L == nullptr) { // push to scheduler
			this->schedulerQueue.push(schedulerTask{ script });
		}
		else { //push a lclosure to the stack
			struct : Luau::BytecodeEncoder {
				auto encodeOp(const std::uint8_t op) -> uint8_t override {
					return op * 227;
				}
			} bytecodeEncoder;

			const std::string& bytecode = Luau::compile(script, { 2, 1, 0 }, {  }, &bytecodeEncoder);
			if (bytecode[0] == 0) {
				std::string err = (bytecode.c_str() + 1);

				lua_pushnil(L);
				lua_pushlstring(L, err.c_str(), err.size());
				return 2;
			}
			else {
				if (chunk == "")
					chunk = utils->randomString(16).c_str();

				if (luau_load(L, chunk.c_str(), bytecode.c_str(), bytecode.size(), 0) == LUA_OK) {
					return 1;
				}
				else {
					std::string err = lua_tostring(L, -1);

					lua_pushnil(L);
					lua_pushlstring(L, err.c_str(), err.size());
					return 2;
				}
			}
		}

		return 0;
	}

	auto execution::send(std::function<void(lua_State* L)> func) -> std::intptr_t {
		std::unique_lock<std::mutex> guard{ mutex };
		this->schedulerQueue.push(schedulerTask{ func });
		return 0;
	}

	auto execution::empty() -> bool {
		return this->schedulerQueue.empty();
	};

	auto execution::top() -> schedulerTask {
		cherry::schedulerTask task{ this->schedulerQueue.front() };
		this->schedulerQueue.pop();
		return task;
	};

}

