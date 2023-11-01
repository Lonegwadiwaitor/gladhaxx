#include "console.hpp"

#include "lualib.h"

#include "../../libraries/os/output.hpp"

int rconsoletitle(lua_State* R) {

	return 0;
}

void GetInfo()
{
	GetConsoleScreenBufferInfo(con::hCon, &con::csbi);
	con::dwConSize = con::csbi.dwSize.X * con::csbi.dwSize.Y;
}

void SetColor(WORD wRGBI, WORD Mask)
{
	GetInfo();
	con::csbi.wAttributes &= Mask;
	con::csbi.wAttributes |= wRGBI;
	SetConsoleTextAttribute(con::hCon, con::csbi.wAttributes);
}

std::unordered_map<std::string_view, WORD> color {
	{ "@@BLACK@@", con::fgBlack },
	{ "@@BLUE@@", con::fgHiBlue },
	{ "@@GREEN@@", con::fgHiGreen },
	{ "@@CYAN@@", con::fgHiCyan },
	{ "@@RED@@", con::fgHiRed },
	{ "@@MAGENTA@@", con::fgHiMagenta },
	{ "@@BROWN@@", 0 },
	{ "@@LIGHT_GRAY@@", con::fgGray },
	{ "@@DARK_GRAY@@", con::fgGray },
	{ "@@LIGHT_BLUE@@", con::fgLoBlue },
	{ "@@LIGHT_GREEN@@", con::fgLoGreen },
	{ "@@LIGHT_CYAN@@", con::fgLoCyan },
	{ "@@LIGHT_RED@@", con::fgLoRed },
	{ "@@LIGHT_MAGENTA@@", con::fgLoMagenta },
	{ "@@YELLOW@@", con::fgHiYellow },
	{ "@@WHITE@@", con::fgHiWhite },
};

const char* str;

int rbx::implement::console::rconsoleprint(lua_State* R) {
	luaL_checkstring(R, 1);

	str = lua_tostring(R, 1);

	if (color.contains(str)) {

		auto color_setter = [](std::ostream& os) -> std::ostream& {
			os.flush();
			SetColor(color[str], con::bgMask);
			return os;
		};

		std::cout << color_setter; // this should work i guess
		return 0;
	}
	std::cout << str;
	return 0;
}

int rbx::implement::console::rconsoleclear(lua_State* R) {
	// lets uh not go over-board with this!

	system("cls");

	// unless UWP blocks that i guess
	return 0;
}
int rbx::implement::console::rconsoleinfo(lua_State* R) {
	luaL_checkstring(R, 1);

	auto str = lua_tostring(R, 1);

	auto color_setter = [](std::ostream& os) -> std::ostream& {
		os.flush();
		SetColor(con::fgHiBlue, con::bgMask);
		return os;
	};

	std::cout << color_setter << "INFO: ";
	std::cout << str;

	return 0;
}
int rbx::implement::console::rconsoleerr(lua_State* R) {
	luaL_checkstring(R, 1);

	auto str = lua_tostring(R, 1);

	auto color_setter = [](std::ostream& os) -> std::ostream& {
		os.flush();
		SetColor(con::fgHiRed, con::bgMask);
		return os;
	};

	std::cout << color_setter << "ERROR: ";
	std::cout << str;

	return 0;
}
int rbx::implement::console::rconsolewarn(lua_State* R) {
	luaL_checkstring(R, 1);

	auto str = lua_tostring(R, 1);

	auto color_setter = [](std::ostream& os) -> std::ostream& {
		os.flush();
		SetColor(con::fgHiYellow, con::bgMask);
		return os;
	};

	std::cout << color_setter << "WARNING: ";
	std::cout << str;

	return 0;
}

int rbx::implement::console::rconsoletitle(lua_State* R) {
	luaL_checkstring(R, 1);

	auto str = lua_tostring(R, 1);
	SetConsoleTitleA(str);
	return 0;
}


int rbx::implement::console::init(lua_State* R) {
	return 0;
}
