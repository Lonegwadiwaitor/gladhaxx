#include "../../src/pch.hpp"
#include "http.hpp"

#include "lua.h"
#include "../runtime/async_closure.hpp"

int rbx::implement::get(lua_State* R) {
	rbx::runtime::yielding yield(R);

	std::string url = lua_tostring(R, 1);

	return yield.execute([url]() {
		auto result = cpr::Get(cpr::Url{ url });

		return [result](lua_State* R) {
			lua_pushstring(R, result.text.c_str());
			return 1;
		};
	});
}
