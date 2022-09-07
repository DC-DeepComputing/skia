/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/render/AnalyticRRectRenderStep.h"

#include "src/core/SkRRectPriv.h"
#include "src/gpu/graphite/DrawParams.h"
#include "src/gpu/graphite/DrawWriter.h"
#include "src/gpu/graphite/render/CommonDepthStencilSettings.h"

// This RenderStep is flexible and can draw filled rectangles, filled quadrilaterals with per-edge
// AA, filled rounded rectangles with arbitrary corner radii, stroked rectangles with any join,
// and stroked rounded rectangles with circular corners (each corner can be different or square).
// We combine all of these together to maximize batching across simple geometric draws and reduce
// the number pipeline specializations. Additionally, these primitives are the most common
// operations and help us avoid triggering MSAA.
//
// Each of these "primitives" is represented by a single instance. The instance attributes are
// flexible enough to describe any of the above shapes without relying on uniforms to define its
// operation. The attributes encode shape as follows:

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
// From the other direction, shapes produce instance values like:
//  - filled rect:   [-1 -1 -1 -1]              [L R R L]             [T T B B]
//  - stroked rect:  [-2 stroke join 0]         [0 0 0 0]             [L T R B]
//  - filled rrect:  [xRadii(tl,tr,br,bl)]      [yRadii(tl,tr,br,bl)] [L T R B]
//  - stroked rrect: [-2 stroke join 0]         [radii(tl,tr,br,bl)]  [L T R B]
//  - per-edge quad: [aa(tl,tr,br,bl) ? -1 : 0] [xs(tl,tr,br,bl)]     [ys(tl,tr,br,bl)]
//
// This encoding relies on the fact that a valid SkRRect with all x radii equal to 0 must have
// y radii equal to 0 (so it's a rectangle and we can treat it as a quadrilateral with
// all edges AA'ed). This avoids other encodings' inability to represent a quad with all edges
// anti-aliased (e.g. checking for negatives in xRadiiOrFlags to turn on per-edge mode).
//
// From this encoding, data can be unpacked for each corner, which are equivalent under
// rotational symmetry. A corner can have an outer curve, be mitered, or be beveled. It can
// have an inner curve, an inner miter, or fill the interior. Per-edge quads are always mitered
// and fill the interior, but the vertices are placed such that the edge coverage ramps can
// collapse to 0 area on non-AA edges.
//
// The vertices that describe each corner are placed so that edges, miters, and bevels calculate
// coverage by interpolating a varying and then clamping in the fragment shader. Triangles that
// cover the inner and outer curves calculate distance to the curve within the fragment shader.
//
// See https://docs.google.com/presentation/d/1MCPstNsSlDBhR8CrsJo0r-cZNbu-sEJEvU9W94GOJoY/edit?usp=sharing
// for diagrams and explanation of how the geometry is defined.

namespace skgpu::graphite {

static skvx::float4 loadXRadii(const SkRRect& rrect) {
    return skvx::float4{rrect.radii(SkRRect::kUpperLeft_Corner).fX,
                        rrect.radii(SkRRect::kUpperRight_Corner).fX,
                        rrect.radii(SkRRect::kLowerRight_Corner).fX,
                        rrect.radii(SkRRect::kLowerLeft_Corner).fX};
}
static skvx::float4 loadYRadii(const SkRRect& rrect) {
    return skvx::float4{rrect.radii(SkRRect::kUpperLeft_Corner).fY,
                        rrect.radii(SkRRect::kUpperRight_Corner).fY,
                        rrect.radii(SkRRect::kLowerRight_Corner).fY,
                        rrect.radii(SkRRect::kLowerLeft_Corner).fY};
}

// With the current simplified vertex structure, there are 28 vertices forming 32 triangles.
// The vertices are defined solely by sk_VertexID as follows:
//   TL:  0|anchor0,  1|anchor1,  2|offset0,  3|offset1,  4|bisector0,  5|bisector1,  6|center
//   TR:  7|anchor0,  8|anchor1,  9|offset0, 10|offset1, 11|bisector0, 12|bisector1, 13|center
//   BR: 14|anchor0, 15|anchor1, 16|offset0, 17|offset1, 18|bisector0, 19|bisector1, 20|center
//   BL: 21|anchor0, 22|anchor1, 23|offset0, 24|offset1, 25|bisector0, 26|bisector1, 27|center
static size_t indexBufferSize() { return 32 * 3 * sizeof(uint16_t); }
static void writeIndexBuffer(VertexWriter writer, size_t size) {
    SkASSERT(indexBufferSize() == size);
    static constexpr uint16_t kIndices[32*3] = {
         0, 2, 4,   0, 4, 5,   0, 5, 1,   1, 5, 3,   0, 1, 6,   1, 3, 9,   1, 9, 7,   1, 7, 6,
         7, 9,11,   7,11,12,   7,12, 8,   8,12,10,   7, 8,13,   8,10,16,   8,16,14,   8,14,13,
        14,16,18,  14,18,19,  14,19,15,  15,19,17,  14,15,20,  15,17,23,  15,23,21,  15,21,20,
        21,23,25,  21,25,26,  21,26,22,  22,26,24,  21,22,27,  22,24, 2,  22, 2, 0,  22, 0,27,
    };
    writer << kIndices;
}

AnalyticRRectRenderStep::AnalyticRRectRenderStep()
        : RenderStep("AnalyticRRectRenderStep",
                     "",
                     Flags::kPerformsShading |
                     Flags::kEmitsCoverage,
                     /*uniforms=*/{},
                     PrimitiveType::kTriangles,
                     kDirectDepthGreaterPass,
                     /*vertexAttrs=*/{},
                     /*instanceAttrs=*/
                            {{"xRadiiOrFlags", VertexAttribType::kFloat4, SkSLType::kFloat4},
                             {"radiiOrQuadXs", VertexAttribType::kFloat4, SkSLType::kFloat4},
                             {"ltrbOrQuadYs", VertexAttribType::kFloat4, SkSLType::kFloat4},
                             // TODO: pack depth and ssboIndex into 32-bits
                             {"depth", VertexAttribType::kFloat, SkSLType::kFloat},
                             {"ssboIndex", VertexAttribType::kInt, SkSLType::kInt},
                             {"maxScale", VertexAttribType::kFloat, SkSLType::kFloat},
                             {"mat0", VertexAttribType::kFloat3, SkSLType::kFloat3},
                             {"mat1", VertexAttribType::kFloat3, SkSLType::kFloat3},
                             {"mat2", VertexAttribType::kFloat3, SkSLType::kFloat3}},
                     /*varyings=*/
                            {{"edgeDistance", SkSLType::kFloat}}) {}

AnalyticRRectRenderStep::~AnalyticRRectRenderStep() {}

const char* AnalyticRRectRenderStep::vertexSkSL() const {
    return R"(
        float3x3 matrix = float3x3(mat0, mat1, mat2);

        int cornerID = sk_VertexID / 7; // Seven vertices per corner currently
        int vertexID = sk_VertexID - cornerID * 7; // ID relative to corner structure

        // Each rounded corner can be calculated independently of the others, except to know the
        // edge vectors coming from the previous corner and going to the next corner. 'p1' is the
        // vertex of the outer bbox for the current corner, p0 is the previous and p2 is the next,
        // in CW winding.
        float2 p0, p1, p2;
        float2 center;

        float2 cornerRadii = float2(0.0);
        float strokeRadius = -1.0; // < 0 for fills, 0 for hairlines (1px in device space)
        float joinLimit = -1.0; // ignored for fills, > 0: miter+join limit, ==0: bevel, <0: round
        if (xRadiiOrFlags.x < -1.0) {
            // Stroked rect or round rect
            strokeRadius = xRadiiOrFlags.y;
            joinLimit = xRadiiOrFlags.z;
            cornerRadii = float2(radiiOrQuadXs[cornerID]);  // strokes require circular corners

            float4 outset = ltrbOrQuadYs.LTRB + float2(-strokeRadius, strokeRadius).xxyy;
            unpack_rect_corner(cornerID, outset, p0, p1, p2);
            center = 0.5*outset.LT + 0.5*outset.RB;
        } else if (any(greaterThan(xRadiiOrFlags, float4(0.0)))) {
            // Filled round rect
            cornerRadii = float2(xRadiiOrFlags[cornerID], radiiOrQuadXs[cornerID]);
            unpack_rect_corner(cornerID, ltrbOrQuadYs, p0, p1, p2);
            center = 0.5*ltrbOrQuadYs.LT + 0.5*ltrbOrQuadYs.RB;
        } else {
            // Per-edge quadrilateral
            unpack_quad_corner(cornerID, radiiOrQuadXs, ltrbOrQuadYs, p0, p1, p2);
            center = float2(0.25*radiiOrQuadXs.x + 0.25*radiiOrQuadXs.y +
                                    0.25*radiiOrQuadXs.z + 0.25*radiiOrQuadXs.w,
                            0.25*ltrbOrQuadYs.x + 0.25*ltrbOrQuadYs.y +
                                    0.25*ltrbOrQuadYs.z + 0.25*ltrbOrQuadYs.w);
        }

        // The current vertex structure that this shader produces is the extreme-simplified case
        // shown in the slide deck in this file's top comment. Each corner has 7 vertices: two
        // co-located anchors set to p1; two outsets (one per edge); two co-located bisectors; and a
        // center vertex. The anchors and bisectors are double-upped so that each edge can have a
        // different distance varying to implement per-edge AA control. The center's distance is
        // tied to the edge between p1 and p2, so once all corners are processed there are 4 center
        // vertices, each interpolating distance to one edge.
        float3 devP0 = matrix * float3(p0, 1.0);
        float3 devP1 = matrix * float3(p1, 1.0);
        float3 devP2 = matrix * float3(p2, 1.0);

        // FIXME check cross produce and handle inverting scale factors
        float3 edge01 = line_between_points(devP0, devP1);
        float3 edge12 = line_between_points(devP1, devP2);

        float3 vertexPos;
        if (vertexID == 0 || vertexID == 1) {
            // Anchor at the outer corner
            vertexPos = devP0;
            edgeDistance = 0.5; // TODO: check AA edge flag for id = 0 or id = 1
        } else if (vertexID == 2) {
            // Outset from edge01
            // TODO: should be set to devP1 if edge01 is non-AA
            float2 n = float2(edge01.y, -edge01.x);
            vertexPos = line_intersection(line_through_point(devP1, n),
                                          shift_line_along_normal(edge01, 10.0));
            edgeDistance = 0.0;
        } else if (vertexID == 3) {
            // Outset from edge12
            // TODO: should be set to devP1 if edge12 is non-AA
            float2 n = float2(edge12.y, -edge12.x);
            vertexPos = line_intersection(line_through_point(devP1, n),
                                          shift_line_along_normal(edge12, 10.0));

            edgeDistance = 0.0;
        } else if (vertexID == 4 || vertexID == 5) {
            // Bisector to make sure AA coverage ramp doesn't get cut off for acute angles
            // TODO: should be set to devP1 if either edge is non-AA
            float2 n = bisect(edge01.xy, edge12.xy);
            vertexPos = translate_point(devP1, n, 10.0);
            edgeDistance = 0.0;
        } else {
            // Center vertex, calculate distance to edge12 (the previous corner will define the
            // central vertex with distance to edge01, since its edge12 == this edge01).
            vertexPos = matrix * float3(center, 1.0);
            edgeDistance = dot(vertexPos, edge12);
        }

        float4 devPosition = float4(vertexPos.xy, depth, vertexPos.z);
    )";
}

const char* AnalyticRRectRenderStep::fragmentCoverageSkSL() const {
    // TODO: May need to reduce upper coverage value for very thin rects
    // TODO: Provide a flag to calculate distance to curve per-pixel.
    return R"(
        outputCoverage = clamp(edgeDistance, 0.0, 1.0);
    )";
}

void AnalyticRRectRenderStep::writeVertices(DrawWriter* writer,
                                           const DrawParams& params,
                                           int ssboIndex) const {
    SkASSERT(params.geometry().isShape());
    const Shape& shape = params.geometry().shape();

    BindBufferInfo indices = writer->bufferManager()->getStaticBuffer(BufferType::kIndex,
                                                                      writeIndexBuffer,
                                                                      indexBufferSize);
    DrawWriter::Instances instance{*writer, {}, indices, 32*3};
    auto vw = instance.append(1);

    if (params.isStroke()) {
        SkASSERT(params.strokeStyle().halfWidth() >= 0.f);
        SkASSERT(shape.isRect() ||
                 (shape.isRRect() && SkRRectPriv::AllCornersCircular(shape.rrect())));

        // Write a negative value outside [-1, 0] to signal a stroked shape, and write style params
        vw << -2.f << params.strokeStyle().halfWidth() << params.strokeStyle().joinLimit() << 0.f;
        if (shape.isRRect()) {
            // X and Y radii are the same, but each corner could be different. Take X arbitrarily.
            const SkRRect& rrect = shape.rrect();
            vw << loadXRadii(rrect) << rrect.getBounds();
        } else {
            // All four corner radii are 0s, then write the LTRB of the rect directly
            vw << skvx::float4(0.f) << shape.rect().ltrb();
        }
    } else {
        // TODO: Add quadrilateral support to Shape with per-edge flags.
        if (shape.isRect() || (shape.isRRect() && shape.rrect().isRect())) {
            // Rectangles (or rectangles embedded in an SkRRect) are converted to the quadrilateral
            // case, but with all edges anti-aliased (== -1).
            skvx::float4 ltrb = shape.bounds().ltrb();
            vw << /*edge flags*/ skvx::float4(-1.f)
               << /*xs*/ skvx::shuffle<0,2,2,0>(ltrb)
               << /*ys*/ skvx::shuffle<1,1,3,3>(ltrb);
        } else {
            // A filled rounded rectangle
            const SkRRect& rrect = shape.rrect();
            SkASSERT(any(loadXRadii(rrect) > 0.f)); // If not the shader won't detect this case
            vw << loadXRadii(rrect) << loadYRadii(rrect) << shape.rrect().rect();
        }
    }

    // All instance types share the remaining instance attribute definitions
    const SkM44& m = params.transform().matrix();
    vw << params.order().depthAsFloat()
       << ssboIndex
       << params.transform().maxScaleFactor()
       << m.rc(0,0) << m.rc(1,0) << m.rc(3,0)
       << m.rc(0,1) << m.rc(1,1) << m.rc(3,1)
       << m.rc(0,3) << m.rc(1,3) << m.rc(3,3);
}

void AnalyticRRectRenderStep::writeUniformsAndTextures(const DrawParams&,
                                                       SkPipelineDataGatherer*) const {
    // All data is uploaded as instance attributes, so no uniforms are needed.
}

}  // namespace skgpu::graphite
