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
                       bool,
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
        , fName1(className) {
    for (auto v : this->vertexAttributes()) {
        fVertexStride += v.sizeAlign4();
    }
    for (auto i : this->instanceAttributes()) {
        fInstanceStride += i.sizeAlign4();
    }
    if (!variantName.empty()) {
        fName1 += "[";
        fName1 += variantName;
        fName1 += "]";
    }

    SkASSERT(!strcmp(RenderStepName(fRenderStepID), fName1.c_str()));
}

Coverage RenderStep::GetCoverage(SkEnumBitMask<Flags> flags) {
    return !(flags & Flags::kEmitsCoverage) ? Coverage::kNone
           : (flags & Flags::kLCDCoverage)  ? Coverage::kLCD
                                            : Coverage::kSingleChannel;
}

#if 0
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
#else
const char* RenderStep::RenderStepName(RenderStepID id) {
    switch (id) {
        case RenderStepID::kInvalid: return "InvalidRenderStep";

        case RenderStepID::kCircularArc: return "CircularArcRenderStep";

        case RenderStepID::kAnalyticRRect: return "AnalyticRRectRenderStep";
        case RenderStepID::kAnalyticBlur : return "AnalyticBlurRenderStep";

        case RenderStepID::kPerEdgeAAQuad: return "PerEdgeAAQuadRenderStep";

        case RenderStepID::kCoverBounds_NonAAFill: return "CoverBoundsRenderStep[non-aa-fill]";
        case RenderStepID::kCoverBounds_RegularCover: return "CoverBoundsRenderStep[regular-cover]";
        case RenderStepID::kCoverBounds_InverseCover : return "CoverBoundsRenderStep[inverse-cover]";

        case RenderStepID::kCoverageMask: return "CoverageMaskRenderStep";

        case RenderStepID::kBitmapText_Mask: return "BitmapTextRenderStep[mask]";
        case RenderStepID::kBitmapText_LCD: return "BitmapTextRenderStep[LCD]";
        case RenderStepID::kBitmapText_Color: return "BitmapTextRenderStep[color]";

        case RenderStepID::kMiddleOutFan_EvenOdd: return "MiddleOutFanRenderStep[even-odd]";
        case RenderStepID::kMiddleOutFan_Winding: return "MiddleOutFanRenderStep[winding]";

        case RenderStepID::kSDFTextLCD: return "SDFTextLCDRenderStep";
        case RenderStepID::kSDFText : return "SDFTextRenderStep";

        case RenderStepID::kTessellateCurves_EvenOdd: return "TessellateCurvesRenderStep[even-odd]";
        case RenderStepID::kTessellateCurves_Winding : return "TessellateCurvesRenderStep[winding]";

        case RenderStepID::kTessellateStrokes: return "TessellateStrokesRenderStep";

        case RenderStepID::kTessellateWedges_Convex : return "TessellateWedgesRenderStep[convex]";
        case RenderStepID::kTessellateWedges_EvenOdd : return "TessellateWedgesRenderStep[evenodd]";  // maybe "even-odd" ?
        case RenderStepID::kTessellateWedges_Winding : return "TessellateWedgesRenderStep[winding]";

        case RenderStepID::kVertices_Tris: return "VerticesRenderStep[tris]";
        case RenderStepID::kVertices_TrisColor: return "VerticesRenderStep[tris-color]";
        case RenderStepID::kVertices_TrisTexCoords: return "VerticesRenderStep[tris-texCoords]";
        case RenderStepID::kVertices_TrisColorTexCoords: return "VerticesRenderStep[tris-color-texCoords]";

        case RenderStepID::kVertices_Tristrips: return "VerticesRenderStep[tristrips]";
        case RenderStepID::kVertices_TristripsColor: return "VerticesRenderStep[tristrips-color]";
        case RenderStepID::kVertices_TristripsTexCoords: return "VerticesRenderStep[tristrips-texCoords]";
        case RenderStepID::kVertices_TristripsColorTexCoords: return "VerticesRenderStep[tristrips-color-texCoords]";
    }

    SkUNREACHABLE;
}
#endif

} // namespace skgpu::graphite
