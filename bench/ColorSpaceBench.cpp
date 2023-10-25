/*
 * Copyright 2023 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "bench/Benchmark.h"
#include "include/core/SkColorSpace.h"
#include "modules/skcms/skcms.h"
#include "src/core/SkColorSpaceXformSteps.h"
#include "src/core/SkRasterPipeline.h"
#include "src/core/SkRasterPipelineOpContexts.h"
#include "src/core/SkRasterPipelineOpList.h"

#include <functional>

class ColorSpaceTransformBench : public Benchmark {
public:
    ColorSpaceTransformBench() {}

protected:
    const char* onGetName() override {
        return "ColorSpaceTransform";
    }

    // Call before draw, allows the benchmark to do setup work outside of the
    // timer. When a benchmark is repeatedly drawn, this should be called once
    // before the initial draw.
    void onDelayedSetup() override {
        // Set the image buffer to solid white.
        std::fill(std::begin(fSrcPixels), std::end(fSrcPixels), 0xCC);

        // Create a conversion from sRGB to Adobe.
        fSRGBCS = SkColorSpace::MakeSRGB();
        fAdobeCS = SkColorSpace::MakeRGB(SkNamedTransferFn::k2Dot2, SkNamedGamut::kAdobeRGB);

        fXformSteps = SkColorSpaceXformSteps(fSRGBCS.get(),  kPremul_SkAlphaType,
                                             fAdobeCS.get(), kPremul_SkAlphaType);

        // Build up a pipeline.
        fSrcCtx = SkRasterPipeline_MemoryCtx{fSrcPixels, kWidth};
        fDstCtx = SkRasterPipeline_MemoryCtx{fDstPixels, kWidth};

        fPipeline.append(SkRasterPipelineOp::load_8888, &fSrcCtx);
        fXformSteps.apply(&fPipeline);
        fPipeline.append(SkRasterPipelineOp::store_8888, &fDstCtx);

        fConversionPipeline = fPipeline.compile();
    }

    void onDraw(int loops, SkCanvas* canvas) override {
        for (int i = 0; i < loops; i++) {
            fConversionPipeline(0, 0, kWidth, kHeight);
        }
    }

private:
    using INHERITED = Benchmark;

    sk_sp<SkColorSpace> fAdobeCS;
    sk_sp<SkColorSpace> fSRGBCS;

    SkRasterPipeline_<256> fPipeline;
    SkRasterPipeline_MemoryCtx fSrcCtx;
    SkRasterPipeline_MemoryCtx fDstCtx;
    SkColorSpaceXformSteps fXformSteps;

    static constexpr int kWidth = 512;
    static constexpr int kHeight = 512;
    static constexpr int kBytesPerPixel = 4;
    uint8_t fSrcPixels[kWidth * kHeight * kBytesPerPixel];
    uint8_t fDstPixels[kWidth * kHeight * kBytesPerPixel];

    std::function<void(size_t, size_t, size_t, size_t)> fConversionPipeline;
};

class SkcmsTransformBench : public Benchmark {
public:
    SkcmsTransformBench() {}

protected:
    const char* onGetName() override {
        return "SkcmsTransform";
    }

    // Call before draw, allows the benchmark to do setup work outside of the
    // timer. When a benchmark is repeatedly drawn, this should be called once
    // before the initial draw.
    void onDelayedSetup() override {
        // Set the image buffer to solid white. NANs/denorms might affect timing, but otherwise it
        // doesn't really matter.
        std::fill(std::begin(fSrcPixels), std::end(fSrcPixels), 0xDD);

        // Create profiles for sRGB to Adobe.
        SkColorSpace::MakeSRGB()->toProfile(&fSRGBProfile);
        SkColorSpace::MakeRGB(SkNamedTransferFn::k2Dot2,
                              SkNamedGamut::kAdobeRGB)->toProfile(&fAdobeProfile);
    }

    void onDraw(int loops, SkCanvas* canvas) override {
        for (int i = 0; i < loops; i++) {
            skcms_Transform(fSrcPixels,
                            skcms_PixelFormat_RGBA_8888,
                            skcms_AlphaFormat_PremulAsEncoded,
                            &fSRGBProfile,
                            fDstPixels,
                            skcms_PixelFormat_RGBA_8888,
                            skcms_AlphaFormat_PremulAsEncoded,
                            &fAdobeProfile,
                            kWidth * kHeight);
        }
    }

private:
    using INHERITED = Benchmark;

    skcms_ICCProfile fAdobeProfile;
    skcms_ICCProfile fSRGBProfile;

    static constexpr int kWidth = 512;
    static constexpr int kHeight = 512;
    static constexpr int kBytesPerPixel = 4;
    uint8_t fSrcPixels[kWidth * kHeight * kBytesPerPixel];
    uint8_t fDstPixels[kWidth * kHeight * kBytesPerPixel];
};

DEF_BENCH(return new ColorSpaceTransformBench;)
DEF_BENCH(return new SkcmsTransformBench;)
