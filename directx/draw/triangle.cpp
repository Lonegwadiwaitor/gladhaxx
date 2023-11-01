#include "../../src/pch.hpp"

#include "triangle.hpp"

draw::triangle::triangle() :
	_filled(false),
	_thickness(1),
	_a({}),
	_b({}),
	_c({})
{
}

void draw::triangle::render() {
	if (is_visible()) {
		const auto& [r, g, b, a] = get_color();
		ImU32 color_u32 = ImGui::GetColorU32({ r, g, b, get_opacity() });

		if (is_filled()) {
			background_draw_list->AddTriangleFilled(_a, _b, _c, color_u32);
		}
		else {
			background_draw_list->AddTriangle(_a, _b, _c, color_u32, _thickness);
		}
	}
}

bool draw::triangle::is_filled() const {
	return _filled;
}

float draw::triangle::get_thickness() const {
	return _thickness;
}

ImVec2 draw::triangle::get_a() const {
	return _a;
}

ImVec2 draw::triangle::get_b() const {
	return _b;
}

ImVec2 draw::triangle::get_c() const {
	return _c;
}

void draw::triangle::set_filled(bool filled) {
	_filled = filled;
}

void draw::triangle::set_thickness(float thickness) {
	_thickness = thickness;
}

void draw::triangle::set_a(const ImVec2& a) {
	_a = a;
}

void draw::triangle::set_b(const ImVec2& b) {
	_b = b;
}

void draw::triangle::set_c(const ImVec2& c) {
	_c = c;
}
