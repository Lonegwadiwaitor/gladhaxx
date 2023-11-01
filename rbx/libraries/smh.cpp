#include "../../src/pch.hpp"
#include "../../src/src.hpp"

#include "smh.hpp"

#include <ranges>

#include "util.hpp"

#include "../sync/rapi.hpp"

#include "../../libraries/cryptography/aes.hpp"
#include "../../libraries/cryptography/base64.hpp"
#include "../../libraries/cryptography/random.hpp"
#include "../../libraries/os/output.hpp"
#include "../../libraries/os/file.hpp"

#include "../../directx/imgui/window.hpp"

#include "../../util/socket_client.hpp"

#include "../../libraries/lz4.hpp"

#include "imgui.hpp"
#include "draw.hpp"
#include "lualib.h"
#include "Luau/Compiler.h"
#include "../runtime/async_closure.hpp"

static int printconsole(lua_State* R) {
	sys::cout(lua_tostring(R, 1), "\n");
	return 0;
}

static int reload(lua_State* R) {
	sys::cout("destroying windows\n");

	for (auto& window : ::ig::windows) {
		window.destroy([](::ig::component* component) { delete component; });
	}
	sys::cout("clearing drawing elements\n");

	for (auto& element : ::draw::elements) {
		element->destroy([](::draw::element* element) { delete element; });
	}

	sys::cout("clearing windows\n");
	ig::windows.clear();
	sys::cout("clearing drawing elements\n");
	draw::elements.clear();
	
	sys::cout("clearing callback cache\n");
	rbx::ig::clear_callback_cache();
	sys::cout("clearing component cache\n");
	rbx::ig::clear_component_cache();
	sys::cout("clearing metatable cache\n");
	rbx::ig::clear_metatable_cache();
	rbx::draw::clear_metatable_cache();
	sys::cout("clearing element cache\n");
	rbx::draw::clear_element_cache();

	sys::cout("spawning thread\n");
	std::thread([]() {
		std::this_thread::sleep_for(3s); // TODO: shitty method, do better

		main();

		ws::async_client& ws_client = util::sockets::get_client();
		ws_client.send(util::sockets::get_connection(), "reload", ws::frame::opcode::text);
	}).detach();

	return 0;
}

static int exit(lua_State* R) {
	std::exit(0);
	return 0;
}

static int import_file(lua_State* R, const char* type, const char* path) {
	//ws::async_client& ws_client = util::sockets::get_client();
	//ws_client.send(util::sockets::get_connection(), (std::string(type)) + " " /* i didnt put a space here LOL */ + path, ws::frame::opcode::text);

	// improvising until arson fixes his backend

 //   auto improvised_json = sys::read_file(sys::get_root_dir() / "json_modules" / (std::string(path) + ".bin"));
	//rbx::script_queue::set(json::parse(improvised_json));


	//sys::cout("sent \"", (std::string(type) + " " + path), "\" over WS\n");
	//sys::cout("getting ", path, '\n');

	//json formatted_message = rbx::script_queue::get();

	//sys::cout("waiting for mod\n");
	//while (formatted_message.empty() && (formatted_message = rbx::script_queue::get()).empty()) {
	//	std::this_thread::sleep_for(1ms);
	//}

	//std::string bytecode = base64::dec(formatted_message["bytecode"].get<std::string>());
	//const auto& filename = formatted_message["path"].get<std::string>();

	//if (formatted_message.contains("originalSize")) { // compressed script
	//	sys::cout("Decompressing\n");
	//	int size = formatted_message["originalSize"].get<int>();
	//	sys::cout("Size:", size, "\n");
	//	bytecode = lz4::decompress(bytecode, size);
	//	sys::cout("Got past decompress\n");
	//}

	auto text = sys::read_file(sys::get_root_dir().string() + std::string("scripts/modules/") + path + ".lua");

	const auto& bytecode = Luau::compile(text, { 1, 2, 2 });

	sys::cout("deserializing ", path, '\n');

	luau_load(R, path, bytecode, 0);

	sys::cout("done deserializing\n");
	return 1;
}

static int httprequest(lua_State* R) {
	luaL_checktype(R, 1, LUA_TTABLE);

	rbx::runtime::yielding yield(R);

		lua_getfield(R, 1, "Url");

		const auto url = std::string(luaL_checklstring(R, -1, nullptr));

		printf("Got URL: %s\n", url.c_str());

		std::string method;
		lua_getfield(R, 1, "Method");
		if (lua_type(R, -1) == LUA_TSTRING)
		{
			method = std::string(luaL_checklstring(R, -1, nullptr));
		}

		if (method.empty())
			method = "GET";

		std::ranges::transform(method, method.begin(),
		                       [](std::uint8_t c) { return std::tolower(c); });

		printf("Got Method: %s\n", method.c_str());

		lua_pop(R, 1);

		cpr::Header headers;

		lua_getfield(R, 1, "Headers");

		if (lua_type(R, -1) == LUA_TTABLE)
		{
			lua_pushnil(R);

			while (lua_next(R, -2))
			{
				auto to_header = std::string(luaL_checklstring(R, -2, nullptr));

				std::ranges::transform(to_header, to_header.begin(),
					[](std::uint8_t c) { return std::tolower(c); });

				const auto header_value = std::string(luaL_checklstring(R, -1, nullptr));
				headers.insert({ to_header, header_value });

				lua_pop(R, 1);
			}
		}

		lua_pop(R, 1);

		for (const auto&
			key : headers | std::views::keys)
		{
			auto first_header = key; // intellisense doesnt like if we use this directly within std::transform

			std::ranges::transform(first_header, first_header.begin(),
				[](std::uint8_t c) { return std::tolower(c); });
		}

		//headers.insert({ "User-Agent", "SirMeme Hub" }); // useragent support!
		headers.insert({ "User-Agent", "SirMeme Hub" });
		auto hwid = aes_128::hash_custom<CryptoPP::SHA512>(security::get_machine_guid());

		headers.insert({ "SirMeme-Hub-Hwid", hwid });
		headers.insert({ "Syn-Fingerprint", hwid });
		headers.insert({ "Syn-User-Identifier", hwid });
		headers.insert({ "SirMeme-Hub-CompatibilityMode", "enabled"});

		std::string body;
		lua_getfield(R, 1, "Body");
		if (lua_type(R, -1) == LUA_TSTRING)
		{
			const auto to_body = std::string(luaL_checklstring(R, -1, nullptr));
			body = to_body;
		}

		cpr::Cookies cookies = cpr::Cookies{};

		lua_pop(R, 1);

		return yield.execute([method, url, headers, cookies, body]() {
			cpr::Response response;

			if (!std::strcmp(method.c_str(), "get"))
				response = Get(cpr::Url{ url }, cookies, headers);
			else if (!std::strcmp(method.c_str(), "head"))
				response = Head(cpr::Url{ url }, cookies, headers);
			else if (!std::strcmp(method.c_str(), "post"))
				response = Post(cpr::Url{ url }, cpr::Body{ body }, cookies, headers);
			else if (!std::strcmp(method.c_str(), "put"))
				response = Put(cpr::Url{ url }, cpr::Body{ body }, cookies, headers);
			else if (!std::strcmp(method.c_str(), "delete"))
				response = Delete(cpr::Url{ url }, cpr::Body{ body }, cookies, headers);
			else if (!std::strcmp(method.c_str(), "options"))
				response = Options(cpr::Url{ url }, cpr::Body{ body }, cookies, headers);
			else
				throw std::exception("invalid request method");

			return [response](lua_State* R) {
				lua_newtable(R);

				lua_pushboolean(R, (response.status_code >= 200 && response.status_code < 300));
				lua_setfield(R, -2, "Success");

				lua_pushnumber(R, response.status_code);
				lua_setfield(R, -2, "StatusCode");

				//lua_pushstring(R, HttpStatus::code_reason(request_method.status_code).c_str());
				//lua_setfield(R, -2, "StatusMessage");

				lua_newtable(R);

				for (const auto& request_header : response.header)
				{
					lua_pushlstring(R, request_header.first.c_str(), request_header.first.size());
					lua_pushlstring(R, request_header.second.c_str(), request_header.second.size());

					lua_settable(R, -3);
				}

				lua_setfield(R, -2, "Headers");

				printf("Got response: %s\n", response.text.c_str());

				lua_pushlstring(R, response.text.c_str(), response.text.size());
				lua_setfield(R, -2, "Body");
				return 1;
			};
		});
}

void rbx::smh::init(lua_State* R) {
	lua_createtable(R, 0, 6);

	rbx::push_method(R, "exit", exit, 0);
	//rbx::push_method(R, "reload", reload, 0);
	//rbx::push_method(R, "import", _import, 0);
	//rbx::push_method(R, "loadgame", loadgame, 0);
	//rbx::push_method(R, "printconsole", printconsole, 0);
	rbx::push_method(R, "request", httprequest, 0);

	

	rbx::setglobal(R, "smh");
}
