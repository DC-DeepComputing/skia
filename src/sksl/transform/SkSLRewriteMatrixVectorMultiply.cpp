/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/SkSLOperator.h"
#include "src/sksl/SkSLPosition.h"
#include "src/sksl/ir/SkSLExpression.h"
#include "src/sksl/ir/SkSLIRHelpers.h"
#include "src/sksl/ir/SkSLType.h"
#include "src/sksl/transform/SkSLTransform.h"

#include <memory>
#include <utility>

namespace SkSL {

class Context;

std::unique_ptr<Expression> Transform::RewriteMatrixVectorMultiply(const Context& context,
                                                                   Position pos,
                                                                   const Expression& left,
                                                                   const Expression& right) {
    // Rewrite m33 * v3 as (m[0] * v[0] + m[1] * v[1] + m[2] * v[2])
    IRHelpers helpers(context);

    std::unique_ptr<Expression> sum;
    for (int n = 0; n < left.type().rows(); ++n) {
        // Get mat[N] with an index expression.
        std::unique_ptr<Expression> matN = helpers.Index(left.clone(), helpers.Int(n));

        // Get vec[N] with another index expression.
        std::unique_ptr<Expression> vecN = helpers.Index(right.clone(), helpers.Int(n));

        // Multiply them together.
        std::unique_ptr<Expression> product = helpers.Binary(std::move(matN), OperatorKind::STAR,
                                                             std::move(vecN));
        // Sum all the components together.
        if (!sum) {
            sum = std::move(product);
        } else {
            sum = helpers.Binary(std::move(sum), OperatorKind::PLUS, std::move(product));
        }
    }

    return sum;
}

}  // namespace SkSL
