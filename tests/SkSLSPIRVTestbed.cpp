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
    SkSL::Program::Settings settings;
    std::unique_ptr<SkSL::Program> program = compiler.convertProgram(kind, std::string(src),
                                                                     settings);
    if (!program) {
        SkDebugf("Unexpected error compiling %s\n%s", src, compiler.errorText().c_str());
        REPORTER_ASSERT(r, program);
    } else {
        std::string output;
        REPORTER_ASSERT(r, compiler.toSPIRV(*program, &output));
    }
}

DEF_TEST(SkSLSPIRVTestbed, r) {
    // Add in your SkSL here.
    test(r,
         *SkSL::ShaderCapsFactory::Default(),
         R"__SkSL__(
uniform half4 colorGreen, colorRed;
uniform half4 testInputs;

half4 main(float2 coords) {
    bool ok = true;

    const half4 vec1234 = half4(1, 2, 3, 4);

    ok = ok && half2x2(testInputs)         == half2x2(-1.25, 0, 0.75, 2.25);
    ok = ok && half2x2(half4(1, 2, 3, 4))  == half2x2(1, 2, 3, 4);
    ok = ok && half2x2(vec1234)            == half2x2(1, 2, 3, 4);
    ok = ok && half2x2(half4(2, 0, 0, 2))  == half2x2(2);
    ok = ok && half2x2(half4(2))           == half2x2(2, 2, 2, 2);

    ok = ok && float2x2(testInputs)        == float2x2(-1.25, 0, 0.75, 2.25);
    ok = ok && float2x2(half4(1, 2, 3, 4)) == float2x2(1, 2, 3, 4);
    ok = ok && float2x2(vec1234)           == float2x2(1, 2, 3, 4);
    ok = ok && float2x2(half4(4, 0, 0, 4)) == float2x2(4);
    ok = ok && float2x2(half4(4))          == float2x2(4, 4, 4, 4);

    ok = ok && half2x2(colorGreen)         == half2x2(0, 1, 0, 1);
    ok = ok && half2x2(colorGreen)         == half2x2(int4(0, 1, 0, 1));
    ok = ok && half2x2(int4(colorGreen))   == half2x2(int4(0, 1, 0, 1));
    ok = ok && half2x2(0, 1, 0, 1)         == half2x2(int4(0, 1, 0, 1));
    ok = ok && half2x2(8, 8, 8, 8)         == half2x2(int4(8));
    ok = ok && half2x2(8)                  == half2x2(int4(8, 0, 0, 8));

    ok = ok && half2x2(colorGreen)               == half2x2(false, true, false, true);
    ok = ok && half2x2(colorGreen)               == half2x2(bool4(false, true, false, true));
    ok = ok && half2x2(bool4(colorGreen))        == half2x2(bool4(false, true, false, true));
    ok = ok && half2x2(false, true, false, true) == half2x2(bool4(false, true, false, true));
    ok = ok && half2x2(true, false, false, true) == half2x2(bool4(true, false, false, true));
    ok = ok && half2x2(true, true, true, true)   == half2x2(bool4(true));

    return ok ? colorGreen : colorRed;
}
         )__SkSL__");
}
