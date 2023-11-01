#include "base64.hpp"

#include "lua.h"
#include "lualib.h"
#include "util.hpp"

#include "../../libraries/cryptography/base64.hpp"

int encode(lua_State* R) {
	size_t size;
	const char* str = luaL_checklstring(R, 1, &size);

	std::string truebase64;

	auto b64 = base64::enc(str);

	for (int i = 0; i < b64.size(); i++) {
		if (b64[i] == ' ')
			break;

		truebase64[i] = b64[i];
	}

	lua_pushlstring(R, truebase64.data(), truebase64.size());

	return 1;
}

int decode(lua_State* R) {
	size_t size;
	const char* str = luaL_checklstring(R, 1, &size);

	

	auto b64 = base64::dec(str);

	lua_pushlstring(R, b64.data(), b64.size());

	return 1;
}

void rbx::base64::init(lua_State* R) {
	STR_ENCRYPT_START

	lua_getglobal(R, "smh");
	lua_getfield(R, -1, "crypt");

	lua_createtable(R, 0, 2);

	push_method(R, "encode", encode, 0);
	push_method(R, "decode", decode, 0);

	lua_setfield(R, -2, "base64");

	lua_pop(R, 2);
	STR_ENCRYPT_END
}
