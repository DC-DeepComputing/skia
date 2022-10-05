/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/SkSLCompiler.h"
#include "src/sksl/analysis/SkSLProgramUsage.h"
#include "src/sksl/ir/SkSLExpression.h"
#include "src/sksl/ir/SkSLSetting.h"
#include "src/sksl/ir/SkSLTernaryExpression.h"
#include "src/sksl/transform/SkSLProgramWriter.h"
#include "src/sksl/transform/SkSLTransform.h"

#include <memory>
#include <string_view>
#include <utility>
#include <vector>

namespace SkSL {

class ProgramElement;

void Transform::EliminateTernaryOnSkCapsRewriteMatrixVectorMultiply(LoadedModule& module,
                                                                    ProgramUsage* usage) {
    class RewriteTernaryEliminator : public ProgramWriter {
    public:
        RewriteTernaryEliminator(ProgramUsage* usage) : fUsage(usage) {}

        using ProgramWriter::visitProgramElement;

        bool visitExpressionPtr(std::unique_ptr<Expression>& expr) override {
            if (expr->is<TernaryExpression>()) {
                TernaryExpression& ternary = expr->as<TernaryExpression>();
                if (ternary.test()->is<Setting>()) {
                    Setting& setting = ternary.test()->as<Setting>();
                    if (setting.name() == "rewriteMatrixVectorMultiply") {
                        fUsage->remove(ternary.ifTrue().get());
                        expr = std::move(ternary.ifFalse());
                    }
                }
            }
            return INHERITED::visitExpressionPtr(expr);
        }

        ProgramUsage* fUsage;

        using INHERITED = ProgramWriter;
    };

    RewriteTernaryEliminator visitor{usage};
    for (std::unique_ptr<ProgramElement>& pe : module.fElements) {
        visitor.visitProgramElement(*pe);
    }
}

}  // namespace SkSL
