/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/FactoryFunctions.h"

#include "src/gpu/graphite/KeyContext.h"
#include "src/gpu/graphite/KeyHelpers.h"
#include "src/gpu/graphite/PaintParams.h"
#include "src/gpu/graphite/PaintParamsKey.h"
#include "src/gpu/graphite/precompile/PrecompileBaseComplete.h"

#include "src/gpu/graphite/precompile/PrecompileBlenderPriv.h"
#include "src/gpu/graphite/precompile/PrecompileShaderPriv.h"

namespace skgpu::graphite {

//--------------------------------------------------------------------------------------------------
class PrecompileYUVImageShader : public PrecompileShader {
public:
    PrecompileYUVImageShader() {}

private:
    // non-cubic and cubic sampling
    inline static constexpr int kNumIntrinsicCombinations = 2;

    int numIntrinsicCombinations() const override { return kNumIntrinsicCombinations; }

    void addToKey(const KeyContext& keyContext,
                  PaintParamsKeyBuilder* builder,
                  PipelineDataGatherer* gatherer,
                  int desiredCombination) const override {
        SkASSERT(desiredCombination < kNumIntrinsicCombinations);

        static constexpr SkSamplingOptions kDefaultCubicSampling(SkCubicResampler::Mitchell());
        static constexpr SkSamplingOptions kDefaultSampling;

        YUVImageShaderBlock::ImageData imgData(desiredCombination == 1 ? kDefaultCubicSampling
                                                                       : kDefaultSampling,
                                               SkTileMode::kClamp, SkTileMode::kClamp,
                                               SkISize::MakeEmpty(), SkRect::MakeEmpty());

        YUVImageShaderBlock::AddBlock(keyContext, builder, gatherer, imgData);
    }
};

sk_sp<PrecompileShader> PrecompileShaders::YUVImage() {
    return sk_make_sp<PrecompileYUVImageShader>();
}

} // namespace skgpu::graphite
