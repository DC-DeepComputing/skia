/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/Test.h"

#include "include/effects/SkRuntimeEffect.h"
#include "include/gpu/graphite/Context.h"
#include "src/core/SkShaderCodeDictionary.h"
#include "src/gpu/graphite/ContextPriv.h"

using namespace skgpu::graphite;

namespace {

// This creates the blender DAG:
//    comboBlender    - 1 uniform ("blendFrac")
//      - redBlender  - 1 uniform ("redColor)
//      - blueBlender - 1 uniform ("blueColor)
//
// TODO: add helper functions and "specializing" uniforms
static sk_sp<SkBlender> gimme_blender() {
    sk_sp<SkBlender> redBlender;

    {
        static SkRuntimeEffect::Result red = SkRuntimeEffect::MakeForBlender(SkString(R"(
            uniform float redColor;
            half4 main(half4 src, half4 dst) {
                return half4(redColor, 0, 0, 1);
            }
        )"));

        SkRuntimeBlendBuilder builder(red.effect);
        builder.uniform("redColor") = 1.0f;
        redBlender = builder.makeBlender();
    }

    sk_sp<SkBlender> blueBlender;

    {
        static SkRuntimeEffect::Result blue = SkRuntimeEffect::MakeForBlender(SkString(R"(
            uniform float blueColor;
            half4 main(half4 src, half4 dst) {
                return half4(0, 0, blueColor, 1);
            }
        )"));

        SkRuntimeBlendBuilder builder(blue.effect);
        builder.uniform("blueColor") = 1.0f;
        blueBlender = builder.makeBlender();
    }

    sk_sp<SkBlender> linearBlender;

    {
        static SkRuntimeEffect::Result comboBlender = SkRuntimeEffect::MakeForBlender(SkString(R"(
            uniform float blendFrac;
            uniform blender a;
            uniform blender b;
            half4 main(half4 src, half4 dst) {
                return (blendFrac * a.eval(src, dst)) + ((1 - blendFrac) * b.eval(src, dst));
            }
        )"));

        SkRuntimeBlendBuilder builder(comboBlender.effect);
        builder.uniform("blendFrac") = 0.5f;
        builder.child("a") = redBlender;
        builder.child("b") = blueBlender;
        linearBlender = builder.makeBlender();
    }

    return linearBlender;
}

} // anonymous namespace

DEF_GRAPHITE_TEST_FOR_CONTEXTS(RTEffectTest, reporter, context) {
    auto dict = context->priv().shaderCodeDictionary();

    sk_sp<SkBlender> blender = gimme_blender();

    SkBlenderID id = context->addUserDefinedSnippet(blender.get());
    SkASSERT(id.isValid());

    auto entry = dict->getEntry(id);
    REPORTER_ASSERT(reporter, entry);

    // TODO:
    //   add runtime effect shaders and color filters
    //   check that the shader snippet has the expected properties: uniforms, children, functions
    //   test out "specializing" uniforms
}
