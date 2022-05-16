/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ganesh/effects/GrGaussianConvolutionFragmentProcessor.h"

#include "include/effects/SkRuntimeEffect.h"
#include "src/core/SkGpuBlurUtils.h"
#include "src/core/SkRuntimeEffectPriv.h"
#include "src/gpu/KeyBuilder.h"
#include "src/gpu/ganesh/GrTexture.h"
#include "src/gpu/ganesh/GrTextureProxy.h"
#include "src/gpu/ganesh/effects/GrSkSLFP.h"
#include "src/gpu/ganesh/effects/GrTextureEffect.h"

// For brevity
using Direction = GrGaussianConvolutionFragmentProcessor::Direction;

enum class LoopType {
    kUnrolled,
    kFixedLength,
    kVariableLength,
};

static LoopType loop_type(const GrShaderCaps& caps) {
    // This checks that bitwise integer operations and array indexing by non-consts are allowed.
    if (caps.generation() < SkSL::GLSLGeneration::k130) {
        return LoopType::kUnrolled;
    }
    // If we're in reduced shader mode and we can have a loop then use a uniform to limit the
    // number of iterations so we don't need a code variation for each width.
    return caps.reducedShaderMode() ? LoopType::kVariableLength : LoopType::kFixedLength;
}

std::unique_ptr<GrFragmentProcessor> GrGaussianConvolutionFragmentProcessor::Make(
        GrSurfaceProxyView view,
        SkAlphaType alphaType,
        Direction dir,
        int halfWidth,
        float gaussianSigma,
        GrSamplerState::WrapMode wm,
        const SkIRect& subset,
        const SkIRect* pixelDomain,
        const GrCaps& caps) {
    // We should sample as nearest if there will be no shader to preserve existing behaviour, but
    // the linear blur requires a linear sample.
    bool isZeroSigma = SkGpuBlurUtils::IsEffectivelyZeroSigma(gaussianSigma);
    GrSamplerState::Filter filter = isZeroSigma ? GrSamplerState::Filter::kNearest
                                                : GrSamplerState::Filter::kLinear;
    GrSamplerState sampler(wm, filter);
    if (isZeroSigma) {
        halfWidth = 0;
    }
    // It's pretty common to blur a subset of an input texture. In reduced shader mode we always
    // apply the wrap mode in the shader.
    std::unique_ptr<GrFragmentProcessor> child;
    bool alwaysUseShaderTileMode = caps.reducedShaderMode();
    if (pixelDomain && !alwaysUseShaderTileMode) {
        // Inset because we expect to be invoked at pixel centers.
        SkRect domain = SkRect::Make(*pixelDomain).makeInset(0.5, 0.5f);
        switch (dir) {
            case Direction::kX: domain.outset(halfWidth, 0); break;
            case Direction::kY: domain.outset(0, halfWidth); break;
        }
        child = GrTextureEffect::MakeSubset(std::move(view),
                                            alphaType,
                                            SkMatrix::I(),
                                            sampler,
                                            SkRect::Make(subset),
                                            domain,
                                            caps,
                                            GrTextureEffect::kDefaultBorder);
    } else {
        child = GrTextureEffect::MakeSubset(std::move(view),
                                            alphaType,
                                            SkMatrix::I(),
                                            sampler,
                                            SkRect::Make(subset),
                                            caps,
                                            GrTextureEffect::kDefaultBorder,
                                            alwaysUseShaderTileMode);
    }

    if (isZeroSigma) {
        return child;
    }

    static constexpr int kMaxKernelWidth = kMaxKernelRadius + 1;
    SkASSERT(halfWidth <= kMaxKernelRadius);
    int width = SkGpuBlurUtils::LinearKernelWidth(halfWidth);

    // The array sizes must be a multiple of 4 because we pass it as an array of float4 uniform
    // values.
    float kernel[SkAlign4(kMaxKernelWidth)];
    float offsets[SkAlign4(kMaxKernelWidth)];
    SkGpuBlurUtils::Compute1DLinearGaussianKernel(kernel, offsets, gaussianSigma, halfWidth);

    LoopType loopType = loop_type(*caps.shaderCaps());
    int arrayCount;
    if (loopType == LoopType::kVariableLength) {
        // For variable-length loops, size the kernel uniform for the maximum width.
        arrayCount = (SkGpuBlurUtils::LinearKernelWidth(kMaxKernelRadius) + 3) / 4;
    } else {
        // For fixed-length loops, size the kernel uniform based on the amount we need.
        arrayCount = (width + 3) / 4;
        SkASSERT(4 * arrayCount >= width);
    }

    SkV2 increment = (dir == Direction::kX) ? SkV2{1, 0}
                                            : SkV2{0, 1};

    // Generate shader code based on our loop type.
    SkString shaderText = SkStringPrintf(R"(
      const int ArrayWidth = %d;
      uniform shader child;
      uniform half2 Increment;
      uniform half4 Kernel[ArrayWidth];
      uniform half4 Offsets[ArrayWidth];
    )", arrayCount);

    if (loopType == LoopType::kUnrolled) {
        // In the unrolled case, we emit a separate call to eval() for each kernel entry.
        // This path also has the advantage of being ES2 compatible (no integer bit-and).
        shaderText += R"(
          half4 main(float2 coord) {
              half4 color = half4(0);
        )";

        for (int i = 0; i < width; ++i) {
            shaderText.appendf(
                    "color += child.eval(coord + Offsets[%d][%d] * Increment) * Kernel[%d][%d];\n",
                    i / 4, i & 3, i / 4, i & 3);
        }

        shaderText += R"(
              return color;
          }
        )";

        sk_sp<SkRuntimeEffect> effect = SkMakeRuntimeEffect(SkRuntimeEffect::MakeForShader,
                                                            shaderText.c_str());

        return GrSkSLFP::Make(effect, "GaussianConvolution", /*inputFP=*/nullptr,
                              GrSkSLFP::OptFlags::kPreservesOpaqueInput |
                              GrSkSLFP::OptFlags::kCompatibleWithCoverageAsAlpha,
                              "child", std::move(child),
                              "Increment", increment,
                              "Kernel", SkMakeSpan<SkV4>((SkV4*)kernel, arrayCount),
                              "Offsets", SkMakeSpan<SkV4>((SkV4*)offsets, arrayCount));
    }

    // For the other cases, we emit a SkSL loop. The "fixed length" loop bakes in the kernel width
    // with a specialization variable; the "variable length" loop sends kernel width via a uniform.
    shaderText += R"(
      uniform int KernelWidth;

      half4 main(float2 coord) {
          half4 color = half4(0);
          for (int i = 0; i < KernelWidth; ++i) {
              color += child.eval(coord + Offsets[i / 4][i & 3] * Increment) * Kernel[i / 4][i & 3];
          }
          return color;
      }
    )";

    sk_sp<SkRuntimeEffect> effect = SkMakeRuntimeEffect(SkRuntimeEffect::MakeForShader,
                                                        shaderText.c_str(),
                                                        SkRuntimeEffectPriv::ES3Options());

    return GrSkSLFP::Make(effect, "GaussianConvolution", /*inputFP=*/nullptr,
                          GrSkSLFP::OptFlags::kPreservesOpaqueInput |
                          GrSkSLFP::OptFlags::kCompatibleWithCoverageAsAlpha,
                          "child", std::move(child),
                          "Increment", increment,
                          "Kernel", SkMakeSpan((SkV4*)kernel, arrayCount),
                          "Offsets", SkMakeSpan((SkV4*)offsets, arrayCount),
                          "KernelWidth", GrSkSLFP::SpecializeIf(loopType == LoopType::kFixedLength,
                                                                width));
}
