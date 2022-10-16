/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/transform/SkSLTransform.h"

#include "include/core/SkSpan.h"
#include "include/private/SkSLDefines.h"
#include "include/private/SkSLProgramElement.h"
#include "include/private/SkSLStatement.h"
#include "src/sksl/SkSLCompiler.h"
#include "src/sksl/ir/SkSLBlock.h"
#include "src/sksl/ir/SkSLFunctionDefinition.h"
#include "src/sksl/transform/SkSLProgramWriter.h"

#include <algorithm>
#include <iterator>
#include <memory>

template <typename Container, typename Predicate>
static void SkContainerRemoveIf(Container& c, Predicate pred) {
    c.resize(std::distance(c.begin(), std::remove_if(c.begin(), c.end(), pred)));
}

namespace SkSL {

class Expression;

static void eliminate_empty_statements(SkSpan<std::unique_ptr<ProgramElement>> elements) {
    class EmptyStatementEliminator : public ProgramWriter {
    public:
        bool visitExpressionPtr(std::unique_ptr<Expression>& expr) override {
            // We don't need to look inside expressions at all.
            return false;
        }

        bool visitStatementPtr(std::unique_ptr<Statement>& stmt) override {
            // Work from the innermost blocks to the outermost.
            INHERITED::visitStatementPtr(stmt);

            if (stmt->is<Block>()) {
                StatementArray& children = stmt->as<Block>().children();
                SkContainerRemoveIf(children, [](std::unique_ptr<Statement>& stmt) {
                    return stmt->isEmpty();
                });
            }

            // We always check the entire program.
            return false;
        }

        using INHERITED = ProgramWriter;
    };

    for (std::unique_ptr<ProgramElement>& pe : elements) {
        if (pe->is<FunctionDefinition>()) {
            EmptyStatementEliminator visitor;
            visitor.visitStatementPtr(pe->as<FunctionDefinition>().body());
        }
    }
}

void Transform::EliminateEmptyStatements(Module& module) {
    return eliminate_empty_statements(SkSpan(module.fElements));
}

}  // namespace SkSL
