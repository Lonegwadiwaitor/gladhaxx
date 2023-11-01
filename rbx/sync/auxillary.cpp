#include "auxillary.hpp"

#include "lualib.h"

lua_State* rbx::base_state;
lua_State* rbx::roblox_state;

lua_State* rbx::newthread(lua_State* R, bool safe) {
    lua_State* thread = lua_newthread(R);

    if (safe) {
        std::printf("thread sandboxed\n");
        luaL_sandboxthread(thread);

        std::printf("patching _G\n");
        lua_createtable(thread, 0, 0);
        lua_setglobal(thread, "_G");

        std::printf("patching shared\n");
        lua_createtable(thread, 0, 0);
        lua_setglobal(thread, "shared");
    }

    return thread;
}
