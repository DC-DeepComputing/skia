/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkTypes.h"
#include "include/private/SkSLProgramKind.h"
#include "src/sksl/SkSLCompiler.h"
#include "src/sksl/SkSLUtil.h"
#include "src/sksl/ir/SkSLProgram.h"
#include "tests/Test.h"

#include <memory>
#include <string>

static void test(skiatest::Reporter* r,
                 const SkSL::ShaderCaps& caps,
                 const char* src,
                 SkSL::ProgramKind kind = SkSL::ProgramKind::kFragment) {
    SkSL::Compiler compiler(&caps);
    SkSL::ProgramSettings settings;
    std::unique_ptr<SkSL::Program> program = compiler.convertProgram(kind, std::string(src),
                                                                     settings);
    if (!program) {
        SkDebugf("Unexpected error compiling %s\n%s", src, compiler.errorText().c_str());
        REPORTER_ASSERT(r, program);
    } else {
        std::string output;
        REPORTER_ASSERT(r, compiler.toGLSL(*program, &output));
        REPORTER_ASSERT(r, output != "");
        //SkDebugf("GLSL output:\n\n%s", output.c_str());
    }
}

DEF_TEST(SkSLGLSLTestbed, r) {
    // Add in your SkSL here.
    test(r,
         *SkSL::ShaderCapsFactory::Default(),
         R"__SkSL__(
uniform half4 colorGreen, colorRed;

float get() {
    @switch (2) {
        case 1: return abs(1);
        case 2: return abs(2);  // Only this case should be preserved.
        case 3: return abs(3);
        case 4: return abs(4);
    }
    // This won't be removed because dead-code elimination is disabled.
    return abs(5);
}

half4 main(float2 coords) {
    float result = get();
    return result == 2 ? colorGreen : colorRed;
}
         )__SkSL__");
}
