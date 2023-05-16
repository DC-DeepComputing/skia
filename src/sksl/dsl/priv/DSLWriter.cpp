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
    if (!additional.fDeclaration) {
        // The additional variable couldn't be declared; nothing needs to change.
        return;
    }

    std::unique_ptr<Statement> stmt = existing.releaseIfPossible();
    if (!stmt || stmt->isEmpty()) {
        // If the variable declaration generated an error, we can end up with a Nop statement here.
        // Jettison the existing statement, and keep the new declaration.
        existing = DSLStatement(std::move(additional.fDeclaration));
        return;
    }

    if (stmt->is<Block>()) {
        SkSL::Block& block = stmt->as<Block>();
        if (!block.isScope()) {
            // The existing statement is a Block without curly braces; append the additional
            // declaration to the end.
            block.children().push_back(std::move(additional.fDeclaration));
            existing = DSLStatement(std::move(stmt));
            return;
        }
    }

    // The statement was not a Block; create a compound-statement block to hold the existing
    // statement alongside the additional one.
    Position pos = stmt->fPosition;
    StatementArray stmts;
    stmts.reserve_back(2);
    stmts.push_back(std::move(stmt));
    stmts.push_back(std::move(additional.fDeclaration));
    existing = DSLStatement(
            SkSL::Block::Make(pos, std::move(stmts), Block::Kind::kCompoundStatement),
            pos);
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
