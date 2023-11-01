#include "../src/pch.hpp"

namespace directx {
	extern bool IsGuiOpen;
	HWND get_hwnd();

	bool get_streamer_mode();
	void set_streamer_mode(bool mode);
}
