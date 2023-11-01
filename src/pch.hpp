#pragma once

#define _SILENCE_CXX17_C_HEADER_DEPRECATION_WARNING

#include <cstdio>
#include <cstddef>
#include <stdint.h>

#include <iostream>
#include <fstream>

#include <functional>

#include <string>
#include <vector>
#include <array>
#include <deque>
#include <map>

#include <memory>
#include <bitset>

#include <thread>

#include <optional>
#include <filesystem>

#include <algorithm>

#include <random>

#include <string_view>

#include <nlohmann/json.hpp>
#include <cpr/cpr.h>
#include <MinHook.h>

#include <cryptopp/hex.h>
#include <cryptopp/filters.h>
#include <cryptopp/cryptlib.h>
#include <cryptopp/aes.h>
#include <cryptopp/osrng.h>
#include <cryptopp/hex.h>
#include <cryptopp/ccm.h>
#include <cryptopp/base64.h>
#include <cryptopp/sha3.h>
#include <cryptopp/channels.h>

#include <websocketpp/config/asio_client.hpp>
#include <websocketpp/client.hpp>

#include <Windows.h>
#include <winnt.h>
#include <intrin.h>
#include <psapi.h>
#include <WbemIdl.h>
#include <TlHelp32.h>
#include <winternl.h>
#include <winhttp.h>

#include <d3d11.h>
#include <dxgi.h>

#include "../security/spoof.hpp"

#include "../security/themida/ThemidaSDK.h"
#include "../libraries/cryptography/XORS.h"
#include "../libraries/cryptography/constexpr_rand.hpp"

static FILE* safe_handle_stream; /* our std handle */

namespace filesystem = std::filesystem;

using nlohmann::json;

namespace ws {
	using namespace websocketpp;
	using async_client = websocketpp::client<websocketpp::config::asio_tls_client>;
	using message_ptr = websocketpp::config::asio_tls_client::message_type::ptr;
}

//namespace ws = websocketpp;

using namespace std::string_literals;
using namespace std::chrono_literals;
