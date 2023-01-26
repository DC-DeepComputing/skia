/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/SkSLAnalysis.h"

#include "include/private/SkSLIRNode.h"
#include "include/private/SkSLStatement.h"
#include "src/sksl/analysis/SkSLProgramVisitor.h"

namespace SkSL {

class Expression;

namespace {

class HasContinueVisitor : public ProgramVisitor {
public:
    HasContinueVisitor() {}

    bool visitExpression(const Expression& expr) override {
        // We can avoid processing expressions entirely.
        return false;
    }

    bool visitStatement(const Statement& stmt) override {
        switch (stmt.kind()) {
            case Statement::Kind::kContinue:
                // We found what we were looking for.
                fHasContinue = true;
                return true;

            case Statement::Kind::kFor:
            case Statement::Kind::kDo:
            case Statement::Kind::kSwitch: {
                // A continue inside of a for-statement or do-statement would not affect the control
                // flow of this statement (any continue would affect the inner loop). SkSL does not
                // allow continues inside of a switch, so we don't need to visit them either.
                return false;
            }

            default:
                return ProgramVisitor::visitStatement(stmt);
        }
    }

    bool fHasContinue = false;
};

}  // namespace

bool Analysis::HasContinue(const Statement& stmt) {
    HasContinueVisitor visitor;
    visitor.visitStatement(stmt);
    return visitor.fHasContinue;
}

}  // namespace SkSL
