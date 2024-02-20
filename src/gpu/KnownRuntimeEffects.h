/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_KnownRuntimeEffects_DEFINED
#define skgpu_KnownRuntimeEffects_DEFINED

#include "include/core/SkTypes.h"
#include "src/gpu/graphite/BuiltInCodeSnippetID.h"

class SkRuntimeEffect;

namespace skgpu {

enum class StableKey : int32_t {
    kStart =   skgpu::graphite::kSkiaKnownRuntimeEffectsStart,

    kInvalid = kStart,
    kBlend,

    kLast =    kBlend,
};

static const int kStableKeyCnt = static_cast<int>(StableKey::kLast) + 1;


static_assert(static_cast<int>(skgpu::StableKey::kLast) < skgpu::graphite::kSkiaKnownRuntimeEffectsEnd);

SkRuntimeEffect* GetKnownRuntimeEffect(StableKey);

} // namespace skgpu

#endif // skgpu_KnownRuntimeEffects_DEFINED
