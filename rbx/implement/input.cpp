#include "../../src/pch.hpp"

#include "input.hpp"

#include "../luau/VM/src/lstate.h"

#include "../../directx/hook.hpp"

int rbx::implement::mousemoverel(lua_State* R) {
	if (GetForegroundWindow() == directx::get_hwnd()) {
		mouse_event(MOUSEEVENTF_MOVE, lua_tointeger(R, 1), lua_tointeger(R, 2), 0, 0);
	}

	return 0;
}

int rbx::implement::mouse1click(lua_State* R) {
	if (GetForegroundWindow() == directx::get_hwnd()) {
		mouse_event(MOUSEEVENTF_LEFTDOWN | MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
	}
	return 0;
}

int rbx::implement::mouse1down(lua_State* R) {
	if (GetForegroundWindow() == directx::get_hwnd()) {
		mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
	}

	return 0;
}

int rbx::implement::mouse1up(lua_State* R) {
	if (GetForegroundWindow() == directx::get_hwnd()) {
		mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
	}

	return 0;
}

int rbx::implement::mouse2click(lua_State* R) {
	if (GetForegroundWindow() == directx::get_hwnd()) {
		mouse_event(MOUSEEVENTF_RIGHTDOWN | MOUSEEVENTF_RIGHTUP, 0, 0, 0, 0);
	}

	return 0;
}

int rbx::implement::mouse2down(lua_State* R) {
	if (GetForegroundWindow() == directx::get_hwnd()) {
		mouse_event(MOUSEEVENTF_RIGHTDOWN, 0, 0, 0, 0);
	}

	return 0;
}

int rbx::implement::mouse2up(lua_State* R) {
	if (GetForegroundWindow() == directx::get_hwnd()) {
		mouse_event(MOUSEEVENTF_RIGHTUP, 0, 0, 0, 0);
	}

	return 0;
}