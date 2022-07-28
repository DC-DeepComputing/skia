/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/private/SkSLModifiers.h"
#include "src/sksl/SkSLBuiltinMap.h"
#include "src/sksl/SkSLContext.h"
#include "src/sksl/SkSLModifiersPool.h"
#include "src/sksl/analysis/SkSLProgramUsage.h"
#include "src/sksl/ir/SkSLExpression.h"
#include "src/sksl/ir/SkSLFunctionDeclaration.h"
#include "src/sksl/ir/SkSLProgram.h"
#include "src/sksl/ir/SkSLVariable.h"
#include "src/sksl/transform/SkSLTransform.h"

namespace SkSL {

void Transform::AddBuiltinFunctions(Program& program) {
    ProgramUsage* usage = program.fUsage.get();
    Context& context = *program.fContext;
    bool runAgain;

    // TODO: optimize
    do {
        std::vector<const FunctionDeclaration*> builtinFunctions;
        for (const auto& [funcDecl, count] : usage->fCallCounts) {
            SkASSERT(count > 0);
            if (funcDecl->isBuiltin()) {
                builtinFunctions.push_back(funcDecl);
            }
        }

        std::sort(builtinFunctions.begin(), builtinFunctions.end(),
                  [](const FunctionDeclaration* a, const FunctionDeclaration* b) {
                      if (a->isBuiltin() != b->isBuiltin()) {
                          return a->isBuiltin() < b->isBuiltin();
                      }
                      if (a->fPosition != b->fPosition) {
                          return a->fPosition < b->fPosition;
                      }
                      if (a->name() != b->name()) {
                          return a->name() < b->name();
                      }
                      return a->description() < b->description();
                  });

        runAgain = false;
        for (const FunctionDeclaration* f : builtinFunctions) {
            if (const ProgramElement* found = context.fBuiltins->findAndInclude(f->description())) {
                program.fSharedElements.insert(program.fSharedElements.begin(), found);
                usage->add(*found);
                runAgain = true;
            }
        }
    } while (runAgain);
}

}  // namespace SkSL
