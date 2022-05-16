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

static bool shouldUseVariableLengthLoop(const GrShaderCaps& caps) {
    // If we're in reduced-shader mode, and we can use variable length loops, then use a uniform to
    // limit the number of iterations, so we don't need a code variation for each width.
    return (caps.generation() >= SkSL::GLSLGeneration::k300es && caps.reducedShaderMode());
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

    // Assemble a gaussian kernel and offset list.
    float kernel[kMaxKernelWidth];
    float offsets[kMaxKernelWidth];
    SkGpuBlurUtils::Compute1DLinearGaussianKernel(kernel, offsets, gaussianSigma, halfWidth);

    // For variable-length loops, size the kernel uniform for the maximum width so we can reuse the
    // same code for any kernel width.
    bool variableLengthLoop = shouldUseVariableLengthLoop(*caps.shaderCaps());
    int arrayCount = variableLengthLoop ? SkGpuBlurUtils::LinearKernelWidth(kMaxKernelRadius)
                                        : width;

    // Interleave the kernel and offsets into an array of SkV2s ("OffsetsAndKernel").
    SkV2 offsetsAndKernel[kMaxKernelWidth] = {};
    for (int index = 0; index < arrayCount; ++index) {
        offsetsAndKernel[index] = {offsets[index], kernel[index]};
    }

    SkV2 increment = (dir == Direction::kX) ? SkV2{1, 0}
                                            : SkV2{0, 1};

    // Generate shader code based on our loop type.
    SkString shaderText;
    SkRuntimeEffect::Options options;
    if (variableLengthLoop) {
        // Variable-length looping sends kernel width via a uniform. This requires ES3
        // support because the loop bounds are no longer known at compile time.
        shaderText = SkStringPrintf("const int ArrayWidth = %d;"
                                    "uniform int KernelWidth;",
                                    arrayCount);
        options = SkRuntimeEffectPriv::ES3Options();
    } else {
        // Fixed-length looping bakes the kernel width directly into the code.
        shaderText = SkStringPrintf("const int ArrayWidth = %d;"
                                    "const int KernelWidth = ArrayWidth;",
                                    arrayCount);
    }

    shaderText += R"(
      uniform shader child;
      uniform half2 Increment;
      uniform half2 OffsetsAndKernel[ArrayWidth];

      half4 Smooth(float2 coord, half2 offsetsAndKernel) {
          return child.eval(coord + offsetsAndKernel.x * Increment) * offsetsAndKernel.y;
      }
      half4 main(float2 coord) {
          half4 color = half4(0);
          for (int i = 0; i < KernelWidth; ++i) {
              color += Smooth(coord, OffsetsAndKernel[i]);
          }
          return color;
      }
    )";

    sk_sp<SkRuntimeEffect> effect = SkMakeRuntimeEffect(SkRuntimeEffect::MakeForShader,
                                                        shaderText.c_str(), options);

    return GrSkSLFP::Make(effect, "GaussianConvolution", /*inputFP=*/nullptr,
                          GrSkSLFP::OptFlags::kPreservesOpaqueInput |
                          GrSkSLFP::OptFlags::kCompatibleWithCoverageAsAlpha,
                          "child", std::move(child),
                          "Increment", increment,
                          "OffsetsAndKernel", SkMakeSpan(offsetsAndKernel, arrayCount),
                          "KernelWidth", GrSkSLFP::When(variableLengthLoop, width));
}
