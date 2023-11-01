#pragma once

#include "element.hpp"

namespace draw {
	class square : public element {
	private:
		bool _filled;
		ImVec2 _size;
		ImVec2 _position;
		float _thickness;
	public:
		void render() override;

		square();

		bool is_filled() const;
		ImVec2 get_size() const;
		ImVec2 get_position() const;
		float get_thickness() const;

		void set_size(const ImVec2& size);
		void set_position(const ImVec2& position);
		void set_filled(bool filled);
		void set_thickness(float thickness);
	};
}
