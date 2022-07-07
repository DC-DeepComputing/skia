/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkSpan.h"
#include "include/private/SkSLProgramElement.h"
#include "include/private/SkSLStatement.h"
#include "include/private/SkTArray.h"
#include "src/sksl/SkSLCompiler.h"
#include "src/sksl/ir/SkSLFunctionDefinition.h"
#include "src/sksl/ir/SkSLIfStatement.h"
#include "src/sksl/ir/SkSLNop.h"
#include "src/sksl/ir/SkSLProgram.h"
#include "src/sksl/transform/SkSLProgramWriter.h"
#include "src/sksl/transform/SkSLTransform.h"

#include <memory>

namespace SkSL {

class Expression;

static void eliminate_unreachable_code(SkSpan<std::unique_ptr<ProgramElement>> elements,
                                       ProgramUsage* usage) {
    class UnreachableCodeEliminator : public ProgramWriter {
    public:
        UnreachableCodeEliminator(ProgramUsage* usage) : fUsage(usage) {
            fFoundFunctionExit.push_back(false);
            fFoundLoopExit.push_back(false);
        }

        bool visitExpressionPtr(std::unique_ptr<Expression>& expr) override {
            // We don't need to look inside expressions at all.
            return false;
        }

        bool visitStatementPtr(std::unique_ptr<Statement>& stmt) override {
            if (fFoundFunctionExit.back() || fFoundLoopExit.back()) {
                // If we already found an exit in this section, anything beyond it is dead code.
                if (!stmt->is<Nop>()) {
                    // Eliminate the dead statement by substituting a Nop.
                    fUsage->remove(stmt.get());
                    stmt = Nop::Make();
                }
                return false;
            }

            switch (stmt->kind()) {
                case Statement::Kind::kReturn:
                case Statement::Kind::kDiscard:
                    // We found a function exit on this path.
                    fFoundFunctionExit.back() = true;
                    break;

                case Statement::Kind::kBreak:
                    // A `break` statement can either be breaking out of a loop or terminating an
                    // individual switch case. We treat both cases the same way: they only apply
                    // to the statements associated with the parent statement (i.e. enclosing loop
                    // block / preciding case label).
                case Statement::Kind::kContinue:
                    fFoundLoopExit.back() = true;
                    break;

                case Statement::Kind::kExpression:
                case Statement::Kind::kNop:
                case Statement::Kind::kVarDeclaration:
                    // These statements don't affect control flow.
                    break;

                case Statement::Kind::kBlock:
                    // Blocks are on the straight-line path and don't affect control flow.
                    return INHERITED::visitStatementPtr(stmt);

                case Statement::Kind::kDo: {
                    // Function-exits are allowed to propagate outside of a do-loop, because it
                    // always executes its body at least once.
                    fFoundLoopExit.push_back(false);
                    bool result = INHERITED::visitStatementPtr(stmt);
                    fFoundLoopExit.pop_back();
                    return result;
                }
                case Statement::Kind::kFor: {
                    // Function-exits are not allowed to propagate out, because a for-loop or while-
                    // loop could potentially run zero times.
                    fFoundFunctionExit.push_back(false);
                    fFoundLoopExit.push_back(false);
                    bool result = INHERITED::visitStatementPtr(stmt);
                    fFoundLoopExit.pop_back();
                    fFoundFunctionExit.pop_back();
                    return result;
                }
                case Statement::Kind::kIf: {
                    // This statement is conditional and encloses two inner sections of code.
                    // If both sides contain a function-exit or loop-exit, that exit is allowed to
                    // propagate out.
                    IfStatement& ifStmt = stmt->as<IfStatement>();

                    fFoundFunctionExit.push_back(false);
                    fFoundLoopExit.push_back(false);
                    bool result = (ifStmt.ifTrue() && this->visitStatementPtr(ifStmt.ifTrue()));
                    bool foundFunctionExitOnTrue = fFoundFunctionExit.back();
                    bool foundLoopExitOnTrue = fFoundLoopExit.back();
                    fFoundFunctionExit.pop_back();
                    fFoundLoopExit.pop_back();

                    fFoundFunctionExit.push_back(false);
                    fFoundLoopExit.push_back(false);
                    result |= (ifStmt.ifFalse() && this->visitStatementPtr(ifStmt.ifFalse()));
                    bool foundFunctionExitOnFalse = fFoundFunctionExit.back();
                    bool foundLoopExitOnFalse = fFoundLoopExit.back();
                    fFoundFunctionExit.pop_back();
                    fFoundLoopExit.pop_back();

                    fFoundFunctionExit.back() |= foundFunctionExitOnTrue &&
                                                 foundFunctionExitOnFalse;
                    fFoundLoopExit.back() |= foundLoopExitOnTrue &&
                                             foundLoopExitOnFalse;
                    return result;
                }
                case Statement::Kind::kSwitch:
                    // In switch statements we consider unreachable code on a per-case statement
                    // basis.
                    return INHERITED::visitStatementPtr(stmt);
                case Statement::Kind::kSwitchCase: {
                    // We eliminate unreachable code only within the statements of the individual
                    // case. Function exits or breaks are not allowed to propagate outside the case
                    // statement itself.
                    fFoundFunctionExit.push_back(false);
                    fFoundLoopExit.push_back(false);
                    bool result = INHERITED::visitStatementPtr(stmt);
                    fFoundFunctionExit.pop_back();
                    fFoundLoopExit.pop_back();
                    return result;
                }
            }

            return false;
        }

        ProgramUsage* fUsage;
        SkSTArray<32, bool> fFoundFunctionExit;
        SkSTArray<32, bool> fFoundLoopExit;

        using INHERITED = ProgramWriter;
    };

    for (std::unique_ptr<ProgramElement>& pe : elements) {
        if (pe->is<FunctionDefinition>()) {
            UnreachableCodeEliminator visitor{usage};
            visitor.visitStatementPtr(pe->as<FunctionDefinition>().body());
        }
    }
}

void Transform::EliminateUnreachableCode(LoadedModule& module, ProgramUsage* usage) {
    return eliminate_unreachable_code(SkSpan(module.fElements), usage);
}

void Transform::EliminateUnreachableCode(Program& program, ProgramUsage* usage) {
    return eliminate_unreachable_code(SkSpan(program.fOwnedElements), usage);
}

}  // namespace SkSL
