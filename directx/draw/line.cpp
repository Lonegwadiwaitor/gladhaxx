#include "../../src/pch.hpp"

#include "line.hpp"

draw::line::line() :
	_to({}),
	_from({}),
	_thickness(1)
{
	set_opacity(1);

	set_color({ 1, 1, 1, 0 });
	set_visible(true);

	set_name(_STR("Line"));
	set_class_name(_STR("Line"));

	elements.push_back(this);
}

void draw::line::render() {
	if (is_visible()) {
		const auto& [r, g, b, a] = get_color();
		ImU32 color_u32 = ImGui::GetColorU32({ r, g, b, get_opacity() });

		background_draw_list->AddLine(_from, _to, color_u32, _thickness);
	}
}

ImVec2 draw::line::get_to() const {
	return _to;
}

ImVec2 draw::line::get_from() const {
	return _from;
}

float draw::line::get_thickness() const {
	return _thickness;
}

void draw::line::set_to(const ImVec2& to) {
	_to = to;
}

void draw::line::set_from(const ImVec2& from) {
	_from = from;
}

void draw::line::set_thickness(float thickness) {
	_thickness = thickness;
}
