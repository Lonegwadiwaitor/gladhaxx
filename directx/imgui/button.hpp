#pragma once

#include "child.hpp"
#include "util.hpp"

namespace ig {
	class button : public child {
	private:
		std::string _text;
		callback_t _callback;
	public:
		void render() override;

		button();

		std::string get_text() const;
		callback_t get_callback() const;

		void set_text(const std::string &text);
		void set_callback(const callback_t& callback);
	};
}
