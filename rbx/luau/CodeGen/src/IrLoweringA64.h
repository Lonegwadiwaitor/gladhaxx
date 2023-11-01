// This file is part of the Luau programming language and is licensed under MIT License; see LICENSE.txt for details
#pragma once

#include "Luau/AssemblyBuilderA64.h"
#include "Luau/IrData.h"

#include "IrRegAllocA64.h"
#include "IrValueLocationTracking.h"

#include <vector>

namespace Luau
{
namespace CodeGen
{

struct ModuleHelpers;
struct AssemblyOptions;

namespace A64
{

struct IrLoweringA64
{
    IrLoweringA64(AssemblyBuilderA64& build, ModuleHelpers& helpers, IrFunction& function);

    void lowerInst(IrInst& inst, uint32_t index, IrBlock& next);
    void finishBlock();
    void finishFunction();

    bool hasError() const;

    bool isFallthroughBlock(IrBlock target, IrBlock next);
    void jumpOrFallthrough(IrBlock& target, IrBlock& next);

    // Operand data build helpers
    // May emit data/address synthesis instructions
    RegisterA64 tempDouble(IrOp op);
    RegisterA64 tempInt(IrOp op);
    RegisterA64 tempUint(IrOp op);
    AddressA64 tempAddr(IrOp op, int offset);

    // May emit restore instructions
    RegisterA64 regOp(IrOp op);

    // Operand data lookup helpers
    IrConst constOp(IrOp op) const;
    uint8_t tagOp(IrOp op) const;
    int intOp(IrOp op) const;
    unsigned uintOp(IrOp op) const;
    double doubleOp(IrOp op) const;

    IrBlock& blockOp(IrOp op) const;
    Label& labelOp(IrOp op) const;

    struct InterruptHandler
    {
        Label self;
        unsigned int pcpos;
        Label next;
    };

    struct ExitHandler
    {
        Label self;
        unsigned int pcpos;
    };

    AssemblyBuilderA64& build;
    ModuleHelpers& helpers;

    IrFunction& function;

    IrRegAllocA64 regs;

    IrValueLocationTracking valueTracker;

    std::vector<InterruptHandler> interruptHandlers;
    std::vector<ExitHandler> exitHandlers;

    bool error = false;
};

} // namespace A64
} // namespace CodeGen
} // namespace Luau
