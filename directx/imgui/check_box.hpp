#pragma once

#include "child.hpp"
#include "util.hpp"

namespace ig {
	class check_box : public child {
	private:
		bool _toggled;
		std::string _text;
		callback_t _callback;
	public:
		void render() override;

		check_box();

		bool is_toggled() const;
		std::string get_text() const;
		callback_t get_callback() const;

		void set_toggled(bool toggled);
		void set_text(const std::string& text);
		void set_callback(const callback_t& callback);
	};
}
