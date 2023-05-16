/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/dsl/priv/DSLWriter.h"

#include "include/core/SkTypes.h"
#include "include/private/SkSLDefines.h"
#include "src/sksl/SkSLContext.h"
#include "src/sksl/SkSLModifiersPool.h"
#include "src/sksl/SkSLPosition.h"
#include "src/sksl/SkSLThreadContext.h"
#include "src/sksl/dsl/DSLExpression.h"
#include "src/sksl/dsl/DSLStatement.h"
#include "src/sksl/dsl/DSLType.h"
#include "src/sksl/dsl/DSLVar.h"
#include "src/sksl/ir/SkSLBlock.h"
#include "src/sksl/ir/SkSLNop.h"
#include "src/sksl/ir/SkSLProgramElement.h"
#include "src/sksl/ir/SkSLStatement.h"
#include "src/sksl/ir/SkSLSymbolTable.h"
#include "src/sksl/ir/SkSLVarDeclarations.h"
#include "src/sksl/ir/SkSLVariable.h"

#include <utility>
#include <vector>

namespace SkSL {

namespace dsl {

void DSLWriter::Var(DSLVarBase& var) {
    SkASSERT(var.fStorage != SkSL::VariableStorage::kParameter);  // use CreateParameterVar instead

    // We haven't even attempted to create a var yet; fVar and fDeclaration ought to be null.
    SkASSERT(!var.fVar);
    SkASSERT(!var.fDeclaration);

    const Context& context = ThreadContext::Context();

    std::unique_ptr<SkSL::Variable> skslvar = SkSL::Variable::Convert(context,
                                                                      var.fPosition,
                                                                      var.fModifiersPos,
                                                                      var.fModifiers,
                                                                      &var.fType.skslType(),
                                                                      var.fNamePosition,
                                                                      var.fName,
                                                                      var.fStorage);

    var.fDeclaration = VarDeclaration::Convert(context,
                                               std::move(skslvar),
                                               var.fInitialValue.releaseIfPossible());
    if (var.fDeclaration) {
        var.fVar = var.fDeclaration->as<VarDeclaration>().var();
    } else {
        var.fDeclaration = SkSL::Nop::Make();
    }
}

std::unique_ptr<SkSL::Variable> DSLWriter::CreateParameterVar(DSLParameter& var) {
    SkASSERT(!var.fVar);
    SkASSERT(!var.fDeclaration);
    SkASSERT(!var.fInitialValue.hasValue());

    return SkSL::Variable::Convert(ThreadContext::Context(),
                                   var.fPosition,
                                   var.fModifiersPos,
                                   var.fModifiers,
                                   &var.fType.skslType(),
                                   var.fNamePosition,
                                   var.fName,
                                   var.fStorage);
}

void DSLWriter::AddVarDeclaration(DSLStatement& existing, DSLVar& additional) {
    Var(additional);

    if (existing.fStatement->is<Block>()) {
        SkSL::Block& block = existing.fStatement->as<Block>();
        SkASSERT(!block.isScope());
        block.children().push_back(std::move(additional.fDeclaration));
    } else if (existing.fStatement->is<VarDeclaration>()) {
        Position pos = existing.fStatement->fPosition;
        StatementArray stmts;
        stmts.reserve_back(2);
        stmts.push_back(std::move(existing.fStatement));
        stmts.push_back(std::move(additional.fDeclaration));
        existing.fStatement = SkSL::Block::Make(pos, std::move(stmts),
                                                Block::Kind::kCompoundStatement);
    } else if (existing.fStatement->isEmpty()) {
        // If the variable declaration generated an error, we can end up with a Nop statement here.
        existing.fStatement = std::move(additional.fDeclaration);
    }
}

void DSLWriter::Reset() {
    SkSL::Context& context = ThreadContext::Context();

    SymbolTable::Pop(&context.fSymbolTable);
    SymbolTable::Push(&context.fSymbolTable);
    ThreadContext::ProgramElements().clear();
    context.fModifiersPool->clear();
}

} // namespace dsl

} // namespace SkSL
