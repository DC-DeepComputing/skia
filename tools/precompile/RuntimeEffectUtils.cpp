/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tools/precompile/RuntimeEffectUtils.h"

#include "include/effects/SkRuntimeEffect.h"
#include "src/core/SkRuntimeEffectPriv.h"

namespace RuntimeEffectUtils {

void SrcBlender(Result* result) {
    static SkRuntimeEffect* sSrcEffect = SkMakeRuntimeEffect(
            SkRuntimeEffect::MakeForBlender,
            "half4 main(half4 src, half4 dst) {"
                "return src;"
            "}"
    );

    result->fEffect = sk_ref_sp(sSrcEffect);
    result->fUniforms = nullptr;
}

void DestBlender(Result* result) {
    static SkRuntimeEffect* sDestEffect = SkMakeRuntimeEffect(
            SkRuntimeEffect::MakeForBlender,
            "half4 main(half4 src, half4 dst) {"
                "return dst;"
            "}"
    );

    result->fEffect = sk_ref_sp(sDestEffect);
    result->fUniforms = nullptr;
}

void ComboBlender(Result* result) {
    static SkRuntimeEffect* sComboEffect = SkMakeRuntimeEffect(
            SkRuntimeEffect::MakeForBlender,
            "uniform float blendFrac;"
            "uniform blender a;"
            "uniform blender b;"
            "half4 main(half4 src, half4 dst) {"
                "return (blendFrac * a.eval(src, dst)) + ((1 - blendFrac) * b.eval(src, dst));"
            "}"
    );

    const float kUniforms[] = { 1.0f };

    result->fEffect = sk_ref_sp(sComboEffect);
    result->fUniforms = SkData::MakeWithCopy(kUniforms, sizeof(kUniforms));
}


} // namespace RuntimeEffectUtils
