#pragma once

#include "container.hpp"
#include "child.hpp"

namespace ig {
	class tool_tip : public container {
		child* _parent;
	public:
		void render(const std::string &title_text);
		void render() override;

		tool_tip();

		child* get_parent() const;
		void set_parent(child* parent);
	};
}
