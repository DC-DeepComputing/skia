/*
 * Copyright 2019 Google, LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ganesh/GrShaderCaps.h"
#include "src/sksl/SkSLCompiler.h"
#include "src/sksl/ir/SkSLProgram.h"

#include "fuzz/Fuzz.h"
#include "fuzz/oss_fuzz/FuzzSkSLShared.h"

bool FuzzSKSL2GLSL(SkSpan<const uint8_t> data) {
    // Clean up the fuzzer input by stripping control characters and replacing high-ASCII characters
    // with SkSL tokens.
    SkString sksl;
    TransformInputToSkSL(&data, &sksl,
                         ControlCharacterBehavior::kRemove,
                         HighBitBehavior::kReplaceWithToken);

    SkSL::Compiler compiler(SkSL::ShaderCapsFactory::Default());
    SkSL::ProgramSettings settings;
    std::unique_ptr<SkSL::Program> program = compiler.convertProgram(SkSL::ProgramKind::kFragment,
                                                                     std::string(sksl.c_str()),
                                                                     settings);
    std::string output;
    if (!program || !compiler.toGLSL(*program, &output)) {
        return false;
    }
    return true;
}

bool FuzzSKSL2GLSL(sk_sp<SkData> fuzz) {
    FuzzSKSL2GLSL(SkSpan(fuzz->bytes(), fuzz->size()));
    return true;
}

#if defined(SK_BUILD_FOR_LIBFUZZER)
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    if (size > 3000) {
        return 0;
    }
    FuzzSKSL2GLSL(SkSpan<const uint8_t>(data, size));
    return 0;
}
#endif
