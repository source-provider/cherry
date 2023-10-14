#include <roblox/environment/environment.h>
#include "dependencies/luau/vm/src/lstate.h"
#include <lualib.h>

auto mainThread(HMODULE h_module) -> void {
    auto console{ cherry::console::getSingleton() };
    auto taskScheduler{ cherry::scheduler::getSingleton() };
    auto execution{ cherry::execution::getSingleton() };
    auto environment{ cherry::environment::getSingleton() };
    

    execution->load();
    console->writeMode("Execution Loaded!", cherry::console::info);

    execution->send(
        [environment, console](lua_State* L) -> void {
            L->extra_space->context_level = 8;
            environment->setRobloxTable(L->gt);
            luaL_sandboxthread(L);
            environment->createEnvironment(L);
            console->writeMode("Environment Registered!", cherry::console::info);
        }
    );

    execution->send(
        [execution, console](lua_State* L) -> void {
            execution->createPipe();
            console->writeMode("Pipe Launched!", cherry::console::info);
        }
    );
}

auto APIENTRY DllMain(HMODULE h_module, DWORD call_reason, LPVOID lp_reserved) -> int {
    if (call_reason == DLL_PROCESS_ATTACH) {
        std::thread(mainThread, h_module).detach();
    }

    return 1;
}

