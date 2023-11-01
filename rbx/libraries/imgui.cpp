#include "../../src/pch.hpp"

#include "../../directx/imgui/window.hpp"
#include "../../directx/imgui/tab_selector.hpp"
#include "../../directx/imgui/tree_view.hpp"
#include "../../directx/imgui/button.hpp"
#include "../../directx/imgui/label.hpp"
#include "../../directx/imgui/slider.hpp"
#include "../../directx/imgui/check_box.hpp"
#include "../../directx/imgui/combo_box.hpp"
#include "../../directx/imgui/radio_box.hpp"
#include "../../directx/imgui/search_box.hpp"
#include "../../directx/imgui/text_box.hpp"
#include "../../directx/imgui/color_picker.hpp"
#include "../../directx/imgui/tool_tip.hpp"
#include "../../directx/imgui/key_bind.hpp"
#include "../../directx/imgui/message_box.hpp"

#include "imgui.hpp"
#include "../sync/rapi.hpp"

#include "../../libraries/os/output.hpp"
#include "../scheduler/task_scheduler.hpp"

#include "util.hpp"

static std::unordered_map<ig::component*, int> component_cache{};
static std::unordered_map<ig::component*, std::vector<int>> callback_cache{};
static std::unordered_map<std::string, int> metatable_cache{};

inline void rbx::ig::clear_component_cache() {
	component_cache.clear();
}

inline void rbx::ig::clear_callback_cache() {
	callback_cache.clear();
}

inline void rbx::ig::clear_metatable_cache() {
	metatable_cache.clear();
}

static int component_index(lua_State* R, const ig::component* component, const char* index) {
	if (rbx::compare(index, "Name")) {
		lua_pushstring(R, component->get_name().data());
	}
	else if (rbx::compare(index, "ClassName")) {
		lua_pushstring(R, component->get_class_name().data());
	}
	else {
		sys::cout((((("\""s + index) + "\" is not a valid member of \"") + component->get_class_name()) + "\""));
		*static_cast<uintptr_t*>(nullptr) = 0;
	}

	return 1;
}

static int component_newindex(lua_State* R, ig::component* component, const char* index) {
	if (rbx::compare(index, "Name")) {
		component->set_name(lua_tostring(R, 3));
	}
	else {
		sys::cout((((("\""s + index) + "\" is not a valid member of \"") + component->get_class_name()) + "\""));
		*static_cast<uintptr_t*>(nullptr) = 0;
	}
	
	return 0;
}

static void free_callbacks(lua_State* R, ::ig::child* child) {
	for (const auto& ref_id : callback_cache[child]) {
		lua_pushnil(R);
		lua_rawseti(R, LUA_REGISTRYINDEX, ref_id);
	}

	callback_cache.erase(child);
}

static int child_index(lua_State* R, ig::child* child, const char* index) {
	if (rbx::compare(index, "Visible")) {
		lua_pushboolean(R, child->is_visible());
	}
	else if (rbx::compare(index, "SameLine")) {
		lua_pushboolean(R, child->is_same_line());
	}
	else if (rbx::compare(index, "Parent")) {
		lua_rawgeti(R, LUA_REGISTRYINDEX, component_cache[child->get_parent()]);
	}
	else if (rbx::compare(index, "ToolTip")) {
		lua_rawgeti(R, LUA_REGISTRYINDEX, component_cache[child->get_tool_tip()]);
	}
	else if (rbx::compare(index, "Destroy")) {
		lua_pushcclosure(R, [](lua_State* R) {
			auto child = rbx::element_cast<ig::child>(R, 1);
			
			free_callbacks(R, child);
			rbx::free<ig::component>(R, component_cache, child);

			child->destroy([](ig::component* child) { delete child; });
			
			return 0;
		}, "", 0);
	}
	else {
		return component_index(R, child, index);
	}

	return 1;
}

static int child_newindex(lua_State* R, ig::child* child, const char* index) {
	if (rbx::compare(index, "Visible")) {
		child->set_visible(lua_toboolean(R, 3));
	}
	else if (rbx::compare(index, "SameLine")) {
		child->set_same_line(lua_toboolean(R, 3));
	}
	else if (rbx::compare(index, "Parent")) {
		child->set_parent(rbx::element_cast<ig::container>(R, 3));
	}
	else if (rbx::compare(index, "ToolTip")) {
		child->set_tool_tip(rbx::element_cast<ig::tool_tip>(R, 3));
	}
	else {
		return component_newindex(R, child, index);
	}

	return 0;
}

static void uncache_children(lua_State* R, ig::container* container) {
	for (auto& child : container->get_children()) {
		if (callback_cache.contains(child)) {
			free_callbacks(R, child);
			callback_cache.erase(child);
		}

		rbx::free<ig::component>(R, component_cache, child);
	}
}

static void window(lua_State* R) {
	lua_createtable(R, 0, 2);
	
	rbx::push_method(R, "__index", [](lua_State* R) {
		const auto& [window, index] = rbx::index_context<ig::window>(R);

		if (rbx::compare(index, "Text")) {
			lua_pushstring(R, window->get_text().data());
		}
		else if (rbx::compare(index, "Size")) {
			rbx::push_vector2(R, window->get_size());
		}
		else if (rbx::compare(index, "Position")) {
			rbx::push_vector2(R, window->get_position());
		}
		else if (rbx::compare(index, "Destroy")) {
			lua_pushcclosure(R, [](lua_State* R) {
				auto window = rbx::element_cast<ig::window>(R, 1);

				uncache_children(R, window);

				window->destroy([](ig::component* child) { delete child; });

				rbx::free<ig::component>(R, component_cache, window);

				ig::windows.erase(std::remove(ig::windows.begin(), ig::windows.end(), *window));
				return 0;
			}, "", 0);
		}
		else if (rbx::compare(index, "GetChildren")) {
			lua_pushcclosure(R, [](lua_State* R) {
				const auto window = rbx::element_cast<ig::window>(R, 1);
				int i = 0;

				lua_createtable(R, 1,  0);

				for (ig::child* child : window->get_children()) {
					lua_rawgeti(R, LUA_REGISTRYINDEX, component_cache[child]);
					lua_rawseti(R, -2, ++i);
				}

				return 1;
			}, "", 0);
		}
		else if (rbx::compare(index, "FindFirstChild")) {
			lua_pushcclosure(R, [](lua_State* R) {
				const auto window = rbx::element_cast<ig::window>(R, 1);

				if (ig::child* child = window->find_first_child(lua_tostring(R, 2), lua_type(R, 3) == LUA_TBOOLEAN && lua_toboolean(R, 3))) {
					lua_rawgeti(R, LUA_REGISTRYINDEX, component_cache[child]);
				}
				else {
					lua_pushnil(R);
				}

				return 1;
			}, "", 0);
		}
		else {
			return component_index(R, window, index);
		}

		return 1;
	}, 0);

	rbx::push_method(R, "__newindex", [](lua_State* R) {
		const auto& [window, index] = rbx::index_context<ig::window>(R);

		if (rbx::compare(index, "Text")) {
			window->set_text(lua_tostring(R, 3));
		}
		else if (rbx::compare(index, "Size")) {
			window->set_size(rbx::to_vector2(R, 3));
		}
		else if (rbx::compare(index, "Position")) {
			window->set_position(rbx::to_vector2(R, 3));
		}
		else {
			return component_newindex(R, window, index);
		}

		return 0;
	}, 0);

	metatable_cache["Window"] = lua_ref(R, LUA_REGISTRYINDEX);
}

static void tab(lua_State* R) {
	lua_createtable(R, 0, 2);

	rbx::push_method(R, "__index", [](lua_State* R) {
		const auto& [tab, index] = rbx::index_context<ig::tab>(R);

		if (rbx::compare(index, "Text")) {
			lua_pushstring(R, tab->get_text().data());
		}
		else if (rbx::compare(index, "GetChildren")) {
			lua_pushcclosure(R, [](lua_State* R) {
				auto tab = rbx::element_cast<ig::tab>(R, 1);
				int i = 0;

				lua_newtable(R);

				for (ig::child* child : tab->get_children()) {
					lua_rawgeti(R, LUA_REGISTRYINDEX, component_cache[child]);
					lua_rawseti(R, -2, ++i);
				}

				return 1;
			}, "", 0);
		}
		else if (rbx::compare(index, "FindFirstChild")) {
			lua_pushcclosure(R, [](lua_State* R) {
				auto tab = rbx::element_cast<ig::tab>(R, 1);

				if (ig::child* child = tab->find_first_child(lua_tostring(R, 2), lua_type(R, 3) == LUA_TBOOLEAN && lua_toboolean(R, 3))) {
					lua_rawgeti(R, LUA_REGISTRYINDEX, component_cache[child]);
				}
				else {
					lua_pushnil(R);
				}

				return 1;
			}, "", 0);
		}
		else {
			return component_index(R, tab, index);
		}

		return 1;
	}, 0);

	rbx::push_method(R, "__newindex", [](lua_State* R) {
		auto [tab, index] = rbx::index_context<ig::tab>(R);
		
		if (rbx::compare(index, "Text")) {
			tab->set_text(lua_tostring(R, 3));
		}
		else {
			return component_newindex(R, tab, index);
		}

		return 0;
	}, 0);

	metatable_cache["Tab"] = lua_ref(R, LUA_REGISTRYINDEX);
}

static void tab_selector(lua_State* R) {
	lua_createtable(R, 0, 2);

	rbx::push_method(R, "__index", [](lua_State* R) {
		const auto& [tab_selector, index] = rbx::index_context<ig::tab_selector>(R);

		if (rbx::compare(index, "Add")) {
			lua_pushcclosure(R, [](lua_State* R) {
				auto tab_selector = rbx::element_cast<ig::tab_selector>(R, 1);
				auto component = rbx::create(R, component_cache);
				*component = tab_selector->add(lua_tostring(R, 2));

				lua_rawgeti(R, LUA_REGISTRYINDEX, metatable_cache[std::string("Tab")]);
				rbx::set_type<::ig::container>(R, *component);
				lua_setmetatable(R, -2);

				return 1;
			}, "", 0);
		}
		else if (rbx::compare(index, "Remove")) {
			lua_pushcclosure(R, [](lua_State* R) {
				auto tab_selector = rbx::element_cast<ig::tab_selector>(R, 1);
				auto tab = rbx::element_cast<ig::tab>(R, 2);

				uncache_children(R, tab);

				rbx::free<ig::component>(R, component_cache, tab);

				tab_selector->remove(tab, [](ig::component* child) { delete child; });

				return 0;
			}, "", 0);
		}
		else if (rbx::compare(index, "Destroy")) {
			lua_pushcclosure(R, [](lua_State* R) {
				auto tab_selector = rbx::element_cast<ig::tab_selector>(R, 1);

				for (auto& tab : tab_selector->get_tabs()) {
					uncache_children(R, &tab);
				}

				rbx::free<ig::component>(R, component_cache, tab_selector);

				tab_selector->destroy([](ig::component* child) { delete child; });

				delete tab_selector;

				return 0;
			}, "", 0);
		}
		else if (rbx::compare(index, "GetChildren")) {
			lua_pushcclosure(R, [](lua_State* R) {
				auto tab_selector = rbx::element_cast<ig::tab_selector>(R, 1);
				int i = 0;
				
				lua_newtable(R);

				for (ig::tab& tab : tab_selector->get_tabs()) {
					lua_rawgeti(R, LUA_REGISTRYINDEX, component_cache[&tab]);
					lua_rawseti(R, -2, ++i);
				}

				return 1;
			}, "", 0);
		}
		else {
			return child_index(R, tab_selector, index);
		}

		return 1;
	}, 0);

	rbx::push_method(R, "__newindex", [](lua_State* R) {
		const auto& [tab_selector, index] = rbx::index_context<ig::tab_selector>(R);

		return child_newindex(R, tab_selector, index);
	}, 0);

	metatable_cache["TabSelector"] = lua_ref(R, LUA_REGISTRYINDEX);
}

static void tree_view(lua_State* R) {
	lua_createtable(R, 0, 2);

	rbx::push_method(R, "__index", [](lua_State* R) {
		const auto& [tree_view, index] = rbx::index_context<ig::tree_view>(R);
		const auto container = reinterpret_cast<ig::container*>(tree_view);

		if (rbx::compare(index, "Text")) {
			lua_pushstring(R, tree_view->get_text().data());
		}
		else if (rbx::compare(index, "Destroy")) {
			lua_pushcclosure(R, [](lua_State* R) {
				const auto tree_view = rbx::element_cast<ig::tree_view>(R, 1);
				const auto container = reinterpret_cast<ig::container*>(tree_view);

				for (auto& child : tree_view->get_children()) {
					free_callbacks(R, child);
					rbx::free<ig::component>(R, component_cache, child);
				}

				container->destroy([](ig::component* child) { delete child; });

				rbx::free<ig::component>(R, component_cache, container);

				delete tree_view;

				return 0;
			}, "", 0);
		}
		else if (rbx::compare(index, "GetChildren")) {
			lua_pushcclosure(R, [](lua_State* R) {
				auto tree_view = rbx::element_cast<ig::tree_view>(R, 1);
				int i = 0;

				lua_newtable(R);

				for (ig::child* child : tree_view->get_children()) {
					lua_rawgeti(R, LUA_REGISTRYINDEX, component_cache[child]);
					lua_rawseti(R, -2, ++i);
				}

				return 1;
			}, "", 0);
		}
		else if (rbx::compare(index, "FindFirstChild")) {
			lua_pushcclosure(R, [](lua_State* R) {
				auto tree_view = rbx::element_cast<ig::tree_view>(R, 1);

				if (ig::child* child = tree_view->find_first_child(lua_tostring(R, 2), lua_type(R, 3) == LUA_TBOOLEAN && lua_toboolean(R, 3))) {
					lua_rawgeti(R, LUA_REGISTRYINDEX, component_cache[child]);
				}
				else {
					lua_pushnil(R);
				}

				return 1;
			}, "", 0);
		}
		else {
			return child_index(R, tree_view, index);
		}

		return 1;
	}, 0);

	rbx::push_method(R, "__newindex", [](lua_State* R) {
		const auto& [tree_view, index] = rbx::index_context<ig::tree_view>(R);
		const auto container = reinterpret_cast<ig::container*>(tree_view);

		if (rbx::compare(index, "Text")) {
			tree_view->set_text(lua_tostring(R, 3));
		}
		else {
			return child_newindex(R, tree_view, index);
		}

		return 0;
	}, 0);

	metatable_cache["TreeView"] = lua_ref(R, LUA_REGISTRYINDEX);
}

static void tool_tip(lua_State* R) {
	lua_createtable(R, 0, 2);

	rbx::push_method(R, "__index", [](lua_State* R) {
		const auto& [tool_tip, index] = rbx::index_context<ig::tool_tip>(R);

		if (rbx::compare(index, "Destroy")) {
			lua_pushcclosure(R, [](lua_State* R) {
				auto tool_tip = rbx::element_cast<ig::tool_tip>(R, 1);
				auto parent = tool_tip->get_parent();

				for (auto& child : tool_tip->get_children()) {
					free_callbacks(R, child);
					rbx::free<ig::component>(R, component_cache, child);
				}

				tool_tip->destroy([](ig::component* child) { delete child; });

				if (parent) {
					parent->set_tool_tip(nullptr);
				}

				rbx::free<ig::component>(R, component_cache, tool_tip);

				return 0;
			}, "", 0);
		}
		else if (rbx::compare(index, "GetChildren")) {
			lua_pushcclosure(R, [](lua_State* R) {
				auto tool_tip = rbx::element_cast<ig::tool_tip>(R, 1);
				int i = 0;

				lua_newtable(R);

				for (ig::child* child : tool_tip->get_children()) {
					lua_rawgeti(R, LUA_REGISTRYINDEX, component_cache[child]);
					lua_rawseti(R, -2, ++i);
				}

				return 1;
			}, "", 0);
		}
		else if (rbx::compare(index, "FindFirstChild")) {
			lua_pushcclosure(R, [](lua_State* R) {
				auto tool_tip = rbx::element_cast<ig::tool_tip>(R, 1);

				if (ig::child* child = tool_tip->find_first_child(lua_tostring(R, 2), lua_type(R, 3) == LUA_TBOOLEAN && lua_toboolean(R, 3))) {
					lua_rawgeti(R, LUA_REGISTRYINDEX, component_cache[child]);
				}
				else {
					lua_pushnil(R);
				}

				return 1;
			}, "", 0);
		}
		else {
			return component_index(R, tool_tip, index);
		}

		return 1;
	}, 0);

	rbx::push_method(R, "__newindex", [](lua_State* R) {
		const auto& [tool_tip, index] = rbx::index_context<ig::tool_tip>(R);

		return component_newindex(R, tool_tip, index);
	}, 0);

	metatable_cache["ToolTip"] = lua_ref(R, LUA_REGISTRYINDEX);
}

static void slider_int(lua_State* R) {
	lua_createtable(R, 0, 2);

	rbx::push_method(R, "__index", [](lua_State* R) { 
		auto [slider_int, index] = rbx::index_context<ig::slider_int>(R);

		if (rbx::compare(index, "Value")) {
			lua_pushnumber(R, slider_int->get());
		}
		else if (rbx::compare(index, "Min")) {
			lua_pushnumber(R, slider_int->get_min());
		}
		else if (rbx::compare(index, "Max")) {
			lua_pushnumber(R, slider_int->get_max());
		}
		else if (rbx::compare(index, "Text")) {
			lua_pushstring(R, slider_int->get_text().c_str());
		}
		else if (rbx::compare(index, "Connect")) {
			lua_getfield(R, LUA_REGISTRYINDEX, "__imgui_connect");
		}
		else {
			return child_index(R, slider_int, index);
		}

		return 1; 
	}, 0);

	rbx::push_method(R, "__newindex", [](lua_State* R) { 
		auto [slider_int, index] = rbx::index_context<ig::slider_int>(R);

		if (rbx::compare(index, "Value")) {
			slider_int->set(lua_tonumber(R, 3));
		}
		else if (rbx::compare(index, "Min")) {
			slider_int->set_min(lua_tonumber(R, 3));
		}
		else if (rbx::compare(index, "Max")) {
			slider_int->set_max(lua_tonumber(R, 3));
		}
		else if (rbx::compare(index, "Text")) {
			slider_int->set_text(lua_tostring(R, 3));
		}
		else {
			return child_newindex(R, slider_int, index);
		}
		
		return 0; 
	}, 0);

	metatable_cache["SliderInt"] = lua_ref(R, LUA_REGISTRYINDEX);
}

static void slider_float(lua_State* R) {
	lua_createtable(R, 0, 2);

	rbx::push_method(R, "__index", [](lua_State* R) { 
		auto [slider_float, index] = rbx::index_context<ig::slider_float>(R);

		if (rbx::compare(index, "Value")) {
			lua_pushnumber(R, slider_float->get());
		}
		else if (rbx::compare(index, "Min")) {
			lua_pushnumber(R, slider_float->get_min());
		}
		else if (rbx::compare(index, "Max")) {
			lua_pushnumber(R, slider_float->get_max());
		}
		else if (rbx::compare(index, "Text")) {
			lua_pushstring(R, slider_float->get_text().c_str());
		}
		else if (rbx::compare(index, "Connect")) {
			lua_getfield(R, LUA_REGISTRYINDEX, "__imgui_connect");
		}
		else {
			return child_index(R, slider_float, index);
		}
		
		return 1; 
	}, 0);

	rbx::push_method(R, "__newindex", [](lua_State* R) { 
		auto [slider_float, index] = rbx::index_context<ig::slider_float>(R);

		if (rbx::compare(index, "Value")) {
			slider_float->set(lua_tonumber(R, 3));
		}
		else if (rbx::compare(index, "Min")) {
			slider_float->set_min(lua_tonumber(R, 3));
		}
		else if (rbx::compare(index, "Max")) {
			slider_float->set_max(lua_tonumber(R, 3));
		}
		else if (rbx::compare(index, "Text")) {
			slider_float->set_text(lua_tostring(R, 3));
		}
		else {
			return child_newindex(R, slider_float, index);
		}
		
		return 0; 
	}, 0);

	metatable_cache["SliderFloat"] = lua_ref(R, LUA_REGISTRYINDEX);
}

static void label(lua_State* R) {
	lua_createtable(R, 0, 2);

	rbx::push_method(R, "__index", [](lua_State* R) {
		auto [label, index] = rbx::index_context<ig::label>(R);
	
		if (rbx::compare(index, "Text")) {
			lua_pushstring(R, label->get_text().data());
		}
		else if (rbx::compare(index, "Color")) {
			rbx::push_color3(R, label->get_color());
		}
		else {
			return child_index(R, label, index);
		}

		return 1;
	}, 0);

	rbx::push_method(R, "__newindex", [](lua_State* R) {
		auto [label, index] = rbx::index_context<ig::label>(R);
		
		if (rbx::compare(index, "Text")) {
			label->set_text(lua_tostring(R, 3));
		}
		else if (rbx::compare(index, "Color")) {
			label->set_color(rbx::to_color3(R, 3));
		}
		else {
			return child_newindex(R, label, index);
		}

		return 0;
	}, 0);

	metatable_cache["Label"] = lua_ref(R, LUA_REGISTRYINDEX);
}

static void button(lua_State* R) {
	lua_createtable(R, 0, 2);

	rbx::push_method(R, "__index", [](lua_State* R) { 
		auto [button, index] = rbx::index_context<ig::button>(R);

		if (rbx::compare(index, "Text")) {
			lua_pushstring(R, button->get_text().data());
		}
		else if (rbx::compare(index, "Connect")) {
			lua_getfield(R, LUA_REGISTRYINDEX, "__imgui_connect");
		}
		else {
			return child_index(R, button, index);
		}

		return 1; 
	}, 0);

	rbx::push_method(R, "__newindex", [](lua_State* R) { 
		auto [button, index] = rbx::index_context<ig::button>(R);

		if (rbx::compare(index, "Text")) {
			button->set_text(lua_tostring(R, 3));
		}
		else {
			return child_newindex(R, button, index);
		}

		return 0; 
	}, 0);

	metatable_cache["Button"] = lua_ref(R, LUA_REGISTRYINDEX);
}

static void text_box(lua_State* R) {
	lua_createtable(R, 0, 2);

	rbx::push_method(R, "__index", [](lua_State* R) { 
		auto [text_box, index] = rbx::index_context<ig::text_box>(R);

		if (rbx::compare(index, "Text")) {
			lua_pushstring(R, text_box->get_title().data());
		}
		else if (rbx::compare(index, "Value")) {
			lua_pushstring(R, text_box->get_text().data());
		}
		else if (rbx::compare(index, "Connect")) {
			lua_getfield(R, LUA_REGISTRYINDEX, "__imgui_connect");
		}
		else {
			return child_index(R, text_box, index);
		}

		return 1; 
	}, 0);

	rbx::push_method(R, "__newindex", [](lua_State* R) { 
		auto [text_box, index] = rbx::index_context<ig::text_box>(R);

		if (rbx::compare(index, "Text")) {
			text_box->set_title(lua_tostring(R, 3));
		}
		else if (rbx::compare(index, "Value")) {
			text_box->set_text(lua_tostring(R, 3));
		}
		else {
			return child_newindex(R, text_box, index);
		}
		
		return 0; 
	}, 0);

	metatable_cache["TextBox"] = lua_ref(R, LUA_REGISTRYINDEX);
}

static void search_box(lua_State* R) {
	lua_createtable(R, 0, 2);

	rbx::push_method(R, "__index", [](lua_State* R) {
		auto [search_box, index] = rbx::index_context<ig::search_box>(R);

		if (rbx::compare(index, "Text")) {
			lua_pushstring(R, search_box->get_title().data());
		} else if (rbx::compare(index, "Value")) {
			lua_pushstring(R, search_box->get_text().data());
		} else if (rbx::compare(index, "Connect")) {
			lua_getfield(R, LUA_REGISTRYINDEX, "__imgui_connect");
		} else if (rbx::compare(index, "Add")) {
			lua_pushcclosure(R, [](lua_State* R) {
				auto search_box = rbx::element_cast<ig::search_box>(R, 1);
				search_box->add(lua_tostring(R, 2));

				return 0;
			}, "", 0);
		} else if (rbx::compare(index, "Remove")) {
			lua_pushcclosure(R, [](lua_State* R) {
				auto search_box = rbx::element_cast<ig::search_box>(R, 1);
				search_box->remove(lua_tostring(R, 2));

				return 0;
			}, "", 0);
		} else {
			return child_index(R, search_box, index);
		}

		return 1;
	}, 0);

	rbx::push_method(R, "__newindex", [](lua_State* R) {
		auto [search_box, index] = rbx::index_context<ig::search_box>(R);

		if (rbx::compare(index, "Text")) {
			search_box->set_title(lua_tostring(R, 3));
		} else if (rbx::compare(index, "Value")) {
			search_box->set_text(lua_tostring(R, 3));
		} else {
			return child_newindex(R, search_box, index);
		}

		return 0;
	}, 0);

	metatable_cache["SearchBox"] = lua_ref(R, LUA_REGISTRYINDEX);
}

static void combo_box(lua_State* R) {
	lua_createtable(R, 0, 2);

	rbx::push_method(R, "__index", [](lua_State* R) { 
		auto [combo_box, index] = rbx::index_context<ig::combo_box>(R);

		if (rbx::compare(index, "Selected") || rbx::compare(index, "Value")) {
			lua_pushstring(R, combo_box->get_selected());
		}
		else if (rbx::compare(index, "SelectedIndex")) {
			lua_pushnumber(R, combo_box->get_selected_index());
		}
		else if (rbx::compare(index, "Text")) {
			lua_pushstring(R, combo_box->get_text());
		}
		else if (rbx::compare(index, "Connect")) {
			lua_getfield(R, LUA_REGISTRYINDEX, "__imgui_connect");
		}
		else if (rbx::compare(index, "Add")) {
			lua_pushcclosure(R, [](lua_State* R) {
				auto combo_box = rbx::element_cast<ig::combo_box>(R, 1);
				combo_box->add(lua_tostring(R, 2));

				return 0;
			}, "", 0);
		}
		else if (rbx::compare(index, "Remove")) {
			lua_pushcclosure(R, [](lua_State* R) {
				auto combo_box = rbx::element_cast<ig::combo_box>(R, 1);
				combo_box->remove(lua_tostring(R, 2));

				return 0;
			}, "", 0);
		}
		else if (rbx::compare(index, "Has")) {
			lua_pushcclosure(R, [](lua_State* R) {
				auto combo_box = rbx::element_cast<ig::combo_box>(R, 1);
				combo_box->has(lua_tostring(R, 2));

				return 0;
			}, "", 0);
		}
		else if (rbx::compare(index, "GetMembers")) {
			lua_pushcclosure(R, [](lua_State* R) {
				auto combo_box = rbx::element_cast<ig::combo_box>(R, 1);
				int i = 0;

				lua_newtable(R);

				for (auto& member : combo_box->get_members()) {
					lua_pushstring(R, member);
					lua_rawseti(R, -2, ++i);
				}

				return 1;
			}, "", 0);
		}
		else if (rbx::compare(index, "ClearAllMembers")) {
			lua_pushcclosure(R, [](lua_State* R) {
				auto combo_box = rbx::element_cast<ig::combo_box>(R, 1);

				combo_box->clear();

				return 0;
			}, "", 0);
		}
		else {
			return child_index(R, combo_box, index);
		}

		return 1; 
	}, 0);

	rbx::push_method(R, "__newindex", [](lua_State* R) {
		auto [combo_box, index] = rbx::index_context<ig::combo_box>(R);

		if (rbx::compare(index, "Selected")) {
			combo_box->set_selected(lua_tostring(R, 3));
		}
		else if (rbx::compare(index, "Text")) {
			combo_box->set_text(lua_tostring(R, 3));
		}
		else {
			return child_newindex(R, combo_box, index);
		}

		return 0; 
	}, 0);

	metatable_cache["ComboBox"] = lua_ref(R, LUA_REGISTRYINDEX);
}

static void radio_box(lua_State* R) {
	lua_createtable(R, 0, 2);

	rbx::push_method(R, "__index", [](lua_State* R) { 
		auto [radio_box, index] = rbx::index_context<ig::radio_box>(R);

		if (rbx::compare(index, "Text")) {
			lua_pushstring(R, radio_box->get_text());
		}
		else if (rbx::compare(index, "Selected") || rbx::compare(index, "Value")) {
			lua_pushstring(R, radio_box->get());
		}
		else if (rbx::compare(index, "Connect")) {
			lua_getfield(R, LUA_REGISTRYINDEX, "__imgui_connect");
		}
		else if (rbx::compare(index, "Add")) {
			lua_pushcclosure(R, [](lua_State* R) {
				auto radio_box = rbx::element_cast<ig::radio_box>(R, 1);
				radio_box->add(lua_tostring(R, 2));

				return 0;
			}, "", 0);
		}
		else if (rbx::compare(index, "Has")) {
			lua_pushcclosure(R, [](lua_State* R) {
				auto radio_box = rbx::element_cast<ig::radio_box>(R, 1);
				radio_box->has(lua_tostring(R, 2));

				return 0;
			}, "", 0);
		}
		else if (rbx::compare(index, "GetMembers")) {
			lua_pushcclosure(R, [](lua_State* R) {
				auto radio_box = rbx::element_cast<ig::radio_box>(R, 1);
				int i = 0;

				lua_newtable(R);

				for (auto& button : radio_box->get_buttons()) {
					lua_pushstring(R, button.get_text());
					lua_rawseti(R, -2, ++i);
				}

				return 1;
			}, "", 0);
		}
		else {
			return child_index(R, radio_box, index);
		}

		return 1; 
	}, 0);

	rbx::push_method(R, "__newindex", [](lua_State* R) { 
		auto [radio_box, index] = rbx::index_context<ig::radio_box>(R);

		if (rbx::compare(index, "Text")) {
			radio_box->set_text(lua_tostring(R, 3));
		}
		else if (rbx::compare(index, "Selected") || rbx::compare(index, "Value")) {
			radio_box->set(lua_tostring(R, 3));
		}
		else {
			return child_newindex(R, radio_box, index);
		}

		return 0; 
	}, 0);

	metatable_cache["RadioBox"] = lua_ref(R, LUA_REGISTRYINDEX);
}

static void check_box(lua_State* R) {
	lua_createtable(R, 0, 2);
	
	rbx::push_method(R, "__index", [](lua_State* R) { 
		auto [check_box, index] = rbx::index_context<ig::check_box>(R);

		if (rbx::compare(index, "Text")) {
			lua_pushstring(R, check_box->get_text());
		}
		else if (rbx::compare(index, "Value") || rbx::compare(index, "Toggled")) {
			lua_pushboolean(R, check_box->is_toggled());
		}
		else if (rbx::compare(index, "Connect")) {
			lua_getfield(R, LUA_REGISTRYINDEX, "__imgui_connect");
		}
		else {
			return child_index(R, check_box, index);
		}

		return 1; 
	}, 0);

	rbx::push_method(R, "__newindex", [](lua_State* R) { 
		auto [check_box, index] = rbx::index_context<ig::check_box>(R);

		if (rbx::compare(index, "Text")) {
			check_box->set_text(lua_tostring(R, 3));
		}
		else if (rbx::compare(index, "Value") || rbx::compare(index, "Toggled")) {
			check_box->set_toggled(lua_toboolean(R, 3));
		}
		else {
			return child_newindex(R, check_box, index);
		}

		return 0; 
	}, 0);

	metatable_cache["CheckBox"] = lua_ref(R, LUA_REGISTRYINDEX);
}

static void color_picker(lua_State* R) {
	lua_createtable(R, 0, 2);

	rbx::push_method(R, "__index", [](lua_State* R) { 
		auto [color_picker, index] = rbx::index_context<ig::color_picker>(R);

		if (rbx::compare(index, "Text")) {
			lua_pushstring(R, color_picker->get_text().c_str());
		}
		else if (rbx::compare(index, "Color") || rbx::compare(index, "Value")) {
			rbx::push_color3(R, color_picker->get_color());
		}
		else if (rbx::compare(index, "Alpha")) {
			lua_pushnumber(R, color_picker->get_color().w);
		}
		else if (rbx::compare(index, "Connect")) {
			lua_getfield(R, LUA_REGISTRYINDEX, "__imgui_connect");
		}
		else {
			return child_index(R, color_picker, index);
		}

		return 1; 
	}, 0);

	rbx::push_method(R, "__newindex", [](lua_State* R) { 
		auto [color_picker, index] = rbx::index_context<ig::color_picker>(R);

		if (rbx::compare(index, "Text")) {
			color_picker->set_text(lua_tostring(R, 3));
		}
		else if (rbx::compare(index, "Color") || rbx::compare(index, "Value")) {
			color_picker->set_color(rbx::to_color3(R, 3));
		}
		else if (rbx::compare(index, "Alpha")) {
			auto [r, g, b, a] = color_picker->get_color();
			color_picker->set_color({ r, g, b, static_cast<float>(lua_tonumber(R, 1)) });
		}
		else {
			return child_newindex(R, color_picker, index);
		}

		return 0; 
	}, 0);

	metatable_cache["ColorPicker"] = lua_ref(R, LUA_REGISTRYINDEX);
}

static void key_bind(lua_State* R) {
	lua_createtable(R, 0, 2);

	rbx::push_method(R, "__index", [](lua_State* R) {
		auto [key_bind, index] = rbx::index_context<ig::key_bind>(R);

		if (rbx::compare(index, "Text")) {
			lua_pushstring(R, key_bind->get_text().c_str());
		} else if (rbx::compare(index, "KeyCode")) {
			lua_getglobal(R, "Enum");
			lua_getfield(R, -1, "KeyCode");
			lua_getfield(R, -1, key_bind->get_key_code().c_str());
		} else if (rbx::compare(index, "Connect")) {
			lua_pushcclosure(R, [](lua_State* R) {
				auto key_bind = rbx::element_cast<::ig::key_bind>(R, 1);
				lua_pushvalue(R, 2);
				callback_cache[key_bind].push_back(lua_ref(R, LUA_REGISTRYINDEX));
				lua_getfield(R, LUA_REGISTRYINDEX, "__imgui_keybinds");
				lua_pushvalue(R, 1);
				lua_gettable(R, -2);

				if (lua_type(R, -1) == LUA_TNIL) {
					MessageBox(nullptr, _STR("KeyCode not set for ImGui KeyBind component"), _STR("Fatal Error"), MB_ICONERROR);
					
					exit(1);
					quick_exit(1);
					PostQuitMessage(1);
				}

				lua_pushvalue(R, 2);
				lua_ref(R, -2);

				return 0;
			}, "", 0);
		} else {
			return child_index(R, key_bind, index);
		}

		return 1;
	}, 0);

	rbx::push_method(R, "__newindex", [](lua_State* R) {
		auto [key_bind, index] = rbx::index_context<ig::key_bind>(R);

		if (rbx::compare(index, "Text")) {
			key_bind->set_text(lua_tostring(R, 3));
		} else if (rbx::compare(index, "KeyCode")) {
			lua_pushvalue(R, 3);
			lua_getmetatable(R, -1);
			lua_getfield(R, -1, "__tostring");
			lua_pushvalue(R, 3);
			lua_pcall(R, 1, 1, 0);
			
			const std::string key_code = lua_tostring(R, -1);
			constexpr std::string_view test = "Enum.KeyCode.";
			
			key_bind->set_key_code(std::string(key_code.begin() + test.size(), key_code.end()));

			lua_getfield(R, LUA_REGISTRYINDEX, "__imgui_keybinds");
			lua_pushvalue(R, 1);
			lua_gettable(R, -2);

			if (lua_type(R, -1) == LUA_TNIL) {
				lua_pop(R, 1);
				lua_pushvalue(R, 1);
				lua_newtable(R);
				lua_settable(R, -3);
			}
		} else {
			return child_newindex(R, key_bind, index);
		}

		return 0;
	}, 0);

	metatable_cache["KeyBind"] = lua_ref(R, LUA_REGISTRYINDEX);
}

static void message_box(lua_State* R) {
	lua_createtable(R, 0, 2);

	rbx::push_method(R, "__index", [](lua_State* R) {
		auto [message_box, index] = rbx::index_context<ig::message_box>(R);

		if (rbx::compare(index, "Title")) {
			lua_pushstring(R, message_box->get_title().c_str());
		}
		else if (rbx::compare(index, "Text") || rbx::compare(index, "Description")) {
			lua_pushstring(R, message_box->get_description().c_str());
		}
		else if (rbx::compare(index, "Type")) {
			lua_pushnumber(R, static_cast<int>(message_box->get_type()));
		}
		else if (rbx::compare(index, "Connect")) {
			lua_getfield(R, LUA_REGISTRYINDEX, "__imgui_connect");
		}
		else if (rbx::compare(index, "Show")) {
			lua_pushcclosure(R, [](lua_State* R) {
				rbx::element_cast<::ig::message_box>(R, 1)->show();
				return 0;
			}, "", 0);
		}
		else {
			return child_index(R, message_box, index);
		}

		return 1;
	}, 0);

	rbx::push_method(R, "__newindex", [](lua_State* R) {
		auto [message_box, index] = rbx::index_context<ig::message_box>(R);

		if (rbx::compare(index, "Title")) {
			message_box->set_title(lua_tostring(R, 3));
		}
		else if (rbx::compare(index, "Text") || rbx::compare(index, "Description")) {
			message_box->set_description(lua_tostring(R, 3));
		}
		else if (rbx::compare(index, "Type")) {
			message_box->set_type(static_cast<::ig::message_type>(lua_tointeger(R, 3)));
		}
		else {
			return child_newindex(R, message_box, index);
		}

		return 0;
	}, 0);

	metatable_cache["MessageBox"] = lua_ref(R, LUA_REGISTRYINDEX);
}

static void initialize_metatables(lua_State* R) {
	window(R);
	slider_int(R);
	slider_float(R);
	button(R);
	label(R);
	text_box(R);
	check_box(R);
	combo_box(R);
	radio_box(R);
	color_picker(R);
	tab_selector(R);
	tree_view(R);
	tool_tip(R);
	search_box(R);
	key_bind(R);
	tab(R);
	message_box(R);
}

template<typename T>
void create_with_event(lua_State* R, ::ig::component** component) {
	auto new_component = new T;

	if constexpr (std::is_same<T, ::ig::message_box>::value) {
		new_component->set_callback([=](const ::ig::message_result& result) {
			rbx::task_scheduler::get_singleton().push([=]() {
				lua_getfield(R, LUA_REGISTRYINDEX, "__imgui_events");

				for (const auto& ref_id : callback_cache[new_component]) {
					lua_newtable(R);
					lua_rawgeti(R, LUA_REGISTRYINDEX, ref_id);
					lua_setfield(R, -2, "Function");
					
					lua_newtable(R);
					lua_pushnumber(R, static_cast<int>(result));
					lua_rawseti(R, -2, 1);
					lua_setfield(R, -2, "Params");

					lua_ref(R, -2);
				}

				lua_pop(R, 1);
			});
		});
	}
	else {
		new_component->set_callback([=]() {
			rbx::task_scheduler::get_singleton().push([=]() {
				lua_getfield(R, LUA_REGISTRYINDEX, "__imgui_events");

				for (const auto& ref_id : callback_cache[new_component]) {
					lua_createtable(R, 0, 1);
					lua_rawgeti(R, LUA_REGISTRYINDEX, ref_id);
					lua_setfield(R, -2, "Function");

					lua_createtable(R, 0, 1);
					lua_setfield(R, -2, "Params");

					lua_ref(R, -2);
				}

				lua_pop(R, 1);
			});
		});
	}

	callback_cache[new_component] = std::vector<int>{};
	*component = new_component;
}

void rbx::ig::init(lua_State *R) {


	initialize_metatables(R);

	lua_pushcclosure(R, [](lua_State* R) {
		const auto child = rbx::element_cast<::ig::child>(R, 1);
		callback_cache[child].push_back(lua_ref(R, LUA_REGISTRYINDEX));
		return 0;
	}, "", 0);
	lua_setfield(R, -10003, "__imgui_connect");

	lua_createtable(R, 0, 4); // 2 tables, 2 functions

	{
		lua_createtable(R, 0, 4); // 4 numbers

		lua_pushnumber(R, static_cast<int>(::ig::message_type::OK));
		lua_setfield(R, -2, "OK");
	
		lua_pushnumber(R, static_cast<int>(::ig::message_type::YESNO));
		lua_setfield(R, -2, "YESNO");
	
		lua_pushnumber(R, static_cast<int>(::ig::message_type::OKCANCEL));
		lua_setfield(R, -2, "OKCANCEL");

		lua_pushnumber(R, static_cast<int>(::ig::message_type::YESNOCANCEL));
		lua_setfield(R, -2, "YESNOCANCEL");

		lua_setfield(R, -2, "MessageType");
	}

	{
		lua_createtable(R, 0, 4); // 4 nu

		lua_pushnumber(R, static_cast<int>(::ig::message_result::OK));
		lua_setfield(R, -2, "OK");

		lua_pushnumber(R, static_cast<int>(::ig::message_result::YES));
		lua_setfield(R, -2, "YES");

		lua_pushnumber(R, static_cast<int>(::ig::message_result::NO));
		lua_setfield(R, -2, "NO");

		lua_pushnumber(R, static_cast<int>(::ig::message_result::CANCEL));
		lua_setfield(R, -2, "CANCEL");

		lua_setfield(R, -2, "MessageResult");
	}
	
	rbx::push_method(R, "IsAnyItemActive", [](lua_State* R) {
		lua_pushboolean(R, ImGui::IsAnyItemActive());
		return 1;
	}, 0);

	rbx::push_method(R, "new", [](lua_State* R) { 
		const char* class_name = lua_tostring(R, 1);
		sys::cout("creating ", class_name, ", len: ", strlen(class_name), '\n');
		auto component = rbx::create(R, component_cache);
		bool is_container = false;
		sys::cout("creating imgui object\n");
		if (rbx::compare(class_name, "Window")) {
			*component = ::ig::window::create();
			is_container = true;
		} else if (rbx::compare(class_name, "SliderInt")) {
			create_with_event<::ig::slider_int>(R, component);
		} else if (rbx::compare(class_name, "SliderFloat")) {
			create_with_event<::ig::slider_float>(R, component);
		} else if (rbx::compare(class_name, "Button")) {
			create_with_event<::ig::button>(R, component);
		} else if (rbx::compare(class_name, "Label")) {
			*component = new ::ig::label;
		} else if (rbx::compare(class_name, "TextBox")) {
			create_with_event<::ig::text_box>(R, component);
		} else if (rbx::compare(class_name, "CheckBox")) {
			create_with_event<::ig::check_box>(R, component);
		} else if (rbx::compare(class_name, "ComboBox")) {
			*component = new ::ig::combo_box;
		} else if (rbx::compare(class_name, "RadioBox")) {
			create_with_event<::ig::radio_box>(R, component);
		} else if (rbx::compare(class_name, "ColorPicker")) {
			create_with_event<::ig::color_picker>(R, component);
		} else if (rbx::compare(class_name, "TabSelector")) {
			*component = new ::ig::tab_selector;
		} else if (rbx::compare(class_name, "SearchBox")) {
			create_with_event<::ig::search_box>(R, component);
		} else if (rbx::compare(class_name, "TreeView")) {
			*component = new ::ig::tree_view;
		} else if (rbx::compare(class_name, "KeyBind")) {
			auto&& key_bind = new ::ig::key_bind;
			callback_cache[key_bind] = std::vector<int>();
			*component = key_bind;
		} else if (rbx::compare(class_name, "ToolTip")) {
			*component = new ::ig::tool_tip;
			is_container = true;
		} else if (rbx::compare(class_name, "MessageBox")) {
			create_with_event<::ig::message_box>(R, component);
		} else {
			lua_pushnil(R);
			return 1;
		}

		sys::cout("getting cached metatable\n");

		lua_rawgeti(R, LUA_REGISTRYINDEX, metatable_cache[std::string(class_name)]);
		
		set_type<::ig::component>(R, *component);
		sys::cout("setting type\n");

		if (!is_container && lua_type(R, 2) == LUA_TUSERDATA) {
		sys::cout("parenting\n");
			auto container = element_cast<::ig::container>(R, 2);
			reinterpret_cast<::ig::child*>(*component)->set_parent(container);
		}
		
		sys::cout("setting metatable\n");
		lua_setmetatable(R, -2);
		sys::cout("done\n");
		return 1; 
	}, 0);
	lua_setglobal(R, "ImGui");
}
