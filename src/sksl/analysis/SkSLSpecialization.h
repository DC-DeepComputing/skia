/*
 * Copyright 2023 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_SPECIALIZATION
#define SKSL_SPECIALIZATION

#include "src/core/SkTHash.h"
#include "src/sksl/ir/SkSLFunctionCall.h"
#include "src/sksl/ir/SkSLProgram.h"
#include "src/sksl/ir/SkSLVariable.h"

#include <functional>

namespace SkSL {

namespace Analysis {

struct SpecializedCallHash {
    size_t operator()(const std::pair<const FunctionCall*, int32_t>& entry) {
        return SkGoodHash()(entry.first) ^ SkGoodHash()(entry.second);
    }
};

using SpecializationIndex = size_t;
static constexpr SpecializationIndex kUnspecialized = -1;

// Global uniforms used by a specialization, maps <function parameter, global uniform>
using SpecializedParameters = skia_private::THashMap<const Variable*, const Variable*>;
// The set of specializated implementations needed for a given function.
using Specializations = skia_private::TArray<SpecializedParameters>;
// The full set of all specializations required by the program.
using SpecializationMap = skia_private::THashMap<const FunctionDeclaration*, Specializations>;
// The mapping of function calls and their inherited specialization to their corresponding
// specialization index in `Specializations`
using SpecializedCalls =
    skia_private::THashMap<std::pair<const FunctionCall*, SpecializationIndex>,
                           SpecializationIndex,
                           SpecializedCallHash>;

struct SpecializationInfo {
    SpecializationMap fSpecializationMap;
    SpecializedCalls fSpecializedCalls;
};

// A function that returns true if the parameter variable fits the criteria
// to create a specialization.
using SpecializationFn = std::function<bool(const Variable&)>;

// Finds functions that contain parameters that should be specialized on and writes the
// specialization info to the provided `SpecializationInfo`.
void FindFunctionsToSpecialize(const Program& program,
                               const SpecializationFn& specializationFn,
                               SpecializationInfo* info);

}  // namespace Analysis
}  // namespace SkSL

#endif
