/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/Test.h"

#if defined(SK_GRAPHITE)

#include "include/core/SkColorSpace.h"
#include "include/effects/SkRuntimeEffect.h"
#include "include/gpu/graphite/precompile/PrecompileBlender.h"
#include "include/gpu/graphite/precompile/PrecompileColorFilter.h"
#include "include/gpu/graphite/precompile/PrecompileRuntimeEffect.h"
#include "include/gpu/graphite/precompile/PrecompileShader.h"
#include "src/gpu/graphite/ContextPriv.h"
#include "src/gpu/graphite/KeyContext.h"
#include "src/gpu/graphite/PipelineData.h"
#include "src/gpu/graphite/PrecompileInternal.h"
#include "src/gpu/graphite/Renderer.h"
#include "src/gpu/graphite/RuntimeEffectDictionary.h"
#include "src/gpu/graphite/precompile/PaintOptionsPriv.h"

#include <array>

using namespace::skgpu::graphite;

namespace {

void no_blend_mode_option_test(const KeyContext& keyContext,
                               PipelineDataGatherer* gatherer,
                               skiatest::Reporter* reporter) {
    PaintOptions paintOptions;
    paintOptions.setShaders({ PrecompileShaders::Color() });


    REPORTER_ASSERT(reporter, paintOptions.priv().numCombinations() == 1);

    std::vector<UniquePaintParamsID> precompileIDs;
    paintOptions.priv().buildCombinations(keyContext,
                                          gatherer,
                                          DrawTypeFlags::kNone,
                                          /* withPrimitiveBlender= */ false,
                                          Coverage::kNone,
                                          [&precompileIDs](UniquePaintParamsID id,
                                                           DrawTypeFlags,
                                                           bool /* withPrimitiveBlender */,
                                                           Coverage) {
                                                               precompileIDs.push_back(id);
                                                           });

    SkASSERT(precompileIDs.size() == 1);
}

} // anonymous namespace

DEF_GRAPHITE_TEST_FOR_ALL_CONTEXTS(ChromeTest, reporter, context,
                                   CtsEnforcement::kNextRelease) {
    ShaderCodeDictionary* dict = context->priv().shaderCodeDictionary();

    auto rtEffectDict = std::make_unique<RuntimeEffectDictionary>();

    SkColorInfo ci(kRGBA_8888_SkColorType, kPremul_SkAlphaType, nullptr);
    KeyContext keyContext(context->priv().caps(),
                          dict,
                          rtEffectDict.get(),
                          ci,
                          /* dstTexture= */ nullptr,
                          /* dstOffset= */ {0, 0});

    PipelineDataGatherer gatherer(Layout::kMetal);

    no_blend_mode_option_test(keyContext, &gatherer, reporter);
}

#endif // SK_GRAPHITE
