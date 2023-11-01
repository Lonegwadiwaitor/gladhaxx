#include "../../src/pch.hpp"

#include "../../libraries/imgui/imgui.h"

#include "combo_box.hpp"
#include "util.hpp"

ig::combo_box::combo_box() :
	_text(random()),
	_selected_index(0)
{
	set_visible(true);
	set_same_line(false);

	set_name(random());
	set_class_name(_STR("ComboBox"));
}

void ig::combo_box::render() {
	if (is_same_line()) {
		ImGui::SameLine();
	}

	if (is_visible()) {
		if (ImGui::BeginCombo(_text.c_str(), _selected.c_str())) {
			int index = 1;

			for (auto& member : _members) {
				bool is_selected = &_selected == &member;

				if (ImGui::Selectable(((std::to_string(index) + ". ") + member).c_str(), is_selected)) {
					_selected = member;
					_selected_index = index;

					if (_callback) {
						_callback();
					}
				}
				if (is_selected) {
					ImGui::SetItemDefaultFocus();
				}

				index++;
			}

			ImGui::EndCombo();
		}

		if (get_tool_tip() != nullptr) {
			get_tool_tip()->render();
		}
	}
}

int ig::combo_box::get_selected_index() const {
	return _selected_index;
}

std::string ig::combo_box::get_text() const {
	return _text;
}

std::string ig::combo_box::get_selected() const {
	return _selected;
}

std::vector<std::string> ig::combo_box::get_members() const {
	return _members;
}

ig::callback_t ig::combo_box::get_callback() const {
	return _callback;
}

void ig::combo_box::add(const std::string& text) {
	_members.push_back(text);
}

bool ig::combo_box::has(const std::string& text) {
	return std::find(_members.begin(), _members.end(), text) != _members.end();
}

void ig::combo_box::remove(const std::string& text) {
	_members.erase(std::remove(_members.begin(), _members.end(), text));
}

void ig::combo_box::set_text(const std::string& text) {
	_text = text;
}

void ig::combo_box::set_selected(const std::string& text) {
	if (std::find(_members.begin(), _members.end(), text) != _members.end()) {
		_selected = text;
	}
}

void ig::combo_box::set_callback(const ig::callback_t& callback) {
	_callback = callback;
}

void ig::combo_box::clear() {
	_members.clear();
	_selected = "";
}
