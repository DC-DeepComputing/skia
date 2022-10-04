/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkTypes.h"
#include "include/private/SkSLProgramElement.h"
#include "include/private/SkSLStatement.h"
#include "include/private/SkSLSymbol.h"
#include "include/private/SkStringView.h"
#include "src/sksl/SkSLAnalysis.h"
#include "src/sksl/SkSLCompiler.h"
#include "src/sksl/ir/SkSLFunctionDeclaration.h"
#include "src/sksl/ir/SkSLFunctionDefinition.h"
#include "src/sksl/ir/SkSLSymbolTable.h"
#include "src/sksl/ir/SkSLVarDeclarations.h"
#include "src/sksl/ir/SkSLVariable.h"
#include "src/sksl/transform/SkSLProgramWriter.h"
#include "src/sksl/transform/SkSLTransform.h"

#include <memory>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace SkSL {

class Context;
class ProgramUsage;

void Transform::RenameLocalVariables(Context& context, LoadedModule& module, ProgramUsage* usage) {

    class VariableRenamer : public ProgramWriter {
    public:
        VariableRenamer(Context& context,
                        ProgramUsage* usage,
                        std::shared_ptr<SymbolTable> symbolBase)
                : fContext(context), fUsage(usage), fSymbolTableStack({std::move(symbolBase)}) {}

        static std::string FindShortNameForSymbol(const Symbol* sym,
                                                  const SymbolTable* symbolTable,
                                                  std::string namePrefix) {
            static constexpr std::string_view kLetters[] = {
                    "a", "b", "c", "d", "e", "f", "g", "h", "i", "j", "k", "l", "m",
                    "n", "o", "p", "q", "r", "s", "t", "u", "v", "w", "x", "y", "z",
                    "A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M",
                    "N", "O", "P", "Q", "R", "S", "T", "U", "V", "W", "X", "Y", "Z"};

            // Try any single-letter option.
            for (std::string_view letter : kLetters) {
                std::string name = namePrefix + std::string(letter);
                if ((*symbolTable)[name] == nullptr) {
                    return name;
                }
            }

            // Try every two-letter option.
            for (std::string_view letterA : kLetters) {
                for (std::string_view letterB : kLetters) {
                    std::string name = namePrefix + std::string(letterA) + std::string(letterB);
                    if ((*symbolTable)[name] == nullptr) {
                        return name;
                    }
                }
            }

            // We struck out. Somehow, all 2700 two-letter names have been claimed.
            SkDEBUGFAILF("Unable to find unique name for '%s'", std::string(sym->name()).c_str());
            return std::string(sym->name());
        }

        void minifyVariableName(const Variable* var) {
            // Look for a new name for this symbol.
            // Note: we always rename _every_ variable, even ones with single-letter names. This is
            // a safeguard: if we claimed a name like `i`, and then the program itself contained an
            // `i` later on, in a nested SymbolTable, the two names would clash. By always renaming
            // everything, we can ignore that problem.
            SymbolTable* symbols = fSymbolTableStack.back().get();
            std::string shortName = FindShortNameForSymbol(var, symbols, "");
            SkASSERT(symbols->getMutableSymbol(shortName) == nullptr);

            // Update the symbol's name.
            Symbol* mutableSym = symbols->getMutableSymbol(var->name());
            SkASSERT(mutableSym == var);
            const std::string* ownedName = symbols->takeOwnershipOfString(std::move(shortName));
            symbols->renameSymbol(mutableSym, *ownedName);
        }

        void minifyPrivateFunctionName(const FunctionDeclaration* funcDecl) {
            // Look for a new name for this function.
            SymbolTable* symbols = fSymbolTableStack.back().get();
            std::string shortName = FindShortNameForSymbol(funcDecl, symbols, "$");
            SkASSERT(symbols->getMutableSymbol(shortName) == nullptr);

            if (shortName.size() < funcDecl->name().size()) {
                // Update the function's name. (If the function has overloads, this will rename all
                // of them at once.)
                Symbol* mutableSym = symbols->getMutableSymbol(funcDecl->name());
                const std::string* ownedName = symbols->takeOwnershipOfString(std::move(shortName));
                symbols->renameSymbol(mutableSym, *ownedName);
            }
        }

        bool visitProgramElement(ProgramElement& elem) override {
            if (!elem.is<FunctionDefinition>()) {
                // This transform only looks at function definitions.
                return false;
            }

            // If the function is private, minify its name.
            const FunctionDefinition& def = elem.as<FunctionDefinition>();
            if (skstd::starts_with(def.declaration().name(), '$') &&
                !(def.declaration().modifiers().fFlags & Modifiers::kExport_Flag)) {
                this->minifyPrivateFunctionName(&def.declaration());
            }

            // Minify the names of each function parameter.
            Analysis::SymbolTableStackBuilder symbolTableStackBuilder(def.body().get(),
                                                                      &fSymbolTableStack);
            for (const Variable* param : def.declaration().parameters()) {
                this->minifyVariableName(param);
            }
            return INHERITED::visitProgramElement(elem);
        }

        bool visitStatementPtr(std::unique_ptr<Statement>& stmt) override {
            Analysis::SymbolTableStackBuilder symbolTableStackBuilder(stmt.get(),
                                                                      &fSymbolTableStack);
            if (stmt->is<VarDeclaration>()) {
                // Minify the variable's name.
                VarDeclaration& decl = stmt->as<VarDeclaration>();
                this->minifyVariableName(&decl.var());
            }

            return INHERITED::visitStatementPtr(stmt);
        }

        Context& fContext;
        ProgramUsage* fUsage;
        std::vector<std::shared_ptr<SymbolTable>> fSymbolTableStack;
        using INHERITED = ProgramWriter;
    };

    VariableRenamer visitor{context, usage, module.fSymbols};

    for (std::unique_ptr<ProgramElement>& pe : module.fElements) {
        visitor.visitProgramElement(*pe);
    }
}

}  // namespace SkSL
