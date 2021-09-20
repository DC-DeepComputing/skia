/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/ir/SkSLSwitchStatement.h"

#include <forward_list>
#include <functional>

#include "include/private/SkTHash.h"
#include "include/sksl/DSLCore.h"
#include "src/sksl/SkSLAnalysis.h"
#include "src/sksl/SkSLConstantFolder.h"
#include "src/sksl/SkSLContext.h"
#include "src/sksl/SkSLMangler.h"
#include "src/sksl/SkSLProgramSettings.h"
#include "src/sksl/ir/SkSLBlock.h"
#include "src/sksl/ir/SkSLForStatement.h"
#include "src/sksl/ir/SkSLIfStatement.h"
#include "src/sksl/ir/SkSLLiteral.h"
#include "src/sksl/ir/SkSLNop.h"
#include "src/sksl/ir/SkSLSymbolTable.h"
#include "src/sksl/ir/SkSLType.h"

namespace SkSL {

std::unique_ptr<Statement> SwitchStatement::clone() const {
    StatementArray cases;
    cases.reserve_back(this->cases().size());
    for (const std::unique_ptr<Statement>& stmt : this->cases()) {
        cases.push_back(stmt->clone());
    }
    return std::make_unique<SwitchStatement>(fOffset,
                                             this->isStatic(),
                                             this->value()->clone(),
                                             std::move(cases),
                                             SymbolTable::WrapIfBuiltin(this->symbols()));
}

String SwitchStatement::description() const {
    String result;
    if (this->isStatic()) {
        result += "@";
    }
    result += String::printf("switch (%s) {\n", this->value()->description().c_str());
    for (const auto& c : this->cases()) {
        result += c->description();
    }
    result += "}";
    return result;
}

static std::forward_list<const SwitchCase*> find_duplicate_case_values(
        const StatementArray& cases) {
    std::forward_list<const SwitchCase*> duplicateCases;
    SkTHashSet<SKSL_INT> intValues;
    bool foundDefault = false;

    for (const std::unique_ptr<Statement>& stmt : cases) {
        const SwitchCase* sc = &stmt->as<SwitchCase>();
        const std::unique_ptr<Expression>& valueExpr = sc->value();

        // A null case-value indicates the `default` switch-case.
        if (!valueExpr) {
            if (foundDefault) {
                duplicateCases.push_front(sc);
                continue;
            }
            foundDefault = true;
            continue;
        }

        // GetConstantInt already succeeded when the SwitchCase was first assembled, so it should
        // succeed this time too.
        SKSL_INT intValue = 0;
        SkAssertResult(ConstantFolder::GetConstantInt(*valueExpr, &intValue));
        if (intValues.contains(intValue)) {
            duplicateCases.push_front(sc);
            continue;
        }
        intValues.add(intValue);
    }

    return duplicateCases;
}

static void move_all_but_break(std::unique_ptr<Statement>& stmt, StatementArray* target) {
    switch (stmt->kind()) {
        case Statement::Kind::kBlock: {
            // Recurse into the block.
            Block& block = stmt->as<Block>();

            StatementArray blockStmts;
            blockStmts.reserve_back(block.children().size());
            for (std::unique_ptr<Statement>& blockStmt : block.children()) {
                move_all_but_break(blockStmt, &blockStmts);
            }

            target->push_back(Block::Make(block.fOffset, std::move(blockStmts),
                                          block.symbolTable(), block.isScope()));
            break;
        }

        case Statement::Kind::kBreak:
            // Do not append a break to the target.
            break;

        default:
            // Append normal statements to the target.
            target->push_back(std::move(stmt));
            break;
    }
}

std::unique_ptr<Statement> SwitchStatement::BlockForCase(StatementArray* cases,
                                                         SwitchCase* caseToCapture,
                                                         std::shared_ptr<SymbolTable> symbolTable) {
    // We have to be careful to not move any of the pointers until after we're sure we're going to
    // succeed, so before we make any changes at all, we check the switch-cases to decide on a plan
    // of action. First, find the switch-case we are interested in.
    auto iter = cases->begin();
    for (; iter != cases->end(); ++iter) {
        const SwitchCase& sc = (*iter)->as<SwitchCase>();
        if (&sc == caseToCapture) {
            break;
        }
    }

    // Next, walk forward through the rest of the switch. If we find a conditional break, we're
    // stuck and can't simplify at all. If we find an unconditional break, we have a range of
    // statements that we can use for simplification.
    auto startIter = iter;
    Statement* stripBreakStmt = nullptr;
    for (; iter != cases->end(); ++iter) {
        std::unique_ptr<Statement>& stmt = (*iter)->as<SwitchCase>().statement();
        if (Analysis::SwitchCaseContainsConditionalExit(*stmt)) {
            // We can't reduce switch-cases to a block when they have conditional exits.
            return nullptr;
        }
        if (Analysis::SwitchCaseContainsUnconditionalExit(*stmt)) {
            // We found an unconditional exit. We can use this block, but we'll need to strip
            // out the break statement if there is one.
            stripBreakStmt = stmt.get();
            break;
        }
    }

    // We fell off the bottom of the switch or encountered a break. We know the range of statements
    // that we need to move over, and we know it's safe to do so.
    StatementArray caseStmts;
    caseStmts.reserve_back(std::distance(startIter, iter) + 1);

    // We can move over most of the statements as-is.
    while (startIter != iter) {
        caseStmts.push_back(std::move((*startIter)->as<SwitchCase>().statement()));
        ++startIter;
    }

    // If we found an unconditional break at the end, we need to move what we can while avoiding
    // that break.
    if (stripBreakStmt != nullptr) {
        SkASSERT((*startIter)->as<SwitchCase>().statement().get() == stripBreakStmt);
        move_all_but_break((*startIter)->as<SwitchCase>().statement(), &caseStmts);
    }

    // Return our newly-synthesized block.
    return Block::Make(caseToCapture->fOffset, std::move(caseStmts), std::move(symbolTable));
}

std::unique_ptr<Statement> SwitchStatement::Convert(const Context& context,
                                                    int offset,
                                                    bool isStatic,
                                                    std::unique_ptr<Expression> value,
                                                    ExpressionArray caseValues,
                                                    StatementArray caseStatements,
                                                    std::shared_ptr<SymbolTable> symbolTable) {
    SkASSERT(caseValues.size() == caseStatements.size());
//    if (context.fConfig->strictES2Mode()) {
//        context.fErrors->error(offset, "switch statements are not supported");
//        return nullptr;
//    }

    value = context.fTypes.fInt->coerceExpression(std::move(value), context);
    if (!value) {
        return nullptr;
    }

    StatementArray cases;
    for (int i = 0; i < caseValues.count(); ++i) {
        int caseOffset;
        std::unique_ptr<Expression> caseValue;
        if (caseValues[i]) {
            caseOffset = caseValues[i]->fOffset;

            // Case values must be the same type as the switch value--`int` or a particular enum.
            caseValue = value->type().coerceExpression(std::move(caseValues[i]), context);
            if (!caseValue) {
                return nullptr;
            }
            // Case values must be a literal integer or a `const int` variable reference.
            SKSL_INT intValue;
            if (!ConstantFolder::GetConstantInt(*caseValue, &intValue)) {
                context.fErrors->error(caseValue->fOffset, "case value must be a constant integer");
                return nullptr;
            }
        } else {
            // The null case-expression corresponds to `default:`.
            caseOffset = offset;
        }
        cases.push_back(std::make_unique<SwitchCase>(caseOffset, std::move(caseValue),
                                                     std::move(caseStatements[i])));
    }

    // Detect duplicate `case` labels and report an error.
    // (Using forward_list here to optimize for the common case of no results.)
    std::forward_list<const SwitchCase*> duplicateCases = find_duplicate_case_values(cases);
    if (!duplicateCases.empty()) {
        duplicateCases.reverse();
        for (const SwitchCase* sc : duplicateCases) {
            if (sc->value() != nullptr) {
                context.fErrors->error(sc->fOffset,
                                       "duplicate case value '" + sc->value()->description() + "'");
            } else {
                context.fErrors->error(sc->fOffset, "duplicate default case");
            }
        }
        return nullptr;
    }

    return SwitchStatement::Make(context, offset, isStatic, std::move(value), std::move(cases),
                                 std::move(symbolTable));
}

// Searches for `continue` statements in a block and calls a helper function to rewrite them.
class SwitchContinueRewriter : public ProgramWriter {
public:
    SwitchContinueRewriter(const std::function<void(std::unique_ptr<Statement>&)> r)
            : fRewriteFn(r) {}

    bool visitStatementPtr(std::unique_ptr<Statement>& s) override {
        switch (s->kind()) {
            case Statement::Kind::kContinue:
                // We found a `continue` statement. Rewrite it.
                fRewriteFn(s);
                return false;

            case Statement::Kind::kDo:
            case Statement::Kind::kFor:
                // `continue` statements inside of inner loops should not be rewritten.
                return false;

            default:
                // Other statements can be traversed normally.
                break;
        }
        return INHERITED::visitStatementPtr(s);
    }

    bool visitExpressionPtr(std::unique_ptr<Expression>&) override {
        // We don't need to traverse expressions.
        return false;
    }

    std::function<void(std::unique_ptr<Statement>&)> fRewriteFn;
    using INHERITED = ProgramWriter;
};

// Create a block of code which emulates a `switch` statement, using if statements inside a
// single-iteration for loop. Useful for SkVM or non-ES3-compatible platforms.
static std::unique_ptr<Statement> make_es2_mode_switch(const Context& context,
                                                       int offset,
                                                       bool isStatic,
                                                       std::unique_ptr<Expression> value,
                                                       StatementArray cases,
                                                       std::shared_ptr<SymbolTable> symbolTable) {
    using namespace SkSL::dsl;

    // Synthesize a unique name for a temp variable and add it to the symbol table.
    auto MangleName = [&](const char* name) -> skstd::string_view {
        return *symbolTable->takeOwnershipOfString(
                context.fMangler->uniqueName(name, symbolTable.get()));
    };

    // Create an outer block.
    AutoSymbolTable table(&symbolTable);
    std::unique_ptr<Block> outerBlock = Block::Make(offset, StatementArray{}, symbolTable);

    // Create scratch variable `switchValue` which stores the switch's test value.
    DSLVar switchValue(kInt_Type, MangleName("switchValue"), DSLExpression(std::move(value)));
    outerBlock->children().push_back(Declare(switchValue).release());

    // Create scratch variable `switchContinue` which is used to rewrite `continue` statements.
    DSLVar switchContinue(kBool_Type, MangleName("switchContinue"), Bool(false));
    outerBlock->children().push_back(Declare(switchContinue).release());
    bool rewroteContinue = false;

    // Create scratch variable `switchFallthrough` which is set to true when cases fall through.
    DSLVar switchFallthrough(kBool_Type, MangleName("switchFallthrough"), Bool(false));
    outerBlock->children().push_back(Declare(switchFallthrough).release());

    // Create scratch variable `switchLoop` which is used to make a one-iteration loop.
    DSLVar switchLoop(kInt_Type, MangleName("switchLoop"), Int(0));

    // Create a loop to wrap our switch cases (allowing `break` to eject).
    std::unique_ptr<Block> innerBlockUniqPtr = Block::Make(offset, StatementArray{}, symbolTable);
    Block* innerBlock = innerBlockUniqPtr.get();
    outerBlock->children().push_back(ForStatement::Make(context,
                                                        offset,
                                                        Declare(switchLoop).release(),
                                                        (switchLoop < 1).release(),
                                                        (switchLoop++).release(),
                                                        std::move(innerBlockUniqPtr),
                                                        /*unrollInfo=*/nullptr,
                                                        symbolTable));

    // Create a helper function to replace `continue` with `{ switchContinue = true; break; }`
    SwitchContinueRewriter rewriter([&](std::unique_ptr<Statement>& stmt) {
        stmt = DSLBlock(
            switchContinue = true,
            Break()
        ).release();
        rewroteContinue = true;
    });

    bool firstCase = true;
    for (std::unique_ptr<Statement>& c : cases) {
        // We can extract everything out of the SwitchCase object, and don't need to clone.
        SwitchCase& switchCase = c->as<SwitchCase>();
        auto CaseValue  = [&] { return DSLExpression(std::move(switchCase.value())); };
        auto SwitchStmt = [&] { return std::move(switchCase.statement()); };

        // Fix up the block from the switch-case. It needs to be scoped, and continue statements
        // need to be rewritten (since we have added a loop that didn't exist before).
        rewriter.visitStatementPtr(switchCase.statement());
        Block& caseBlock = switchCase.statement()->as<Block>();
        caseBlock.setIsScope(true);

        // The default case. `default` must always be last, so we can just dump the code in.
        // No additional checks are necessary.
        if (!switchCase.value()) {
            innerBlock->children().push_back(SwitchStmt());
            continue;
        }

        // Tack on `switch_fallthrough = true;` at the end of the user code. (It'll be dead-stripped
        // during optimization if it's unreachable.)
        caseBlock.children().push_back(DSLStatement(switchFallthrough = true).release());

        // For the first case, emit `if (switchValue == 123) {...}`.
        if (firstCase) {
            innerBlock->children().push_back(If(switchValue == CaseValue(),
                                                SwitchStmt()).release());
            firstCase = false;
            continue;
        }

        // For each remaining case, emit `if (switchFallthrough || switchValue == 123) {...}`.
        innerBlock->children().push_back(If(switchFallthrough || (switchValue == CaseValue()),
                                            SwitchStmt()).release());
    }

    // Emit `if (switchContinue) continue;` if any continues have been rewritten.
    if (rewroteContinue) {
        outerBlock->children().push_back(If(switchContinue, Continue()).release());
    }

    return std::move(outerBlock);
}

std::unique_ptr<Statement> SwitchStatement::Make(const Context& context,
                                                 int offset,
                                                 bool isStatic,
                                                 std::unique_ptr<Expression> value,
                                                 StatementArray cases,
                                                 std::shared_ptr<SymbolTable> symbolTable) {
    // Confirm that every statement in `cases` is a SwitchCase.
    SkASSERT(std::all_of(cases.begin(), cases.end(), [&](const std::unique_ptr<Statement>& stmt) {
        return stmt->is<SwitchCase>();
    }));

    // Confirm that every switch-case has been coerced to the proper type.
    SkASSERT(std::all_of(cases.begin(), cases.end(), [&](const std::unique_ptr<Statement>& stmt) {
        return !stmt->as<SwitchCase>().value() ||  // `default` case has a null value
               value->type() == stmt->as<SwitchCase>().value()->type();
    }));

    // Confirm that every switch-case value is unique.
    SkASSERT(find_duplicate_case_values(cases).empty());

    // Flatten @switch statements.
    if (isStatic || context.fConfig->fSettings.fOptimize) {
        SKSL_INT switchValue;
        if (ConstantFolder::GetConstantInt(*value, &switchValue)) {
            SwitchCase* defaultCase = nullptr;
            SwitchCase* matchingCase = nullptr;
            for (const std::unique_ptr<Statement>& stmt : cases) {
                SwitchCase& sc = stmt->as<SwitchCase>();
                if (!sc.value()) {
                    defaultCase = &sc;
                    continue;
                }

                SKSL_INT caseValue;
                SkAssertResult(ConstantFolder::GetConstantInt(*sc.value(), &caseValue));
                if (caseValue == switchValue) {
                    matchingCase = &sc;
                    break;
                }
            }

            if (!matchingCase) {
                // No case value matches the switch value.
                if (!defaultCase) {
                    // No default switch-case exists; the switch had no effect.
                    // We can eliminate the entire switch!
                    return Nop::Make();
                }
                // We had a default case; that's what we matched with.
                matchingCase = defaultCase;
            }

            // Convert the switch-case that we matched with into a block.
            std::unique_ptr<Statement> newBlock = BlockForCase(&cases, matchingCase, symbolTable);
            if (newBlock) {
                return newBlock;
            }

            // Report an error if this was a static switch and BlockForCase failed us.
            if (isStatic && !context.fConfig->fSettings.fPermitInvalidStaticTests) {
                context.fErrors->error(value->fOffset,
                                       "static switch contains non-static conditional exit");
                return nullptr;
            }
        }
    }

    // On ES2 devices, we don't have a native switch instruction, but we can fake it.
    if (true || context.fConfig->strictES2Mode()) {
        return make_es2_mode_switch(context, offset, isStatic, std::move(value), std::move(cases),
                                    std::move(symbolTable));
    }

    // The switch couldn't be optimized away; emit it normally.
    return std::make_unique<SwitchStatement>(offset, isStatic, std::move(value), std::move(cases),
                                             std::move(symbolTable));
}

}  // namespace SkSL
