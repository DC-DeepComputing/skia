/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef PrecompileEffectFactories_DEFINED
#define PrecompileEffectFactories_DEFINED

#include "include/core/SkRefCnt.h"

class SkShader;
namespace skgpu::graphite {
 class PrecompileShader;
}

namespace skiatest::graphite {

// These factory functions returns a normal-API/precompile-API pair that represent the same
// effect.
namespace PrecompileFactories {

    const char* foo();
    std::pair<sk_sp<SkShader>, sk_sp<skgpu::graphite::PrecompileShader>> CreateRuntimeShader();

}
} // skiatest::graphite

#endif // PrecompileEffectFactories_DEFINED
