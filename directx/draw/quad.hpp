#pragma once

#include "element.hpp"

namespace draw {
	class quad : public element {
	private:
		bool _filled;
		float _thickness;
		ImVec2 _a, _b, _c, _d;
	public:
		void render() override;

		quad();

		ImVec2 get_a() const;
		ImVec2 get_b() const;
		ImVec2 get_c() const;
		ImVec2 get_d() const;
		bool is_filled() const;
		float get_thickness() const;

		void set_a(const ImVec2& a);
		void set_b(const ImVec2& b);
		void set_c(const ImVec2& c);
		void set_d(const ImVec2& d);
		void set_filled(bool filled);
		void set_thickness(float thickness);
	};
}
