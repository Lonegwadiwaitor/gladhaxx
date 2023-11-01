// This file is part of the Luau programming language and is licensed under MIT License; see LICENSE.txt for details
// This code is based on Lua 5.x implementation licensed under MIT License; see lua_LICENSE.txt for details
#pragma once

#include <emmintrin.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <xxhash.h>
#include <zstd.h>

#include "luaconf.h"

#include "../../../bypass/obfuscation.hpp"

#define r_setobj(o1, o2) (*(o1) = *(o2))

/*class lua_CFunction {
private:
    std::uintptr_t _;
public:
    lua_CFunction();

    int operator()(struct lua_State* L) {
        return reinterpret_cast<int(*)(struct lua_State*)>(_)(L);
    }

    auto operator=() {

    }
};*/

typedef int (*lua_CFunction)(struct lua_State* L);
inline std::vector<Closure*> our_closures;
inline std::unordered_map<Closure*, lua_CFunction> closure_map;
inline std::unordered_map<Closure*, Closure*> newcclosure_map;
inline std::unordered_map<lua_State*, bool> state_map;


/* option for multiple returns in `lua_pcall' and `lua_call' */
#define LUA_MULTRET (-1)

/*
** pseudo-indices
*/
#define LUA_REGISTRYINDEX (-10000)
#define LUA_ENVIRONINDEX (-10001)
#define LUA_GLOBALSINDEX (-10002)
#define lua_upvalueindex(i) (LUA_GLOBALSINDEX - (i))
#define lua_ispseudo(i) ((i) <= LUA_REGISTRYINDEX)

/* thread status; 0 is OK */
enum lua_Status
{
    LUA_OK = 0,
    LUA_YIELD,
    LUA_ERRRUN,
    LUA_ERRSYNTAX,
    LUA_ERRMEM,
    LUA_ERRERR,
    LUA_BREAK, /* yielded for a debug breakpoint */
};

enum lua_CoStatus
{
    LUA_CORUN = 0, // running
    LUA_COSUS,     // suspended
    LUA_CONOR,     // 'normal' (it resumed another coroutine)
    LUA_COFIN,     // finished
    LUA_COERR,     // finished with error
};

typedef int (*lua_Continuation)(lua_State* L, int status);

/*
** prototype for memory-allocation functions
*/

typedef void* (*lua_Alloc)(void* ud, void* ptr, size_t osize, size_t nsize);

/* non-return type */
#define l_noret void LUA_NORETURN

/*
** basic types
*/
// version-d3bea54eff824cef
/*#define VM_SHUFFLE3(sep, a0, a1, a2) a2 sep a0 sep a1
#define VM_SHUFFLE4(sep, a0, a1, a2, a3) a0 sep a1 sep a3 sep a2
#define VM_SHUFFLE7(sep, a0, a1, a2, a3, a4, a5, a6) a6 sep a2 sep a5 sep a3 sep a1 sep a0 sep a4
#define VM_SHUFFLE6(sep, a0, a1, a2, a3, a4, a5) a2 sep a0 sep a1 sep a4 sep a3 sep a5
#define VM_SHUFFLE5(sep, a0, a1, a2, a3, a4) a4 sep a2 sep a0 sep a3 sep a1
#define VM_SHUFFLE8(sep, a0, a1, a2, a3, a4, a5, a6, a7) a4 sep a0 sep a7 sep a2 sep a3 sep a1 sep a6 sep a5*/
// version-97a663e03d314b4c
/*#define VM_SHUFFLE3(sep, a0, a1, a2) a0 sep a1 sep a2
#define VM_SHUFFLE4(sep, a0, a1, a2, a3) a2 sep a0 sep a1 sep a3
#define VM_SHUFFLE7(sep, a0, a1, a2, a3, a4, a5, a6) a0 sep a1 sep a6 sep a5 sep a3 sep a4 sep a2
#define VM_SHUFFLE6(sep, a0, a1, a2, a3, a4, a5) a1 sep a5 sep a3 sep a0 sep a4 sep a2
#define VM_SHUFFLE5(sep, a0, a1, a2, a3, a4) a0 sep a1 sep a4 sep a3 sep a2
#define VM_SHUFFLE8(sep, a0, a1, a2, a3, a4, a5, a6, a7) a5 sep a1 sep a0 sep a6 sep a2 sep a7 sep a3 sep a4*/
// version-c9e77bcac5104752
/*#define VM_SHUFFLE3(sep, a0, a1, a2) a0 sep a1 sep a2
#define VM_SHUFFLE4(sep, a0, a1, a2, a3) a2 sep a0 sep a1 sep a3
#define VM_SHUFFLE7(sep, a0, a1, a2, a3, a4, a5, a6) a0 sep a1 sep a6 sep a5 sep a3 sep a4 sep a2
#define VM_SHUFFLE6(sep, a0, a1, a2, a3, a4, a5) a1 sep a5 sep a3 sep a0 sep a4 sep a2
#define VM_SHUFFLE5(sep, a0, a1, a2, a3, a4) a0 sep a1 sep a4 sep a3 sep a2
#define VM_SHUFFLE8(sep, a0, a1, a2, a3, a4, a5, a6, a7) a5 sep a1 sep a0 sep a6 sep a2 sep a7 sep a3 sep a4*/
// version-5e79a2caa8534595
/*#define VM_SHUFFLE3(sep, a0, a1, a2) a1 sep a0 sep a2
#define VM_SHUFFLE4(sep, a0, a1, a2, a3) a1 sep a3 sep a2 sep a0
#define VM_SHUFFLE5(sep, a0, a1, a2, a3, a4) a1 sep a3 sep a2 sep a0 sep a4
#define VM_SHUFFLE6(sep, a0, a1, a2, a3, a4, a5) a0 sep a2 sep a1 sep a5 sep a4 sep a3
#define VM_SHUFFLE7(sep, a0, a1, a2, a3, a4, a5, a6) a5 sep a1 sep a6 sep a2 sep a0 sep a3 sep a4
#define VM_SHUFFLE8(sep, a0, a1, a2, a3, a4, a5, a6, a7) a5 sep a3 sep a0 sep a1 sep a7 sep a4 sep a6 sep a2*/
// version-9045f70ea522489c
/*#define VM_SHUFFLE3(sep, a0, a1, a2) a2 sep a0 sep a1
#define VM_SHUFFLE4(sep, a0, a1, a2, a3) a2 sep a3 sep a1 sep a0
#define VM_SHUFFLE5(sep, a0, a1, a2, a3, a4) a4 sep a1 sep a3 sep a2 sep a0
#define VM_SHUFFLE6(sep, a0, a1, a2, a3, a4, a5) a1 sep a5 sep a3 sep a0 sep a4 sep a2
#define VM_SHUFFLE7(sep, a0, a1, a2, a3, a4, a5, a6) a6 sep a4 sep a5 sep a2 sep a0 sep a1 sep a3
#define VM_SHUFFLE8(sep, a0, a1, a2, a3, a4, a5, a6, a7) a5 sep a2 sep a6 sep a0 sep a4 sep a3 sep a1 sep a7*/
// version-9a6f7467af154650
/*#define VM_SHUFFLE3(sep, a0, a1, a2) a2 sep a1 sep a0
#define VM_SHUFFLE4(sep, a0, a1, a2, a3) a2 sep a3 sep a1 sep a0
#define VM_SHUFFLE7(sep, a0, a1, a2, a3, a4, a5, a6) a3 sep a0 sep a2 sep a4 sep a6 sep a1 sep a5
#define VM_SHUFFLE6(sep, a0, a1, a2, a3, a4, a5) a4 sep a0 sep a5 sep a2 sep a3 sep a1
#define VM_SHUFFLE5(sep, a0, a1, a2, a3, a4) a2 sep a4 sep a3 sep a0 sep a1
#define VM_SHUFFLE8(sep, a0, a1, a2, a3, a4, a5, a6, a7) a4 sep a3 sep a7 sep a2 sep a1 sep a0 sep a6 sep a5*/
//// version-36caca58a17a48f0
//#define VM_SHUFFLE3(sep, a0, a1, a2) a1 sep a0 sep a2
//#define VM_SHUFFLE4(sep, a0, a1, a2, a3) a0 sep a2 sep a1 sep a3
//#define VM_SHUFFLE7(sep, a0, a1, a2, a3, a4, a5, a6) a5 sep a0 sep a1 sep a4 sep a2 sep a6 sep a3
//#define VM_SHUFFLE6(sep, a0, a1, a2, a3, a4, a5) a0 sep a2 sep a1 sep a5 sep a4 sep a3
//#define VM_SHUFFLE5(sep, a0, a1, a2, a3, a4) a0 sep a1 sep a4 sep a3 sep a2
//#define VM_SHUFFLE8(sep, a0, a1, a2, a3, a4, a5, a6, a7) a5 sep a3 sep a0 sep a1 sep a7 sep a4 sep a6 sep a2
// version-b730954edd9d452e
//#define VM_SHUFFLE3(sep, a0, a1, a2) a0 sep a2 sep a1
//#define VM_SHUFFLE4(sep, a0, a1, a2, a3) a0 sep a2 sep a1 sep a3
//#define VM_SHUFFLE7(sep, a0, a1, a2, a3, a4, a5, a6) a5 sep a0 sep a3 sep a1 sep a4 sep a6 sep a2
//#define VM_SHUFFLE6(sep, a0, a1, a2, a3, a4, a5) a2 sep a5 sep a3 sep a1 sep a0 sep a4
//#define VM_SHUFFLE5(sep, a0, a1, a2, a3, a4) a2 sep a4 sep a3 sep a0 sep a1
//#define VM_SHUFFLE8(sep, a0, a1, a2, a3, a4, a5, a6, a7) a5 sep a1 sep a0 sep a6 sep a2 sep a7 sep a3 sep a4
// version-4f8d630c304246d9
//#define VM_SHUFFLE3(sep, a0, a1, a2) a1 sep a0 sep a2
//#define VM_SHUFFLE4(sep, a0, a1, a2, a3) a1 sep a3 sep a2 sep a0
//#define VM_SHUFFLE5(sep, a0, a1, a2, a3, a4) a4 sep a2 sep a1 sep a0 sep a3
//#define VM_SHUFFLE6(sep, a0, a1, a2, a3, a4, a5) a1 sep a0 sep a3 sep a5 sep a4 sep a2
//#define VM_SHUFFLE7(sep, a0, a1, a2, a3, a4, a5, a6) a5 sep a0 sep a1 sep a4 sep a2 sep a6 sep a3
//#define VM_SHUFFLE8(sep, a0, a1, a2, a3, a4, a5, a6, a7) a0 sep a3 sep a4 sep a2 sep a7 sep a1 sep a6 sep a5
// version-a10a6fc51c06421b
//#define VM_SHUFFLE3(sep, a0, a1, a2) a0 sep a2 sep a1
//#define VM_SHUFFLE4(sep, a0, a1, a2, a3) a1 sep a3 sep a2 sep a0
//#define VM_SHUFFLE5(sep, a0, a1, a2, a3, a4) a4 sep a2 sep a0 sep a3 sep a1
//#define VM_SHUFFLE6(sep, a0, a1, a2, a3, a4, a5) a4 sep a0 sep a5 sep a2 sep a3 sep a1
//#define VM_SHUFFLE7(sep, a0, a1, a2, a3, a4, a5, a6) a0 sep a1 sep a6 sep a5 sep a3 sep a4 sep a2
// version-cdc9cb5aa0dc4432
//#define VM_SHUFFLE3(sep, a0, a1, a2) a1 sep a0 sep a2
//#define VM_SHUFFLE4(sep, a0, a1, a2, a3) a1 sep a2 sep a0 sep a3
//#define VM_SHUFFLE5(sep, a0, a1, a2, a3, a4) a2 sep a4 sep a3 sep a0 sep a1
//#define VM_SHUFFLE6(sep, a0, a1, a2, a3, a4, a5) a1 sep a5 sep a0 sep a4 sep a3 sep a2
//#define VM_SHUFFLE7(sep, a0, a1, a2, a3, a4, a5, a6) a0 sep a1 sep a6 sep a5 sep a3 sep a4 sep a2
//#define VM_SHUFFLE8(sep, a0, a1, a2, a3, a4, a5, a6, a7) a4 sep a0 sep a7 sep a2 sep a3 sep a1 sep a6 sep a5
// version-a5b3a0b177fa44c5
//#define VM_SHUFFLE3(sep, a0, a1, a2) a0 sep a2 sep a1
//#define VM_SHUFFLE4(sep, a0, a1, a2, a3) a2 sep a1 sep a0 sep a3
//#define VM_SHUFFLE5(sep, a0, a1, a2, a3, a4) a4 sep a1 sep a3 sep a2 sep a0
//#define VM_SHUFFLE6(sep, a0, a1, a2, a3, a4, a5) a5 sep a4 sep a0 sep a1 sep a2 sep a3
//#define VM_SHUFFLE7(sep, a0, a1, a2, a3, a4, a5, a6) a3 sep a2 sep a6 sep a1 sep a5 sep a4 sep a0
//#define VM_SHUFFLE8(sep, a0, a1, a2, a3, a4, a5, a6, a7) a5 sep a1 sep a0 sep a6 sep a2 sep a7 sep a3 sep a4
// UWP 2.582.400.0
//#define VM_SHUFFLE3(sep, a0, a1, a2) a0 sep a2 sep a1
//#define VM_SHUFFLE4(sep, a0, a1, a2, a3) a2 sep a3 sep a1 sep a0
//#define VM_SHUFFLE5(sep, a0, a1, a2, a3, a4) a0 sep a4 sep a1 sep a2 sep a3
//#define VM_SHUFFLE6(sep, a0, a1, a2, a3, a4, a5) a1 sep a5 sep a3 sep a0 sep a4 sep a2
//#define VM_SHUFFLE7(sep, a0, a1, a2, a3, a4, a5, a6) a0 sep a2 sep a3 sep a1 sep a6 sep a5 sep a4
//#define VM_SHUFFLE8(sep, a0, a1, a2, a3, a4, a5, a6, a7) a5 sep a3 sep a0 sep a1 sep a7 sep a4 sep a6 sep a2
//#define VM_SHUFFLE9(sep, a0, a1, a2, a3, a4, a5, a6, a7, a8) a4 sep a0 sep a6 sep a2 sep a3 sep a8 sep a1 sep a5 sep a7
// UWP 2.586.0
//#define VM_SHUFFLE3(sep, a0, a1, a2) a2 sep a1 sep a0
//#define VM_SHUFFLE4(sep, a0, a1, a2, a3) a3 sep a2 sep a0 sep a1
//#define VM_SHUFFLE7(sep, a0, a1, a2, a3, a4, a5, a6) a3 sep a4 sep a0 sep a6 sep a1 sep a2 sep a5
//#define VM_SHUFFLE5(sep, a0, a1, a2, a3, a4) a4 sep a1 sep a3 sep a2 sep a0
//#define VM_SHUFFLE6(sep, a0, a1, a2, a3, a4, a5) a5 sep a4 sep a2 sep a1 sep a0 sep a3
//#define VM_SHUFFLE8(sep, a0, a1, a2, a3, a4, a5, a6, a7) a0 sep a6 sep a2 sep a7 sep a4 sep a5 sep a3 sep a1
//#define VM_SHUFFLE9(sep, a0, a1, a2, a3, a4, a5, a6, a7, a8) a3 sep a6 sep a7 sep a5 sep a0 sep a8 sep a4 sep a2 sep a1

struct ret {
    rbx::bypass::obfuscation obfuscation;
    rbx::bypass::direction direction;
};

template<rbx::bypass::obfuscation o_obf, rbx::bypass::direction o_dir>
FORCEINLINE static constexpr ret get_opposite()
{
    using namespace rbx::bypass;

    rbx::bypass::obfuscation obf = {};
    rbx::bypass::direction dir = {};

    if constexpr (o_obf == ADD) {
        obf = SUB;
    }
    if constexpr (o_obf == SUB) {
        obf = ADD;
    }
    if constexpr (o_obf == XOR) {
        obf = XOR;
    }

    if constexpr (o_dir == LEFT) {
        dir = RIGHT;
    }

    if constexpr (o_dir == RIGHT) {
        dir = LEFT;
    }

    if constexpr (o_obf == SUB && o_dir == RIGHT) {
        obf = SUB;
        dir = RIGHT;
    }

    if constexpr (o_obf == XOR)
        dir = o_dir;

    return { obf, dir };
}

template <typename T, rbx::bypass::obfuscation deobfuscation, rbx::bypass::direction direction> class obfuscated_field
{
public:
    constexpr operator const T() const
    {
        return rbx::bypass::deobfuscate<T, deobfuscation, direction>(reinterpret_cast<uintptr_t>(this), 0);
    }

    constexpr obfuscated_field& operator=(const T& value)
    {
        static constexpr auto opposite = get_opposite<deobfuscation, direction>();

        storage = rbx::bypass::obfuscate<T, opposite.obfuscation, opposite.direction>(reinterpret_cast<uintptr_t>(this), 0, (uintptr_t)value);

        return *this;
    }

    constexpr T operator->() const
    {
        return operator const T();
    }

private:
    T storage;
};

LUAI_FUNC void __fastcall luau_execute(lua_State* L);

#define VM_SHUFFLE3(sep, a0, a1, a2) a0 sep a2 sep a1
#define VM_SHUFFLE4(sep, a0, a1, a2, a3) a2 sep a1 sep a0 sep a3
#define VM_SHUFFLE7(sep, a0, a1, a2, a3, a4, a5, a6) a0 sep a2 sep a3 sep a1 sep a6 sep a5 sep a4
#define VM_SHUFFLE5(sep, a0, a1, a2, a3, a4) a2 sep a4 sep a3 sep a0 sep a1
#define VM_SHUFFLE6(sep, a0, a1, a2, a3, a4, a5) a1 sep a3 sep a2 sep a0 sep a4 sep a5
// SHUFFLE 8 NOT USED
#define VM_SHUFFLE9(sep, a0, a1, a2, a3, a4, a5, a6, a7, a8) a6 sep a3 sep a5 sep a8 sep a0 sep a7 sep a1 sep a4 sep a2 

//static double number_ = *reinterpret_cast<double*>(mem::rebase(0x3D30B30, 0x400000)); // done
//
//class RBXObfuscatedDouble // I want this to behave exactly like Roblox's
//{
//    double storage;
//public:
//    FORCEINLINE operator const double() const {
//        __m128d xmmKey = _mm_load_pd(&number_);
//        __m128d xmmData = _mm_load_sd(&storage);
//        __m128d xmmResult = _mm__pd(xmmData, xmmKey);
//        return _mm_cvtsd_f64(xmmResult);
//    }
//
//    FORCEINLINE void operator=(const double& value)
//    {
//        __m128d xmmKey = _mm_load_pd(&number_);
//        __m128d xmmData = _mm_load_sd(&value);
//        __m128d xmmResult = _mm__pd(xmmData, xmmKey);
//        storage = _mm_cvtsd_f64(xmmResult);
//    }
//};

//typedef union
//{
//    union GCObject* gc;
//    void* p;
//    RBXObfuscatedDouble n;
//    int b;
//    float v[2]; // v[0], v[1] live here; v[2] lives in TValue::extra
//} Value;
//
//typedef struct lua_TValue
//{
//    Value value;
//    int extra[LUA_EXTRA_SIZE];
//    int tt;
//} TValue;

// TODO: UPDATE FOR RELEASE

#define LUA_TNONE (-1)

// NOTE: Everything we need to update is right here.
// The main thing we need are these shuffles, grab all 9 and you have now know every shuffled offset in Roblox's modified Luau environment.

#define LUAVM_SHUFFLE_COMMA ,



typedef union
{
    union GCObject* gc;
    void* p;
    double n;
    int b;
    float v[2]; // v[0], v[1] live here; v[2] lives in TValue::extra
} Value;

/*
** Tagged Values
*/

typedef struct lua_TValue
{
    Value value;
    int extra[LUA_EXTRA_SIZE];
    int tt;
} TValue;



//inline auto deserializer = mem::rebase(0x18de190, 0x400000); // INLINED IN version-f59870803f534575
//inline auto fcall_address = mem::rebase(0x18dc830, 0x400000); // INLINED IN version-f59870803f534575
//inline auto auxgetinfo_address = mem::rebase(0x18EC4B0, 0x400000); // INLINED IN version-f59870803f534575

//inline auto callcheck_address = mem::rebase(0x1b356e5, 0x400000); // Doesn't exist in UWP
//inline auto pcall_run = mem::rebase(0x197beb0, 0x400000); // No need - we just call the VM directly

inline auto spoof_address = mem::rebase(0x42c1ab, 0x400000); // Done
const TValue luaO_nilobject_ = *reinterpret_cast<TValue*>(mem::rebase(0x2D05080, 0x400000)); // Done

inline auto fireclickdetector_address = mem::rebase(0x168A0B0, 0x400000); // Done
inline auto firetouchinterest_address = mem::rebase(0x1A3F5F0, 0x400000); // Done
inline auto fireproximityprompt_address = mem::rebase(0x15E3E50, 0x400000); // Done

inline auto rawrunprotected_address = mem::rebase(0x1c24470, 0x400000); // Done
inline auto luac_step_address = mem::rebase(0x1C650F0, 0x400000); // Done

inline auto dummynode_address = mem::rebase(0x2d05060, 0x400000); // Done

inline auto scheduler_ref = mem::rebase(0x383EF74, 0x400000); // Done

inline auto luau_load_address = mem::rebase(0x980FE0, 0x400000); // Done

inline auto luau_execute_address = mem::rebase(0x1c67f80, 0x400000); // Done


static auto push_inst = mem::rebase(0x7EBBF0, 0x400000); // Done

#define setsvalue2n setsvalue

//inline auto objtypenamestr = mem::rebase(0x1A09660, 0x400000); // this version has specific fixes for roblox, done

typedef obfuscated_field<lua_CFunction, rbx::bypass::ADD, rbx::bypass::RIGHT> CLOSURE_FUNC_OBF; // Done
typedef obfuscated_field<lua_Continuation, rbx::bypass::SUB, rbx::bypass::RIGHT> CLOSURE_CONT_OBF; // Done
typedef obfuscated_field<const char*, rbx::bypass::SUB, rbx::bypass::LEFT> CLOSURE_DEBUGNAME_OBF; // Done
typedef obfuscated_field<struct Proto*, rbx::bypass::ADD, rbx::bypass::RIGHT> CLOSURE_PROTO_OBF; // Done

typedef obfuscated_field<struct Table*, rbx::bypass::ADD, rbx::bypass::RIGHT> TABLE_METATABLE_OBF; // Done
typedef obfuscated_field<struct LuaNode*, rbx::bypass::ADD, rbx::bypass::RIGHT> TABLE_NODE_OBF; // Done
typedef obfuscated_field<TValue*, rbx::bypass::ADD, rbx::bypass::RIGHT> TABLE_ARRAY_OBF; // Done

typedef obfuscated_field<unsigned int, rbx::bypass::ADD, rbx::bypass::RIGHT> TSTRING_HASH_OBF; // Done
typedef obfuscated_field<unsigned int, rbx::bypass::SUB, rbx::bypass::RIGHT> TSTRING_LEN_OBF; // Done

typedef obfuscated_field<Table*, rbx::bypass::SUB, rbx::bypass::RIGHT> UDATA_MT_OBF;

constexpr rbx::bypass::obfuscation PROTO_OBF = rbx::bypass::XOR; /* the below obfuscations are all the same */
constexpr rbx::bypass::direction PROTO_DIR = rbx::bypass::RIGHT; // same with this

typedef obfuscated_field<TValue*, PROTO_OBF, PROTO_DIR> PROTO_CONSTANT_OBF;
typedef obfuscated_field<uint32_t*, PROTO_OBF, PROTO_DIR> PROTO_CODE_OBF;
typedef obfuscated_field<struct TString*, PROTO_OBF, PROTO_DIR> PROTO_SOURCE_OBF;
typedef obfuscated_field<uint8_t*, PROTO_OBF, PROTO_DIR> PROTO_LINEINFO_OBF;
typedef obfuscated_field<Proto**, PROTO_OBF, PROTO_DIR> PROTO_PROTOS_OBF;
typedef obfuscated_field<int*, PROTO_OBF, PROTO_DIR> PROTO_ABSLINEINFO_OBF;
typedef obfuscated_field<struct LocVar*, PROTO_OBF, PROTO_DIR> PROTO_LOCVARS_OBF;
typedef obfuscated_field<TString**, PROTO_OBF, PROTO_DIR> PROTO_UPVALUES_OBF;

typedef obfuscated_field<TString*, rbx::bypass::XOR, rbx::bypass::RIGHT> PROTO_DEBUGNAME_OBF; // Done
typedef obfuscated_field<uint8_t*, rbx::bypass::XOR, rbx::bypass::RIGHT> PROTO_DEBUGINSN_OBF;

typedef obfuscated_field<TString*, rbx::bypass::ADD, rbx::bypass::RIGHT> GLOBAL_OBFUSCATED_TMNAME; // Done
typedef obfuscated_field<TString*, rbx::bypass::ADD, rbx::bypass::RIGHT> GLOBAL_OBFUSCATED_TTNAME; // Done

typedef obfuscated_field<global_State*, rbx::bypass::SUB, rbx::bypass::LEFT> LS_GLOBAL_OBFUSCATION; // Done
typedef obfuscated_field<int, rbx::bypass::SUB, rbx::bypass::LEFT> LS_STACKSIZE_OBFUSCATION; // Done

/*
 * WARNING: if you change the order of this enumeration,
 * grep "ORDER TYPE"
 */

 // clang-format off
enum lua_Type
{
    LUA_TNIL = 0,     /* must be 0 due to lua_isnoneornil */
    LUA_TBOOLEAN = 1, /* must be 1 due to l_isfalse */


    VM_SHUFFLE3(LUAVM_SHUFFLE_COMMA,
    LUA_TLIGHTUSERDATA,
    LUA_TNUMBER,
    LUA_TVECTOR),

    LUA_TSTRING = 5, /* all types above this must be value types, all types below this must be GC types - see iscollectable */


    VM_SHUFFLE4(LUAVM_SHUFFLE_COMMA,
    LUA_TTABLE,
    LUA_TFUNCTION,
    LUA_TUSERDATA,
    LUA_TTHREAD),

    /* values below this line are used in GCObject tags but may never show up in TValue type tags */
    LUA_TPROTO = 10,
    LUA_TUPVAL,
    LUA_TDEADKEY,

    /* the count of TValue type tags */
    LUA_T_COUNT = LUA_TPROTO
};
// clang-format on

/* type of numbers in Luau */
typedef double lua_Number;

// type for integer functions
typedef int lua_Integer;

// unsigned integer type
typedef unsigned lua_Unsigned;

/*
** state manipulation
*/
LUA_API lua_State* lua_newstate(lua_Alloc f, void* ud);
LUA_API void lua_close(lua_State* L);
LUA_API lua_State* lua_newthread(lua_State* L);
LUA_API lua_State* lua_mainthread(lua_State* L);
LUA_API void lua_resetthread(lua_State* L);
LUA_API int lua_isthreadreset(lua_State* L);

/*
** basic stack manipulation
*/
LUA_API int lua_absindex(lua_State* L, int idx);
LUA_API int lua_gettop(lua_State* L);
LUA_API void lua_settop(lua_State* L, int idx);
LUA_API void lua_pushvalue(lua_State* L, int idx);
LUA_API void lua_remove(lua_State* L, int idx);
LUA_API void lua_insert(lua_State* L, int idx);
LUA_API void lua_replace(lua_State* L, int idx);
LUA_API int lua_checkstack(lua_State* L, int sz);
LUA_API void lua_rawcheckstack(lua_State* L, int sz); // allows for unlimited stack frames

LUA_API void lua_xmove(lua_State* from, lua_State* to, int n);
LUA_API void lua_xpush(lua_State* from, lua_State* to, int idx);

/*
** access functions (stack -> C)
*/

LUA_API int lua_isnumber(lua_State* L, int idx);
LUA_API int lua_isstring(lua_State* L, int idx);
LUA_API int lua_iscfunction(lua_State* L, int idx);
LUA_API int lua_isLfunction(lua_State* L, int idx);
LUA_API int lua_isuserdata(lua_State* L, int idx);
LUA_API int lua_type(lua_State* L, int idx);
LUA_API const char* lua_typename(lua_State* L, int tp);

LUA_API int lua_equal(lua_State* L, int idx1, int idx2);
LUA_API int lua_rawequal(lua_State* L, int idx1, int idx2);
LUA_API int lua_lessthan(lua_State* L, int idx1, int idx2);

LUA_API double lua_tonumberx(lua_State* L, int idx, int* isnum);
LUA_API int lua_tointegerx(lua_State* L, int idx, int* isnum);
LUA_API unsigned lua_tounsignedx(lua_State* L, int idx, int* isnum);
LUA_API const float* lua_tovector(lua_State* L, int idx);
LUA_API int lua_toboolean(lua_State* L, int idx);
LUA_API const char* lua_tolstring(lua_State* L, int idx, size_t* len);
LUA_API const char* lua_tostringatom(lua_State* L, int idx, int* atom);
LUA_API const char* lua_namecallatom(lua_State* L, int* atom);
LUA_API int lua_objlen(lua_State* L, int idx);
LUA_API lua_CFunction lua_tocfunction(lua_State* L, int idx);
LUA_API void* lua_tolightuserdata(lua_State* L, int idx);
LUA_API void* lua_touserdata(lua_State* L, int idx);
LUA_API void* lua_touserdatatagged(lua_State* L, int idx, int tag);
LUA_API int lua_userdatatag(lua_State* L, int idx);
LUA_API lua_State* lua_tothread(lua_State* L, int idx);
LUA_API const void* lua_topointer(lua_State* L, int idx);

/*
** push functions (C -> stack)
*/
LUA_API void lua_pushnil(lua_State* L);
LUA_API void lua_pushnumber(lua_State* L, double n);
LUA_API void lua_pushinteger(lua_State* L, int n);
LUA_API void lua_pushunsigned(lua_State* L, unsigned n);
#if LUA_VECTOR_SIZE == 4
LUA_API void lua_pushvector(lua_State* L, float x, float y, float z, float w);
#else
LUA_API void lua_pushvector(lua_State* L, float x, float y, float z);
#endif
LUA_API void lua_pushlstring(lua_State* L, const char* s, size_t l);
LUA_API void lua_pushstring(lua_State* L, const char* s);
FORCEINLINE void lua_pushstring(lua_State* L, std::string str) {
    return lua_pushstring(L, str.data());
}
LUA_API const char* lua_pushvfstring(lua_State* L, const char* fmt, va_list argp);
LUA_API LUA_PRINTF_ATTR(2, 3) const char* lua_pushfstringL(lua_State* L, const char* fmt, ...);
LUA_API void lua_pushcclosurek(lua_State* L, lua_CFunction fn, const char* debugname, int nup, lua_Continuation cont);
LUA_API void lua_pushboolean(lua_State* L, int b);
LUA_API int lua_pushthread(lua_State* L);

LUA_API void lua_pushlightuserdata(lua_State* L, void* p);
LUA_API void* lua_newuserdatatagged(lua_State* L, size_t sz, int tag);
LUA_API void* lua_newuserdatadtor(lua_State* L, size_t sz, void (*dtor)(void*));

/*
** get functions (Lua -> stack)
*/
LUA_API int lua_gettable(lua_State* L, int idx);
LUA_API int lua_getfield(lua_State* L, int idx, const char* k);
LUA_API int lua_rawgetfield(lua_State* L, int idx, const char* k);
LUA_API int lua_rawget(lua_State* L, int idx);
LUA_API int lua_rawgeti(lua_State* L, int idx, int n);
LUA_API void lua_createtable(lua_State* L, int narr, int nrec);

LUA_API void lua_setreadonly(lua_State* L, int idx, int enabled);
LUA_API int lua_getreadonly(lua_State* L, int idx);
LUA_API void lua_setsafeenv(lua_State* L, int idx, int enabled);

LUA_API int lua_getmetatable(lua_State* L, int objindex);
LUA_API void lua_getfenv(lua_State* L, int idx);

/*
** set functions (stack -> Lua)
*/
LUA_API void lua_settable(lua_State* L, int idx);
LUA_API void lua_setfield(lua_State* L, int idx, const char* k);
LUA_API void lua_rawsetfield(lua_State* L, int idx, const char* k);
LUA_API void lua_rawset(lua_State* L, int idx);
LUA_API void lua_rawseti(lua_State* L, int idx, int n);
LUA_API int lua_setmetatable(lua_State* L, int objindex);
LUA_API int lua_setfenv(lua_State* L, int idx);

/*
** `load' and `call' functions (load and run Luau bytecode)
*/
LUA_API int meme_load(lua_State* L, const char* chunkname, const char* data, size_t size, int env);
static int __fastcall luau_load(lua_State* L, const char* chunkname, std::string bytecode, int env) {
    /*int i = reinterpret_cast<int(__fastcall*)(lua_State*, const char*, const char*, size_t, int)>(deserializer)(L, chunkname, data, size, env);
    /*int i = spoof::fastcall<int>(deserializer, L, chunkname, data, size, env);#1#
    __asm add esp, 12*/


    static const char kBytecodeMagic[] = "RSB1";
    static const unsigned int kBytecodeHashSeed = 42;
    static const unsigned int kBytecodeHashMultiplier = 41;

    int data_size = bytecode.size();
    const int max_size = ZSTD_compressBound(data_size);

    std::vector<char> compressed(max_size);
    const int compressed_size = ZSTD_compress(compressed.data(), max_size, bytecode.data(), bytecode.size(), 1);

    std::string result = kBytecodeMagic;
    result.append(reinterpret_cast<char*>(&data_size), sizeof(data_size));
    result.append(compressed.data(), compressed_size);

    const unsigned int hash = XXH32(result.data(), result.size(), kBytecodeHashSeed);

    unsigned char hb[4];
    memcpy(hb, &hash, sizeof(hash));

    for (size_t i = 0; i < result.size(); ++i)
        result[i] ^= hb[i % 4] + i * kBytecodeHashMultiplier;

    static int oebp, oesp;

    __asm {
        mov oebp, ebp
        mov oesp, esp
    }


    int res = reinterpret_cast<int(__fastcall*)(lua_State*, std::string*, const char*, int env)>(luau_load_address)(L, &result, chunkname, env);


    __asm {
        mov ebp, oebp
        mov esp, oesp
    }

    //const int res = spoof::fastcall<int>(memory::rebase(0x95d430), state, &result, chunkname, env);

    std::printf("luau_deserialize result: %i\n", res);


    return res;
}
LUA_API void lua_call(lua_State* L, int nargs, int nresults);
LUA_API int lua_pcall(lua_State* L, int nargs, int nresults, int errfunc);

/*
** coroutine functions
*/
LUA_API int lua_yield(lua_State* L, int nresults);
LUA_API int lua_break(lua_State* L);
LUA_API int lua_resume(lua_State* L, lua_State* from, int narg);
LUA_API int lua_resumeerror(lua_State* L, lua_State* from);
LUA_API int lua_status(lua_State* L);
LUA_API int lua_isyieldable(lua_State* L);
LUA_API void* lua_getthreaddata(lua_State* L);
LUA_API void lua_setthreaddata(lua_State* L, void* data);
LUA_API int lua_costatus(lua_State* L, lua_State* co);

/*
** garbage-collection function and options
*/

enum lua_GCOp
{
    // stop and resume incremental garbage collection
    LUA_GCSTOP,
    LUA_GCRESTART,

    // run a full GC cycle; not recommended for latency sensitive applications
    LUA_GCCOLLECT,

    // return the heap size in KB and the remainder in bytes
    LUA_GCCOUNT,
    LUA_GCCOUNTB,

    // return 1 if GC is active (not stopped); note that GC may not be actively collecting even if it's running
    LUA_GCISRUNNING,

    /*
    ** perform an explicit GC step, with the step size specified in KB
    **
    ** garbage collection is handled by 'assists' that perform some amount of GC work matching pace of allocation
    ** explicit GC steps allow to perform some amount of work at custom points to offset the need for GC assists
    ** note that GC might also be paused for some duration (until bytes allocated meet the threshold)
    ** if an explicit step is performed during this pause, it will trigger the start of the next collection cycle
    */
    LUA_GCSTEP,

    /*
    ** tune GC parameters G (goal), S (step multiplier) and step size (usually best left ignored)
    **
    ** garbage collection is incremental and tries to maintain the heap size to balance memory and performance overhead
    ** this overhead is determined by G (goal) which is the ratio between total heap size and the amount of live data in it
    ** G is specified in percentages; by default G=200% which means that the heap is allowed to grow to ~2x the size of live data.
    **
    ** collector tries to collect S% of allocated bytes by interrupting the application after step size bytes were allocated.
    ** when S is too small, collector may not be able to catch up and the effective goal that can be reached will be larger.
    ** S is specified in percentages; by default S=200% which means that collector will run at ~2x the pace of allocations.
    **
    ** it is recommended to set S in the interval [100 / (G - 100), 100 + 100 / (G - 100))] with a minimum value of 150%; for example:
    ** - for G=200%, S should be in the interval [150%, 200%]
    ** - for G=150%, S should be in the interval [200%, 300%]
    ** - for G=125%, S should be in the interval [400%, 500%]
    */
    LUA_GCSETGOAL,
    LUA_GCSETSTEPMUL,
    LUA_GCSETSTEPSIZE,
};

LUA_API int lua_gc(lua_State* L, int what, int data);

/*
** memory statistics
** all allocated bytes are attributed to the memory category of the running thread (0..LUA_MEMORY_CATEGORIES-1)
*/

LUA_API void lua_setmemcat(lua_State* L, int category);
LUA_API size_t lua_totalbytes(lua_State* L, int category);

/*
** miscellaneous functions
*/

LUA_API l_noret lua_error(lua_State* L);

LUA_API int lua_next(lua_State* L, int idx);
LUA_API int lua_rawiter(lua_State* L, int idx, int iter);

LUA_API void lua_concat(lua_State* L, int n);

LUA_API uintptr_t lua_encodepointer(lua_State* L, uintptr_t p);

LUA_API double lua_clock();

LUA_API void lua_setuserdatatag(lua_State* L, int idx, int tag);

typedef void (*lua_Destructor)(lua_State* L, void* userdata);

LUA_API void lua_setuserdatadtor(lua_State* L, int tag, lua_Destructor dtor);
LUA_API lua_Destructor lua_getuserdatadtor(lua_State* L, int tag);

LUA_API void lua_clonefunction(lua_State* L, int idx);

LUA_API void lua_cleartable(lua_State* L, int idx);

/*
** reference system, can be used to pin objects
*/
#define LUA_NOREF -1
#define LUA_REFNIL 0

LUA_API int lua_ref(lua_State* L, int idx);
LUA_API void lua_unref(lua_State* L, int ref);

#define lua_getref(L, ref) lua_rawgeti(L, LUA_REGISTRYINDEX, (ref))

/*
** ===============================================================
** some useful macros
** ===============================================================
*/
#define lua_tonumber(L, i) lua_tonumberx(L, i, NULL)
#define lua_tointeger(L, i) lua_tointegerx(L, i, NULL)
#define lua_tounsigned(L, i) lua_tounsignedx(L, i, NULL)

#define lua_pop(L, n) lua_settop(L, -(n)-1)

#define lua_newtable(L) lua_createtable(L, 0, 0)
#define lua_newuserdata(L, s) lua_newuserdatatagged(L, s, 0)

#define lua_strlen(L, i) lua_objlen(L, (i))

#define lua_isfunction(L, n) (lua_type(L, (n)) == LUA_TFUNCTION)
#define lua_istable(L, n) (lua_type(L, (n)) == LUA_TTABLE)
#define lua_islightuserdata(L, n) (lua_type(L, (n)) == LUA_TLIGHTUSERDATA)
#define lua_isnil(L, n) (lua_type(L, (n)) == LUA_TNIL)
#define lua_isboolean(L, n) (lua_type(L, (n)) == LUA_TBOOLEAN)
#define lua_isvector(L, n) (lua_type(L, (n)) == LUA_TVECTOR)
#define lua_isthread(L, n) (lua_type(L, (n)) == LUA_TTHREAD)
#define lua_isnone(L, n) (lua_type(L, (n)) == LUA_TNONE)
#define lua_isnoneornil(L, n) (lua_type(L, (n)) <= LUA_TNIL)

#define lua_pushliteral(L, s) lua_pushlstring(L, "" s, (sizeof(s) / sizeof(char)) - 1)
#define lua_pushcfunction(L, fn, debugname) lua_pushcclosurek(L, fn, debugname, 0, NULL)
#define lua_pushcclosure(L, fn, debugname, nup) lua_pushcclosurek(L, fn, debugname, nup, NULL)

#define lua_setglobal(L, s) lua_setfield(L, LUA_GLOBALSINDEX, (s))
#define lua_getglobal(L, s) lua_getfield(L, LUA_GLOBALSINDEX, (s))

#define lua_tostring(L, i) lua_tolstring(L, (i), NULL)

#define lua_pushfstring(L, fmt, ...) lua_pushfstringL(L, fmt, ##__VA_ARGS__)

/*
** {======================================================================
** Debug API
** =======================================================================
*/

typedef struct lua_Debug lua_Debug; // activation record

// Functions to be called by the debugger in specific events
typedef void (*lua_Hook)(lua_State* L, lua_Debug* ar);

LUA_API int lua_stackdepth(lua_State* L);
LUA_API int lua_getinfo(lua_State* L, int level, const char* what, lua_Debug* ar);
LUA_API int lua_getargument(lua_State* L, int level, int n);
LUA_API const char* lua_getlocal(lua_State* L, int level, int n);
LUA_API const char* lua_setlocal(lua_State* L, int level, int n);
LUA_API const char* lua_getupvalue(lua_State* L, int funcindex, int n);
LUA_API const char* lua_setupvalue(lua_State* L, int funcindex, int n);

LUA_API void lua_singlestep(lua_State* L, int enabled);
LUA_API int lua_breakpoint(lua_State* L, int funcindex, int line, int enabled);

typedef void (*lua_Coverage)(void* context, const char* function, int linedefined, int depth, const int* hits, size_t size);

LUA_API void lua_getcoverage(lua_State* L, int funcindex, void* context, lua_Coverage callback);

// Warning: this function is not thread-safe since it stores the result in a shared global array! Only use for debugging.
LUA_API const char* lua_debugtrace(lua_State* L);

struct lua_Debug
{
    const char* name;      // (n)
    const char* what;      // (s) `Lua', `C', `main', `tail'
    const char* source;    // (s)
    const char* short_src; // (s)
    int linedefined;       // (s)
    int currentline;       // (l)
    unsigned char nupvals; // (u) number of upvalues
    unsigned char nparams; // (a) number of parameters
    char isvararg;         // (a)
    void* userdata;        // only valid in luau_callhook

    char ssbuf[LUA_IDSIZE];
};

// }======================================================================

/* Callbacks that can be used to reconfigure behavior of the VM dynamically.
 * These are shared between all coroutines.
 *
 * Note: interrupt is safe to set from an arbitrary thread but all other callbacks
 * can only be changed when the VM is not running any code */
struct lua_Callbacks
{
    void* userdata; // arbitrary userdata pointer that is never overwritten by Luau

    void (*interrupt)(lua_State* L, int gc);  // gets called at safepoints (loop back edges, call/ret, gc) if set
    void (*panic)(lua_State* L, int errcode); // gets called when an unprotected error is raised (if longjmp is used)

    void (*userthread)(lua_State* LP, lua_State* L); // gets called when L is created (LP == parent) or destroyed (LP == NULL)
    int16_t(*useratom)(const char* s, size_t l);    // gets called when a string is created; returned atom can be retrieved via tostringatom

    void (*debugbreak)(lua_State* L, lua_Debug* ar);     // gets called when BREAK instruction is encountered
    void (*debugstep)(lua_State* L, lua_Debug* ar);      // gets called after each instruction in single step mode
    void (*debuginterrupt)(lua_State* L, lua_Debug* ar); // gets called when thread execution is interrupted by break in another thread
    void (*debugprotectederror)(lua_State* L);           // gets called when protected call results in an error
};
typedef struct lua_Callbacks lua_Callbacks;

LUA_API lua_Callbacks* lua_callbacks(lua_State* L);

/******************************************************************************
 * Copyright (c) 2019-2022 Roblox Corporation
 * Copyright (C) 1994-2008 Lua.org, PUC-Rio.  All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 ******************************************************************************/
