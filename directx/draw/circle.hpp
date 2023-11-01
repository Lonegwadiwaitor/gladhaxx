#pragma once

#include "element.hpp"

namespace draw {
	class circle : public element {
	private:
		int _sides;
		bool _filled;
		ImVec2 _position;
		float _radius;
		float _thickness;
	public:
		void render() override;

		circle();

		int get_sides() const;
		bool is_filled() const;
		ImVec2 get_position() const;
		float get_radius() const;
		float get_thickness() const;

		void set_sides(int sides);
		void set_filled(bool filled);
		void set_radius(float radius);
		void set_position(const ImVec2& vec);
		void set_thickness(float thickness);
	};
}
