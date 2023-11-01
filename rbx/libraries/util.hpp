#include "../sync/rapi.hpp"
#include "../../libraries/imgui/imgui.h"

namespace rbx {
	extern void** allocate(lua_State* R);
	extern bool compare(const char* str, const char* str1);
	extern void push_color3(lua_State* R, const ImVec4& color);
	extern ImVec4 to_color3(lua_State* R, int idx);
	extern void push_method(lua_State* R, const char* name, lua_CFunction f, int nups);
	extern void push_vector2(lua_State* R, const ImVec2& vec);
	extern ImVec2 to_vector2(lua_State* R, int idx);

	template<typename T>
	constexpr T* element_cast(lua_State* R, int idx) {
		return *static_cast<T**>(lua_touserdata(R, idx));
	}

	template<typename T>
	constexpr std::pair<T*, const char*> index_context(lua_State* R) {
		return { element_cast<T>(R, 1), lua_tostring(R, 2) };
	}

	template<typename T>
	T** create(lua_State* R, std::unordered_map<T*, int>& cache) {
		void** userdata = allocate(R);

		lua_pushvalue(R, -1);
		cache[element_cast<T>(R, -1)] = lua_ref(R, LUA_REGISTRYINDEX);

		return reinterpret_cast<T**>(userdata);
	}

	template<typename T>
	void free(lua_State* R, std::unordered_map<T*, int>& cache, T* element) {
		lua_pushnil(R);
		lua_rawseti(R, LUA_REGISTRYINDEX, cache[element]);
		cache.erase(element);
	}

	template<typename T>
	void set_type(lua_State* R, void* userdata) {
		lua_pushstring(R, reinterpret_cast<T*>(userdata)->get_class_name().c_str());
		lua_setfield(R, -2, "__type");

		push_method(R, "__tostring", [](lua_State* R) {
			lua_pushstring(R, element_cast<T>(R, 1)->get_name().c_str());
			return 1;
		}, 0);
	}

	template<typename func>
	auto gc_step(lua_State* R) {
		
	}

	namespace ig {
		bool is_same_line(lua_State* R);
	}

}
