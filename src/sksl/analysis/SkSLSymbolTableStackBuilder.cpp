/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/SkSLAnalysis.h"
#include "src/sksl/ir/SkSLBlock.h"
#include "src/sksl/ir/SkSLForStatement.h"
#include "src/sksl/ir/SkSLIRNode.h"
#include "src/sksl/ir/SkSLStatement.h"

#include <memory>
#include <vector>

namespace SkSL {

class SymbolTable;

namespace Analysis {

SymbolTableStackBuilder::SymbolTableStackBuilder(const Statement* stmt,
                                                 std::vector<SymbolTable*>* stack) {
    if (stmt) {
        switch (stmt->kind()) {
            case Statement::Kind::kBlock:
                if (SymbolTable* symbols = stmt->as<Block>().symbolTable()) {
                    stack->push_back(symbols);
                    fStackToPop = stack;
                }
                break;

            case Statement::Kind::kFor: {
                // A for loop's induction variable (or variables) exist in the symbol table of the
                // body statement. This matters for scoping rules--variables are allowed to shadow
                // symbols in a parent scope, but _cannot_ redeclare names in the current scope.
                // Since induction variables cannot be redeclared at the top level of a for-loop's
                // body, they must _exist_ in the for-loop's body.
                //
                // In order to ensure that ProgramVisitors don't encounter induction variables
                // before encountering the symbol table that encloses them, we push the symbol table
                // for the for-loop statement here. (It will get pushed again when we actually enter
                // the for-loop statement; that's harmless.)
                if (const Statement* body = stmt->as<ForStatement>().statement().get()) {
                    if (body->is<Block>()) {
                        if (SymbolTable* symbols = body->as<Block>().symbolTable()) {
                            stack->push_back(symbols);
                            fStackToPop = stack;
                        }
                    }
                }
                break;
            }

            default:
                break;
        }
    }
}

SymbolTableStackBuilder::~SymbolTableStackBuilder() {
    if (fStackToPop) {
        fStackToPop->pop_back();
    }
}

}  // namespace Analysis
}  // namespace SkSL
