#include "../../src/pch.hpp"

#include "slider.hpp"

#include "../../libraries/imgui/imgui.h"

ig::slider_float::slider_float() :
	_text()
{
	set_visible(true);
	set_same_line(false);

	set_name(random());
	set_class_name(_STR("SliderFloat"));
}

ig::slider_int::slider_int() :
	_text()
{
	set_visible(true);
	set_same_line(false);

	set_name(random());
	set_class_name(_STR("SliderInt"));
}

void ig::slider_float::render() {
	if (is_same_line()) {
		ImGui::SameLine();
	}

	if (is_visible()) {
		if (ImGui::SliderFloat(_text.c_str(), &_value, _min, _max) && _callback) {
			_callback();
		}

		if (get_tool_tip() != nullptr) {
			get_tool_tip()->render(_text.c_str());
		}
	}
}

void ig::slider_int::render() {
	if (is_same_line()) {
		ImGui::SameLine();
	}

	if (is_visible()) {
		if (ImGui::SliderInt(_text.c_str(), &_value, _min, _max) && _callback) {
			_callback();
		}

		if (get_tool_tip() != nullptr) {
			get_tool_tip()->render(_text.c_str());
		}
	}
}

float ig::slider_float::get() const {
	return _value;
}

int ig::slider_int::get() const {
	return _value;
}

float ig::slider_float::get_min() const {
	return _min;
}

int ig::slider_int::get_min() const {
	return _min;
}

float ig::slider_float::get_max() const {
	return _max;
}

int ig::slider_int::get_max() const {
	return _min;
}

std::string ig::slider_float::get_text() const {
	return _text;
}

std::string ig::slider_int::get_text() const {
	return _text;
}

ig::callback_t ig::slider_float::get_callback() const {
	return _callback;
}

ig::callback_t ig::slider_int::get_callback() const {
	return _callback;
}

void ig::slider_float::set(float value) {
	_value = value;
}

void ig::slider_int::set(int value) {
	_value = value;
}

void ig::slider_float::set_min(float min) {
	_min = min;
}

void ig::slider_int::set_min(int min) {
	_min = min;
}

void ig::slider_float::set_max(float max) {
	_max = max;
}

void ig::slider_int::set_max(int max) {
	_max = max;
}

void ig::slider_float::set_text(const std::string& text) {
	_text = text;
}

void ig::slider_int::set_text(const std::string& text) {
	_text = text;
}

void ig::slider_float::set_callback(const callback_t& callback) {
	_callback = callback;
}

void ig::slider_int::set_callback(const callback_t& callback) {
	_callback = callback;
}
