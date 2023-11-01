#pragma once

#include <string>

namespace sys::bios {
	void init();
	std::string get_serial_id();
	std::string get_disk_id();
	std::string get_uuid();
}
