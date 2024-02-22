/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkKnownRuntimeEffects.h"

#include "include/effects/SkRuntimeEffect.h"
#include "src/core/SkRuntimeEffectPriv.h"
#include "src/gpu/BlurUtils.h"

namespace {

SkRuntimeEffect* make_blur_1D_effect(int kernelWidth) {
    SkASSERT(kernelWidth <= skgpu::kMaxBlurSamples);
    // The SkSL structure performs two kernel taps; if the kernel has an odd width the last
    // sample will be skipped with the current loop limit calculation.
    SkASSERT(kernelWidth % 2 == 0);
    return SkMakeRuntimeEffect(SkRuntimeEffect::MakeForShader,
            SkStringPrintf(
                    // The coefficients are always stored for the max radius to keep the
                    // uniform block consistent across all effects.
                    "const int kMaxUniformKernelSize = %d / 2;"
                    // But we generate an exact loop over the kernel size. Note that this
                    // program can be used for kernels smaller than the constructed max as long
                    // as the kernel weights for excess entries are set to 0.
                    "const int kMaxLoopLimit = %d / 2;"

                    "uniform half4 offsetsAndKernel[kMaxUniformKernelSize];"
                    "uniform half2 dir;"

                    "uniform shader child;"

                    "half4 main(float2 coord) {"
                        "half4 sum = half4(0);"
                        "for (int i = 0; i < kMaxLoopLimit; ++i) {"
                            "half4 s = offsetsAndKernel[i];"
                            "sum += s.y * child.eval(coord + s.x*dir);"
                            "sum += s.w * child.eval(coord + s.z*dir);"
                        "}"
                        "return sum;"
                    "}", skgpu::kMaxBlurSamples, kernelWidth).c_str());
}

SkRuntimeEffect* make_blur_2D_effect(int maxKernelSize) {
    SkASSERT(maxKernelSize % 4 == 0);
    return SkMakeRuntimeEffect(SkRuntimeEffect::MakeForShader,
            SkStringPrintf(
                    // The coefficients are always stored for the max radius to keep the
                    // uniform block consistent across all effects.
                    "const int kMaxUniformKernelSize = %d / 4;"
                    "const int kMaxUniformOFfsetsSize = 2*kMaxUniformKernelSize;"
                    // But we generate an exact loop over the kernel size. Note that this
                    // program can be used for kernels smaller than the constructed max as long
                    // as the kernel weights for excess entries are set to 0.
                    "const int kMaxLoopLimit = %d / 4;"

                    // Pack scalar coefficients into half4 for better packing on std140, and
                    // upload offsets to avoid having to transform the 1D index into a 2D coord
                    "uniform half4 kernel[kMaxUniformKernelSize];"
                    "uniform half4 offsets[kMaxUniformOFfsetsSize];"

                    "uniform shader child;"

                    "half4 main(float2 coord) {"
                        "half4 sum = half4(0);"

                        "for (int i = 0; i < kMaxLoopLimit; ++i) {"
                            "half4 k = kernel[i];"
                            "half4 o = offsets[2*i];"
                            "sum += k.x * child.eval(coord + o.xy);"
                            "sum += k.y * child.eval(coord + o.zw);"
                            "o = offsets[2*i + 1];"
                            "sum += k.z * child.eval(coord + o.xy);"
                            "sum += k.w * child.eval(coord + o.zw);"
                        "}"
                        "return sum;"
                    "}", skgpu::kMaxBlurSamples, maxKernelSize).c_str());
}

} // anonymous namespace

const SkRuntimeEffect* GetKnownRuntimeEffect(StableKey stableKey) {

    SkRuntimeEffect::Options options;
    SkRuntimeEffectPriv::SetStableKey(&options, static_cast<uint32_t>(stableKey));

    switch (stableKey) {
        case StableKey::kInvalid:
            return nullptr;
        case StableKey::kBlend: {
            static SkRuntimeEffect* sBlendEffect =
                    SkMakeRuntimeEffect(SkRuntimeEffect::MakeForShader,
                                        "uniform shader s, d;"
                                        "uniform blender b;"
                                        "half4 main(float2 xy) {"
                                            "return b.eval(s.eval(xy), d.eval(xy));"
                                        "}",
                                        options);
            return sBlendEffect;
        }
        case StableKey::k1DBlur4: {
            static SkRuntimeEffect* s1DBlurEffect = make_blur_1D_effect(4);
            return s1DBlurEffect;
        }
        case StableKey::k1DBlur8: {
            static SkRuntimeEffect* s1DBlurEffect = make_blur_1D_effect(8);
            return s1DBlurEffect;
        }
        case StableKey::k1DBlur12: {
            static SkRuntimeEffect* s1DBlurEffect = make_blur_1D_effect(12);
            return s1DBlurEffect;
        }
        case StableKey::k1DBlur16: {
            static SkRuntimeEffect* s1DBlurEffect = make_blur_1D_effect(16);
            return s1DBlurEffect;
        }
        case StableKey::k1DBlur20: {
            static SkRuntimeEffect* s1DBlurEffect = make_blur_1D_effect(20);
            return s1DBlurEffect;
        }
        case StableKey::k1DBlur28: {
            static SkRuntimeEffect* s1DBlurEffect = make_blur_1D_effect(28);
            return s1DBlurEffect;
        }
        case StableKey::k2DBlur4: {
            static SkRuntimeEffect* s2DBlurEffect = make_blur_2D_effect(4);
            return s2DBlurEffect;
        }
        case StableKey::k2DBlur8: {
            static SkRuntimeEffect* s2DBlurEffect = make_blur_2D_effect(8);
            return s2DBlurEffect;
        }
        case StableKey::k2DBlur12: {
            static SkRuntimeEffect* s2DBlurEffect = make_blur_2D_effect(12);
            return s2DBlurEffect;
        }
        case StableKey::k2DBlur16: {
            static SkRuntimeEffect* s2DBlurEffect = make_blur_2D_effect(16);
            return s2DBlurEffect;
        }
        case StableKey::k2DBlur20: {
            static SkRuntimeEffect* s2DBlurEffect = make_blur_2D_effect(20);
            return s2DBlurEffect;
        }
        case StableKey::k2DBlur28: {
            static SkRuntimeEffect* s2DBlurEffect = make_blur_2D_effect(28);
            return s2DBlurEffect;
        }
    }

    SkUNREACHABLE;
}
