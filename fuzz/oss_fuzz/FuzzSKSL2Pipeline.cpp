/*
 * Copyright 2019 Google, LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ganesh/GrShaderCaps.h"
#include "src/sksl/SkSLCompiler.h"
#include "src/sksl/codegen/SkSLPipelineStageCodeGenerator.h"
#include "src/sksl/ir/SkSLProgram.h"
#include "src/sksl/ir/SkSLVarDeclarations.h"
#include "src/sksl/ir/SkSLVariable.h"

#include "fuzz/Fuzz.h"
#include "fuzz/oss_fuzz/FuzzSkSLShared.h"

bool FuzzSKSL2Pipeline(SkSpan<const uint8_t> data) {
    // Clean up the fuzzer input by stripping control characters and replacing high-ASCII characters
    // with SkSL tokens.
    SkString sksl;
    TransformInputToSkSL(&data, &sksl,
                         ControlCharacterBehavior::kRemove,
                         HighBitBehavior::kReplaceWithToken);

    SkSL::Compiler compiler(SkSL::ShaderCapsFactory::Default());
    SkSL::ProgramSettings settings;
    std::unique_ptr<SkSL::Program> program = compiler.convertProgram(SkSL::ProgramKind::kRuntimeShader,
                                                                     std::string(sksl.c_str()),
                                                                     settings);
    if (!program) {
        return false;
    }

    class Callbacks : public SkSL::PipelineStage::Callbacks {
        std::string declareUniform(const SkSL::VarDeclaration* decl) override {
            return std::string(decl->var().name());
        }

        void defineFunction(const char* /*decl*/, const char* /*body*/, bool /*isMain*/) override {}
        void declareFunction(const char* /*decl*/) override {}
        void defineStruct(const char* /*definition*/) override {}
        void declareGlobal(const char* /*declaration*/) override {}

        std::string sampleShader(int index, std::string coords) override {
            return "child_" + std::to_string(index) + ".eval(" + coords + ")";
        }

        std::string sampleColorFilter(int index, std::string color) override {
            return "child_" + std::to_string(index) + ".eval(" + color + ")";
        }

        std::string sampleBlender(int index, std::string src, std::string dst) override {
            return "child_" + std::to_string(index) + ".eval(" + src + ", " + dst + ")";
        }

        std::string toLinearSrgb(std::string color) override { return color; }
        std::string fromLinearSrgb(std::string color) override { return color; }
    };

    Callbacks callbacks;
    SkSL::PipelineStage::ConvertProgram(*program, "coords", "inColor", "half4(1)", &callbacks);
    return true;
}

bool FuzzSKSL2Pipeline(sk_sp<SkData> fuzz) {
    FuzzSKSL2Pipeline(SkSpan(fuzz->bytes(), fuzz->size()));
    return true;
}

#if defined(SK_BUILD_FOR_LIBFUZZER)
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    if (size > 3000) {
        return 0;
    }
    FuzzSKSL2Pipeline(SkSpan<const uint8_t>(data, size));
    return 0;
}
#endif
