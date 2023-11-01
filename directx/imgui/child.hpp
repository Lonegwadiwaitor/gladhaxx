#pragma once

#include "component.hpp"
#include "container.hpp"
#include "tool_tip.hpp"

namespace ig {
	class container;
	class tool_tip;

	class child : public component {
	private:
		container* _parent;
		tool_tip* _tool_tip;
		bool _visible, _same_line;
	public:
		void destroy(const std::function<void(component*)> &callback = nullptr) override;

		child();

		bool is_visible() const;
		bool is_same_line() const;

		container* get_parent() const;
		tool_tip* get_tool_tip() const;

		void set_parent(container* parent);
		void set_visible(bool visible);
		void set_same_line(bool same_line);
		void set_tool_tip(tool_tip* tool_tip);
	};
}
