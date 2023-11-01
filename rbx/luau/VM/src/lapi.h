// This file is part of the Luau programming language and is licensed under MIT License; see LICENSE.txt for details
// This code is based on Lua 5.x implementation licensed under MIT License; see lua_LICENSE.txt for details
#pragma once

#include "lobject.h"

LUAI_FUNC const TValue* luaA_toobject(lua_State* L, int idx);
LUAI_FUNC void luaA_pushobject(lua_State* L, const TValue* o);
LUAI_FUNC TValue* index2addr(lua_State* L, int idx);
LUAI_FUNC void lua_clonefunction(lua_State* L, int idx);
LUAI_FUNC void lua_clonecfunction(lua_State* L, int idx);

LUAI_FUNC void __fastcall lua_pushinstance(lua_State* R, class Instance* instance);
LUAI_FUNC int lua_getidentity(lua_State* R);
LUAI_FUNC void lua_setidentity(lua_State* R, int identity);