#include "../../src/pch.hpp"

#include "element.hpp"

#include "../../libraries/imgui/imgui.h"

namespace draw {
	class triangle : public element {
	private:
		bool _filled;
		float _thickness;
		ImVec2 _a, _b, _c;
	public:
		void render() override;

		triangle();

		bool is_filled() const;
		float get_thickness() const;
		ImVec2 get_a() const;
		ImVec2 get_b() const;
		ImVec2 get_c() const;

		void set_filled(bool filled);
		void set_thickness(float thickness);
		void set_a(const ImVec2& a);
		void set_b(const ImVec2& b);
		void set_c(const ImVec2& c);
	};
}
