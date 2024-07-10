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
#include "src/sksl/ir/SkSLProgram.h"
#include "src/sksl/ir/SkSLVariable.h"

namespace SkSL {

class Templater {
public:
    // Global uniforms used by a specialization, maps <function parameter, global uniform>
    using SpecializedParameters = skia_private::THashMap<const Variable*, const Variable*>;
    // The set of specializated implementations needed for a given function.
    using Specializations = skia_private::TArray<SpecializedParameters>;
    // The full set of all specializations required by the program.
    using SpecializationMap = skia_private::THashMap<const FunctionDeclaration*, Specializations>;
    // The mapping of function calls to their corresponding specialization
    // index in `Specializations`
    using SpecializedCalls = skia_private::THashMap<const FunctionCall*, size_t>;

    bool specialize(Program& program);

    bool findSpecializations(Program& program);

    bool replaceSpecializedFunctions(Program& program);
private:
    SpecializationMap fSpecializationMap;
    SpecializedCalls fSpecializedCalls;
};
}

#endif
