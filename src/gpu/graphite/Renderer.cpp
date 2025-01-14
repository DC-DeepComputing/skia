/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/Renderer.h"

namespace skgpu::graphite {

RenderStep::RenderStep(std::string_view className,
                       std::string_view variantName,
                       RenderStepID renderStepID,
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
        , fInstanceStride(0)
        , fName(className) {
    for (auto v : this->vertexAttributes()) {
        fVertexStride += v.sizeAlign4();
    }
    for (auto i : this->instanceAttributes()) {
        fInstanceStride += i.sizeAlign4();
    }
    if (!variantName.empty()) {
        fName += "[";
        fName += variantName;
        fName += "]";
    }
}

Coverage RenderStep::GetCoverage(SkEnumBitMask<Flags> flags) {
    return !(flags & Flags::kEmitsCoverage) ? Coverage::kNone
           : (flags & Flags::kLCDCoverage)  ? Coverage::kLCD
                                            : Coverage::kSingleChannel;
}

} // namespace skgpu::graphite
