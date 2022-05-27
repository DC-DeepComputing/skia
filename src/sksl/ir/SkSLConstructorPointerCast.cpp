/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/sksl/SkSLErrorReporter.h"
#include "src/sksl/SkSLProgramSettings.h"
#include "src/sksl/ir/SkSLConstructorPointerCast.h"
#include "src/sksl/ir/SkSLLiteral.h"

namespace SkSL {

std::unique_ptr<Expression> ConstructorPointerCast::Convert(const Context& context,
                                                            Position pos,
                                                            const Type& type,
                                                            ExpressionArray args) {
    // As you might expect, pointer-cast constructors should only be created with pointer types.
    SkASSERT(type.isScalar());

    if (args.size() != 1) {
        context.fErrors->error(pos, "invalid arguments to '" + type.displayName() +
                "' constructor, (expected exactly 1 argument, but found " +
                std::to_string(args.size()) + ")");
        return nullptr;
    }

    const Type& argType = args[0]->type();
    if (!argType.canCoerceTo(type, false)) {
        context.fErrors->error(pos, "cannot cast '" + argType.displayName() +
                "' to '" + type.displayName() + "'");
        return nullptr;
    }
    return ConstructorPointerCast::Make(context, pos, type, std::move(args[0]));
}

std::unique_ptr<Expression> ConstructorPointerCast::Make(const Context& context,
                                                         Position pos,
                                                         const Type& type,
                                                         std::unique_ptr<Expression> arg) {
    SkASSERT(type.isPointer());
    SkASSERT(arg->type().canCoerceTo(type, false));

    // No cast required when the types match.
    if (arg->type().matches(type)) {
        return arg;
    }
    return std::make_unique<ConstructorPointerCast>(pos, type, std::move(arg));
}

}  // namespace SkSL
