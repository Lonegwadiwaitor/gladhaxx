#pragma once

#include "child.hpp"

namespace ig {
	class tab_selector;

	class tab : public container {
	private:
		std::string _text;
		tab_selector* _tab_selector;
	public:
		tab(tab_selector* tab_selector, const std::string& text);

		void render() override;

		std::string get_text() const;
		tab_selector* get_tab_selector() const;

		void set_text(const std::string& text);
	};

	class tab_selector : public child {
	private:
		std::list<tab> _tabs;
		std::string _id;
	public:
		void render() override;
		void destroy(const std::function<void(component*)>&) override;

		tab_selector();

		std::list<tab>& get_tabs();
		tab* add(const std::string& text);
		void remove(tab* tab, const std::function<void(ig::component*)>& callback = nullptr);
	};
}
