#pragma once

#include "child.hpp"
#include "util.hpp"

#include <functional>
#include <vector>

namespace ig {
	class combo_box : public child {
	private:
		int _selected_index;
		std::string _text;
		std::string _selected;
		std::vector<std::string> _members;
		callback_t _callback;
	public:
		void render() override;

		combo_box();

		int get_selected_index() const;
		std::string get_text() const;
		std::string get_selected() const;
		std::vector<std::string> get_members() const;
		callback_t get_callback() const;

		void add(const std::string& text);
		void remove(const std::string& text);
		bool has(const std::string& text);
		void set_text(const std::string& text);
		void set_selected(const std::string& text);
		void set_callback(const callback_t& callback);
		void clear();
	};
}
