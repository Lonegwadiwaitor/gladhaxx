// This file is part of the Luau programming language and is licensed under MIT License; see LICENSE.txt for details
#pragma once

#include "Luau/Ast.h"

#include <string>

namespace Luau
{

void buildTypeMap(DenseHashMap<AstExprFunction*, std::string>& typeMap, AstNode* root);

} // namespace Luau
