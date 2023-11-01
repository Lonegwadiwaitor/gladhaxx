#pragma once

#include "child.hpp"

namespace ig {
	enum class message_type {
		OK,
		YESNO,
		OKCANCEL,
		YESNOCANCEL
	};

	enum class message_result {
		OK,
		YES,
		NO,
		CANCEL
	};

	class message_box : public child {
	public:
		using callback_t = std::function<void(const ig::message_result&)>;
	private:
		bool _showing;
		std::string _title;
		std::string _description;
		message_type _type;

		message_box::callback_t _callback;
	public:
		void render() override;

		message_box();

		void show();
		void set_title(const std::string& title);
		void set_description(const std::string& description);
		void set_type(const message_type& type);
		void set_callback(const message_box::callback_t& callbacks);

		message_type get_type() const;
		std::string get_title() const;
		std::string get_description() const;
	};
}
