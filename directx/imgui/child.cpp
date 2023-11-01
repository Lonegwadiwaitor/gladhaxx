#include "../../src/pch.hpp"
#include "../../libraries/os/output.hpp"

#include "child.hpp"
#include "tree_view.hpp"

ig::child::child() :
	_tool_tip(nullptr),
	_parent(nullptr),
	_same_line(false),
	_visible(true)
{
}

bool ig::child::is_visible() const {
	return _visible;
}

bool ig::child::is_same_line() const {
	return _same_line;
}

ig::container* ig::child::get_parent() const {
	return _parent;
}

ig::tool_tip* ig::child::get_tool_tip() const {
	return _tool_tip;
}

void ig::child::set_parent(container* container) {
	if (_parent) {
		if (_parent->get_class_name() == _STR("TreeView")) {
			reinterpret_cast<ig::tree_view*>(_parent)->remove(this);
		}
		else {
			_parent->remove(this);
		}
	}
	
	if (container->get_class_name() == _STR("TreeView")) {
		reinterpret_cast<tree_view*>(container)->add(this);
	}
	else {
		container->add(this);
	}

	_parent = container;
}

void ig::child::set_visible(bool visible) {
	_visible = visible;
}

void ig::child::set_same_line(bool same_line) {
	_same_line = same_line;
}

void ig::child::set_tool_tip(ig::tool_tip* tool_tip) {
	_tool_tip = tool_tip;

	tool_tip->set_parent(this);
}

void ig::child::destroy(const std::function<void(component*)>& callback) {
	// remove it
	if (_parent) {
		if (_parent->get_class_name() == _STR("TreeView")) {
			reinterpret_cast<ig::tree_view*>(_parent)->remove(this);
		}
		else {
			_parent->remove(this);
		}
	}

	if (_tool_tip) {
		_tool_tip->destroy(callback);

		if (callback) {
			callback(_tool_tip);
		}

		_tool_tip = nullptr;
	}

	// run any cleanup function
	if (callback) {
		callback(this);
	}
}
