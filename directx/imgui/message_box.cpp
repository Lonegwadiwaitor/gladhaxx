#include "../../src/pch.hpp"

#include "message_box.hpp"

#include "../../libraries/imgui/imgui.h"
#include "../../libraries/os/output.hpp"

#include "util.hpp"

ig::message_box::message_box() :
	_showing(false),
	_title(random()),
	_description(random()),
	_type(ig::message_type::OK)
{
	set_name(random());
	set_class_name(_STR("MessageBox"));
}

void ig::message_box::render() {
	if (ImGui::BeginPopupModal(_title.c_str())) {
		ImGui::TextWrapped(_description.c_str());

		switch (_type) {
		case ig::message_type::OK: {
			if (ImGui::Button("OK") && _callback) {
				_callback(ig::message_result::OK);
				ImGui::CloseCurrentPopup();
				_showing = false;
			}

			break;
		}
		case ig::message_type::YESNO: {
			if (ImGui::Button("Yes") && _callback) {
				_callback(ig::message_result::YES);
				ImGui::CloseCurrentPopup();
				_showing = false;
			}

			ImGui::SameLine();
			
			if (ImGui::Button("No") && _callback) {
				_callback(ig::message_result::NO);
				ImGui::CloseCurrentPopup();
				_showing = false;
			}

			break;
		}
		case ig::message_type::OKCANCEL: {
			if (ImGui::Button("OK") && _callback) {
				_callback(ig::message_result::OK);
				ImGui::CloseCurrentPopup();
				_showing = false;
			}

			ImGui::SameLine();

			if (ImGui::Button("Cancel") && _callback) {
				_callback(ig::message_result::CANCEL);
				ImGui::CloseCurrentPopup();
				_showing = false;
			}

			break;
		}
		case ig::message_type::YESNOCANCEL: {
			if (ImGui::Button("Yes") && _callback) {
				_callback(ig::message_result::YES);
				ImGui::CloseCurrentPopup();
				_showing = false;
			}

			ImGui::SameLine();

			if (ImGui::Button("No") && _callback) {
				_callback(ig::message_result::NO);
				ImGui::CloseCurrentPopup();
				_showing = false;
			}

			ImGui::SameLine();

			if (ImGui::Button("Cancel") && _callback) {
				_callback(ig::message_result::CANCEL);
				ImGui::CloseCurrentPopup();
				_showing = false;
			}

			break;
		}
		}

		ImGui::EndPopup();
	}
	else if (_showing) {
		ImGui::OpenPopup(_title.c_str());
	}
}

void ig::message_box::show() {
	_showing = true;
}

void ig::message_box::set_title(const std::string& title) {
	_title = title;
}

void ig::message_box::set_description(const std::string& description) {
	_description = description;
}

void ig::message_box::set_type(const message_type& type) {
	_type = type;
}

void ig::message_box::set_callback(const message_box::callback_t& callback) {
	_callback = callback;
}

ig::message_type ig::message_box::get_type() const {
	return _type;
};

std::string ig::message_box::get_title() const {
	return _title;
};

std::string ig::message_box::get_description() const {
	return _description;
};
