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

    const char* GetAnnulusShaderCode();
    std::pair<sk_sp<SkShader>, sk_sp<skgpu::graphite::PrecompileShader>> CreateAnnulusRuntimeShader();

    const char* GetSrcBlenderCode();
    std::pair<sk_sp<SkBlender>, sk_sp<skgpu::graphite::PrecompileBlender>> CreateSrcRuntimeBlender();
    const char* GetDstBlenderCode();
    std::pair<sk_sp<SkBlender>, sk_sp<skgpu::graphite::PrecompileBlender>> CreateDstRuntimeBlender();
    const char* GetComboBlenderCode();
    std::pair<sk_sp<SkBlender>, sk_sp<skgpu::graphite::PrecompileBlender>> CreateComboRuntimeBlender();

    const char* GetDoubleColorFilterCode();
    std::pair<sk_sp<SkColorFilter>, sk_sp<skgpu::graphite::PrecompileColorFilter>> CreateDoubleRuntimeColorFilter();
    const char* GetHalfColorFilterCode();
    std::pair<sk_sp<SkColorFilter>, sk_sp<skgpu::graphite::PrecompileColorFilter>> CreateHalfRuntimeColorFilter();
    const char* GetComboColorFilterCode();
    std::pair<sk_sp<SkColorFilter>, sk_sp<skgpu::graphite::PrecompileColorFilter>> CreateComboRuntimeColorFilter();

} // namespace PrecompileFactories

} // namespace skiatest::graphite

#endif // PrecompileEffectFactories_DEFINED
