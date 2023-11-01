#include "../../src/pch.hpp"

#include "radio_box.hpp"
#include "util.hpp"

#include "../../libraries/imgui/imgui.h"

ig::radio_button::radio_button(const ig::radio_box& parent, const std::string& text, int* container, int id) :
	_text(text),
	_container(container),
	_id(id),
	_callback(parent.get_callback())
{
	set_visible(true);
	set_same_line(false);

	set_name(random());
	set_class_name(_STR("RadioButton"));
}

ig::radio_box::radio_box() :
	_text(random()),
	_id(0xDEADBEEF)
{
	set_visible(true);
	set_same_line(false);

	set_name(_STR("RadioBox"));
	set_class_name(_STR("RadioBox"));
}

void ig::radio_button::render() {
	if (ImGui::RadioButton(_text.c_str(), _container, _id) && _callback) {
		_callback();
	}
}

void ig::radio_box::render() {
	if (is_visible()) {
		ImGui::Text(_text.c_str());

		if (get_tool_tip() != nullptr) {
			get_tool_tip()->render();
		}

		for (auto& button : _buttons) {
			button.render();
		}
	}
}

ig::radio_button* ig::radio_box::add(const std::string& text) {
	return &_buttons.emplace_back(*this, text, &_id, _buttons.size());
}

int ig::radio_button::get_id() const {
	return _id;
}

std::string ig::radio_button::get_text() const {
	return _text;
}

ig::callback_t ig::radio_button::get_callback() const {
	return _callback;
}

void ig::radio_button::set_callback(const callback_t& callback) {
	_callback = callback;
}

std::string ig::radio_box::get() const {
	for (const auto &button : _buttons) {
		if (button.get_id() == _id) {
			return button.get_text();
		}
	}

	return {};
}

bool ig::radio_box::has(const std::string& text) {
	for (const auto& button : _buttons) {
		if (button.get_text() == text) {
			return true;
		}
	}

	return false;
}

std::string ig::radio_box::get_text() const {
	return _text;
}

ig::callback_t ig::radio_box::get_callback() const {
	return _callback;
}

std::list<ig::radio_button> ig::radio_box::get_buttons() const {
	return _buttons;
}

void ig::radio_box::set(const std::string& text) {
	for (const auto &button : _buttons) {
		if (button.get_text() == text) {
			_id = button.get_id();
			return;
		}
	}
}

void ig::radio_button::set_text(const std::string& text) {
	_text = text;
}

void ig::radio_box::set_text(const std::string &text) {
	_text = text;
}

void ig::radio_box::set_callback(const callback_t& callback) {
	_callback = callback;

	for (auto &button : _buttons) {
		if (!button.get_callback()) {
			button.set_callback(callback);
		}
	}
}
