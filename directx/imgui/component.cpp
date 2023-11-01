#include "../../src/pch.hpp"

#include "component.hpp"

bool ig::component::operator==(const ig::component& other) {
	return this == &other;
}

//bool ig::component::operator==(const ig::component& left, const ig::component& right) {
//	return &left == &right;
//}

std::string ig::component::get_name() const {
	return _name;
}

std::string ig::component::get_class_name() const {
	return _class_name;
}

void ig::component::set_name(const std::string& name) {
	_name = name;
}

void::ig::component::set_class_name(const std::string& name) {
	_class_name = name;
}
