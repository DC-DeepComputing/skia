/*
 * Copyright 2023 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/private/SkGainmapShader.h"

#include "include/private/SkGainmapInfo.h"
#include "src/core/SkColorFilterBase.h"
#include "src/core/SkColorSpaceXformSteps.h"
#include "src/core/SkEffectPriv.h"
#include "src/core/SkRasterPipeline.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkWriteBuffer.h"

#if SK_SUPPORT_GPU
#include "src/gpu/ganesh/GrColorInfo.h"
#include "src/gpu/ganesh/GrColorSpaceXform.h"
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////
// SkColorSpaceXformColorFilter

namespace {

class SkColorSpaceXformColorFilter final : public SkColorFilterBase {
public:
    static sk_sp<SkColorSpaceXformColorFilter> Make(sk_sp<SkColorSpace> src,
                                                    SkAlphaType srcAlphaType,
                                                    sk_sp<SkColorSpace> dst,
                                                    SkAlphaType dstAlphaType) {
        if (!src || !dst) {
            return nullptr;
        }
        return sk_ref_sp(new SkColorSpaceXformColorFilter(src, srcAlphaType, dst, dstAlphaType));
    }

#if SK_SUPPORT_GPU
    GrFPResult asFragmentProcessor(std::unique_ptr<GrFragmentProcessor> inputFP,
                                   GrRecordingContext* context,
                                   const GrColorInfo& dstColorInfo,
                                   const SkSurfaceProps& props) const override {
        return GrFPSuccess(GrColorSpaceXformEffect::Make(
                std::move(inputFP), fSrc.get(), fSrcAlphaType, fDst.get(), fDstAlphaType));
    }
#endif

    bool onAppendStages(const SkStageRec& rec, bool shaderIsOpaque) const override {
        if (!shaderIsOpaque) {
            rec.fPipeline->append(SkRasterPipelineOp::unpremul);
        }

        fSteps.apply(rec.fPipeline);

        if (!shaderIsOpaque) {
            rec.fPipeline->append(SkRasterPipelineOp::premul);
        }
        return true;
    }

    skvm::Color onProgram(skvm::Builder* p,
                          skvm::Color c,
                          const SkColorInfo& dst,
                          skvm::Uniforms* uniforms,
                          SkArenaAlloc* alloc) const override {
        return premul(fSteps.program(p, uniforms, unpremul(c)));
    }

protected:
    void flatten(SkWriteBuffer& buffer) const override {
        buffer.writeDataAsByteArray(fSrc->serialize().get());
        buffer.write32(fSrcAlphaType);
        buffer.writeDataAsByteArray(fDst->serialize().get());
        buffer.write32(fDstAlphaType);
    }

private:
    SkColorSpaceXformColorFilter(sk_sp<SkColorSpace> src,
                                 SkAlphaType srcAlphaType,
                                 sk_sp<SkColorSpace> dst,
                                 SkAlphaType dstAlphaType)
            : fSrc(std::move(src))
            , fSrcAlphaType(srcAlphaType)
            , fDst(std::move(dst))
            , fDstAlphaType(dstAlphaType)
            , fSteps(fSrc.get(), fSrcAlphaType, fDst.get(), fDstAlphaType) {}

    SK_FLATTENABLE_HOOKS(SkColorSpaceXformColorFilter)

    const sk_sp<SkColorSpace> fSrc;
    const SkAlphaType fSrcAlphaType;
    const sk_sp<SkColorSpace> fDst;
    const SkAlphaType fDstAlphaType;
    SkColorSpaceXformSteps fSteps;

    friend class SkColorFilter;
    using INHERITED = SkColorFilterBase;
};

sk_sp<SkFlattenable> SkColorSpaceXformColorFilter::CreateProc(SkReadBuffer& buffer) {
    sk_sp<SkColorSpace> colorSpaces[2];
    SkAlphaType alphaTypes[2];
    for (int i = 0; i < 2; ++i) {
        auto data = buffer.readByteArrayAsData();
        if (buffer.validate(data != nullptr)) {
            return nullptr;
        }
        colorSpaces[i] = SkColorSpace::Deserialize(data->data(), data->size());
        if (!buffer.validate(colorSpaces[i] != nullptr)) {
            return nullptr;
        }
        int32_t alphaType = buffer.read32();
        if (!buffer.validate(alphaType >= 0 && alphaType <= kLastEnum_SkAlphaType)) {
            return nullptr;
        }
        alphaTypes[i] = static_cast<SkAlphaType>(alphaType);
    }
    return SkColorSpaceXformColorFilter::Make(
            colorSpaces[0], alphaTypes[0], colorSpaces[1], alphaTypes[1]);
}

}  // namespace

///////////////////////////////////////////////////////////////////////////////////////////////////
// Gainmap shader

static constexpr char gGainmapApplySKSL[] =
        "uniform shader sdr;"
        "uniform shader gainmap;"
        "uniform half4 logGainmapMin;"
        "uniform half4 logGainmapMax;"
        "uniform half4 gainmapGamma;"
        "uniform half epsilonSdr;"
        "uniform half epsilonHdr;"
        "uniform half W;"
        ""
        "half4 main(float2 coord) {"
        "half4 S = sdr.eval(coord);"
        "half4 G = gainmap.eval(coord);"
        "half3 L = mix(logGainmapMin.rgb, logGainmapMax.rgb, pow(G.rgb, gainmapGamma.rgb));"
        "half3 H = (S.rgb + epsilonSdr) * exp(L * W) - epsilonHdr;"
        "return half4(H.r, H.g, H.b, S.a);"
        "}";

static sk_sp<SkRuntimeEffect> gainmap_apply_effect() {
    static const SkRuntimeEffect* effect =
            SkRuntimeEffect::MakeForShader(SkString(gGainmapApplySKSL), {}).effect.release();
    SkASSERT(effect);
    return sk_ref_sp(effect);
}

sk_sp<SkShader> SkDrawImageWithGainmapShader(sk_sp<SkImage> sdrImage,
                                             const SkRect& sdrRect,
                                             const SkSamplingOptions& sdrSamplingOptions,
                                             sk_sp<SkImage> gainmapImage,
                                             const SkRect& gainmapRect,
                                             const SkSamplingOptions& gainmapSamplingOptions,
                                             const SkGainmapInfo& gainmapInfo,
                                             const SkRect& dstRect,
                                             sk_sp<SkColorSpace> dstColorSpace,
                                             float dstHdrRatio) {
    sk_sp<SkColorSpace> sdrColorSpace =
            sdrImage->colorSpace() ? sdrImage->refColorSpace() : SkColorSpace::MakeSRGB();
    sk_sp<SkColorSpace> gainmapMathColorSpace = sdrColorSpace->makeLinearGamma();
    if (!dstColorSpace) {
        dstColorSpace = SkColorSpace::MakeSRGB();
    }

    // Create a color filter to transform from the SDR image's color space and alpha type to the
    // color space in which the gainmap is to be applied.
    auto colorXformSdrToGainmap = SkColorSpaceXformColorFilter::Make(
            sdrColorSpace, kPremul_SkAlphaType, gainmapMathColorSpace, kUnpremul_SkAlphaType);

    // Create a color filter to transform from the color space in which the gainmap is applied to
    // the destination color space.
    auto colorXformGainmapToDst = SkColorSpaceXformColorFilter::Make(
            gainmapMathColorSpace, kUnpremul_SkAlphaType, dstColorSpace, kPremul_SkAlphaType);

    // The SDR image shader will convert into the color space in which the gainmap is applied.
    const SkMatrix sdrRectToDstRect = SkMatrix::RectToRect(sdrRect, dstRect);
    auto sdrImageShader = sdrImage->reinterpretColorSpace(dstColorSpace)
                                  ->makeShader(sdrSamplingOptions, &sdrRectToDstRect)
                                  ->makeWithColorFilter(colorXformSdrToGainmap);

    // The gainmap image shader will ignore any color space that the gainmap has.
    const SkMatrix gainmapRectToDstRect = SkMatrix::RectToRect(gainmapRect, dstRect);
    auto gainmapImageShader = gainmapImage->reinterpretColorSpace(dstColorSpace)
                                      ->makeShader(gainmapSamplingOptions, &gainmapRectToDstRect);

    // Create the shader to apply the gainmap.
    SkRuntimeShaderBuilder builder(gainmap_apply_effect());
    builder.child("sdr") = sdrImageShader;
    builder.child("gainmap") = gainmapImageShader;
    const SkColor4f logGainmapMin({sk_float_log(gainmapInfo.fGainmapMin.fR),
                                   sk_float_log(gainmapInfo.fGainmapMin.fG),
                                   sk_float_log(gainmapInfo.fGainmapMin.fB),
                                   1.f});
    const SkColor4f logGainmapMax({sk_float_log(gainmapInfo.fGainmapMax.fR),
                                   sk_float_log(gainmapInfo.fGainmapMax.fG),
                                   sk_float_log(gainmapInfo.fGainmapMax.fB),
                                   1.f});
    const float Wunclamped =
            (sk_float_log(dstHdrRatio) - sk_float_log(gainmapInfo.fHdrRatioMin)) /
            (sk_float_log(gainmapInfo.fHdrRatioMax) - sk_float_log(gainmapInfo.fHdrRatioMin));
    const float W = std::max(std::min(Wunclamped, 1.f), 0.f);

    builder.uniform("logGainmapMin") = logGainmapMin;
    builder.uniform("logGainmapMax") = logGainmapMax;
    builder.uniform("gainmapGamma") = gainmapInfo.fGainmapGamma;
    builder.uniform("epsilonSdr") = gainmapInfo.fEpsilonSdr;
    builder.uniform("epsilonHdr") = gainmapInfo.fEpsilonHdr;
    builder.uniform("W") = W;
    return builder.makeShader()->makeWithColorFilter(colorXformGainmapToDst);
}
