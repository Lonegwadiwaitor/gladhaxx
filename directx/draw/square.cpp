#include "../../src/pch.hpp"

#include "square.hpp"

draw::square::square() :
	_size({}),
	_position({}),
	_filled(false),
	_thickness(1)
{
	set_opacity(1);

	set_color({ 1, 1, 1, 0 });
	set_visible(true);

	set_name(_STR("Square"));
	set_class_name(_STR("Square"));

	elements.push_back(this);
}

void draw::square::render() {
	if (is_visible()) {
		const auto& [r, g, b, a] = get_color();
		ImU32 color_u32 = ImGui::GetColorU32({ r, g, b, get_opacity() });

		if (is_filled()) {
			background_draw_list->AddRectFilled(_position, { _position.x + _size.x, _position.y + _size.y }, color_u32);
		}
		else {
			background_draw_list->AddRect(_position, { _position.x + _size.x, _position.y + _size.y }, color_u32, 0, 15, _thickness);
		}
	}
}

bool draw::square::is_filled() const {
	return _filled;
}

ImVec2 draw::square::get_size() const {
	return _size;
}

ImVec2 draw::square::get_position() const {
	return _position;
}

float draw::square::get_thickness() const {
	return _thickness;
}

void draw::square::set_filled(bool filled) {
	_filled = filled;
}

void draw::square::set_size(const ImVec2& size) {
	_size = size;
}

void draw::square::set_position(const ImVec2& position) {
	_position = position;
}

void draw::square::set_thickness(float thickness) {
	_thickness = thickness;
}
