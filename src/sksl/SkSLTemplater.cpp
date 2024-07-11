/*
 * Copyright 2023 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/SkSLTemplater.h"
#include "src/sksl/analysis/SkSLProgramVisitor.h"
#include "src/sksl/ir/SkSLFunctionDefinition.h"
#include "src/sksl/ir/SkSLProgram.h"
#include "src/sksl/ir/SkSLProgramElement.h"
#include "src/sksl/ir/SkSLSymbolTable.h"
#include "src/sksl/ir/SkSLVarDeclarations.h"
#include "src/sksl/ir/SkSLVariableReference.h"

namespace SkSL {

using namespace skia_private;

template <typename K, typename V>
static bool maps_are_equal(const THashMap<K, V>& left, const THashMap<K, V>& right) {
    if (left.count() != right.count()) {
        return false;
    }
    for (const auto& [key, leftValue] : left) {
        const V* rightValue = right.find(key);
        if (!rightValue) {
            return false;
        }
        if (leftValue != *rightValue) {
            return false;
        }
    }
    return true;
}

bool Templater::findSpecializations(Program& program) {
    class Searcher : public ProgramVisitor {
    public:
        using ProgramVisitor::visitProgramElement;
        using INHERITED = ProgramVisitor;

        Searcher(Program& program)
            : fSpecializationMap(program.fSpecializationMap)
            , fSpecializedCalls(program.fSpecializedCalls) {}

        bool visitExpression(const Expression& expr) override {
            if (expr.is<FunctionCall>()) {
                const FunctionCall& call = expr.as<FunctionCall>();
                const FunctionDeclaration& decl = call.function();
                if (!decl.isIntrinsic()) {
                    SpecializedParameters specialization;

                    const int numParams = decl.parameters().size();
                    SkASSERT(call.arguments().size() == numParams);
                    for (int i = 0; i < numParams; i++) {
                        const Expression& arg = *call.arguments()[i];
                        if (!arg.is<VariableReference>()) {
                            continue;
                        }

                        const Variable* var = arg.as<VariableReference>().variable();
                        const Variable* param = decl.parameters()[i];
                        const Type& varType = var->type();

                        if (!varType.isSampler()) {
                            continue;
                        }

                        if (var->storage() == Variable::Storage::kGlobal) {
                            specialization[param] = var;
                        } else if (var->storage() == Variable::Storage::kParameter) {
                            const Variable** uniformVar = fInheritedSpecializations.find(var);
                            SkASSERT(uniformVar);
                            specialization[param] = *uniformVar;
                        } else {
                            SK_ABORT("Specialization requires a uniform or parameter sampler");
                        }
                    }

                    // Only create specialization if there are variables to specialize on.
                    if (specialization.count() > 0) {
                        Specializations& specializations = fSpecializationMap[&decl];

                        for (int i = 0; i < specializations.size(); i++) {
                            const SpecializedParameters& entry = specializations[i];
                            if (maps_are_equal(specialization, entry)) {
                                // This specialization has already been tracked.
                                fSpecializedCalls[{&call, fInheritedSpecializationIndex}] = i;
                                return false;
                            }
                        }

                        // Set the index to the corresponding specialization this function call
                        // requires.
                        SpecializationIndex specializationIndex = specializations.size();
                        fSpecializedCalls[{&call, fInheritedSpecializationIndex}]
                                = specializationIndex;
                        specializations.push_back(specialization);

                        fInheritedSpecializations.swap(specialization);
                        std::swap(fInheritedSpecializationIndex, specializationIndex);

                        this->visitProgramElement(*decl.definition());

                        std::swap(fInheritedSpecializationIndex, specializationIndex);
                        fInheritedSpecializations.swap(specialization);
                    }
                }
            }
            return INHERITED::visitExpression(expr);
        }

    private:
        SpecializationMap& fSpecializationMap;
        SpecializedCalls& fSpecializedCalls;
        SpecializedParameters fInheritedSpecializations;
        SpecializationIndex fInheritedSpecializationIndex = kUnspecialized;
    };

    for (const std::unique_ptr<ProgramElement>& elem : program.fOwnedElements) {
        if (elem->is<FunctionDefinition>() &&
            elem->as<FunctionDefinition>().declaration().isMain()) {
            // Visit through main callstack aggregated any necessary template specializations.
            Searcher(program).visitProgramElement(*elem);
            break;
        }
    }
    return true;
}

}  // namespace SkSL

