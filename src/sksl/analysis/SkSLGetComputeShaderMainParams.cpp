/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkTypes.h"
#include "include/private/SkSLLayout.h"
#include "include/private/SkSLModifiers.h"
#include "include/private/SkSLProgramElement.h"
#include "include/private/SkSLStatement.h"
#include "include/private/SkTArray.h"
#include "include/sksl/SkSLErrorReporter.h"
#include "src/sksl/SkSLAnalysis.h"
#include "src/sksl/SkSLContext.h"
#include "src/sksl/ir/SkSLProgram.h"
#include "src/sksl/ir/SkSLType.h"
#include "src/sksl/ir/SkSLVarDeclarations.h"
#include "src/sksl/ir/SkSLVariable.h"

#include <memory>
#include <string>

namespace SkSL {

SkTArray<const SkSL::Variable*> Analysis::GetComputeShaderMainParams(const Context& context,
        const Program& program) {
    // Since we're going to fill a vector mapping locations to vars, we impose a limit on the number
    // of locations to prevent us from trying to allocate gigabytes in the face of something like
    // layout(location=1000000000)
    static const int MAX_LOCATION = 100;
    SkTArray<const SkSL::Variable*> result;
    for (const ProgramElement* e : program.elements()) {
        if (e->is<GlobalVarDeclaration>()) {
            const Variable& var =
                    e->as<GlobalVarDeclaration>().declaration()->as<VarDeclaration>().var();
            if (var.modifiers().fFlags & Modifiers::kUniform_Flag) {
                context.fErrors->error(var.fPosition,
                        "'uniform' variables are not permitted in compute shaders");
            }
            if (var.modifiers().fFlags & (Modifiers::kIn_Flag | Modifiers::kOut_Flag)) {
                int location = var.modifiers().fLayout.fLocation;
                if (location == -1) {
                    if (var.modifiers().fLayout.fBuiltin == -1 &&
                            var.type().typeKind() != Type::TypeKind::kTexture) {
                        context.fErrors->error(var.fPosition,
                                "'in'/'out' variables must have a layout(location)");
                    }
                } else if (location > MAX_LOCATION) {
                    context.fErrors->error(var.fPosition,
                            "layout(location) must be less than or equal to " +
                            std::to_string(MAX_LOCATION));
                } else {
                    SkASSERT(location >= 0);
                    while ((int)result.count() <= location) {
                        result.push_back(nullptr);
                    }
                    if (result[location]) {
                        context.fErrors->error(var.fPosition,
                                "both '" + std::string(result[location]->name()) + "' and '" +
                                std::string(var.name()) + "' have layout(location=" +
                                std::to_string(location) + ")");
                    }
                    result[location] = &var;
                }
            }
        }
    }
    for (int i = 0; i < result.count(); ++i) {
        if (!result[i]) {
            int next = i + 1;
            while (!result[next]) {
                ++next;
            }
            context.fErrors->error(result[next]->fPosition, "found layout(location=" +
                    std::to_string(next) + "), but no layout(location=" + std::to_string(i) + ")");
            // We only report the first missing location, because a single location definition could
            // result in tons of missing location slots.
            // Return an empty list to avoid nulls in the output.
            return {};
        }
    }
    return result;
}

}  // namespace SkSL
