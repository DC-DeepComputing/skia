/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkTypes.h"
#include "include/private/SkSLProgramElement.h"
#include "include/private/SkSLStatement.h"
#include "src/sksl/SkSLAnalysis.h"
#include "src/sksl/SkSLCompiler.h"
#include "src/sksl/SkSLMangler.h"
#include "src/sksl/ir/SkSLFunctionDefinition.h"
#include "src/sksl/ir/SkSLSymbolTable.h"
#include "src/sksl/ir/SkSLVarDeclarations.h"
#include "src/sksl/ir/SkSLVariableReference.h"
#include "src/sksl/transform/SkSLProgramWriter.h"
#include "src/sksl/transform/SkSLTransform.h"

#include <memory>
#include <vector>

namespace SkSL {

class Context;
class ProgramUsage;
class SymbolTable;

void Transform::RenameLocalVariables(Context& context, LoadedModule& module, ProgramUsage* usage) {

    class VariableRenamer : public ProgramWriter {
    public:
        VariableRenamer(Context& context, ProgramUsage* usage) : fContext(context), fUsage(usage) {}

        static std::string FindShortNameForSymbol(const Symbol* sym,
                                                  const SymbolTable* symbolTable) {
            // Try the first character of the name.
            std::string_view firstChar = sym->name().substr(0, 1);
            if ((*symbolTable)[firstChar] == nullptr) {
                return std::string(firstChar);
            }

            static constexpr std::string_view kLetters[] = {
                    "a", "b", "c", "d", "e", "f", "g", "h", "i", "j", "k", "l", "m",
                    "n", "o", "p", "q", "r", "s", "t", "u", "v", "w", "x", "y", "z",
                    "A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M",
                    "N", "O", "P", "Q", "R", "S", "T", "U", "V", "W", "X", "Y", "Z"};

            // Try any single-letter option.
            for (std::string_view letter : kLetters) {
                if ((*symbolTable)[letter] == nullptr) {
                    return std::string(letter);
                }
            }

            // Try two letters.
            for (std::string_view letterA : kLetters) {
                for (std::string_view letterB : kLetters) {
                    std::string name = SkSL::String::printf("%c%c", letterA[0], letterB[0]);
                    if ((*symbolTable)[name] == nullptr) {
                        return name;
                    }
                }
            }

            // We struck out, somehow.
            return std::string(sym->name());
        }

        void minifyName(const Symbol* sym) {
            // If the variable name could be shorter than it is now...
            if (sym->name().size() > 1) {
                // ... look for a shorter name...
                SymbolTable* symbols = fSymbolTableStack.back().get();
                std::string shortName = FindShortNameForSymbol(sym, symbols);
                if (shortName.size() < sym->name().size()) {
                    // ... and rename the symbol to this shorter name.
                    Symbol* mutableSym = symbols->getMutableSymbol(sym->name());
                    SkASSERT(mutableSym == sym);
                    const std::string* ownedName =
                            symbols->takeOwnershipOfString(std::move(shortName));
                    symbols->renameSymbol(mutableSym, *ownedName);
                }
            }
        }

        bool visitProgramElement(ProgramElement& elem) override {
            if (elem.is<FunctionDefinition>()) {
                // Minify the names of each function parameter.
                const FunctionDefinition& def = elem.as<FunctionDefinition>();
                Analysis::SymbolTableStackBuilder symbolTableStackBuilder(def.body().get(),
                                                                          &fSymbolTableStack);
                for (const Variable* param : def.declaration().parameters()) {
                    this->minifyName(param);
                }
            }

            return INHERITED::visitProgramElement(elem);
        }

        bool visitStatementPtr(std::unique_ptr<Statement>& stmt) override {
            Analysis::SymbolTableStackBuilder symbolTableStackBuilder(stmt.get(),
                                                                      &fSymbolTableStack);

            if (stmt->is<VarDeclaration>()) {
                // Minify the variable's name.
                VarDeclaration& decl = stmt->as<VarDeclaration>();
                this->minifyName(&decl.var());
            }

            return INHERITED::visitStatementPtr(stmt);
        }

        Context& fContext;
        ProgramUsage* fUsage;
        std::vector<std::shared_ptr<SymbolTable>> fSymbolTableStack;
        using INHERITED = ProgramWriter;
    };

    VariableRenamer visitor{context, usage};

    for (std::unique_ptr<ProgramElement>& pe : module.fElements) {
        if (pe->is<FunctionDefinition>()) {
            visitor.visitProgramElement(*pe);
        }
    }
}

}  // namespace SkSL
