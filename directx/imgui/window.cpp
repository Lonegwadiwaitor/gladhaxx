#include "../../src/pch.hpp"

#include "window.hpp"
#include "child.hpp"

#include "../../libraries/imgui/imgui.h"

#include "util.hpp"

std::vector<ig::window> ig::windows;

ig::window::window() :
	_text(random()),
	_size({ 0, 0 }),
	_position({ 0, 0 })
{
	set_name(random());
	set_class_name(_STR("Window"));
}

ig::window* ig::window::create() {
	return &ig::windows.emplace_back();
}

void ig::window::render() {
	if (_size.x != 0) {
		ImGui::SetNextWindowSize(_size, ImGuiCond_Once);
		ImGui::SetNextWindowPos(_position, ImGuiCond_Once);
	}

	ImGui::Begin(_text.c_str());

	_size = ImGui::GetWindowSize();
	_position = ImGui::GetWindowPos();

	for (ig::child* child : get_children()) {
		child->render();
	}

	ImGui::End();
}

std::string ig::window::get_text() const {
	return _text;
}

ImVec2 ig::window::get_size() const {
	return _size;
}

ImVec2 ig::window::get_position() const {
	return _position;
}

void ig::window::set_size(const ImVec2& size) {
	_size = size;
}

void ig::window::set_position(const ImVec2& position) {
	_position = position;
}

void ig::window::set_text(const std::string& text) {
	_text = text;
}
