/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_NOP
#define SKSL_NOP

#include "include/private/SkSLStatement.h"
#include "include/sksl/SkSLPosition.h"
#include "src/sksl/ir/SkSLSymbolTable.h"

namespace SkSL {

/**
 * A no-op statement that does nothing.
 */
class Nop final : public Statement {
public:
    inline static constexpr Kind kStatementKind = Kind::kNop;

    Nop(Position pos) : INHERITED(pos, kStatementKind) {}

    static std::unique_ptr<Statement> Make(Position pos = {}) {
        return std::make_unique<Nop>(pos);
    }

    bool isEmpty() const override {
        return true;
    }

    std::string description() const override {
        return ";";
    }

    std::unique_ptr<Statement> clone() const override {
        return std::make_unique<Nop>(this->fPosition);
    }

private:
    using INHERITED = Statement;
};

}  // namespace SkSL

#endif
