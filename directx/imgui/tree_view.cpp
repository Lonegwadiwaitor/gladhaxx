#include "../../src/pch.hpp"

#include "tree_view.hpp"
#include "tab_selector.hpp"

#include "../../libraries/imgui/imgui.h"
#include "../../libraries/os/output.hpp"

#include "util.hpp"

ig::tree_view::tree_view() :
	_text(random())
{
	set_visible(true);
	set_same_line(false);

	reinterpret_cast<ig::child*>(this)->set_name(random());
	reinterpret_cast<ig::child*>(this)->set_class_name(_STR("TreeView"));
}

void ig::tree_view::render() {
	if (is_visible()) {
		if (ImGui::TreeNodeEx(_text.c_str())) {
			for (ig::child* child : get_children()) {
				child->render();
			}

			ImGui::TreePop();
		}
	}
}

void ig::tree_view::add(child* child) {
	_children.push_back(child);
}

void ig::tree_view::remove(child* child) {
	_children.erase(std::remove(_children.begin(), _children.end(), child));
}

void ig::tree_view::destroy(const std::function<void(component*)>& callback) {
	for (ig::child* child : _children) {
		if (child->get_class_name() == _STR("TreeView")) {
			reinterpret_cast<tree_view*>(child)->destroy(callback);
		}
		else {
			child->destroy(callback);
		}
	}

	_children.clear();

	if (ig::container* parent = get_parent(); parent->get_class_name() == _STR("TreeView")) {
		reinterpret_cast<ig::tree_view*>(parent)->remove(this);
	}
	else {
		parent->remove(this);
	}

	if (callback) {
		callback(this);
	}
}

std::list<ig::child*> ig::tree_view::get_children() {
	return _children;
}

ig::child* ig::tree_view::find_first_child(const std::string& name, bool recursive) {
	for (ig::child* child : _children) {
		if (child->get_name() == name) {
			return child;
		}
		else if (const std::string& class_name = child->get_class_name(); recursive) {
			if (class_name == "TreeView") {
				return reinterpret_cast<ig::tree_view*>(child)->find_first_child(name, true);
			}
			else if (class_name == "TabSelector") {
				auto tab_selector = reinterpret_cast<ig::tab_selector*>(child);

				for (auto& tab : tab_selector->get_tabs()) {
					if (ig::child* child = tab.find_first_child(name, recursive)) {
						return child;
					}
				}
			}
		}
	}

	return nullptr;
}


std::string ig::tree_view::get_text() const {
	return _text;
}

void ig::tree_view::set_text(const std::string& text) {
	_text = text;
}
