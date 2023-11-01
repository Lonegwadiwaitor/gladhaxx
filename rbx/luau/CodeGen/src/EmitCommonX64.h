// This file is part of the Luau programming language and is licensed under MIT License; see LICENSE.txt for details
#pragma once

#include "Luau/AssemblyBuilderX64.h"

#include "EmitCommon.h"

#include "lobject.h"
#include "ltm.h"

// MS x64 ABI reminder:
// Arguments: rcx, rdx, r8, r9 ('overlapped' with xmm0-xmm3)
// Return: rax, xmm0
// Nonvolatile: r12-r15, rdi, rsi, rbx, rbp
// SIMD: only xmm6-xmm15 are non-volatile, all ymm upper parts are volatile

// AMD64 ABI reminder:
// Arguments: rdi, rsi, rdx, rcx, r8, r9 (xmm0-xmm7)
// Return: rax, rdx, xmm0, xmm1
// Nonvolatile: r12-r15, rbx, rbp
// SIMD: all volatile

namespace Luau
{
namespace CodeGen
{

enum class IrCondition : uint8_t;
struct NativeState;
struct IrOp;

namespace X64
{

struct IrRegAllocX64;

constexpr uint32_t kFunctionAlignment = 32;

// Data that is very common to access is placed in non-volatile registers
constexpr RegisterX64 rState = r15;         // lua_State* L
constexpr RegisterX64 rBase = r14;          // StkId base
constexpr RegisterX64 rNativeContext = r13; // NativeContext* context
constexpr RegisterX64 rConstants = r12;     // TValue* k

// Native code is as stackless as the interpreter, so we can place some data on the stack once and have it accessible at any point
// See CodeGenX64.cpp for layout
constexpr unsigned kStackSize = 32 + 16;               // 4 home locations for registers, 16 bytes for additional function call arguments
constexpr unsigned kSpillSlots = 4;                    // locations for register allocator to spill data into
constexpr unsigned kLocalsSize = 24 + 8 * kSpillSlots; // 3 extra slots for our custom locals (also aligns the stack to 16 byte boundary)

constexpr OperandX64 sClosure = qword[rsp + kStackSize + 0]; // Closure* cl
constexpr OperandX64 sCode = qword[rsp + kStackSize + 8];    // Instruction* code
constexpr OperandX64 sTemporarySlot = addr[rsp + kStackSize + 16];
constexpr OperandX64 sSpillArea = addr[rsp + kStackSize + 24];

inline OperandX64 luauReg(int ri)
{
    return xmmword[rBase + ri * sizeof(TValue)];
}

inline OperandX64 luauRegAddress(int ri)
{
    return addr[rBase + ri * sizeof(TValue)];
}

inline OperandX64 luauRegValue(int ri)
{
    return qword[rBase + ri * sizeof(TValue) + offsetof(TValue, value)];
}

inline OperandX64 luauRegTag(int ri)
{
    return dword[rBase + ri * sizeof(TValue) + offsetof(TValue, tt)];
}

inline OperandX64 luauRegValueInt(int ri)
{
    return dword[rBase + ri * sizeof(TValue) + offsetof(TValue, value)];
}

inline OperandX64 luauRegValueVector(int ri, int index)
{
    return dword[rBase + ri * sizeof(TValue) + offsetof(TValue, value) + (sizeof(float) * index)];
}

inline OperandX64 luauConstant(int ki)
{
    return xmmword[rConstants + ki * sizeof(TValue)];
}

inline OperandX64 luauConstantAddress(int ki)
{
    return addr[rConstants + ki * sizeof(TValue)];
}

inline OperandX64 luauConstantTag(int ki)
{
    return dword[rConstants + ki * sizeof(TValue) + offsetof(TValue, tt)];
}

inline OperandX64 luauConstantValue(int ki)
{
    return qword[rConstants + ki * sizeof(TValue) + offsetof(TValue, value)];
}

inline OperandX64 luauNodeKeyValue(RegisterX64 node)
{
    return qword[node + offsetof(LuaNode, key) + offsetof(TKey, value)];
}

// Note: tag has dirty upper bits
inline OperandX64 luauNodeKeyTag(RegisterX64 node)
{
    return dword[node + offsetof(LuaNode, key) + kOffsetOfTKeyTagNext];
}

inline OperandX64 luauNodeValue(RegisterX64 node)
{
    return xmmword[node + offsetof(LuaNode, val)];
}

inline void setLuauReg(AssemblyBuilderX64& build, RegisterX64 tmp, int ri, OperandX64 op)
{
    LUAU_ASSERT(op.cat == CategoryX64::mem);

    build.vmovups(tmp, op);
    build.vmovups(luauReg(ri), tmp);
}

inline void jumpIfTagIs(AssemblyBuilderX64& build, int ri, lua_Type tag, Label& label)
{
    build.cmp(luauRegTag(ri), tag);
    build.jcc(ConditionX64::Equal, label);
}

inline void jumpIfTagIsNot(AssemblyBuilderX64& build, int ri, lua_Type tag, Label& label)
{
    build.cmp(luauRegTag(ri), tag);
    build.jcc(ConditionX64::NotEqual, label);
}

// Note: fallthrough label should be placed after this condition
inline void jumpIfFalsy(AssemblyBuilderX64& build, int ri, Label& target, Label& fallthrough)
{
    jumpIfTagIs(build, ri, LUA_TNIL, target);             // false if nil
    jumpIfTagIsNot(build, ri, LUA_TBOOLEAN, fallthrough); // true if not nil or boolean

    build.cmp(luauRegValueInt(ri), 0);
    build.jcc(ConditionX64::Equal, target); // true if boolean value is 'true'
}

// Note: fallthrough label should be placed after this condition
inline void jumpIfTruthy(AssemblyBuilderX64& build, int ri, Label& target, Label& fallthrough)
{
    jumpIfTagIs(build, ri, LUA_TNIL, fallthrough);   // false if nil
    jumpIfTagIsNot(build, ri, LUA_TBOOLEAN, target); // true if not nil or boolean

    build.cmp(luauRegValueInt(ri), 0);
    build.jcc(ConditionX64::NotEqual, target); // true if boolean value is 'true'
}

void jumpOnNumberCmp(AssemblyBuilderX64& build, RegisterX64 tmp, OperandX64 lhs, OperandX64 rhs, IrCondition cond, Label& label);
void jumpOnAnyCmpFallback(IrRegAllocX64& regs, AssemblyBuilderX64& build, int ra, int rb, IrCondition cond, Label& label);

void getTableNodeAtCachedSlot(AssemblyBuilderX64& build, RegisterX64 tmp, RegisterX64 node, RegisterX64 table, int pcpos);
void convertNumberToIndexOrJump(AssemblyBuilderX64& build, RegisterX64 tmp, RegisterX64 numd, RegisterX64 numi, Label& label);

void callArithHelper(IrRegAllocX64& regs, AssemblyBuilderX64& build, int ra, int rb, OperandX64 c, TMS tm);
void callLengthHelper(IrRegAllocX64& regs, AssemblyBuilderX64& build, int ra, int rb);
void callPrepareForN(IrRegAllocX64& regs, AssemblyBuilderX64& build, int limit, int step, int init);
void callGetTable(IrRegAllocX64& regs, AssemblyBuilderX64& build, int rb, OperandX64 c, int ra);
void callSetTable(IrRegAllocX64& regs, AssemblyBuilderX64& build, int rb, OperandX64 c, int ra);
void checkObjectBarrierConditions(AssemblyBuilderX64& build, RegisterX64 tmp, RegisterX64 object, int ra, int ratag, Label& skip);
void callBarrierObject(IrRegAllocX64& regs, AssemblyBuilderX64& build, RegisterX64 object, IrOp objectOp, int ra, int ratag);
void callBarrierTableFast(IrRegAllocX64& regs, AssemblyBuilderX64& build, RegisterX64 table, IrOp tableOp);
void callStepGc(IrRegAllocX64& regs, AssemblyBuilderX64& build);

void emitClearNativeFlag(AssemblyBuilderX64& build);
void emitExit(AssemblyBuilderX64& build, bool continueInVm);
void emitUpdateBase(AssemblyBuilderX64& build);
void emitInterrupt(AssemblyBuilderX64& build);
void emitFallback(IrRegAllocX64& regs, AssemblyBuilderX64& build, int offset, int pcpos);

void emitUpdatePcAndContinueInVm(AssemblyBuilderX64& build);
void emitContinueCallInVm(AssemblyBuilderX64& build);

void emitReturn(AssemblyBuilderX64& build, ModuleHelpers& helpers);

} // namespace X64
} // namespace CodeGen
} // namespace Luau
