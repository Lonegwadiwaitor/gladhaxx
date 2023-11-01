#include "../../src/pch.hpp"


#include "../../directx/draw/line.hpp"
#include "../../directx/draw/square.hpp"
#include "../../directx/draw/circle.hpp"
#include "../../directx/draw/quad.hpp"
#include "../../directx/draw/text.hpp"
#include "../../directx/draw/triangle.hpp"

#include "draw.hpp"
#include "util.hpp"

#include "../sync/rapi.hpp"

std::unordered_map<draw::element*, int> element_cache{};
std::unordered_map<std::string, int> metatable_cache{};

inline void rbx::draw::clear_element_cache() {
	element_cache.clear();
}

inline void rbx::draw::clear_metatable_cache() {
	metatable_cache.clear();
}

static int element_index(lua_State* R, draw::element* element, const char* index) {
	if (rbx::compare(index, "Name")) {
		lua_pushstring(R, element->get_name());
	}
	else if (rbx::compare(index, "ClassName")) {
		lua_pushstring(R, element->get_class_name());
	}
	else if (rbx::compare(index, "Transparency")) {
		lua_pushnumber(R, element->get_opacity());
	}
	else if (rbx::compare(index, "Color")) {
		rbx::push_color3(R, element->get_color());
	}
	else if (rbx::compare(index, "Visible")) {
		lua_pushboolean(R, element->is_visible());
	}
	else if (rbx::compare(index, "__OBJECT_EXISTS") || rbx::compare(index, "Removed")) {
		lua_pushboolean(R, element_cache.contains(element));
	}
	else if (rbx::compare(index, "Remove")) {
		lua_pushcclosure(R, [](lua_State* R) {
			auto element = rbx::element_cast<draw::element>(R, 1);
			rbx::free<draw::element>(R, element_cache, element);
			element->destroy([](draw::element* element) { delete element; });

			return 0;
		}, "", 0);
	}
	else if (rbx::compare(index, "Destroy")) {
		lua_pushcclosure(R, [](lua_State* R) {
			auto element = rbx::element_cast<draw::element>(R, 1);
			rbx::free<draw::element>(R, element_cache, element);
			element->destroy([](draw::element* element) { delete element; });

			return 0;
			}, "", 0);
	}
	else {
		std::printf("\"%s\" is not a valid member of \"%s\"\n", index, element->get_class_name().data());
	}

	return 1;
}

static int element_newindex(lua_State* R, draw::element* element, const char* index) {
	if (rbx::compare(index, "Name")) {
		element->set_name(lua_tostring(R, 3));
	}
	else if (rbx::compare(index, "Transparency")) {
		element->set_opacity(lua_tonumber(R, 3));
	}
	else if(rbx::compare(index, "Color")) {
		element->set_color(rbx::to_color3(R, 3));
	}
	else if(rbx::compare(index, "Visible")) {
		element->set_visible(lua_toboolean(R, 3));
	}
	else {
		std::printf("\"%s\" is not a valid member of \"%s\"\n", index, element->get_class_name().data());
	}

	return 0;
}

static void line(lua_State* R) {
	lua_createtable(R, 0, 2);

	rbx::push_method(R, "__index", [](lua_State* R) {
		auto [line, index] = rbx::index_context<draw::line>(R);

		if (rbx::compare(index, "To")) {
			rbx::push_vector2(R, line->get_to());
		}
		else if (rbx::compare(index, "From")) {
			rbx::push_vector2(R, line->get_from());
		}
		else if (rbx::compare(index, "Thickness")) {
			lua_pushnumber(R, line->get_thickness());
		}
		else {
			return element_index(R, line, index);
		}

		return 1;
	}, 0);

	rbx::push_method(R, "__newindex", [](lua_State* R) {
		auto [line, index] = rbx::index_context<draw::line>(R);

		if (rbx::compare(index, "To")) {
			line->set_to(rbx::to_vector2(R, 3));
		}
		else if (rbx::compare(index, "From")) {
			line->set_from(rbx::to_vector2(R, 3));
		}
		else if (rbx::compare(index, "Thickness")) {
			line->set_thickness(lua_tonumber(R, 3));
		}
		else {
			return element_newindex(R, line, index);
		}

		return 0;
	}, 0);

	metatable_cache["Line"] = lua_ref(R, LUA_REGISTRYINDEX);
}

static void circle(lua_State* R) {
	lua_createtable(R, 0, 2);
	
	rbx::push_method(R, "__index", [](lua_State* R) {
		auto [circle, index] = rbx::index_context<draw::circle>(R);

		if (rbx::compare(index, "Thickness")) {
			lua_pushnumber(R, circle->get_thickness());
		}
		else if (rbx::compare(index, "Radius")) {
			lua_pushnumber(R, circle->get_radius());
		}
		else if (rbx::compare(index, "Sides") || rbx::compare(index, "NumSides")) {
			lua_pushnumber(R, circle->get_sides());
		}
		else if (rbx::compare(index, "Filled")) {
			lua_pushboolean(R, circle->is_filled());
		}
		else if (rbx::compare(index, "Position")) {
			rbx::push_vector2(R, circle->get_position());
		}
		else {
			return element_index(R, circle, index);
		}

		return 1;
	}, 0);

	rbx::push_method(R, "__newindex", [](lua_State* R) {
		auto [circle, index] = rbx::index_context<draw::circle>(R);

		if (rbx::compare(index, "Thickness")) {
			circle->set_thickness(lua_tonumber(R, 3));
		}
		else if (rbx::compare(index, "Radius")) {
			circle->set_radius(lua_tonumber(R, 3));
		}
		else if (rbx::compare(index, "Sides") || rbx::compare(index, "NumSides")) {
			circle->set_sides(lua_tonumber(R, 3));
		}
		else if (rbx::compare(index, "Filled")) {
			circle->set_filled(lua_toboolean(R, 3));
		}
		else if (rbx::compare(index, "Position")) {
			circle->set_position(rbx::to_vector2(R, 3));
		}
		else {
			return element_newindex(R, circle, index);
		}

		return 0;
	}, 0);

	metatable_cache["Circle"] = lua_ref(R, LUA_REGISTRYINDEX);
}

static void text(lua_State* R) {
	lua_createtable(R, 0, 2);

	rbx::push_method(R, "__index", [](lua_State* R) {
		auto [text, index] = rbx::index_context<draw::text>(R);

		if (rbx::compare(index, "OutlineColor")) {
			rbx::push_color3(R, text->get_outline_color());
		}
		else if (rbx::compare(index, "Text")) {
			lua_pushstring(R, text->get_text().data());
		}
		else if (rbx::compare(index, "Size")) {
			lua_pushnumber(R, text->get_size());
		}
		else if (rbx::compare(index, "Position")) {
			rbx::push_vector2(R, text->get_position());
		}
		else if (rbx::compare(index, "Center")) {
			lua_pushboolean(R, text->is_centered());
		}
		else if (rbx::compare(index, "Outline")) {
			lua_pushboolean(R, text->is_outlined());
		}
		else {
			return element_index(R, text, index);
		}

		return 1;
	}, 0);

	rbx::push_method(R, "__newindex", [](lua_State* R) {
		auto [text, index] = rbx::index_context<draw::text>(R);

		if (rbx::compare(index, "OutlineColor")) {
			text->set_outline_color(rbx::to_color3(R, 3));
		}
		else if (rbx::compare(index, "Text")) {
			text->set_text(lua_tostring(R, 3));
		}
		else if (rbx::compare(index, "Size")) {
			text->set_size(lua_tonumber(R, 3));
		}
		else if (rbx::compare(index, "Position")) {
			text->set_position(rbx::to_vector2(R, 3));
		}
		else if (rbx::compare(index, "Center")) {
			text->set_centered(lua_toboolean(R, 3));
		}
		else if (rbx::compare(index, "Outline")) {
			text->set_outline(lua_toboolean(R, 3));
		}
		else {
			return element_newindex(R, text, index);
		}

		return 0;
	}, 0);

	metatable_cache["Text"] = lua_ref(R, LUA_REGISTRYINDEX);
}

static void square(lua_State* R) {
	lua_createtable(R, 0, 2);

	rbx::push_method(R, "__index", [](lua_State* R) {
		auto [square, index] = rbx::index_context<draw::square>(R);

		if (rbx::compare(index, "Size")) {
			rbx::push_vector2(R, square->get_size());
		}
		else if (rbx::compare(index, "Position")) {
			rbx::push_vector2(R, square->get_position());
		}
		else if (rbx::compare(index, "Thickness")) {
			lua_pushnumber(R, square->get_thickness());
		}
		else if (rbx::compare(index, "Filled")) {
			lua_pushboolean(R, square->is_filled());
		}
		else {
			return element_index(R, square, index);
		}

		return 1;
	}, 0);

	rbx::push_method(R, "__newindex", [](lua_State* R) {
		auto [square, index] = rbx::index_context<draw::square>(R);

		if (rbx::compare(index, "Size")) {
			square->set_size(rbx::to_vector2(R, 3));
		}
		else if (rbx::compare(index, "Position")) {
			square->set_position(rbx::to_vector2(R, 3));	
		}
		else if (rbx::compare(index, "Thickness")) {
			square->set_thickness(lua_tonumber(R, 3));
		}
		else if (rbx::compare(index, "Filled")) {
			square->set_filled(lua_toboolean(R, 3));
		}
		else {
			return element_newindex(R, square, index);
		}

		return 1;
	}, 0);

	metatable_cache["Square"] = lua_ref(R, LUA_REGISTRYINDEX);
}

static void quad(lua_State* R) {
	lua_createtable(R, 0, 2);

	rbx::push_method(R, "__index", [](lua_State* R) {
		auto [quad, index] = rbx::index_context<draw::quad>(R);

		if (rbx::compare(index, "PointA")) {
			rbx::push_vector2(R, quad->get_a());
		}
		else if (rbx::compare(index, "PointB")) {
			rbx::push_vector2(R, quad->get_b());
		}
		else if (rbx::compare(index, "PointC")) {
			rbx::push_vector2(R, quad->get_c());
		}
		else if (rbx::compare(index, "PointD")) {
			rbx::push_vector2(R, quad->get_d());
		}
		else if (rbx::compare(index, "Thickness")) {
			lua_pushnumber(R, quad->get_thickness());
		}
		else if (rbx::compare(index, "Filled")) {
			lua_pushboolean(R, quad->is_filled());
		}
		else {
			return element_index(R, quad, index);
		}

		return 1;
	}, 0);

	rbx::push_method(R, "__newindex", [](lua_State* R) {
		auto [quad, index] = rbx::index_context<draw::quad>(R);

		if (rbx::compare(index, "PointA")) {
			quad->set_a(rbx::to_vector2(R, 3));
		}
		else if (rbx::compare(index, "PointB")) {
			quad->set_b(rbx::to_vector2(R, 3));
		}
		else if (rbx::compare(index, "PointC")) {
			quad->set_c(rbx::to_vector2(R, 3));
		}
		else if (rbx::compare(index, "PointD")) {
			quad->set_d(rbx::to_vector2(R, 3));
		}
		else if (rbx::compare(index, "Thickness")) {
			quad->set_thickness(lua_tonumber(R, 3));
		}
		else if (rbx::compare(index, "Filled")) {
			quad->set_filled(lua_toboolean(R, 3));
		}
		else {
			return element_newindex(R, quad, index);
		}

		return 1;
	}, 0);

	metatable_cache["Quad"] = lua_ref(R, LUA_REGISTRYINDEX);
}

static void triangle(lua_State* R) {
	lua_createtable(R, 0, 2);

	rbx::push_method(R, "__index", [](lua_State* R) {
		auto [triangle, index] = rbx::index_context<draw::triangle>(R);

		if (rbx::compare(index, "PointA")) {
			rbx::push_vector2(R, triangle->get_a());
		}
		else if (rbx::compare(index, "PointB")) {
			rbx::push_vector2(R, triangle->get_b());
		}
		else if (rbx::compare(index, "PointC")) {
			rbx::push_vector2(R, triangle->get_c());
		}
		else if (rbx::compare(index, "Thickness")) {
			lua_pushnumber(R, triangle->get_thickness());
		}
		else if (rbx::compare(index, "Filled")) {
			lua_pushboolean(R, triangle->is_filled());
		}
		else {
			return element_index(R, triangle, index);
		}

		return 1;
	}, 0);

	rbx::push_method(R, "__newindex", [](lua_State* R) {
		auto [triangle, index] = rbx::index_context<draw::triangle>(R);

		if (rbx::compare(index, "PointA")) {
			triangle->set_a(rbx::to_vector2(R, 3));
		}
		else if (rbx::compare(index, "PointB")) {
			triangle->set_b(rbx::to_vector2(R, 3));
		}
		else if (rbx::compare(index, "PointC")) {
			triangle->set_c(rbx::to_vector2(R, 3));
		}
		else if (rbx::compare(index, "Thickness")) {
			triangle->set_thickness(lua_tonumber(R, 3));
		}
		else if (rbx::compare(index, "Filled")) {
			triangle->set_filled(lua_toboolean(R, 3));
		}
		else {
			return element_index(R, triangle, index);
		}

		return 1;
	}, 0);

	metatable_cache["Triangle"] = lua_ref(R, LUA_REGISTRYINDEX);
}

static void initialize_metatables(lua_State* R) {
	line(R);
	circle(R);
	text(R);
	square(R);
	quad(R);
	triangle(R);
}

void rbx::draw::init(lua_State* R) {
	initialize_metatables(R);

	lua_createtable(R, 0, 1);

	rbx::push_method(R, "new", [](lua_State* R) {
		const char* class_name = lua_tostring(R, 1);
		auto element = rbx::create<::draw::element>(R, element_cache);

		if (rbx::compare(class_name, "Line")) {
			*element = new ::draw::line;
		}
		else if (rbx::compare(class_name, "Circle")) {
			*element = new ::draw::circle;
		}
		else if (rbx::compare(class_name, "Text")) {
			*element = new ::draw::text;
		}
		else if (rbx::compare(class_name, "Square")) {
			*element = new ::draw::square;
		}
		else if (rbx::compare(class_name, "Quad")) {
			*element = new ::draw::quad;
		}
		else if (rbx::compare(class_name, "Triangle")) {
			*element = new ::draw::triangle;
		}
		else {
			lua_pushnil(R);
			return 1;
		}

		lua_rawgeti(R, LUA_REGISTRYINDEX, metatable_cache[std::string(class_name)]);

		set_type<::draw::element>(R, *element);
		
		lua_setmetatable(R, -2);

		return 1;
	}, 0);

	lua_setglobal(R, "Drawing");
}
