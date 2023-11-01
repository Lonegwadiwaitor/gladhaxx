#pragma once 

struct lua_State;

union GCObject;

namespace rbx {
	enum gc_status {
		GC_STEP,
		GC_BREAK
	};

	void step_gc(lua_State* R, const std::function<gc_status(GCObject*)>& callback);

	namespace implement {
		int getrenv(lua_State* R);
		int getgenv(lua_State* R);
		int getgc(lua_State* R);
		int getsenv(lua_State* R);
		int getobjects(lua_State* R);
		int getthreadidentity(lua_State* R);
		int setthreadidentity(lua_State* R);
		int getcallingscript(lua_State* R);
		int is_luau(lua_State* R);
		int get_reg(lua_State* R);
		int get_hidden_ui(lua_State* R);
		int set_fps_cap(lua_State* R);
	}
}
