/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/private/SkSLProgramElement.h"
#include "include/private/SkSLStatement.h"
#include "include/private/SkStringView.h"
#include "src/sksl/SkSLCompiler.h"
#include "src/sksl/SkSLContext.h"
#include "src/sksl/SkSLProgramSettings.h"
#include "src/sksl/analysis/SkSLProgramUsage.h"
#include "src/sksl/ir/SkSLProgram.h"
#include "src/sksl/ir/SkSLVarDeclarations.h"
#include "src/sksl/ir/SkSLVariable.h"
#include "src/sksl/transform/SkSLTransform.h"

#include <algorithm>
#include <cstddef>
#include <memory>
#include <vector>

template <typename Container, typename Predicate>
static void SkContainerRemoveIf(Container& c, Predicate pred) {
    c.resize(std::distance(c.begin(), std::remove_if(c.begin(), c.end(), pred)));
}

namespace SkSL {

static bool is_dead_variable(const ProgramElement& element,
                             ProgramUsage* usage,
                             bool onlyPrivateGlobals) {
    if (!element.is<GlobalVarDeclaration>()) {
        return false;
    }
    const GlobalVarDeclaration& global = element.as<GlobalVarDeclaration>();
    const VarDeclaration& varDecl = global.declaration()->as<VarDeclaration>();
    if (onlyPrivateGlobals && !skstd::starts_with(varDecl.var()->name(), '$')) {
        return false;
    }
    if (!usage->isDead(*varDecl.var())) {
        return false;
    }
    return true;
}

bool Transform::EliminateDeadGlobalVariables(const Context& context,
                                             Module& module,
                                             ProgramUsage* usage,
                                             bool onlyPrivateGlobals) {
    auto isDeadVariable = [&](const ProgramElement& element) {
        return is_dead_variable(element, usage, onlyPrivateGlobals);
    };

    size_t numElements = module.fElements.size();
    if (context.fConfig->fSettings.fRemoveDeadVariables) {
        SkContainerRemoveIf(module.fElements, [&](const std::unique_ptr<ProgramElement>& pe) {
            return isDeadVariable(*pe);
        });
    }
    return module.fElements.size() < numElements;
}

bool Transform::EliminateDeadGlobalVariables(Program& program) {
    auto isDeadVariable = [&](const ProgramElement& element) {
        return is_dead_variable(element, program.fUsage.get(), /*onlyPrivateGlobals=*/false);
    };

    size_t numOwnedElements = program.fOwnedElements.size();
    size_t numSharedElements = program.fSharedElements.size();
    if (program.fConfig->fSettings.fRemoveDeadVariables) {
        SkContainerRemoveIf(program.fOwnedElements, [&](const std::unique_ptr<ProgramElement>& pe) {
            return isDeadVariable(*pe);
        });
        SkContainerRemoveIf(program.fSharedElements, [&](const ProgramElement* pe) {
            return isDeadVariable(*pe);
        });
    }
    return program.fOwnedElements.size() < numOwnedElements ||
           program.fSharedElements.size() < numSharedElements;
}

}  // namespace SkSL
