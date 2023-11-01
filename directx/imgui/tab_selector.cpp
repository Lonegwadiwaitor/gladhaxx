#include "../../src/pch.hpp"

#include "tab_selector.hpp"
#include "util.hpp"

#include "../../libraries/imgui/imgui.h"

#include "../../libraries/os/output.hpp"

ig::tab::tab(ig::tab_selector* tab_selector, const std::string& text) : 
	_text(text),
	_tab_selector(tab_selector)
{
	set_name(text);
	set_class_name(_STR("Tab"));
}

ig::tab_selector::tab_selector() :
	_id()
{
	set_name(random());
	set_class_name(_STR("TabSelector"));
}

void ig::tab::render() {
	if (ImGui::BeginTabItem(_text.c_str())) {
		for (ig::component* element : this->get_children()) {
			element->render();
		}

		ImGui::EndTabItem();
	}
}

static std::unordered_map<ImGuiCol_, const char*> theming_names
{
	{ ImGuiCol_Text, "Text" },
	{ ImGuiCol_TextDisabled, "Text (Disabled)" },
	{ ImGuiCol_WindowBg, "Window Background" },
	{ ImGuiCol_ChildBg, "Child Background" },
	{ ImGuiCol_PopupBg, "Popup Background" },
	{ ImGuiCol_Border, "Border" },
	{ ImGuiCol_BorderShadow, "Border (Shadow)" },
	{ ImGuiCol_FrameBg, "Frame Background" },
	{ ImGuiCol_FrameBgHovered, "Frame Background (Hovered)" },
	{ ImGuiCol_FrameBgActive, "Frame Background (Active)" },
	{ ImGuiCol_TitleBg, "Title Background" },
	{ ImGuiCol_TitleBgActive, "Title Background (Active)" },
	{ ImGuiCol_TitleBgCollapsed, "Title Background (Collapsed)" },
	{ ImGuiCol_MenuBarBg, "Menu Bar Background" },
	{ ImGuiCol_ScrollbarBg, "Scroll Bar Background" },
	{ ImGuiCol_ScrollbarGrab, "Scroll Bar Grab" },
	{ ImGuiCol_ScrollbarGrabHovered, "Scroll Bar Grab (Hovered)" },
	{ ImGuiCol_ScrollbarGrabActive, "Scroll Bar Grab (Active)" },
	{ ImGuiCol_CheckMark, "Check Mark" },
	{ ImGuiCol_SliderGrab, "Slider Grab" },
	{ ImGuiCol_SliderGrabActive, "Slider Grab (Active)" },
	{ ImGuiCol_Button, "Button" },
	{ ImGuiCol_ButtonHovered, "Button (Hovered)"},
	{ ImGuiCol_ButtonActive, "Button (Active)"},
	{ ImGuiCol_Header, "Header" },
	{ ImGuiCol_HeaderHovered, "Header (Hovered) "},
	{ ImGuiCol_HeaderActive, "Header (Active) "},
	{ ImGuiCol_Separator, "Separator" },
	{ ImGuiCol_SeparatorHovered, "Separator (Hovered)" },
	{ ImGuiCol_SeparatorActive, "Separator (Active)" },
	{ ImGuiCol_ResizeGrip, "Resize Grip" },
	{ ImGuiCol_ResizeGripHovered, "Resize Grip (Hovered)" },
	{ ImGuiCol_ResizeGripActive, "Resize Grip (Active)" },
	{ ImGuiCol_Tab, "Tab" },
	{ ImGuiCol_TabHovered, "Tab (Hovered)" },
	{ ImGuiCol_TabActive, "Tab (Active)" },
	{ ImGuiCol_TabUnfocused, "Tab (Unfocused)" },
	{ ImGuiCol_TabUnfocusedActive, "Tab (Unfocused Active)" },
	{ ImGuiCol_PlotLines, "Plot Lines" },
	{ ImGuiCol_PlotLinesHovered, "Plot Lines (Hovered)" },
	{ ImGuiCol_PlotHistogram, "Plot Histogram" },
	{ ImGuiCol_PlotHistogramHovered, "Plot Histogram (Hovered)" },
	{ ImGuiCol_TextSelectedBg, "Text Selected Background" },
	{ ImGuiCol_DragDropTarget, "Drag Drop Target" },
	{ ImGuiCol_NavHighlight, "Nav Highlight" },
	{ ImGuiCol_NavWindowingHighlight, "Nav Windowing Highlight" },
	{ ImGuiCol_NavWindowingDimBg, "Nav Windowing (Dim) Background" },
	{ ImGuiCol_ModalWindowDimBg, "Modal Window (Dim) Background"}
};

static void render_settings_tab() {
	// only runs in ImGui::BeginTabBar context

	ImGuiStyle* style = &ImGui::GetStyle();

	if (ImGui::BeginTabItem("Settings")) {
		ImGui::Spacing();

		if (ImGui::TreeNodeEx("Theming")) {
			auto& colors = style->Colors;

			//ImGui::StyleColorsSirMeme();

			for (int i = 0; i < theming_names.size(); i++) { // theming
				float Color[4] = { colors[i].x, colors[i].y, colors[i].z, colors[i].w };
				ImGui::ColorEdit4(theming_names[static_cast<ImGuiCol_>(i)], Color);
				colors[i] = *reinterpret_cast<ImVec4*>(Color);
			}

			ImGui::TreePop();
		}

		ImGui::EndTabItem();
	}
}

void ig::tab_selector::render() {
	if (is_same_line()) {
		ImGui::SameLine();
	}

	if (is_visible() && ImGui::BeginTabBar(_id.c_str())) {
		for (ig::tab& tab : _tabs) {
			tab.render();
		}

		// TODO: Render settings tab

		render_settings_tab();

		ImGui::EndTabBar();
	}
}

void ig::tab_selector::destroy(const std::function<void(ig::component*)>& callback) {
	// run cleanup function on children in tabs & destroy
	for (ig::tab &tab : _tabs) {
		tab.destroy(callback);
	}
	
	// clear tabs
	_tabs.clear();

	// run any cleanup function
	if (callback) {
		callback(this);
	}

	// finally remove it
	get_parent()->remove(this);
}

std::string ig::tab::get_text() const {
	return _text;
}

ig::tab_selector* ig::tab::get_tab_selector() const {
	return _tab_selector;
}

std::list<ig::tab>& ig::tab_selector::get_tabs() {
	return _tabs;
}

ig::tab* ig::tab_selector::add(const std::string& text) {
	return &_tabs.emplace_back(this, text);
}

void ig::tab_selector::remove(ig::tab* tab, const std::function<void(ig::component*)>& callback) {
	tab->destroy(callback);
	_tabs.erase(std::remove(_tabs.begin(), _tabs.end(), *tab));
}

void ig::tab::set_text(const std::string& text) {
	_text = text;
}

