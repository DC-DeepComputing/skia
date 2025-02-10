/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tools/graphite/precompile/PrecompileEffectFactories.h"

#include "include/effects/SkRuntimeEffect.h"
#include "include/gpu/graphite/precompile/PrecompileRuntimeEffect.h"
#include "include/gpu/graphite/precompile/PrecompileShader.h"

namespace skiatest::graphite {

using namespace skgpu::graphite;

namespace PrecompileFactories {

// draw a circle centered at "center" w/ inner and outer radii in "radii"
const char* foo() {
    static const char* kSkSLShaderString =
            "uniform float2 center;"
            "uniform float2 radii;"
            "half4 main(float2 xy) {"
            "float len = length(xy - center);"
            "half value = len < radii.x ? 0.0 : (len > radii.y ? 0.0 : 1.0);"
            "return half4(value);"
            "}";
    return kSkSLShaderString;
}

std::pair<sk_sp<SkShader>, sk_sp<PrecompileShader>> CreateRuntimeShader() {
    SkRuntimeEffect::Result result = SkRuntimeEffect::MakeForShader(SkString(foo()));
    SkASSERT(result.effect);

    static const float kUniforms[4] = { 50.0f, 50.0f, 40.0f, 50.0f };

    sk_sp<SkData> uniforms = SkData::MakeWithCopy(kUniforms, sizeof(kUniforms));

    sk_sp<SkShader> s = result.effect->makeShader(std::move(uniforms), /* children= */ {});
    sk_sp<PrecompileShader> o = PrecompileRuntimeEffects::MakePrecompileShader(result.effect);
    return { std::move(s), std::move(o) };
}


}

} // namespace skiatest::graphite
