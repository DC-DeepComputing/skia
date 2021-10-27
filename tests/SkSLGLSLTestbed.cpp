/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/SkSLCompiler.h"

#include "tests/Test.h"

static void test(skiatest::Reporter* r,
                 const GrShaderCaps& caps,
                 const char* src,
                 SkSL::ProgramKind kind = SkSL::ProgramKind::kFragment) {
    SkSL::Compiler compiler(&caps);
    SkSL::Program::Settings settings;
    SkSL::String output;
    std::unique_ptr<SkSL::Program> program = compiler.convertProgram(kind, SkSL::String(src),
                                                                     settings);
    if (!program) {
        SkDebugf("Unexpected error compiling %s\n%s", src, compiler.errorText().c_str());
        REPORTER_ASSERT(r, program);
    } else {
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
// Expect 8 errors

const int intMin         = -2147483648;
const int intMinMinusOne = -2147483649; // error
const int intMax         = 2147483647;
const int intMaxPlusOne  = 2147483648;  // error

void main() {
    ushort4 us4 = ushort4(-1,     // error
                          0,
                          65535,
                          65536); // error

    short4 s4 = short4(-32768,
                       -32769, // error
                       32768,  // error
                       32767);

    int4 i4 = int4(intMin,
                   intMinMinusOne,
                   intMax,
                   intMaxPlusOne);  // error

    uint4 ui4 = uint4(intMin,          // error
                      intMinMinusOne,
                      intMax,
                      intMaxPlusOne);
}
         )__SkSL__");
}
