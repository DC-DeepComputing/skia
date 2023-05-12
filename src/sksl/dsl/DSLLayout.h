/*
 * Copyright 2021 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_DSL_LAYOUT
#define SKSL_DSL_LAYOUT

#include "src/sksl/SkSLPosition.h"
#include "src/sksl/ir/SkSLLayout.h"

namespace SkSL {

namespace dsl {

class DSLLayout {
public:
    DSLLayout() {}

    explicit DSLLayout(SkSL::Layout skslLayout)
        : fSkSLLayout(skslLayout) {}

private:
    SkSL::Layout fSkSLLayout;

    friend class DSLModifiers;
};

} // namespace dsl

} // namespace SkSL

#endif
