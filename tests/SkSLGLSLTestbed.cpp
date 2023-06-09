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
                 SkSL::ProgramKind kind = SkSL::ProgramKind::kRuntimeShader) {
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
uniform half4 colorGreen, colorRed;

bool switch_fallthrough(int value) {
    bool ok = false;
    switch (value) {
        case 2:  break;
        case 1:
        case 0:  ok = true; break;
        default: break;
    }
    return ok;
}

bool switch_fallthrough_twice(int value) {
    bool ok = false;
    switch (value) {
        case 0:  break;
        case 1:
        case 2:
        case 3:  ok = true; break;
        default: break;
    }
    return ok;
}

bool switch_fallthrough_with_vardecls(int value) {
    switch (value) {
        case 0:  bool a;
        case 1:  bool b = true;
        case 2:  a = b;
        case 3:  return a; break;
    }
    return false;
}

half4 main(float2 coords) {
    int x = int(colorGreen.g);
    return (switch_fallthrough(x) &&
            switch_fallthrough_twice(x) &&
            switch_fallthrough_with_vardecls(x)) ? colorGreen : colorRed;
}
         )__SkSL__");
}
