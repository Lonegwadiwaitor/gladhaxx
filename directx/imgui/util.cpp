#include "../../src/pch.hpp"

#include "../draw/text.hpp"
#include "../../libraries/os/file.hpp"
#include "../../libraries/imgui/imgui.h"

#include "util.hpp"

static std::random_device device;
static std::uniform_int_distribution<uint64_t> generator(0, pow(2, 63));

std::string ig::random() {
	return std::to_string(generator(device));
}

static json get_theme_file() {
	if (auto theme_file = sys::get_root_dir() / _STR("..") / _STR("theme.json"); filesystem::exists(theme_file)) {
		return json::parse(sys::read_file(theme_file));
	}
	else {
		json theme{ {"font", "Calibri"} };
		sys::write_file(theme_file, theme.dump());

		return theme;
	}
}

void ig::initialize_font() {
	const filesystem::path fonts_directory = sys::get_root_dir() / _STR("..") / _STR("fonts");
	const json theme = get_theme_file();

	if (theme.contains("font") && filesystem::exists(fonts_directory)) {
		const auto& font_file = fonts_directory / (theme["font"].get<std::string>() + _STR(".ttf"));

		if (filesystem::exists(font_file)) {
			const auto& fonts = ImGui::GetIO().Fonts;

			if (!fonts->Fonts.empty()) {
				fonts->Fonts.clear();
			}

			draw::set_font(fonts->AddFontFromFileTTF(font_file.string().c_str(), 16));
		}
	}
}
