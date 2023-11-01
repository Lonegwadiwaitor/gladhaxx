#pragma once

#include "element.hpp"

namespace draw {
	class line : public element {
	private:
		ImVec2 _to;
		ImVec2 _from;
		float _thickness;
	public:
		void render() override;

		line();

		ImVec2 get_to() const;
		ImVec2 get_from() const;
		float get_thickness() const;

		void set_to(const ImVec2& to);
		void set_from(const ImVec2& from);
		void set_thickness(float thickness);
	};
}
