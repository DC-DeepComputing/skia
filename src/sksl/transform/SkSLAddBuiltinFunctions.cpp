/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkTypes.h"
#include "include/private/SkSLProgramElement.h"
#include "include/private/SkTHash.h"
#include "include/sksl/SkSLPosition.h"
#include "src/sksl/SkSLBuiltinMap.h"
#include "src/sksl/SkSLContext.h"
#include "src/sksl/analysis/SkSLProgramUsage.h"
#include "src/sksl/ir/SkSLFunctionDeclaration.h"
#include "src/sksl/ir/SkSLFunctionDefinition.h"
#include "src/sksl/ir/SkSLProgram.h"
#include "src/sksl/transform/SkSLTransform.h"

#include <algorithm>
#include <cstddef>
#include <memory>
#include <vector>

namespace SkSL {

void Transform::AddBuiltinFunctions(Program& program) {
    ProgramUsage* usage = program.fUsage.get();
    Context& context = *program.fContext;

    std::vector<const FunctionDefinition*> addedBuiltins;
    for (;;) {
        // Find all the built-ins referenced by the program but not yet included in the code.
        size_t numBuiltinsAtStart = addedBuiltins.size();
        for (const auto& [fn, count] : usage->fCallCounts) {
            if (!fn->isBuiltin() || count == 0) {
                // Not a built-in; skip it.
                continue;
            }
            const ProgramElement* added = context.fBuiltins->findAndInclude(fn->description());
            if (!added) {
                // This built-in has already been dealt with; skip it.
                continue;
            }
            addedBuiltins.push_back(&added->as<FunctionDefinition>());
        }

        if (addedBuiltins.size() == numBuiltinsAtStart) {
            // If we didn't reference any more built-in functions than before, we're done.
            break;
        }

        // Sort the referenced builtin functions into a consistent order; otherwise our output will
        // become non-deterministic. Order by position so that our order generally matches the order
        // in the source (reversed, because we re-reverse it at the end).
        std::sort(addedBuiltins.begin() + numBuiltinsAtStart,
                  addedBuiltins.end(),
                  [](const FunctionDefinition* a, const FunctionDefinition* b) {
                      SkASSERT(a->fPosition != b->fPosition);
                      return a->fPosition > b->fPosition;
                  });

        // Update the ProgramUsage to track all these newly discovered functions.
        int usageCallCounts = usage->fCallCounts.count();

        for (size_t index = numBuiltinsAtStart; index < addedBuiltins.size(); ++index) {
            usage->add(*addedBuiltins[index]);
        }

        if (usage->fCallCounts.count() == usageCallCounts) {
            // If we aren't making any more unique function calls than before, we're done.
            break;
        }
    }

    // Insert the new functions into the program's shared elements, right at the front.
    // They are added in reverse so that the deepest dependencies are added to the top.
    program.fSharedElements.insert(program.fSharedElements.begin(),
                                   addedBuiltins.rbegin(), addedBuiltins.rend());
}

}  // namespace SkSL
