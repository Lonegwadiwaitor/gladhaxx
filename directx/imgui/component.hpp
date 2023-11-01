#pragma once

#include "../../src/pch.hpp"

namespace ig {
	class component {
	private:
		std::string _name;
		std::string _class_name;
	public:
		virtual void render() = 0;
		virtual void destroy(const std::function<void(component*)> &callback) = 0;

		std::string get_name() const;
		std::string get_class_name() const;
		
		void set_name(const std::string &name);
		void set_class_name(const std::string& name);

		bool operator==(const component& other);
		//bool operator==(const component& left, const component& right);
	};
}
