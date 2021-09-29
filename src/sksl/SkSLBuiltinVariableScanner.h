/*
 * Copyright 2021 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_BUILTINVARIABLESCANNER
#define SKSL_BUILTINVARIABLESCANNER

#include "include/private/SkSLProgramKind.h"
#include "src/sksl/SkSLAnalysis.h"

#include <vector>

namespace SkSL {

class Context;
class ProgramElement;
class String;

class BuiltinVariableScanner : public ProgramVisitor {
public:
    static void FindAndDeclareBuiltinVariables(const Context& context, ProgramKind programKind,
            std::vector<const ProgramElement*>& sharedElements);

private:
    BuiltinVariableScanner(const Context& context)
        : fContext(context) {}

    void addDeclaringElement(const String& name);

    bool visitProgramElement(const ProgramElement& pe) override;

    bool visitExpression(const Expression& e) override;

    const Context& fContext;
    std::vector<const ProgramElement*> fNewElements;
    bool fPreserveFragColor = false;

    using INHERITED = ProgramVisitor;
    using INHERITED::visitProgramElement;
};

} // namespace SkSL

#endif
