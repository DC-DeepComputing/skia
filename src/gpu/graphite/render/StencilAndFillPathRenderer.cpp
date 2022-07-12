/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/Renderer.h"

#include "include/core/SkPathTypes.h"
#include "include/core/SkVertices.h"
#include "src/gpu/graphite/render/CoverBoundsRenderStep.h"
#include "src/gpu/graphite/render/MiddleOutFanRenderStep.h"
#include "src/gpu/graphite/render/StencilAndCoverDSS.h"
#include "src/gpu/graphite/render/TessellateCurvesRenderStep.h"
#include "src/gpu/graphite/render/TessellateWedgesRenderStep.h"
#include "src/gpu/graphite/render/VerticesRenderStep.h"


namespace skgpu::graphite {

namespace {

const RenderStep* cover_step() {
    static const CoverBoundsRenderStep kFill{false};
    return &kFill;
}

const RenderStep* inverse_cover_step() {
    static const CoverBoundsRenderStep kInverseFill{true};
    return &kInverseFill;
}

static constexpr DepthStencilSettings kDirectShadingPass = {
        /*frontStencil=*/{},
        /*backStencil=*/ {},
        /*refValue=*/    0,
        /*stencilTest=*/ false,
        /*depthCompare=*/CompareOp::kGreater,
        /*depthTest=*/   true,
        /*depthWrite=*/  true
};

}  // namespace

const Renderer& Renderer::StencilTessellatedCurvesAndTris(SkPathFillType fillType) {
    // Because each fill type uses a different stencil settings, there is one Renderer per type.
    // However, at each stage (stencil vs. cover), there are only two RenderSteps to branch on.
    static const MiddleOutFanRenderStep kWindingStencilFan{false};
    static const MiddleOutFanRenderStep kEvenOddStencilFan{true};
    static const TessellateCurvesRenderStep kWindingStencilCurves{false};
    static const TessellateCurvesRenderStep kEvenOddStencilCurves{true};

    static const Renderer kWindingRenderer{"StencilTessellatedCurvesAndTris[winding]",
                                           &kWindingStencilFan,
                                           &kWindingStencilCurves,
                                           cover_step()};
    static const Renderer kInverseWindingRenderer{"StencilTessellatedCurvesAndTris[inverse-winding]",
                                                  &kWindingStencilFan,
                                                  &kWindingStencilCurves,
                                                  inverse_cover_step()};
    static const Renderer kEvenOddRenderer{"StencilTessellatedCurvesAndTris[evenodd]",
                                           &kEvenOddStencilFan,
                                           &kEvenOddStencilCurves,
                                           cover_step()};
    static const Renderer kInverseEvenOddRenderer{"StencilTessellatedCurvesAndTris[inverse-evenodd]",
                                                  &kEvenOddStencilFan,
                                                  &kEvenOddStencilCurves,
                                                  inverse_cover_step()};

    switch(fillType) {
        case SkPathFillType::kWinding: return kWindingRenderer;
        case SkPathFillType::kEvenOdd: return kEvenOddRenderer;
        case SkPathFillType::kInverseWinding: return kInverseWindingRenderer;
        case SkPathFillType::kInverseEvenOdd: return kInverseEvenOddRenderer;
    }
    SkUNREACHABLE;
}

const Renderer& Renderer::StencilTessellatedWedges(SkPathFillType fillType) {
    static const TessellateWedgesRenderStep kWindingStencilWedges{"winding",  kWindingStencilPass};
    static const TessellateWedgesRenderStep kEvenOddStencilWedges{"even-odd", kEvenOddStencilPass};

    static const Renderer kWindingRenderer{"StencilTessellatedWedges[winding]",
                                           &kWindingStencilWedges,
                                           cover_step()};
    static const Renderer kInverseWindingRenderer{"StencilTessellatedWedges[inverse-winding]",
                                                  &kWindingStencilWedges,
                                                  inverse_cover_step()};
    static const Renderer kEvenOddRenderer{"StencilTessellatedWedges[evenodd]",
                                           &kEvenOddStencilWedges,
                                           cover_step()};
    static const Renderer kInverseEvenOddRenderer{"StencilTessellatedWedges[inverse-evenodd]",
                                                  &kEvenOddStencilWedges,
                                                  inverse_cover_step()};

    switch(fillType) {
        case SkPathFillType::kWinding: return kWindingRenderer;
        case SkPathFillType::kEvenOdd: return kEvenOddRenderer;
        case SkPathFillType::kInverseWinding: return kInverseWindingRenderer;
        case SkPathFillType::kInverseEvenOdd: return kInverseEvenOddRenderer;
    }
    SkUNREACHABLE;
}

const Renderer& Renderer::ConvexTessellatedWedges() {
    static const TessellateWedgesRenderStep kConvexWedges{"convex", kDirectShadingPass};
    static const Renderer kConvexWedgeRenderer{"ConvexTessellatedWedges", &kConvexWedges};
    return kConvexWedgeRenderer;
}

const Renderer& Renderer::VertTrisPositionOnly() {
    static const VerticesRenderStep kTrianglesStep{PrimitiveType::kTriangles, "Triangles"};
    static const Renderer kTrianglesVerticesRenderer{"Vertices[Triangles]", &kTrianglesStep};
    return kTrianglesVerticesRenderer;
}

const Renderer& Renderer::VertTrisWithColor() {
    static const VerticesRenderStep kTrianglesColorStep{PrimitiveType::kTriangles, "TrianglesColor",
                                                        /*hasColor=*/true};
    static const Renderer kTrianglesColorVerticesRenderer{"Vertices[TrianglesColor]",
                                                          &kTrianglesColorStep};
    return kTrianglesColorVerticesRenderer;
}

const Renderer& Renderer::VertTrisWithTex() {
    static const VerticesRenderStep kTrianglesTexStep{PrimitiveType::kTriangles, "TrianglesTex",
                                                      /*hasColor=*/false, /*hasTexture=*/true};
    static const Renderer kTrianglesTexVerticesRenderer{"Vertices[TrianglesTex]",
                                                        &kTrianglesTexStep};
    return kTrianglesTexVerticesRenderer;
}

const Renderer& Renderer::VertTrisWithColorAndTex() {
    static const VerticesRenderStep kTrianglesColorTexStep{PrimitiveType::kTriangles,
                                                           "TrianglesColorTex", /*hasColor=*/true,
                                                           /*hasTexture=*/true};
    static const Renderer kTrianglesColorTexVerticesRenderer{"Vertices[TrianglesColorTex]",
                                                             &kTrianglesColorTexStep};
    return kTrianglesColorTexVerticesRenderer;
}

const Renderer& Renderer::VertTriStripPositionOnly() {
    static const VerticesRenderStep kTriangleStripStep{PrimitiveType::kTriangleStrip,
                                                       "TriangleStrip"};
    static const Renderer kTriangleStripVerticesRenderer{"Vertices[TriangleStrip]",
                                                         &kTriangleStripStep};
    return kTriangleStripVerticesRenderer;
}

const Renderer& Renderer::VertTriStripWithColor() {
    static const VerticesRenderStep kTriangleStripColorStep{PrimitiveType::kTriangleStrip,
                                                            "TriangleColorStrip",
                                                            /*hasColor=*/true};
    static const Renderer kTriangleStripColorVerticesRenderer{"Vertices[TriangleColorStrip]",
                                                              &kTriangleStripColorStep};
    return kTriangleStripColorVerticesRenderer;
}

const Renderer& Renderer::VertTriStripWithTex() {
    static const VerticesRenderStep kTriangleStripTexStep{PrimitiveType::kTriangleStrip,
                                                          "TriangleStripTex", /*hasColor=*/false,
                                                          /*hasTexture=*/true};
    static const Renderer kTriangleStripTexVerticesRenderer{"Vertices[TriangleStripTex]",
                                                            &kTriangleStripTexStep};
    return kTriangleStripTexVerticesRenderer;
}

const Renderer& Renderer::VertTriStripWithColorAndTex() {
    static const VerticesRenderStep kTriangleStripColorTexStep{PrimitiveType::kTriangleStrip,
                                                               "TriangleStripColorTex",
                                                               /*hasColor=*/true,
                                                               /*hasTexture=*/true};
    static const Renderer kTriangleStripColorTexVerticesRenderer{"Vertices[TriangleStripColorTex]",
                                                                 &kTriangleStripColorTexStep};
    return kTriangleStripColorTexVerticesRenderer;
}
}
