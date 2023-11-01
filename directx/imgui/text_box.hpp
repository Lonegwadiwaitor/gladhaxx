#pragma once

#include "child.hpp"
#include "util.hpp"

namespace ig {
	class text_box : public child {
	private:
		bool _multi_line;
		std::string _title;
		std::array<char, 2048> _text;
		callback_t _callback;
	public:
		void render() override;

		text_box();

		bool is_multi_line();
		std::string get_title() const;
		std::string get_text() const;
		callback_t get_callback() const;

		void set_title(const std::string& text);
		void set_text(const std::string& text);
		void set_multi_line(bool multi_line);
		void set_callback(const callback_t& callback);
	};
}
