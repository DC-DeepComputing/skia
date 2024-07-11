/*
 * Copyright 2023 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_TEMPLATER
#define SKSL_TEMPLATER

#include "src/core/SkTHash.h"
#include "src/sksl/ir/SkSLFunctionCall.h"
#include "src/sksl/ir/SkSLVariable.h"

namespace SkSL {

struct Program;

struct Templater {
private:
    struct SpecializedCallHash {
        size_t operator()(const std::pair<const FunctionCall*, int32_t>& entry) {
            return SkGoodHash()(entry.first) ^ SkGoodHash()(entry.second);
        }
    };
public:
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

    static bool findSpecializations(Program& program);
};

}

#endif
