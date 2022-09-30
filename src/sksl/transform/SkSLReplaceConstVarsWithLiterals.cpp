/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkSpan.h"
#include "include/core/SkTypes.h"
#include "include/private/SkSLProgramElement.h"
#include "include/private/SkSLStatement.h"
#include "include/private/SkTHash.h"
#include "src/sksl/SkSLAnalysis.h"
#include "src/sksl/SkSLCompiler.h"
#include "src/sksl/SkSLConstantFolder.h"
#include "src/sksl/SkSLProgramSettings.h"
#include "src/sksl/analysis/SkSLProgramUsage.h"
#include "src/sksl/ir/SkSLBinaryExpression.h"
#include "src/sksl/ir/SkSLExpression.h"
#include "src/sksl/ir/SkSLExpressionStatement.h"
#include "src/sksl/ir/SkSLFunctionDefinition.h"
#include "src/sksl/ir/SkSLNop.h"
#include "src/sksl/ir/SkSLProgram.h"
#include "src/sksl/ir/SkSLVarDeclarations.h"
#include "src/sksl/ir/SkSLVariable.h"
#include "src/sksl/ir/SkSLVariableReference.h"
#include "src/sksl/transform/SkSLProgramWriter.h"
#include "src/sksl/transform/SkSLTransform.h"

#include <memory>
#include <utility>

namespace SkSL {

class Context;

static void replace_const_vars_with_literals(SkSpan<std::unique_ptr<ProgramElement>> elements,
                                             ProgramUsage* usage) {
    class ConstVarReplacer : public ProgramWriter {
    public:
        ConstVarReplacer(ProgramUsage* usage) : fUsage(usage) {}

        using ProgramWriter::visitProgramElement;

        bool visitExpressionPtr(std::unique_ptr<Expression>& expr) override {
            // If this is a variable...
            if (expr->is<VariableReference>()) {
                VariableReference& var = expr->as<VariableReference>();
                // ... and it's read-from...
                if (var.refKind() == VariableRefKind::kRead) {
                    // ... and it could be replaced with a constant value...
                    const Expression* value = ConstantFolder::GetConstantValueForVariable(var);
                    if (value != expr.get()) {
                        // ... replace it with that value.
                        fUsage->remove(expr.get());
                        expr = value->clone();
                        fUsage->add(expr.get());
                        return false;
                    }
                }
            }
            return INHERITED::visitExpressionPtr(expr);
        }

        ProgramUsage* fUsage;

        using INHERITED = ProgramWriter;
    };

    ConstVarReplacer visitor{usage};

    for (std::unique_ptr<ProgramElement>& pe : elements) {
        if (pe->is<FunctionDefinition>()) {
            visitor.visitProgramElement(*pe);
        }
    }
}

void Transform::ReplaceConstVarsWithLiterals(LoadedModule& module, ProgramUsage* usage) {
    replace_const_vars_with_literals(SkSpan(module.fElements), usage);
}

}  // namespace SkSL
