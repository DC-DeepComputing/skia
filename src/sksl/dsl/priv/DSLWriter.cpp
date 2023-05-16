/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/dsl/priv/DSLWriter.h"

#include "include/core/SkTypes.h"
#include "src/sksl/SkSLContext.h"
#include "src/sksl/SkSLModifiersPool.h"
#include "src/sksl/SkSLPosition.h"
#include "src/sksl/SkSLThreadContext.h"
#include "src/sksl/dsl/DSLExpression.h"
#include "src/sksl/ir/SkSLProgramElement.h"
#include "src/sksl/ir/SkSLStatement.h"
#include "src/sksl/ir/SkSLSymbolTable.h"
#include "src/sksl/ir/SkSLVarDeclarations.h"
#include "src/sksl/ir/SkSLVariable.h"

#include <utility>
#include <vector>

namespace SkSL::dsl {

DSLVarDecl DSLWriter::Var(Position overallPos,
                          Position modifiersPos,
                          const Modifiers& modifiers,
                          const Type& type,
                          Position namePos,
                          std::string_view name,
                          VariableStorage storage,
                          DSLExpression initialValue) {
    SkASSERT(storage != SkSL::VariableStorage::kParameter);  // use CreateParameterVar instead

    const Context& context = ThreadContext::Context();

    std::unique_ptr<SkSL::Variable> var = SkSL::Variable::Convert(context,
                                                                  overallPos,
                                                                  modifiersPos,
                                                                  modifiers,
                                                                  &type,
                                                                  namePos,
                                                                  name,
                                                                  storage);
    DSLVarDecl result;
    result.fDeclaration = VarDeclaration::Convert(context,
                                                  std::move(var),
                                                  initialValue.releaseIfPossible());
    result.fVar = result.fDeclaration ? result.fDeclaration->as<VarDeclaration>().var()
                                      : nullptr;
    return result;
}

void DSLWriter::Reset() {
    SkSL::Context& context = ThreadContext::Context();

    SymbolTable::Pop(&context.fSymbolTable);
    SymbolTable::Push(&context.fSymbolTable);
    ThreadContext::ProgramElements().clear();
    context.fModifiersPool->clear();
}

}  // namespace SkSL::dsl
