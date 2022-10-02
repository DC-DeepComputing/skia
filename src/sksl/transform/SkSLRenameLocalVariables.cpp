/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkTypes.h"
#include "include/private/SkSLProgramElement.h"
#include "include/private/SkSLStatement.h"
#include "src/sksl/SkSLAnalysis.h"
#include "src/sksl/SkSLCompiler.h"
#include "src/sksl/SkSLMangler.h"
#include "src/sksl/ir/SkSLFunctionDefinition.h"
#include "src/sksl/ir/SkSLVarDeclarations.h"
#include "src/sksl/ir/SkSLVariableReference.h"
#include "src/sksl/transform/SkSLProgramWriter.h"
#include "src/sksl/transform/SkSLTransform.h"

#include <memory>
#include <vector>

namespace SkSL {

class Context;
class ProgramUsage;
class SymbolTable;

void Transform::RenameLocalVariables(Context& context, LoadedModule& module, ProgramUsage* usage) {

    class VariableRenamer : public ProgramWriter {
    public:
        VariableRenamer(Context& context, ProgramUsage* usage) : fContext(context), fUsage(usage) {}

        using ProgramWriter::visitProgramElement;

        bool visitStatementPtr(std::unique_ptr<Statement>& stmt) override {
            Analysis::SymbolTableStackBuilder symbolTableStackBuilder(*stmt, &fSymbolTableStack);

            // If this is a variable declaration...
            if (stmt->is<VarDeclaration>()) {
                VarDeclaration& decl = stmt->as<VarDeclaration>();
                const Variable* var = &decl.var();
                // ... and the variable name could be shortened...
                if (var->name().size() > 1) {
                    // ... replace it with a renamed clone.
                    Variable::ScratchVariable scratchVar = Variable::MakeScratchVariable(
                            fContext,
                            fMangler,
                            "",
                            &var->type(),
                            var->modifiers(),
                            fSymbolTableStack.back().get(),
                            var->initialValue() ? var->initialValue()->clone() : nullptr);
                    stmt = std::move(scratchVar.fVarDecl);
                    // Keep track of our replacements so we can fix them up throughout the program.
                    fReplacements[var] = scratchVar.fVarSymbol;
                    fReplacements[scratchVar.fVarSymbol] = nullptr;
                    if (std::unique_ptr<Expression>& value = stmt->as<VarDeclaration>().value()) {
                        return this->visitExpressionPtr(value);
                    }
                }
            }

            return INHERITED::visitStatementPtr(stmt);
        }

        bool visitExpressionPtr(std::unique_ptr<Expression>& expr) override {
            // If this is a variable...
            if (expr->is<VariableReference>()) {
                VariableReference& decl = expr->as<VariableReference>();
                const Variable* var = decl.variable();
                // ... and it's on the replacements list...
                if (const Variable** replacement = fReplacements.find(var)) {
                    // ... replace this variable reference with a clone.
                    expr = VariableReference::Make(decl.fPosition, *replacement, decl.refKind());
                    return false;
                }
            }
            return INHERITED::visitExpressionPtr(expr);
        }

        Context& fContext;
        ProgramUsage* fUsage;
        Mangler fMangler;
        SkTHashMap<const Variable*, const Variable*> fReplacements;
        std::vector<std::shared_ptr<SymbolTable>> fSymbolTableStack;
        using INHERITED = ProgramWriter;
    };

    VariableRenamer visitor{context, usage};

    for (std::unique_ptr<ProgramElement>& pe : module.fElements) {
        if (pe->is<FunctionDefinition>()) {
            visitor.visitProgramElement(*pe);
        }
    }
}

}  // namespace SkSL
