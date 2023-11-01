#pragma once

#include "container.hpp"
#include "child.hpp"

namespace ig {
	class tree_view : public child {
	private:
		std::string _text;
		std::list<child*> _children;
	public:
		void destroy(const std::function<void(component*)>& callback = nullptr) override;

		tree_view();

		void add(child* child);
		void remove(child* child);
		
		std::list<child*> get_children();
		child* find_first_child(const std::string& name, bool recursive = false);
		std::string get_text() const;

		void set_text(const std::string &text);
		void render() override;
	};
}
