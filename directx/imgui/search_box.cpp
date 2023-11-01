#include "../../src/pch.hpp"

#include "search_box.hpp"

#include "../../libraries/imgui/imgui.h"
#include "../../libraries/imgui/imgui_internal.h"

#include "../../libraries/os/output.hpp"

#include "util.hpp"

ig::search_box::search_box() :
	_title(random()),
	_open(false),
	_multi_line(false)
{
	set_visible(true);
	set_same_line(false);

	set_name(random());
	set_class_name(_STR("SearchBox"));
}

void ig::search_box::render() {
	if (is_visible()) {
		if (is_same_line()) {
			ImGui::SameLine();
		}

		if (_multi_line) {
			if (ImGui::InputTextMultiline(_title.c_str(), _text.data(), _text.size(), ImVec2(0, 0), ImGuiInputTextFlags_AllowTabInput | ImGuiInputTextFlags_EnterReturnsTrue) && _callback) {
				_callback();
			}
		}
		else {
			if (ImGui::InputText(_title.c_str(), _text.data(), _text.size()/*, ImGuiInputTextFlags_AllowTabInput | ImGuiInputTextFlags_EnterReturnsTrue*/) && _callback) {
				_callback();
			}
		}

		bool focused = ImGui::IsItemFocused();
		_open |= ImGui::IsItemActive();
		if (_open)
		{
			ImGui::SetNextWindowPos({ ImGui::GetItemRectMin().x, ImGui::GetItemRectMax().y });
			ImGui::SetNextWindowSize({ ImGui::GetItemRectSize().x, 0 });
			if (ImGui::Begin(_STR("##popup"), &_open, ImGuiWindowFlags_Tooltip | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings))
			{
				ImGui::BringWindowToDisplayFront(ImGui::GetCurrentWindow());
				focused |= ImGui::IsWindowFocused();
				for (const auto& result : _results)
				{
					if (strstr(result.c_str(), _text.data()) == NULL)
						continue;

					bool clicked = ImGui::Button(result.c_str());

					if (clicked)
					{
						strcpy_s(_text.data(), _text.size(), result.c_str());
						_open = false;
					}
				}
			}
			ImGui::End();
			_open &= focused;
		}

		/*bool focused = ImGui::IsItemFocused();
		_open |= ImGui::IsItemActive();

		if (_open) {
			ImGui::SetNextWindowPos({ ImGui::GetItemRectMin().x, ImGui::GetItemRectMax().y });
			ImGui::SetNextWindowSize({ ImGui::GetItemRectSize().x, ImGui::GetItemRectSize().y * 8 });
		
			if (ImGui::Begin(get_name().c_str(), &_open, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_Tooltip)) {
				ImGui::BringWindowToDisplayFront(ImGui::GetCurrentWindow());
				focused |= ImGui::IsWindowFocused();
			
				std::string lower_text = _text.data();
				std::transform(lower_text.begin(), lower_text.end(), lower_text.begin(), [](unsigned char c) { return std::tolower(c); });

				for (const auto& result : _results) {
					std::string lower_result = result;
					std::transform(lower_result.begin(), lower_result.end(), lower_result.begin(), [](unsigned char c) { return std::tolower(c); });

					if (lower_result.substr(0, lower_text.size()) != lower_text) {
						continue;
					}

					if (ImGui::Selectable(result.c_str())) {
						sys::cout("yay\n");
					}
				}
			}

			ImGui::End();
			_open &= focused;
		}*/

		if (get_tool_tip() != nullptr) {
			get_tool_tip()->render();
		}
	}
}

std::string ig::search_box::get_text() const {
	return _text.data();
}

std::string ig::search_box::get_title() const {
	return _title;
}

bool ig::search_box::is_multi_line() {
	return _multi_line;
}

ig::callback_t ig::search_box::get_callback() const {
	return _callback;
}

void ig::search_box::set_text(const std::string& text) {
	strcpy_s(_text.data(), _text.size(), text.c_str());
}

void ig::search_box::set_title(const std::string& title) {
	_title = title;
}

void ig::search_box::set_multi_line(bool multi_line) {
	_multi_line = multi_line;
}

void ig::search_box::set_callback(const callback_t& callback) {
	_callback = callback;
}

void ig::search_box::add(const std::string& result) {
	_results.push_back(result);
}

void ig::search_box::remove(const std::string& result) {
	std::remove(_results.begin(), _results.end(), result);
}
