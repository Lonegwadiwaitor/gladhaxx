#include "../../src/pch.hpp"

#include "text.hpp"

static ImFont* font;

ImFont* draw::get_font() {
	return font;
}

void draw::set_font(ImFont* new_font) {
	font = new_font;
}

draw::text::text() :
	_text({}),
	_size(16),
	_outline(false),
	_centered(false),
	_position({}),
	_outline_opacity(1),
	_outline_color({ 0, 0, 0, 0 })
{
	set_opacity(1);

	set_color({ 1, 1, 1, 1 });
	set_visible(true);

	set_name(_STR("Text"));
	set_class_name(_STR("Text"));

	elements.push_back(this);
}

void draw::text::render() {
	if (is_visible()) {
		auto &[x, y] = _position;
		const char* text = _text.c_str();
		const auto &[r, g, b, a] = get_color();
		ImU32 color_u32 = ImGui::GetColorU32({ r, g, b, get_opacity() });

		font = ImGui::GetFont();

		if (_centered) {
			x = (x - font->CalcTextSizeA(_size, FLT_MAX, 0, text).x / 2);
		}

		if (_outline) {
			const auto& [out_r, out_g, out_b, out_a] = _outline_color;
			ImU32 outline_color_u32 = ImGui::GetColorU32({ out_r, out_g, out_b, _outline_opacity });

			background_draw_list->AddText(font, _size, { x + 1, y + 1 }, outline_color_u32, text);
			background_draw_list->AddText(font, _size, { x - 1, y - 1 }, outline_color_u32, text);
			background_draw_list->AddText(font, _size, { x + 1, y - 1 }, outline_color_u32, text);
			background_draw_list->AddText(font, _size, { x - 1, y + 1 }, outline_color_u32, text);
		}

		background_draw_list->AddText(font, _size, { x, y }, color_u32, _text.c_str());
	}
}

bool draw::text::is_centered() const {
	return _centered;
}

bool draw::text::is_outlined() const {
	return _outline;
}

float draw::text::get_size() const {
	return _size;
}

float draw::text::get_outline_opacity() const {
	return _outline_opacity;
}

ImVec2 draw::text::get_position() const {
	return _position;
}

ImVec4 draw::text::get_outline_color() const {
	return _outline_color;
}

std::string draw::text::get_text() const {
	return _text;
}

void draw::text::set_centered(bool centered) {
	_centered = centered;
}

void draw::text::set_outline(bool outline) {
	_outline = outline;
}

void draw::text::set_text(const char* text) {
	_text = text;
}

void draw::text::set_size(float size) {
	_size = size;
}

void draw::text::set_position(ImVec2 position) {
	_position = position;
}

void draw::text::set_outline_color(const ImVec4& outline_color) {
	_outline_color = outline_color;
}

void draw::text::set_outline_transparency(float transparency) {
	_outline_opacity = transparency;
}
