/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/render/CircularArcRenderStep.h"

#include "include/core/SkM44.h"
#include "src/core/SkSLTypeShared.h"
#include "src/gpu/BufferWriter.h"
#include "src/gpu/graphite/Attribute.h"
#include "src/gpu/graphite/DrawOrder.h"
#include "src/gpu/graphite/DrawParams.h"
#include "src/gpu/graphite/DrawTypes.h"
#include "src/gpu/graphite/DrawWriter.h"
#include "src/gpu/graphite/geom/Geometry.h"
#include "src/gpu/graphite/geom/Shape.h"
#include "src/gpu/graphite/geom/Transform_graphite.h"
#include "src/gpu/graphite/render/CommonDepthStencilSettings.h"
//#include "include/core/SkPaint.h"
//#include "include/core/SkRRect.h"
//#include "include/core/SkScalar.h"
//#include "include/private/base/SkAssert.h"
//#include "include/private/base/SkFloatingPoint.h"
//#include "include/private/base/SkPoint_impl.h"
//#include "src/base/SkEnumBitMask.h"
//#include "src/base/SkVx.h"
//#include "src/core/SkRRectPriv.h"
//#include "src/gpu/graphite/BufferManager.h"
//#include "src/gpu/graphite/geom/EdgeAAQuad.h"
//#include "src/gpu/graphite/geom/Rect.h"
//
//#include <cstdint>
//#include <string_view>


//***** Fix this
// This RenderStep is flexible and can draw filled rectangles, filled quadrilaterals with per-edge
// AA, filled rounded rectangles with arbitrary corner radii, stroked rectangles with any join,
// stroked lines with any cap, stroked rounded rectangles with circular corners (each corner can be
// different or square), hairline rectangles, hairline lines, and hairline rounded rectangles with
// arbitrary corners.
//
// We combine all of these together to maximize batching across simple geometric draws and reduce
// the number pipeline specializations. Additionally, these primitives are the most common
// operations and help us avoid triggering MSAA.
//
// Each of these "primitives" is represented by a single instance. The instance attributes are
// flexible enough to describe any of the above shapes without relying on uniforms to define its
// operation. The attributes encode shape as follows:
//
// float4 xRadiiOrFlags - if any components is > 0, the instance represents a filled round rect
//    with elliptical corners and these values specify the X radii in top-left CW order.
//    Otherwise, if .x < -1, the instance represents a stroked or hairline [round] rect or line,
//    where .y differentiates hairline vs. stroke. If .y is negative, then it is a hairline [round]
//    rect and xRadiiOrFlags stores (-2 - X radii); if .y is zero, it is a regular stroked [round]
//    rect; if .y is positive, then it is a stroked *or* hairline line. For .y >= 0, .z holds the
//    stroke radius and .w stores the join limit (matching StrokeStyle's conventions).
//    Lastly, if -1 <= .x <= 0, it's a filled quadrilateral with per-edge AA defined by each by the
//    component: aa != 0.
// float4 radiiOrQuadXs - if in filled round rect or hairline [round] rect mode, these values
//    provide the Y radii in top-left CW order. If in stroked [round] rect mode, these values
//    provide the circular corner radii (same order). Otherwise, when in per-edge quad mode, these
//    values provide the X coordinates of the quadrilateral (same order).
// float4 ltrbOrQuadYs - if in filled round rect mode or stroked [round] rect mode, these values
//    define the LTRB edge coordinates of the rectangle surrounding the round rect (or the
//    rect itself when the radii are 0s). In stroked line mode, LTRB is treated as (x0,y0) and
//    (x1,y1) that defines the line. Otherwise, in per-edge quad mode, these values provide
//    the Y coordinates of the quadrilateral.
//
// From the other direction, shapes produce instance values like:
//  - filled rect:    [-1 -1 -1 -1]            [L R R L]             [T T B B]
//  - stroked rect:   [-2 0 stroke join]       [0 0 0 0]             [L T R B]
//  - hairline rect:  [-2 -2 -2 -2]            [0 0 0 0]             [L T R B]
//  - filled rrect:   [xRadii(tl,tr,br,bl)]    [yRadii(tl,tr,br,bl)] [L T R B]
//  - stroked rrect:  [-2 0 stroke join]       [radii(tl,tr,br,bl)]  [L T R B]
//  - hairline rrect: [-2-xRadii(tl,tr,br,bl)] [radii(tl,tr,br,bl)]  [L T R B]
//  - filled line:    N/A, discarded higher in the stack
//  - stroked line:   [-2 1 stroke cap]        [0 0 0 0]             [x0,y0,x1,y1]
//  - hairline line:  [-2 1 0 1]               [0 0 0 0]             [x0,y0,x1,y1]
//  - per-edge quad:  [aa(t,r,b,l) ? -1 : 0]   [xs(tl,tr,br,bl)]     [ys(tl,tr,br,bl)]
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
//
// CircularArcRenderStep uses the common technique of approximating distance to the level set by
// one expansion of the Taylor's series for the level set's equation. Given a level set function
// C(x,y), this amounts to calculating C(px,py)/|∇C(px,py)|. For the straight edges the level set
// is linear and calculated in the vertex shader and then interpolated exactly over the rectangle.
// This provides distances to all four exterior edges within the fragment shader and allows it to
// reconstruct a relative position per elliptical corner. Unfortunately this requires the fragment
// shader to calculate the length of the gradient for straight edges instead of interpolating
// exact device-space distance.
//
// All four corner radii are potentially evaluated by the fragment shader although each corner's
// coverage is only calculated when the pixel is within the bounding box of its quadrant. For fills
// and simple strokes it's theoretically valid to have each pixel calculate a single corner's
// coverage that was controlled via the vertex shader. However, testing all four corners is
// necessary in order to correctly handle self-intersecting stroke interiors. Similarly, all four
// edges must be evaluated in order to handle extremely thin shapes; whereas often you could get
// away with tracking a single edge distance per pixel.
//
// Analytic derivatives are used so that a single pipeline can be used regardless of HW derivative
// support or for geometry that would prove difficult for forward differencing. The device-space
// gradient for ellipses is calculated per-pixel by transforming a per-pixel local gradient vector
// with the Jacobian of the inverse local-to-device transform:
//
// (px,py) is the projected point of (u,v) transformed by a 3x3 matrix, M:
//                [x(u,v) / w(u,v)]       [x]   [m00 m01 m02] [u]
//      (px,py) = [y(u,v) / w(u,v)] where [y] = [m10 m11 m12]X[v] = M*(u,v,1)
//                                        [w]   [m20 m21 m22] [1]
//
// C(px,py) can be defined in terms of a local Cl(u,v) as C(px,py) = Cl(p^-1(px,py)), where p^-1 =
//
//               [x'(px,py) / w'(px,py)]       [x']   [m00' m01' * m02'] [px]
//      (u,v) =  [y'(px,py) / w'(px,py)] where [y'] = [m10' m11' * m12']X[py] = M^-1*(px,py,0,1)
//                                             [w']   [m20' m21' * m22'] [ 1]
//
// Note that if the 3x3 M was arrived by dropping the 3rd row and column from a 4x4 since we assume
// a local 3rd coordinate of 0, M^-1 is not equal to the 4x4 inverse with dropped rows and columns.
//
// Using the chain rule, then ∇C(px,py)
//   =  ∇Cl(u,v)X[1/w'(px,py)     0       -x'(px,py)/w'(px,py)^2] [m00' m01']
//               [    0       1/w'(px,py) -y'(px,py)/w'(px,py)^2]X[m10' m11']
//                                                                [m20' m21']
//
//   = 1/w'(px,py)*∇Cl(u,v)X[1 0 -x'(px,py)/w'(px,py)] [m00' m01']
//                          [0 1 -y'(px,py)/w'(px,py)]X[m10' m11']
//                                                     [m20' m21']
//
//   = w(u,v)*∇Cl(u,v)X[1 0 0 -u] [m00' m01']
//                     [0 1 0 -v]X[m10' m11']
//                                [m20' m21']
//
//   = w(u,v)*∇Cl(u,v)X[m00'-m20'u m01'-m21'u]
//                     [m10'-m20'v m11'-m21'v]
//
// The vertex shader calculates the rightmost 2x2 matrix and interpolates it across the shape since
// each component is linear in (u,v). ∇Cl(u,v) is evaluated per pixel in the fragment shader and
// depends on which corner and edge being evaluated. w(u,v) is the device-space W coordinate, so
// its reciprocal is provided in sk_FragCoord.w.
//*****
namespace skgpu::graphite {

// Represents the per-vertex attributes used in each instance.
struct Vertex {
    SkV2 fPosition;
};

static constexpr int kVertexCount = 9;
static constexpr int kIndexCount = 24;

static void write_index_buffer(VertexWriter writer) {
    static constexpr uint16_t kCenter = 0;
    static constexpr uint16_t kEdgeStart = 1;

    static const uint16_t kIndices[kIndexCount] = {
        // Eight wedges
        kCenter, kEdgeStart+0, kEdgeStart+1,
        kCenter, kEdgeStart+1, kEdgeStart+2,
        kCenter, kEdgeStart+2, kEdgeStart+3,
        kCenter, kEdgeStart+3, kEdgeStart+4,
        kCenter, kEdgeStart+4, kEdgeStart+5,
        kCenter, kEdgeStart+5, kEdgeStart+6,
        kCenter, kEdgeStart+6, kEdgeStart+7,
        kCenter, kEdgeStart+7, kEdgeStart+0,
    };

    if (writer) {
        writer << kIndices;
    } // otherwise static buffer creation failed, so do nothing; Context initialization will fail.
}

static void write_vertex_buffer(VertexWriter writer) {
    // Normalized geometry for octagons that circumscribe and lie on a unit circle:
    static constexpr SkScalar kOctOffset = 0.41421356237f;  // sqrt(2) - 1

    static constexpr SkPoint kOctagonVertices[kVertexCount] = {
        { 0,           0},
        {-kOctOffset, -1},
        { kOctOffset, -1},
        { 1,          -kOctOffset},
        { 1,           kOctOffset},
        { kOctOffset,  1},
        {-kOctOffset,  1},
        {-1,           kOctOffset},
        {-1,          -kOctOffset},
    };

    if (writer) {
        writer << kOctagonVertices;
    } // otherwise static buffer creation failed, so do nothing; Context initialization will fail.
}

CircularArcRenderStep::CircularArcRenderStep(StaticBufferManager* bufferManager)
        : RenderStep("CircularArcRenderStep",
                     "",
                     Flags::kPerformsShading | Flags::kEmitsCoverage | Flags::kOutsetBoundsForAA,
                     /*uniforms=*/{},
                     PrimitiveType::kTriangles,
                     kDirectDepthGreaterPass,
                     /*vertexAttrs=*/{
                             {"position", VertexAttribType::kFloat2, SkSLType::kFloat2},
                     },
                     /*instanceAttrs=*/{
                             // Center plus half width, used to transform to devPosition
                             {"devXform", VertexAttribType::kFloat3, SkSLType::kFloat3},
                             // Outer (device space) and inner (normalized) radii
                             {"radii", VertexAttribType::kFloat2, SkSLType::kFloat2},
                             // Half-planes used to clip to arc shape.
                             {"inClipPlane", VertexAttribType::kFloat3, SkSLType::kFloat3},
                             {"inIsectPlane", VertexAttribType::kFloat3, SkSLType::kFloat3},
                             {"inUnionPlane", VertexAttribType::kFloat3, SkSLType::kFloat3},
                     },
                     /*varyings=*/{
                             // Normalized offset vector plus radii
                             {"circleEdge", SkSLType::kFloat4},
                             // Half-planes used to clip to arc shape.
                             {"clipPlane", SkSLType::kFloat3},
                             {"isectPlane", SkSLType::kFloat3},
                             {"unionPlane", SkSLType::kFloat3},
                     }) {
    // Initialize the static buffers we'll use when recording draw calls.
    // NOTE: Each instance of this RenderStep gets its own copy of the data. Since there should only
    // ever be one CircularArcRenderStep at a time, this shouldn't be an issue.
    write_vertex_buffer(bufferManager->getVertexWriter(sizeof(Vertex) * kVertexCount,
                                                       &fVertexBuffer));
    write_index_buffer(bufferManager->getIndexWriter(sizeof(uint16_t) * kIndexCount,
                                                     &fIndexBuffer));
}

CircularArcRenderStep::~CircularArcRenderStep() {}

std::string CircularArcRenderStep::vertexSkSL() const {
    // Returns the body of a vertex function, which must define a float4 devPosition variable and
    // must write to an already-defined float2 stepLocalCoords variable.
    return "float4 devPosition = circular_arc_vertex_fn("
                   // Vertex Attributes
                   "position, "
                   // Instance Attributes
                   "devXform, radii, inClipPlane, inIsectPlane, inUnionPlane, depth, "
                   // Varyings
                   "circleEdge, clipPlane, isectPlane, unionPlane, "
                   // Render Step
                   "stepLocalCoords);\n";
}

const char* CircularArcRenderStep::fragmentCoverageSkSL() const {
    // The returned SkSL must write its coverage into a 'half4 outputCoverage' variable (defined in
    // the calling code) with the actual coverage splatted out into all four channels.
    return "outputCoverage = circular_arc_coverage_fn(circleEdge, "
                                                     "clipPlane, "
                                                     "isectPlane, "
                                                     "unionPlane);";
}

void CircularArcRenderStep::writeVertices(DrawWriter* writer,
                                          const DrawParams& params,
                                          skvx::uint2 ssboIndices) const {
    SkASSERT(params.geometry().isShape() && params.geometry().shape().isArc());

    DrawWriter::Instances instance{*writer, fVertexBuffer, fIndexBuffer, kIndexCount};
    auto vw = instance.append(1);

    // The bounds of a arc is the rect.
    Rect bounds = params.geometry().bounds();

    const Shape& shape = params.geometry().shape();
    const SkArc& arc = shape.arc();



    // All instance types share the remaining instance attribute definitions
//    const SkM44& m = params.transform().matrix();
//    auto center = params.geometry().isEdgeAAQuad() ? quad_center(params.geometry().edgeAAQuad())
//                                                   : bounds.center();
//    vw << center << centerWeight << aaRadius
//       << params.order().depthAsFloat()
//       << ssboIndices
//       << m.rc(0,0) << m.rc(1,0) << m.rc(3,0)  // mat0
//       << m.rc(0,1) << m.rc(1,1) << m.rc(3,1)  // mat1
//       << m.rc(0,3) << m.rc(1,3) << m.rc(3,3); // mat2
}

void CircularArcRenderStep::writeUniformsAndTextures(const DrawParams&,
                                                     PipelineDataGatherer*) const {
    // All data is uploaded as instance attributes, so no uniforms are needed.
}

}  // namespace skgpu::graphite
