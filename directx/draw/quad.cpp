#include "../../src/pch.hpp"

#include "quad.hpp"

draw::quad::quad() :
	_a({}),
	_b({}),
	_c({}),
	_d({}),
	_filled(false),
	_thickness(1)
{
	set_opacity(1);

	set_color({ 1, 1, 1, 0 });
	set_visible(true);

	set_name(_STR("Quad"));
	set_class_name(_STR("Quad"));

	elements.push_back(this);
}

void draw::quad::render() {
	if (is_visible()) {
		const auto& [r, g, b, a] = get_color();
		ImU32 color_u32 = ImGui::GetColorU32({ r, g, b, get_opacity() });

		if (is_filled()) {
			background_draw_list->AddQuadFilled(_a, _b, _c, _d, color_u32);
		}
		else {
			background_draw_list->AddQuad(_a, _b, _c, _d, color_u32, _thickness);
		}
	}
}

bool draw::quad::is_filled() const {
	return _filled;
}

ImVec2 draw::quad::get_a() const {
	return _a;
}

ImVec2 draw::quad::get_b() const {
	return _b;
}

ImVec2 draw::quad::get_c() const {
	return _c;
}

ImVec2 draw::quad::get_d() const {
	return _d;
}

float draw::quad::get_thickness() const {
	return _thickness;
}

void draw::quad::set_a(const ImVec2& a) {
	_a = a;
}

void draw::quad::set_b(const ImVec2& b) {
	_b = b;
}

void draw::quad::set_c(const ImVec2& c) {
	_c = c;
}

void draw::quad::set_d(const ImVec2& d) {
	_d = d;
}

void draw::quad::set_filled(bool filled) {
	_filled = filled;
}

void draw::quad::set_thickness(float thickness) {
	_thickness = thickness;
}
