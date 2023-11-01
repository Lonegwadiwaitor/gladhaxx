#include "../../src/pch.hpp"

#include "container.hpp"
#include "child.hpp"
#include "tree_view.hpp"
#include "tab_selector.hpp"

void ig::container::add(child* child) {
	_children.push_back(child);
}

void ig::container::remove(child* child) {
	_children.erase(std::remove(_children.begin(), _children.end(), child));
}

void ig::container::destroy(const std::function<void(component*)> &callback) {
	for (ig::child* child : _children) {
		if (child->get_class_name() == _STR("TreeView")) {
			reinterpret_cast<tree_view*>(child)->destroy(callback);
		}
		else {
			child->destroy(callback);
		}
	}
}

std::list<ig::child*> ig::container::get_children() {
	return _children;
}

ig::child* ig::container::find_first_child(const std::string& name, bool recursive) {
	for (ig::child* child : _children) {
		if (child->get_name() == name) {
			return child;
		}
		else if (const std::string& class_name = child->get_class_name(); recursive) {
			if (class_name == _STR("TreeView")) {
				return reinterpret_cast<ig::tree_view*>(child)->find_first_child(name, true);
			}
			else if (class_name == _STR("TabSelector")) {
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
