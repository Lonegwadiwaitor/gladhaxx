#include "../../src/pch.hpp"

#include "element.hpp"

ImDrawList* draw::background_draw_list;
std::list<draw::element*> draw::elements;

void draw::element::destroy(const std::function<void(element*)> &callback) {
	// finally remove from the container
	elements.erase(std::remove(elements.begin(), elements.end(), this));
	
	// call any cleanup function
	if (callback) {
		callback(this);
	}
}

bool draw::element::operator==(const element& other) {
	return this == &other;
}

bool draw::element::is_visible() const {
	return _visible;
}

float draw::element::get_opacity() const {
	return _opacity;
}

ImVec4 draw::element::get_color() const {
	return _color;
}

std::string draw::element::get_name() const {
	return _name;
}

std::string draw::element::get_class_name() const {
	return _class_name;
}

void draw::element::set_visible(bool visible) {
	_visible = visible;
}

void draw::element::set_opacity(float opacity) {
	_opacity = opacity;
}

void draw::element::set_color(const ImVec4& color) {
	_color = color;
}

void draw::element::set_name(const std::string& name) {
	_name = name;
}

void draw::element::set_class_name(const std::string& class_name) {
	_class_name = class_name;
}
