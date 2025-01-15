/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/Renderer.h"

namespace skgpu::graphite {

RenderStep::RenderStep(RenderStepID renderStepID,
                       SkEnumBitMask<Flags> flags,
                       std::initializer_list<Uniform> uniforms,
                       PrimitiveType primitiveType,
                       DepthStencilSettings depthStencilSettings,
                       SkSpan<const Attribute> vertexAttrs,
                       SkSpan<const Attribute> instanceAttrs,
                       SkSpan<const Varying> varyings)
        : fRenderStepID(renderStepID)
        , fFlags(flags)
        , fPrimitiveType(primitiveType)
        , fDepthStencilSettings(depthStencilSettings)
        , fUniforms(uniforms)
        , fVertexAttrs(vertexAttrs.begin(), vertexAttrs.end())
        , fInstanceAttrs(instanceAttrs.begin(), instanceAttrs.end())
        , fVaryings(varyings.begin(), varyings.end())
        , fVertexStride(0)
        , fInstanceStride(0) {
    for (auto v : this->vertexAttributes()) {
        fVertexStride += v.sizeAlign4();
    }
    for (auto i : this->instanceAttributes()) {
        fInstanceStride += i.sizeAlign4();
    }
}

Coverage RenderStep::GetCoverage(SkEnumBitMask<Flags> flags) {
    return !(flags & Flags::kEmitsCoverage) ? Coverage::kNone
           : (flags & Flags::kLCDCoverage)  ? Coverage::kLCD
                                            : Coverage::kSingleChannel;
}

const char* RenderStep::RenderStepName(skgpu::graphite::RenderStep::RenderStepID id) {
#define CASE1(BaseName) case RenderStepID::k##BaseName: return #BaseName "RenderStep";
#define CASE3(BaseName, VariantName, VariantTag) \
    case RenderStepID::k##BaseName##_##VariantName: return #BaseName "RenderStep[" VariantTag "]";

    switch (id) {
        SKGPU_RENDERSTEP_TYPES(CASE1, CASE3)
    }
#undef CASE1
#undef CASE3

    SkUNREACHABLE;
}

static bool IsValidRenderStepID(uint32_t renderStepID) {
    return renderStepID <= (int) RenderStep::RenderStepID::kLast;
}

} // namespace skgpu::graphite
