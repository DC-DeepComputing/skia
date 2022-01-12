/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_SWITCHCASE
#define SKSL_SWITCHCASE

#include "include/private/SkSLStatement.h"
#include "src/sksl/ir/SkSLExpression.h"

#include <inttypes.h>

namespace SkSL {

/**
 * A single case of a 'switch' statement.
 */
class SwitchCase final : public Statement {
public:
    inline static constexpr Kind kStatementKind = Kind::kSwitchCase;

    SwitchCase(int line, SKSL_INT value, std::unique_ptr<Statement> statement)
        : INHERITED(line, kStatementKind)
        , fDefault(false)
        , fValue(std::move(value))
        , fStatement(std::move(statement)) {}

    // Default case
    SwitchCase(int line, std::unique_ptr<Statement> statement)
        : INHERITED(line, kStatementKind)
        , fDefault(true)
        , fStatement(std::move(statement)) {}

    bool isDefault() const {
        return fDefault;
    }

    SKSL_INT value() const {
        SkASSERT(!this->isDefault());
        return fValue;
    }

    std::unique_ptr<Statement>& statement() {
        return fStatement;
    }

    const std::unique_ptr<Statement>& statement() const {
        return fStatement;
    }

    std::unique_ptr<Statement> clone() const override {
        return std::make_unique<SwitchCase>(fLine,
                                            this->value(),
                                            this->statement()->clone());
    }

    String description() const override {
        if (this->isDefault()) {
            return String::printf("default:\n%s", fStatement->description().c_str());
        } else {
            return String::printf("case %" PRId64 ":\n%s",
                                  (int64_t) this->value(),
                                  fStatement->description().c_str());
        }
    }

private:
    bool fDefault;
    SKSL_INT fValue;
    std::unique_ptr<Statement> fStatement;

    using INHERITED = Statement;
};

}  // namespace SkSL

#endif
