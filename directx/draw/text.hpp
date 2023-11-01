#pragma once

#include "element.hpp"

namespace draw {
	class text : public element {
	private:
		float _size;
		std::string _text;
		bool _centered;
		bool _outline;
		float _outline_opacity;
		ImVec4 _outline_color;
		ImVec2 _position;
	public:
		void render() override;

		text();

		bool is_centered() const;
		bool is_outlined() const;
		float get_size() const;
		float get_outline_opacity() const;
		ImVec2 get_position() const;
		std::string get_text() const;
		ImVec4 get_outline_color() const;
		void set_centered(bool centered);
		void set_outline(bool outline);
		void set_text(const char* text);
		void set_size(float size);
		void set_position(ImVec2 position);
		void set_outline_color(const ImVec4& outline_color);
		void set_outline_transparency(float transparency);
	};

	ImFont* get_font();
	void set_font(ImFont* new_font);
}
