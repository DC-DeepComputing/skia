/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_DEFINES
#define SKSL_DEFINES

#include <cstdint>

#include "include/core/SkTypes.h"
#include "include/private/base/SkTArray.h"

using SKSL_INT = int64_t;
using SKSL_FLOAT = float;

namespace SkSL {

class Expression;
class Statement;

class ExpressionArray : public skia_private::STArray<2, std::unique_ptr<Expression>> {
public:
    using STArray::STArray;

    /** Returns a new ExpressionArray containing a clone of every element. */
    ExpressionArray clone() const;
};

using StatementArray = skia_private::STArray<2, std::unique_ptr<Statement>>;

// Functions larger than this (measured in IR nodes) will not be inlined. This growth factor
// accounts for the number of calls being inlined--i.e., a function called five times (that is, with
// five inlining opportunities) would be considered 5x larger than if it were called once. This
// default threshold value is arbitrary, but tends to work well in practice.
static constexpr int kDefaultInlineThreshold = 50;
// Modules are allowed to inline more heavily than programs since they are processed once, and
// simple utility functions are often used in many places even though in the eventual program,
// most of the usages would not be referenced.
static constexpr int kDefaultModuleInlineThreshold = 1000;

// A hard upper limit on the number of variable slots allowed in a function/global scope.
// This is an arbitrary limit, but is needed to prevent code generation from taking unbounded
// amounts of time or space. This applies only to programs, not modules.
static constexpr int kVariableSlotLimit = 100000;

}  // namespace SkSL

#endif
