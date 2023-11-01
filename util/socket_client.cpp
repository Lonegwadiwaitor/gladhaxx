#include "../src/pch.hpp"
#include "socket_client.hpp"

static ws::async_client ws_client{};
static ws::connection_hdl ws_connection{};

ws::async_client& util::sockets::get_client() {
	return ws_client;
}

ws::connection_hdl util::sockets::get_connection() {
	return ws_connection;
}

void util::sockets::set_connection(ws::connection_hdl hdl) {
	ws_connection = hdl;
}
