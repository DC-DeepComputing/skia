/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_KnownRuntimeEffects_DEFINED
#define skgpu_KnownRuntimeEffects_DEFINED

class SkRuntimeEffect;

namespace skgpu {

enum class StableKey : int {
    kInvalid = 55,
    kBlend,

    kLast =     kBlend,
};

static const int kStableKeyCnt = static_cast<int>(StableKey::kLast) + 1;

SkRuntimeEffect* GetKnownRuntimeEffect(StableKey);

} // namespace skgpu

#endif // skgpu_KnownRuntimeEffects_DEFINED
