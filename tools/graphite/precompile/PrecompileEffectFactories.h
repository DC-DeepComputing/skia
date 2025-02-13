/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef PrecompileEffectFactories_DEFINED
#define PrecompileEffectFactories_DEFINED

#include "include/core/SkRefCnt.h"

class SkBlender;
class SkColorFilter;
class SkRuntimeEffect;
class SkShader;

namespace skgpu::graphite {
    class PrecompileBlender;
    class PrecompileColorFilter;
    class PrecompileShader;
}

namespace skiatest::graphite {

// These factory functions returns a normal-API/precompile-API pair that represent the same
// effect.
// For SkSL-based effects the generating SkSL can be accessed via a Get*Code call.
namespace PrecompileFactories {

    SkRuntimeEffect* GetAnnulusShaderEffect();
    std::pair<sk_sp<SkShader>, sk_sp<skgpu::graphite::PrecompileShader>> CreateAnnulusRuntimeShader();

    SkRuntimeEffect* GetSrcBlenderEffect();
    std::pair<sk_sp<SkBlender>, sk_sp<skgpu::graphite::PrecompileBlender>> CreateSrcRuntimeBlender();
    SkRuntimeEffect* GetDstBlenderEffect();
    std::pair<sk_sp<SkBlender>, sk_sp<skgpu::graphite::PrecompileBlender>> CreateDstRuntimeBlender();
    SkRuntimeEffect* GetComboBlenderEffect();
    std::pair<sk_sp<SkBlender>, sk_sp<skgpu::graphite::PrecompileBlender>> CreateComboRuntimeBlender();

    SkRuntimeEffect* GetDoubleColorFilterEffect();
    std::pair<sk_sp<SkColorFilter>, sk_sp<skgpu::graphite::PrecompileColorFilter>> CreateDoubleRuntimeColorFilter();
    SkRuntimeEffect* GetHalfColorFilterEffect();
    std::pair<sk_sp<SkColorFilter>, sk_sp<skgpu::graphite::PrecompileColorFilter>> CreateHalfRuntimeColorFilter();
    SkRuntimeEffect* GetComboColorFilterEffect();
    std::pair<sk_sp<SkColorFilter>, sk_sp<skgpu::graphite::PrecompileColorFilter>> CreateComboRuntimeColorFilter();

} // namespace PrecompileFactories

} // namespace skiatest::graphite

#endif // PrecompileEffectFactories_DEFINED
