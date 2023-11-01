#include "../../src/pch.hpp"

#include "tool_tip.hpp"

#include "../../libraries/imgui/imgui.h"

#include "util.hpp"

ig::tool_tip::tool_tip() :
	_parent(nullptr)
{
	set_name(random());
	set_class_name(_STR("ToolTip"));
}

void ig::tool_tip::render(const std::string& title_text) {
	ImGui::SameLine();
	ImGui::TextDisabled(_STR("(?)"));

	if (ImGui::IsItemHovered()) {
		ImGui::BeginTooltip();

		if (!title_text.empty()) {
			ImGui::Text(title_text.c_str());
		}

		for (auto element : get_children()) {
			element->render();
		}

		ImGui::EndTooltip();
	}
}

void ig::tool_tip::render() {
	return render({});
}

ig::child* ig::tool_tip::get_parent() const {
	return _parent;
}

void ig::tool_tip::set_parent(ig::child* parent) {
	_parent = parent;
}
