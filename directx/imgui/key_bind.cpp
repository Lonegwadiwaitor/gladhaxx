#include "../../src/pch.hpp"

#include "key_bind.hpp"
#include "util.hpp"

#include "../../libraries/imgui/imgui.h"
#include "../../libraries/os/output.hpp"

ig::key_bind::key_bind() :
	_text(random())
{
	set_visible(true);
	set_same_line(false);

	set_name(random());

	sys::cout("setting class name\n");
	set_class_name(_STR("KeyBind"));
	sys::cout("class name set\n");
}

void ig::key_bind::render() {
	if (is_same_line()) {
		ImGui::SameLine();
	}

	if (is_visible()) {
		//ImGui::LabelTextBackground(_key_code.c_str());
		ImGui::LabelText("%s", _key_code.c_str());
		ImGui::SameLine();
		//ImGui::LabelTextClipped(_text.c_str());
		ImGui::LabelText("%s", _text.c_str());
		ImGui::SameLine();

		if (get_tool_tip() != nullptr) {
			get_tool_tip()->render(_text.c_str());
		}
	}
}

std::string ig::key_bind::get_text() const {
	return _text;
}

std::string ig::key_bind::get_key_code() const {
	return _key_code;
}

void ig::key_bind::set_text(const std::string& text) {
	_text = text;
}

void ig::key_bind::set_key_code(const std::string& key_code) {
	_key_code = key_code;
}
