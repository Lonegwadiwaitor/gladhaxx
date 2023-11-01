#pragma once

#include "../../src/pch.hpp"

namespace con {
	extern std::ostream& fg_magenta(std::ostream& os);
	extern std::ostream& fg_green(std::ostream& os);
	extern std::ostream& fg_none(std::ostream& os);
    extern WORD bgMask;
    extern WORD fgBlack;
    extern WORD fgLoRed;
    extern WORD fgLoGreen;
    extern WORD fgLoBlue;
    extern WORD fgLoCyan;
    extern WORD fgLoMagenta;
    extern WORD fgLoYellow;
    extern WORD fgLoWhite;
    extern WORD fgNone;
    extern WORD fgGray;
    extern WORD fgHiWhite;
    extern WORD fgHiBlue;
    extern WORD fgHiGreen;
    extern WORD fgHiRed;
    extern WORD fgHiCyan;
    extern WORD fgHiMagenta;
    extern WORD fgHiYellow;
    extern WORD bgBlack;
    extern WORD bgLoRed;
    extern WORD bgLoGreen;
    extern WORD bgLoBlue;
    extern WORD bgLoCyan;
    extern WORD bgLoMagenta;
    extern WORD bgLoYellow;
    extern WORD bgLoWhite;
    extern WORD bgGray;
    extern WORD bgHiWhite;
    extern WORD bgHiBlue;
    extern WORD bgHiGreen;
    extern WORD bgHiRed;
    extern WORD bgHiCyan;
    extern WORD bgHiMagenta;
    extern WORD bgHiYellow;
    extern HANDLE hCon;
    extern DWORD                       cCharsWritten;
    extern CONSOLE_SCREEN_BUFFER_INFO  csbi;
    extern DWORD                       dwConSize;
}

namespace sys {
	enum {
		DEBUG = 1
	};

	template<typename ...args_t>
	constexpr void cout(args_t... args) {
		if constexpr (DEBUG == 1) {
			std::cout << "[" << con::fg_magenta << 'S' << con::fg_green << 'M' << con::fg_none << ']' << " : ";
			(std::cout << ... << args);
			std::cout << con::fg_none;
		}
	}
}
