/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkTypes.h"
#include "src/sksl/SkSLCompiler.h"
#include "src/sksl/SkSLProgramKind.h"
#include "src/sksl/SkSLProgramSettings.h"
#include "src/sksl/SkSLUtil.h"
#include "src/sksl/ir/SkSLProgram.h"
#include "tests/Test.h"

#include <memory>
#include <string>

static void test(skiatest::Reporter* r,
                 const char* src,
                 SkSL::ProgramKind kind = SkSL::ProgramKind::kFragment) {
    SkSL::Compiler compiler(SkSL::ShaderCapsFactory::Default());
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
         R"__SkSL__(
uniform half4 a, b;
uniform uint2 c, d;
uniform int3 e, f;
void main() {
    const int4 int4_zero = int4(0);
    bool4 expectTTFF = equal(half4(3, 3, 3.25, 100), half4(3));
    bool4 expectFFTT = equal(int4_zero, int4(-100, -50, 0, 0));
    bool4 expectTTTT = equal(bool4(1), bool4(7));

    sk_FragColor.x = equal(a, b).x ? 1 : 0;
    sk_FragColor.y = equal(c, d).y ? 1 : 0;
    sk_FragColor.z = equal(e, f).z ? 1 : 0;
    sk_FragColor.w = any(expectTTFF) || any(expectFFTT) || any(expectTTTT) ? 1 : 0;
}
         )__SkSL__");
}
