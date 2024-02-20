/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/KnownRuntimeEffects.h"

#include "include/effects/SkRuntimeEffect.h"
#include "include/private/base/SkAssert.h"
#include "src/core/SkRuntimeEffectPriv.h"

namespace skgpu {

SkRuntimeEffect* GetKnownRuntimeEffect(StableKey stableKey) {
    switch (stableKey) {
        case StableKey::kInvalid:
            return nullptr;
        case StableKey::kBlend: {
            static SkRuntimeEffect* sBlendEffect = SkMakeRuntimeEffect(SkRuntimeEffect::MakeForShader,
                                                                       "uniform shader s, d;"
                                                                       "uniform blender b;"
                                                                       "half4 main(float2 xy) {"
                                                                       "return b.eval(s.eval(xy), d.eval(xy));"
                                                                       "}");
            return sBlendEffect;
        }

    }

    SkUNREACHABLE;
}


}  // namespace skgpu
