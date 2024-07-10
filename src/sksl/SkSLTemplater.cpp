/*
 * Copyright 2023 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/SkSLTemplater.h"
#include "src/sksl/analysis/SkSLProgramVisitor.h"
#include "src/sksl/ir/SkSLFunctionDefinition.h"
#include "src/sksl/ir/SkSLProgramElement.h"
#include "src/sksl/ir/SkSLSymbolTable.h"
#include "src/sksl/ir/SkSLVarDeclarations.h"
#include "src/sksl/ir/SkSLVariableReference.h"

namespace SkSL {

using namespace skia_private;

bool Templater::specialize(Program& program) {
    this->findSpecializations(program);
    this->replaceSpecializedFunctions(program);
    return false;
}

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

        Searcher(Program& program,
                 SpecializationMap& specializationMap,
                 SpecializedCalls& specializedCalls)
            : fProgram(program)
            , fSpecializationMap(specializationMap)
            , fSpecializedCalls(specializedCalls) {}

        bool visitExpression(const Expression& expr) override {
            if (expr.is<FunctionCall>()) {
                const FunctionCall& call = expr.as<FunctionCall>();
                const FunctionDeclaration& decl = call.function();
                const FunctionDefinition* func = decl.definition();
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

                        Specializations& specializations = fSpecializationMap[&decl];

                        for (const SpecializedParameters& entry : specializations) {
                            if (maps_are_equal(specialization, entry)) {
                                // This specialization has already been tracked.
                                return false;
                            }
                        }

                        // Set the index to the corresponding specialization this function call
                        // requires.
                        fSpecializedCalls[&call] = specializations.size();
                        specializations.push_back(specialization);

                        fInheritedSpecializations.swap(specialization);

                        std::string specializedName;
                        for (auto [left, right] : specialization) {
                            specializedName += left->name();
                        }
                        TArray<Variable*> newParams;
                        for (Variable* const entry : decl.parameters()) {
                            // Don't add specialized parameters
                            if (specialization.find(entry)) {
                                continue;
                            }
                            newParams.push_back(entry);
                        }
                        FunctionDeclaration* specializedDecl =
                            fProgram.fSymbols->add(*fProgram.fContext,
                                    std::make_unique<FunctionDeclaration>(
                                        *fProgram.fContext,
                                        decl.position(),
                                        decl.modifierFlags(),
                                        decl.name(),
                                        specializedName,
                                        newParams,
                                        &decl.returnType(),
                                        decl.intrinsicKind()));
                        std::unique_ptr<FunctionDefinition> specializedFunc =
                            std::make_unique<FunctionDefinition>(
                                    func->position(),
                                    specializedDecl,
                                    func->isBuiltin(),
                                    func->body()
                                    );
                        fProgram.fSymbols->addWithoutOwnership(*fProgram.fContext,
                                                               specializedDecl.get());
                        fProgram.fOwnedElements.push_back(std::move(specializedFunc));

                        this->visitProgramElement(*decl.definition());

                        fInheritedSpecializations.swap(specialization);
                    }
                }
            }
            return INHERITED::visitExpression(expr);
        }

    private:
        Program& fProgram;
        SpecializationMap& fSpecializationMap;
        SpecializedCalls& fSpecializedCalls;
        SpecializedParameters fInheritedSpecializations;
    };

    for (const std::unique_ptr<ProgramElement>& elem : program.fOwnedElements) {
        if (elem->is<FunctionDefinition>() &&
            elem->as<FunctionDefinition>().declaration().isMain()) {
            // Visit through main callstack aggregated any necessary template specializations.
            Searcher(program, fSpecializationMap, fSpecializedCalls).visitProgramElement(*elem);
            break;
        }
    }
    return true;
}

bool Templater::replaceSpecializedFunctions(Program& program) {
    for (const std::unique_ptr<ProgramElement>& elem : program.fOwnedElements) {
        if (elem->is<FunctionDefinition>()) {
            const FunctionDefinition& func = elem->as<FunctionDefinition>();
            const FunctionDeclaration& decl = func.declaration();
            Specializations* specializations = fSpecializationMap.find(&decl);

            if (!specializations) {
                continue;
            }
        }
    }

    return true;
}

}  // namespace SkSL

