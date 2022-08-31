/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/render/AnalyticRectRenderStep.h"

#include "src/gpu/graphite/DrawParams.h"
#include "src/gpu/graphite/DrawWriter.h"
#include "src/gpu/graphite/render/CommonDepthStencilSettings.h"

namespace skgpu::graphite {

AnalyticRectRenderStep::AnalyticRectRenderStep()
        : RenderStep("AnalyticRectRenderStep",
                     "",
                     Flags::kPerformsShading,
                     /*uniforms=*/{},
                     PrimitiveType::kTriangles,
                     kDirectDepthGreaterPass,
                     /*vertexAttrs=*/{},
                     /*instanceAttrs=*/
                            {{"xRadiiOrFlags", VertexAttribType::kFloat4, SkSLType::kFloat4},
                             {"radiiOrQuadXs", VertexAttribType::kFloat4, SkSLType::kFloat4},
                             {"ltrbOrQuadYs", VertexAttribType::kFloat4, SkSLType::kFloat4},
                             {"depth", VertexAttribType::kUShort_norm, SkSLType::kFloat},
                             {"ssboIndex", VertexAttribType::kUShort_norm, SkSLType::kFloat},
                             {"maxScale", VertexAttribType::kFloat, SkSLType::kFloat},
                             {"mat0", VertexAttribType::kFloat3, SkSLType::kFloat3},
                             {"mat0", VertexAttribType::kFloat3, SkSLType::kFloat3},
                             {"mat0", VertexAttribType::kFloat3, SkSLType::kFloat3}} {}

AnalyticRectRenderStep::~AnalyticRectRenderStep() {}

const char* AnalyticRectRenderStep::vertexSkSL() const {
    return R"(
        float3x3 matrix = float3x3(mat0, mat1, mat2);
        float2 corner = float2(float(sk_VertexID / 2), float(sk_VertexID % 2));

        float4 devPosition;
        if (bounds.L <= bounds.R && bounds.T <= bounds.B) {
            // A regular fill
            corner = (1.0 - corner) * bounds.LT + corner * bounds.RB;
            float3 devCorner = matrix * float3(corner, 1.0);
            devPosition = float4(devCorner.xy, depth, devCorner.z);
        } else {
            // An inverse fill
            corner = corner * bounds.LT + (1.0 - corner) * bounds.RB;
            devPosition = float4(corner, depth, 1.0);
            // TODO: transform corner by matrix to get updated local coordinates
        }
    )";
}

void AnalyticRectRenderStep::writeVertices(DrawWriter* writer,
                                          const DrawParams& params,
                                          int ssboIndex) const {
    // The instance attributes are flexible enough to describe a filled rectangle, a filled
    // quadrilateral with per-edge AA, a filled rounded rectangle with arbitrary corner radii,
    // or a stroked rounded rectangle with circular corners (independent per corner, can can be
    // square, respecting the join style). This is encoded as follows:
    //
    // float4 xRadiiOrFlags - if any components is > 0, the instance represents a filled round rect
    //    with elliptical corners and these values specify the X radii in top-left CW order.
    //    Otherwise, if .x < -1, the instance represents a stroked [round] rect and .y holds the
    //    stroke radius and .z holds the join limit (matching StrokeStyle's conventions).
    //    Else it's a filled quadrilateral with per-edge AA defined by each component: aa != 0.
    // float4 radiiOrQuadXs - if in filled round rect mode, these values provide the Y radii in
    //    top-left CW order. If in stroked [round] rect mode, these values provide the circular
    //    corner radii (same order). Otherwise, when in per-edge quad mode, these values provide
    //    the X coordinates of the quadrilateral (same order).
    // float4 ltrbOrQuadYs - if in filled round rect mode or stroked [round] rect mode, these values
    //    define the LTRB edge coordinates of the rectangle surrounding the round rect (or the
    //    rect itself when the radii are 0s). Otherwise, in per-edge quad mode, these values provide
    //    the Y coordinates of the quadrilateral.
    //
    // This encoding relies on the fact that a valid SkRRect with all x radii equal to 0 must have
    // y radii equal to 0 (so it's a rectangle and we can treat it as a quadrilateral with
    // all edges AA'ed). This avoids other encodings' ambiguities identifying a quad with all edges
    // anti-aliased (e.g. checking for negatives in xRadiiOrFlags to turn on per-edge mode).
    //
    // Some common scenarios are encoded as follows:
    //
    //  - filled rect: [-1 -1 -1 -1] [L R R L] [T T B B]
    //  - stroked rect: [-2 stroke join 0] [0 0 0 0] [L T R B]
    //  - filled rrect: [xRadii(tl,tr,br,bl)] [yRadii(tl,tr,br,bl)] [L T R B]
    //  - stroked rrect: [-2 stroke join 0] [radii(tl,tr,br,bl)] [L T R B]
    //  - per-edge quad: [aa(tl,tr,br,bl) ? -1 : 0] [xs(tl,tr,br,bl)] [ys(tl,tr,br,bl)]
    //
    // From this encoding, data can be unpacked for each corner, which are equivalent under
    // rotational symmetry. A corner can have an outer curve, be mitered, or be beveled. It can
    // have an inner curve, an inner miter, or fill the interior. Per-edge quads are always mitered
    // and fill the interior, but the vertices are placed such that the edge coverage ramps can
    // collapse to 0 area on non-AA edges.
    //
    // The vertices the describe each corner are placed so that edges, miters, and bevels,

    // Each instance is 4 vertices, forming 2 triangles from a single triangle strip, so no indices
    // are needed. sk_VertexID is used to place vertex positions, so no vertex buffer is needed.
    DrawWriter::Instances instances{*writer, {}, {}, 4};

    skvx::float4 bounds;
    const SkM44* m;
    if (fInverseFill) {
        // Normally all bounding boxes are sorted such that l<r and t<b. We upload an inverted
        // rectangle [r,b,l,t] when it's an inverse fill to encode that the bounds are already in
        // device space and then use the inverse of the transform to compute local coordinates.
        bounds = skvx::shuffle</*R*/2, /*B*/3, /*L*/0, /*T*/1>(
                skvx::cast<float>(skvx::int4::Load(&params.clip().scissor())));
        m = &params.transform().inverse();
    } else {
        bounds = params.geometry().bounds().ltrb();
        m = &params.transform().matrix();
    }

    // Since the local coords always have Z=0, we can discard the 3rd row and column of the matrix.
    instances.append(1) << bounds << params.order().depthAsFloat() << ssboIndex
                        << m->rc(0,0) << m->rc(1,0) << m->rc(3,0)
                        << m->rc(0,1) << m->rc(1,1) << m->rc(3,1)
                        << m->rc(0,3) << m->rc(1,3) << m->rc(3,3);
}

void AnalyticRectRenderStep::writeUniformsAndTextures(const DrawParams&,
                                                      SkPipelineDataGatherer*) const {
    // All data is uploaded as instance attributes, so no uniforms are needed.
}

}  // namespace skgpu::graphite
