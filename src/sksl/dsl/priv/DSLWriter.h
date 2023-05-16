/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_DSLWRITER
#define SKSL_DSLWRITER

#include "include/core/SkTypes.h"
#include "src/sksl/ir/SkSLStatement.h"

#include <cstdint>
#include <memory>
#include <string_view>

namespace SkSL {

struct Modifiers;
class Position;
class Type;
class Variable;
enum class VariableStorage : int8_t;

namespace dsl {

class DSLExpression;

struct DSLVarDecl {
    std::unique_ptr<SkSL::Statement> fDeclaration;
    SkSL::Variable* fVar = nullptr;
};

/**
 * Various utility methods needed by DSL code.
 */
class DSLWriter {
public:
    /**
     * Creates an SkSL Variable and VarDeclaration for a global or local variable.
     */
    static DSLVarDecl Var(Position overallPos,
                          Position modifiersPos,
                          const Modifiers& modifiers,
                          const Type& type,
                          Position namePos,
                          std::string_view name,
                          VariableStorage storage,
                          DSLExpression initialValue);

    /**
     * Clears any elements or symbols which have been output.
     */
    static void Reset();
};

} // namespace dsl

} // namespace SkSL

#endif
