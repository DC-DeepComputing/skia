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

static bool compare(const SkSL::Variable* a, const SkSL::Variable* b) {
    int setA = a->modifiers().fLayout.fSet;
    int setB = b->modifiers().fLayout.fSet;
    int bindingA = a->modifiers().fLayout.fBinding;
    int bindingB = b->modifiers().fLayout.fBinding;
    if (setA > setB) {
        return false;
    }
    if (setA < setB) {
        return true;
    }
    return bindingA < bindingB;
}

SkTArray<const SkSL::Variable*> Analysis::GetComputeShaderMainParams(const Context& context,
        const Program& program) {
    SkTArray<const SkSL::Variable*> result;
    for (const ProgramElement* e : program.elements()) {
        if (e->is<GlobalVarDeclaration>()) {
            const Variable& var =
                    e->as<GlobalVarDeclaration>().declaration()->as<VarDeclaration>().var();
            if (var.modifiers().fFlags & Modifiers::kUniform_Flag) {
                context.fErrors->error(var.fPosition,
                        "'uniform' variables are not permitted in compute shaders");
            }
            if ((var.modifiers().fFlags & (Modifiers::kIn_Flag | Modifiers::kOut_Flag)) &&
                    var.type().typeKind() != Type::TypeKind::kTexture) {
                if (var.modifiers().fLayout.fBinding != -1 && var.modifiers().fLayout.fSet != -1) {
                    if (!var.type().isArray()) {
                        context.fErrors->error(var.fPosition,
                                "unsupported compute shader in / out type");
                    }
                    result.push_back(&var);
                } else if (var.modifiers().fLayout.fBuiltin == -1) {
                    context.fErrors->error(var.fPosition,
                            "compute shader in / out variables must have a layout binding and set");
                }
            }
        }
    }
    std::sort(result.begin(), result.end(), compare);
    for (size_t i = 1; i < result.size(); ++i) {
        const Layout& layout1 = result[i]->modifiers().fLayout;
        const Layout& layout2 = result[i - 1]->modifiers().fLayout;
        if (layout1.fSet == layout2.fSet && layout1.fBinding == layout2.fBinding) {
            Position p = result[i - 1]->fPosition > result[i]->fPosition ? result[i - 1]->fPosition
                                                                         : result[i]->fPosition;
            context.fErrors->error(p,
                    "location(set=" + std::to_string(layout1.fSet) + ", binding=" +
                    std::to_string(layout1.fBinding) + ") has already been defined");
        }
    }
    return result;
}

}  // namespace SkSL
