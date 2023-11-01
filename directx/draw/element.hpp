#pragma once

#include "../../src/pch.hpp"

#include "../../libraries/imgui/imgui.h"

namespace draw {
	class element {
	private:
		ImVec4 _color;
		bool _visible;
		float _opacity;
		std::string _name;
		std::string _class_name;
	protected:
		void set_class_name(const std::string& class_name);
	public:
		virtual void render() = 0;
		void destroy(const std::function<void(draw::element*)> &callback = nullptr);

		bool is_visible() const;
		float get_opacity() const;
		ImVec4 get_color() const;
		std::string get_name() const;
		std::string get_class_name() const;

		void set_visible(bool visible);
		void set_opacity(float opacity);
		void set_color(const ImVec4 &color);
		void set_name(const std::string& name);

		bool operator==(const element& other);
		//bool operator==(const element& left, const element& right);
	};

	extern ImDrawList* background_draw_list;
	extern std::list<draw::element*> elements;
}
