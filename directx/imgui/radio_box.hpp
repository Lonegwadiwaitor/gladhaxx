#pragma once

#include "child.hpp"
#include "util.hpp"

namespace ig {
	class radio_box;

	class radio_button : public child {
		int* _container;
		int _id;
		
		std::string _text;
		callback_t _callback;
	public:
		radio_button(const radio_box& parent, const std::string& text, int* container, int id);
		void render() override;

		int get_id() const;
		std::string get_text() const;
		callback_t get_callback() const;

		void set_text(const std::string& text);
		void set_callback(const callback_t& callback);
	};

	class radio_box : public child {
	private:
		int _id;
		std::string _text;
		callback_t _callback;
		std::list<radio_button> _buttons;
	public:
		void render() override;
		
		radio_box();

		radio_button* add(const std::string& text);

		bool has(const std::string& text);
		std::string get() const;
		std::string get_text() const;
		callback_t get_callback() const;
		std::list<ig::radio_button> get_buttons() const;

		void set(const std::string& text);
		void set_text(const std::string& text);
		void set_callback(const callback_t& callback);
	};
}
