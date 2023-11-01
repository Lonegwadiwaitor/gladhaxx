#pragma once

#include "component.hpp"

namespace ig {
	class child;

	class container : public component {
	private:
		std::list<child*> _children;
	public:
		void add(child* child);
		void remove(child* child);
		void destroy(const std::function<void(component*)>& callback = nullptr) override;

		std::list<child*> get_children();
		child* find_first_child(const std::string &name, bool recursive = false);
	};
}
