#include "../../src/pch.hpp"

#include "circle.hpp"

draw::circle::circle() :
	_sides(0),
	_filled(false),
	_position({0, 0}),
	_radius(0),
	_thickness(0)
{
	set_opacity(1);

	set_color({1, 1, 1, 0});
	set_visible(true);

	set_name(_STR("Circle"));
	set_class_name(_STR("Circle"));

	elements.push_back(this);
}

void draw::circle::render() {
	if (is_visible()) {
		const auto& [r, g, b, a] = get_color();
		ImU32 color_u32 = ImGui::GetColorU32({ r, g, b, get_opacity() });

		if (is_filled()) {
			background_draw_list->AddCircleFilled(_position, _radius, color_u32, _sides);
		}
		else {
			background_draw_list->AddCircle(_position, _radius, color_u32, _sides, _thickness);
		}
	}
}

int draw::circle::get_sides() const {
	return _sides;
}

bool draw::circle::is_filled() const {
	return _filled;
}

ImVec2 draw::circle::get_position() const {
	return _position;
}

float draw::circle::get_radius() const {
	return _radius;
}

float draw::circle::get_thickness() const {
	return _thickness;
}

void draw::circle::set_sides(int sides) {
	_sides = sides;
}

void draw::circle::set_filled(bool filled){
	_filled = filled;
}

void draw::circle::set_radius(float radius) {
	_radius = radius;
}

void draw::circle::set_position(const ImVec2& vec) {
	_position = vec;
}

void draw::circle::set_thickness(float thickness) {
	_thickness = thickness;
}
