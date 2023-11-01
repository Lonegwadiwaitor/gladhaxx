#include "../../src/pch.hpp"
#include "script.hpp"

#include "lapi.h"
#include "lobject.h"
#include "lstate.h"
#include "lualib.h"
#include "lvm.h"
#include "Luau/Bytecode.h"
#include "Luau/Common.h"
#include "Luau/Compiler.h"

struct shared_string
{
	BYTE pad[16];
	std::string value;
};

class LocalScript
{
public:
	char pad_0000[12]; //0x0000
	int32_t ClassDescriptor; //0x000C
	char pad_0010[28]; //0x0010
	char* Name; //0x002C
	char pad_0030[8]; //0x0030
	void* Child; //0x0038
	char pad_003C[252]; //0x003C
    std::string* ProtectedSource; //0x0138
	char pad_013C[4]; //0x013C
    std::string* protected_bytecode; //0x0140
	char pad_0144[8]; //0x0144
	char* script_hash; //0x014C
}; //Size: 0x0150
class ModuleScript
{
public:
	char pad_0000[12]; //0x0000
	void* ClassDescriptor; //0x000C
	char pad_0010[28]; //0x0010
	char* Name; //0x002C
	char pad_0030[8]; //0x0030
	void* Child; //0x0038
	char pad_003C[224]; //0x003C
	std::string* ProtectedSource; //0x011C
	char pad_0120[4]; //0x0120
	std::string* protected_bytecode; //0x0124
	char pad_0128[8]; //0x0128
	char* script_hash; //0x0130
}; //Size: 0x0134

std::optional<std::string> decompress_bytecode(const std::string& code)
{
    std::string ss = code;

    const char* kBytecodeMagic = "RSB1";
    static constexpr std::uint32_t kBytecodeHashMultiplier = 41;
    static constexpr std::uint32_t kBytecodeHashSeed = 42;

    char hb[4];
    std::memcpy(hb, ss.data(), 4);

    for (std::size_t i = 0; i < 4; ++i)
    {
        hb[i] ^= kBytecodeMagic[i];
        hb[i] -= i * kBytecodeHashMultiplier;
    }

    for (std::size_t i = 0; i < ss.size(); ++i)
        ss[i] ^= hb[i % 4] + i * kBytecodeHashMultiplier;

    unsigned int hash;
    std::memcpy(&hash, hb, 4);

    const auto rehash = XXH32(ss.data(), ss.size(), kBytecodeHashSeed);

    if (rehash != hash)
        return std::nullopt;

    int decompressed_size;
    std::memcpy(&decompressed_size, &ss[4], 4);

    std::vector<char> decompressed(decompressed_size);

    ZSTD_decompress(decompressed.data(), decompressed_size, ss.data() + 8, ss.size() - 8);

    return std::string(decompressed.begin(), decompressed.end());
}

std::optional<std::string> get_bytecode(lua_State* R, void* udata) {
    lua_pushinstance(R, (Instance*)&udata);
    lua_getfield(R, -1, "ClassName");
    std::string class_name = lua_tostring(R, -1);
    lua_pop(R, 2);

    std::uintptr_t script = (uintptr_t)udata;

	std::optional<std::string> protected_bytecode;

	if (class_name == "ModuleScript") {
        auto compressed_bytecode = *reinterpret_cast<std::string*>(*reinterpret_cast<std::uintptr_t*>(script + 0x124) + 0x10);
        std::printf("compressed size: %i\n", compressed_bytecode.size());
        return decompress_bytecode(compressed_bytecode);;
	}

	if (class_name == "LocalScript") {
        auto compressed_bytecode = *reinterpret_cast<std::string*>(*reinterpret_cast<std::uintptr_t*>(script + 0x140) + 0x10);
        std::printf("compressed size: %i\n", compressed_bytecode.size());
        return decompress_bytecode(compressed_bytecode);;
	}

	return protected_bytecode;
}

int rbx::implement::get_script_hash(lua_State* R) {
	luaL_checktype(R, 1, LUA_TUSERDATA);

	auto udata = *(LocalScript**)lua_touserdata(R, 1);

    lua_pushinstance(R, (Instance*)&udata);
    lua_getfield(R, -1, "ClassName");
    std::string class_name = lua_tostring(R, -1);
    lua_pop(R, 2);

	if (class_name == "ModuleScript") {
		lua_pushstring(R, reinterpret_cast<ModuleScript*>(udata)->script_hash);
		return 1;
	}

	if (class_name == "LocalScript") {
		lua_pushstring(R, udata->script_hash);
		return 1;
	}

	luaL_error(R, "LocalScript/ModuleScript expected");
}

int rbx::implement::get_script_bytecode(lua_State* R) {
	luaL_checktype(R, 1, LUA_TUSERDATA);

	auto script = *static_cast<void**>(lua_touserdata(R, 1));

	auto protected_bytecode = get_bytecode(R, script);

	if (protected_bytecode.has_value()) {
		auto bytecode = protected_bytecode.value();

		lua_pushstring(R, bytecode.data());

        return 1;
	}

	luaL_error(R, "LocalScript/ModuleScript expected");
}

int rbx::implement::get_script_closure(lua_State* R) {
    luaL_checktype(R, 1, LUA_TUSERDATA);

    auto script = *static_cast<void**>(lua_touserdata(R, 1));

    auto protected_bytecode = get_bytecode(R, script);

    if (protected_bytecode.has_value()) {
        auto bytecode = protected_bytecode.value();

        std::printf("bytecode size: %i\n", bytecode.size());

        if (bytecode.empty()) {
            lua_pushnil(R);

            return 1;
        }

        if (luau_load(R, "", bytecode, 0)) {
            lua_pushnil(R);
        }

        return 1;
    }

    return 0;
}

int rbx::implement::load_string(lua_State* R) {
    size_t l;
    const char* s = luaL_checklstring(R, 1, &l);
    const char* chunkname = luaL_optstring(R, 2, ""); // taken from 5.1 lol

    auto bytecode = Luau::compile({ s, l }, { 2, 1, 0 });

    if (luau_load(R, chunkname, bytecode, 0)) {
        lua_pushnil(R);
    }

    return 1;
}

int rbx::implement::identify_executor(lua_State* R) {
    lua_pushstring(R, "SirMeme Hub");
    return 1;
}
