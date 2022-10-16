/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/private/SkSLProgramElement.h"
#include "src/sksl/SkSLProgramSettings.h"
#include "src/sksl/analysis/SkSLProgramUsage.h"
#include "src/sksl/ir/SkSLFunctionDeclaration.h"
#include "src/sksl/ir/SkSLFunctionDefinition.h"
#include "src/sksl/ir/SkSLProgram.h"
#include "src/sksl/transform/SkSLTransform.h"

#include <algorithm>
#include <memory>
#include <vector>

namespace SkSL {

template <typename Container, typename Predicate>
static void SkContainerRemoveIf(Container& c, Predicate pred) {
    c.resize(std::distance(c.begin(), std::remove_if(c.begin(), c.end(), pred)));
}

bool Transform::EliminateDeadFunctions(Program& program) {
    ProgramUsage* usage = program.fUsage.get();
    bool madeChanges = false;

    if (program.fConfig->fSettings.fRemoveDeadFunctions) {
        auto isDeadFunction = [&](const ProgramElement* element) {
            if (!element->is<FunctionDefinition>()) {
                return false;
            }
            const FunctionDefinition& fn = element->as<FunctionDefinition>();
            if (fn.declaration().isMain() || usage->get(fn.declaration()) > 0) {
                return false;
            }
            usage->remove(*element);
            madeChanges = true;
            return true;
        };

        SkContainerRemoveIf(program.fOwnedElements, [&](const std::unique_ptr<ProgramElement>& pe) {
            return isDeadFunction(pe.get());
        });
        SkContainerRemoveIf(program.fSharedElements, isDeadFunction);
    }
    return madeChanges;
}

}  // namespace SkSL
