#pragma once

namespace util::sockets {
	ws::async_client& get_client();
	ws::connection_hdl get_connection();
	void set_connection(ws::connection_hdl hdl);
}
