#include <Windows.h>
#include <thread>

#include "environment.hpp"

using namespace cherry::global; /* were not namespace poisoning okay its just like 5 things */

int sschedur_hooke(lua_State* L) {
    task_scheduler->step_scheduler();
    return 0;
}

cherry::exception_filter ex{  };
auto init(HMODULE h_module) -> void {
   // console->show(); /* opens console */
    task_scheduler->load(); /* loads scheduler (state, context, datamodel) and hooks it */

    lua_State* RL = lua_newthread((lua_State*)task_scheduler->state());
    lua_getfield(RL, LUA_GLOBALSINDEX, "game");
    lua_getfield(RL, -1, "GetService");
    lua_pushvalue(RL, -2);
    lua_pushstring(RL, "RunService");
    lua_pcall(RL, 2, 1, 0);

    lua_getfield(RL, -1, "RenderStepped");
    lua_getfield(RL, -1, "Connect");
    lua_pushvalue(RL, -2);
    lua_pushcclosure(RL, sschedur_hooke, NULL, 0);
    lua_pcall(RL, 2, 0, 0);
    lua_pop(RL, 2);

    execution->send(NULL, "warn'I LOVE TWINKIES'");

    //utilities->location(h_module); /* sets our dll location */

    //int state = task_scheduler->state(); /* gets the state */

    /*console->set_color(cherry::console::red);
    console->write(Logo + "\n");
    console->reset_color();*/
    
    /* printing for debugging */
    //console->debug_write(utilities->tostring_hex(state), "STATE");
   // console->debug_write(utilities->tostring_hex(task_scheduler->context()), "CONTEXT");
    //console->debug_write(utilities->tostring_hex(task_scheduler->datamodel()), "DATAMODEL");

    /*try {
        memcheck->bypass();
    }
    catch (std::runtime_error ex) {
        console->debug_error(ex.what(), "MEMCHECK");
    }*/

    /* registering (include memcheck bypass) */
    //cherry::environment::luauopen_register(RL);

    /* finally we create our pipe for script execution (probably should run after memcheck is done) */
    //execution->send((int)RL, "print'gay'", true); // loadstring(game:HttpGet(\"https://pastebin.com/raw/1qPaVS6E\"))()
    //lua_pcall(RL, 0, 0, 0);
    
    
    //execution->launch_pipe();

    /*Sleep(1500);
    console->hide();
    console->clear();*/
    
    return;
}

auto __stdcall DllMain(HMODULE h_module, DWORD call_reason, LPVOID lp_reserved) -> BOOL
{
    if (call_reason == DLL_PROCESS_ATTACH)
    {
        std::thread(init, h_module).detach();
    }

    return TRUE;
}