/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_CONSTRUCTOR_POINTER_CAST
#define SKSL_CONSTRUCTOR_POINTER_CAST

#include "include/private/SkSLDefines.h"
#include "src/sksl/SkSLContext.h"
#include "src/sksl/ir/SkSLConstructor.h"
#include "src/sksl/ir/SkSLExpression.h"

#include <memory>

namespace SkSL {

/**
 * Represents the construction of a pointer cast, such as `int*(intArrayVariable)`.
 */
class ConstructorPointerCast final : public SingleArgumentConstructor {
public:
    inline static constexpr Kind kExpressionKind = Kind::kConstructorPointerCast;

    ConstructorPointerCast(Position pos, const Type& type, std::unique_ptr<Expression> arg)
        : INHERITED(pos, kExpressionKind, &type, std::move(arg)) {}

    // ConstructorPointerCast::Convert will typecheck and create pointer-constructor expressions.
    // Reports errors via the ErrorReporter; returns null on error.
    static std::unique_ptr<Expression> Convert(const Context& context,
                                               Position pos,
                                               const Type& rawType,
                                               ExpressionArray args);

    // ConstructorPointerCast::Make casts a scalar expression. Errors reported via SkASSERT.
    static std::unique_ptr<Expression> Make(const Context& context,
                                            Position pos,
                                            const Type& type,
                                            std::unique_ptr<Expression> arg);

    std::unique_ptr<Expression> clone() const override {
        return std::make_unique<ConstructorPointerCast>(fPosition, this->type(),
                argument()->clone());
    }

    bool isCompileTimeConstant() const override {
        return false;
    }

private:
    using INHERITED = SingleArgumentConstructor;
};

}  // namespace SkSL

#endif
