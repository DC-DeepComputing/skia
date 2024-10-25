/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/render/PerEdgeAAQuadRenderStep.h"

#include "include/core/SkM44.h"
#include "include/private/base/SkAssert.h"
#include "include/private/base/SkDebug.h"
#include "include/private/base/SkFloatingPoint.h"
#include "src/base/SkEnumBitMask.h"
#include "src/base/SkVx.h"
#include "src/core/SkSLTypeShared.h"
#include "src/gpu/BufferWriter.h"
#include "src/gpu/graphite/Attribute.h"
#include "src/gpu/graphite/BufferManager.h"
#include "src/gpu/graphite/DrawOrder.h"
#include "src/gpu/graphite/DrawParams.h"
#include "src/gpu/graphite/DrawTypes.h"
#include "src/gpu/graphite/DrawWriter.h"
#include "src/gpu/graphite/PipelineData.h"
#include "src/gpu/graphite/geom/EdgeAAQuad.h"
#include "src/gpu/graphite/geom/Geometry.h"
#include "src/gpu/graphite/geom/Rect.h"
#include "src/gpu/graphite/geom/Transform_graphite.h"
#include "src/gpu/graphite/render/CommonDepthStencilSettings.h"

#include <cstdint>
#include <string_view>

// This RenderStep is specialized to draw filled rectangles with per-edge AA.
//
// Each of these "primitives" is represented by a single instance. The instance attributes are
// flexible enough to describe per-edge AA quads without relying on uniforms to define its
// operation. The attributes encode shape as follows:
//
// float4 edgeFlags - per-edge AA defined by each component: aa != 0.
// float4 quadXs - these values provide the X coordinates of the quadrilateral in top-left CW order.
// float4 quadYs - these values provide the Y coordinates of the quadrilateral.
//
// From the other direction, per-edge AA quads produce instance values like:
//  - [aa(t,r,b,l) ? 255 : 0]   [xs(tl,tr,br,bl)]     [ys(tl,tr,br,bl)]
//
// From this encoding, data can be unpacked for each corner, which are equivalent under
// rotational symmetry. Per-edge quads are always mitered and fill the interior, but the
// vertices are placed such that the edge coverage ramps can collapse to 0 area on non-AA edges.
//
// The vertices that describe each corner are placed so that edges and miters calculate
// coverage by interpolating a varying and then clamping in the fragment shader. Triangles that
// cover the inner and outer curves calculate distance to the curve within the fragment shader.
//
// See https://docs.google.com/presentation/d/1MCPstNsSlDBhR8CrsJo0r-cZNbu-sEJEvU9W94GOJoY/edit?usp=sharing
// for diagrams and explanation of how the geometry is defined.
//
// PerEdgeAAQuadRenderStep uses the common technique of approximating distance to the level set by
// one expansion of the Taylor's series for the level set's equation. Given a level set function
// C(x,y), this amounts to calculating C(px,py)/|∇C(px,py)|. For the straight edges the level set
// is linear and calculated in the vertex shader and then interpolated exactly over the rectangle.
// This provides distances to all four exterior edges within the fragment shader and allows it to
// reconstruct a relative position per elliptical corner. Unfortunately this requires the fragment
// shader to calculate the length of the gradient for straight edges instead of interpolating
// exact device-space distance.
//
// Unlike AnalyticRRectRenderStep, for per-edge AA quads it's valid to have each pixel calculate a
// single corner's coverage that's controlled via the vertex shader. Any bias is a constant 1/2,
// so this is also added in the vertex shader.
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
namespace skgpu::graphite {

using AAFlags = EdgeAAQuad::Flags;

static bool is_clockwise(const EdgeAAQuad& quad) {
    if (quad.isRect()) {
        return true; // by construction, these are always locally clockwise
    }

    // This assumes that each corner has a consistent winding, which is the case for convex inputs,
    // which is an assumption of the per-edge AA API. Check the sign of cross product between the
    // first two edges.
    const skvx::float4& xs = quad.xs();
    const skvx::float4& ys = quad.ys();

    float winding = (xs[0] - xs[3])*(ys[1] - ys[0]) - (ys[0] - ys[3])*(xs[1] - xs[0]);
    if (winding == 0.f) {
        // The input possibly forms a triangle with duplicate vertices, so check the opposite corner
        winding = (xs[2] - xs[1])*(ys[3] - ys[2]) - (ys[2] - ys[1])*(xs[3] - xs[2]);
    }

    // At this point if winding is < 0, the quad's vertices are CCW. If it's still 0, the vertices
    // form a line, in which case the vertex shader constructs a correct CW winding. Otherwise,
    // the quad or triangle vertices produce a positive winding and are CW.
    return winding >= 0.f;
}

// Represents the per-vertex attributes used in each instance.
struct Vertex {
    SkV2 fNormal;
};

// Allowed values for the center weight instance value (selected at record time based on style
// and transform), and are defined such that when (insance-weight > vertex-weight) is true, the
// vertex should be snapped to the center instead of its regular calculation.
static constexpr int kCornerVertexCount = 4; // sk_VertexID is divided by this in SkSL
static constexpr int kVertexCount = 4 * kCornerVertexCount;
static constexpr int kIndexCount = 29;

static void write_index_buffer(VertexWriter writer) {
    static constexpr uint16_t kTL = 0 * kCornerVertexCount;
    static constexpr uint16_t kTR = 1 * kCornerVertexCount;
    static constexpr uint16_t kBR = 2 * kCornerVertexCount;
    static constexpr uint16_t kBL = 3 * kCornerVertexCount;

    static const uint16_t kIndices[kIndexCount] = {
        // Exterior AA ramp outset
        kTL+1,kTL+2,kTL+3,kTR+0,kTR+3,kTR+1,
        kTR+1,kTR+2,kTR+3,kBR+0,kBR+3,kBR+1,
        kBR+1,kBR+2,kBR+3,kBL+0,kBL+3,kBL+1,
        kBL+1,kBL+2,kBL+3,kTL+0,kTL+3,kTL+1,
        kTL+3,
        // Fill triangles
        kTL+3,kTR+3,kBL+3,kBR+3
    };

    if (writer) {
        writer << kIndices;
    } // otherwise static buffer creation failed, so do nothing; Context initialization will fail.
}

static void write_vertex_buffer(VertexWriter writer) {
    static constexpr float kHR2 = 0.5f * SK_FloatSqrt2; // "half root 2"

    // This template is repeated 4 times in the vertex buffer, for each of the four corners.
    // The vertex ID is used to lookup per-corner instance properties such as positions,
    // but otherwise this vertex data produces a consistent clockwise mesh from
    // TL -> TR -> BR -> BL.
    static constexpr Vertex kCornerTemplate[kCornerVertexCount] = {
        // Normals for device-space AA outsets from outer curve
        { {1.0f, 0.0f} },
        { {kHR2, kHR2} },
        { {0.0f, 1.0f} },

        // Normal for outer anchor (zero length to signal no local or device-space normal outset)
        { {0.0f, 0.0f} },
    };

    if (writer) {
        writer << kCornerTemplate  // TL
               << kCornerTemplate  // TR
               << kCornerTemplate  // BR
               << kCornerTemplate; // BL
    } // otherwise static buffer creation failed, so do nothing; Context initialization will fail.
}

PerEdgeAAQuadRenderStep::PerEdgeAAQuadRenderStep(StaticBufferManager* bufferManager)
        : RenderStep("PerEdgeAAQuadRenderStep",
                     "",
                     Flags::kPerformsShading | Flags::kEmitsCoverage | Flags::kOutsetBoundsForAA |
                     Flags::kUseNonAAInnerFill,
                     /*uniforms=*/{
                        {"d0", SkSLType::kFloat4},
                        {"d1", SkSLType::kFloat4},
                        {"d2", SkSLType::kFloat4},
                        {"d3", SkSLType::kFloat4},
                        {"d4", SkSLType::kFloat4},
                        {"d5", SkSLType::kFloat4},
                        {"d6", SkSLType::kFloat4},
                        {"d7", SkSLType::kFloat4},
                        {"d8", SkSLType::kFloat4},
                        {"d9", SkSLType::kFloat4},
                        {"d10", SkSLType::kFloat4},
                        {"d11", SkSLType::kFloat4},
                        {"d12", SkSLType::kFloat4},
                        {"d13", SkSLType::kFloat4},
                        {"d14", SkSLType::kFloat4},
                        {"d15", SkSLType::kFloat4},
                        {"p0", SkSLType::kFloat4},
                        {"p1", SkSLType::kFloat4},
                        {"p2", SkSLType::kFloat4},
                        {"p3", SkSLType::kFloat4},
                        {"p4", SkSLType::kFloat4},
                        {"p5", SkSLType::kFloat4},
                        {"p6", SkSLType::kFloat4},
                        {"p7", SkSLType::kFloat4},
                        {"p8", SkSLType::kFloat4},
                        {"p9", SkSLType::kFloat4},
                        {"p10", SkSLType::kFloat4},
                        {"p11", SkSLType::kFloat4},
                        {"p12", SkSLType::kFloat4},
                        {"p13", SkSLType::kFloat4},
                        {"p14", SkSLType::kFloat4},
                        {"p15", SkSLType::kFloat4},
                     },
                     PrimitiveType::kTriangleStrip,
                     kDirectDepthGreaterPass,
                     /*vertexAttrs=*/{
                            {"normal", VertexAttribType::kFloat2, SkSLType::kFloat2},
                     },
                     /*instanceAttrs=*/
                            {{"edgeFlags", VertexAttribType::kUByte4_norm, SkSLType::kFloat4},
                             {"quadXs", VertexAttribType::kFloat4, SkSLType::kFloat4},
                             {"quadYs", VertexAttribType::kFloat4, SkSLType::kFloat4},

                             // TODO: pack depth and ssbo index into one 32-bit attribute, if we can
                             // go without needing both render step and paint ssbo index attributes.
                             {"depth", VertexAttribType::kFloat, SkSLType::kFloat},
                             {"ssboIndices", VertexAttribType::kUInt2, SkSLType::kUInt2},

                             {"mat0", VertexAttribType::kFloat3, SkSLType::kFloat3},
                             {"mat1", VertexAttribType::kFloat3, SkSLType::kFloat3},
                             {"mat2", VertexAttribType::kFloat3, SkSLType::kFloat3}},
                     /*varyings=*/{
                             // Device-space distance to LTRB edges of quad.
                             {"edgeDistances", SkSLType::kFloat4}, // distance to LTRB edges
                     }) {
    // Initialize the static buffers we'll use when recording draw calls.
    // NOTE: Each instance of this RenderStep gets its own copy of the data. Since there should only
    // ever be one PerEdgeAAQuadRenderStep at a time, this shouldn't be an issue.
    write_vertex_buffer(bufferManager->getVertexWriter(sizeof(Vertex) * kVertexCount,
                                                       &fVertexBuffer));
    write_index_buffer(bufferManager->getIndexWriter(sizeof(uint16_t) * kIndexCount,
                                                     &fIndexBuffer));
}

PerEdgeAAQuadRenderStep::~PerEdgeAAQuadRenderStep() {}

std::string PerEdgeAAQuadRenderStep::vertexSkSL() const {
    // Returns the body of a vertex function, which must define a float4 devPosition variable and
    // must write to an already-defined float2 stepLocalCoords variable.
    return "float4 devPosition = per_edge_aa_quad_vertex_fn("
                   // Vertex Attributes
                   "normal, "
                   // Instance Attributes
                   "edgeFlags, quadXs, quadYs, depth, "
                   "float3x3(mat0, mat1, mat2), "
                   "float4[16](d0, d1, d2, d3, d4, d5, d6, d7, "
                   "d8, d9, d10, d11, d12, d13, d14, d15), "
                   "float4[16](p0, p1, p2, p3, p4, p5, p6, p7, "
                   "p8, p9, p10, p11, p12, p13, p14, p15), "
                   // Varyings
                   "edgeDistances, "
                   // Render Step
                   "stepLocalCoords);\n";
}

const char* PerEdgeAAQuadRenderStep::fragmentCoverageSkSL() const {
    // The returned SkSL must write its coverage into a 'half4 outputCoverage' variable (defined in
    // the calling code) with the actual coverage splatted out into all four channels.
    return "outputCoverage = per_edge_aa_quad_coverage_fn(sk_FragCoord, edgeDistances);";
}

namespace {

using skvx::float4;
using bool4 = skvx::Vec<4, bool>;
using bool2 = skvx::Vec<4, bool>;

using uint = uint32_t;

float4& assign(float4& l, const skvx::byte4& r) {
    l[0] = r[0];
    l[1] = r[1];
    l[2] = r[2];
    l[3] = r[3];
    return l;
}

SkV4 fromFloat4(const float4& v) {
    return {v[0], v[1], v[2], v[3]};
}

float sign(float s) {
    if (SkScalarNearlyEqual(s, 0.0f)) {
        return 0.0f;
    }
    return s < 0.0f ? -1.0f : 1.0f;
}

float4 sign4(const float4& v) {
    return {SkScalarNearlyEqual(v[0], 0.0f) ? 0.0f : 1.0f,
            SkScalarNearlyEqual(v[1], 0.0f) ? 0.0f : 1.0f,
            SkScalarNearlyEqual(v[2], 0.0f) ? 0.0f : 1.0f,
            SkScalarNearlyEqual(v[3], 0.0f) ? 0.0f : 1.0f};
}

bool4 equal4(const float4& a, const float4& b) {
    return {SkScalarNearlyEqual(a[0], b[0]),
            SkScalarNearlyEqual(a[1], b[1]),
            SkScalarNearlyEqual(a[2], b[2]),
            SkScalarNearlyEqual(a[3], b[3])};
}

bool any4(const bool4& v) { return v[0] || v[1] || v[2] || v[3]; }

bool all(const bool4& v) { return v[0] && v[1] && v[2] && v[3]; }

float4 mix(const float4& a, const float4& b, const float4& t) { return a + (b - a) * t; }

float4 inversesqrt(const float4& v) {
    return {1.0f / std::sqrt(v[0]),
            1.0f / std::sqrt(v[1]),
            1.0f / std::sqrt(v[2]),
            1.0f / std::sqrt(v[3])};
}

class float2;
class float3 {
public:
    float3() : fX(0.0f), fY(0.0f), fZ(0.0f) {}
    float3(float x, float y, float z) : fX(x), fY(y), fZ(z) {}

    float x() const { return fX; }
    float y() const { return fY; }
    float z() const { return fZ; }
    float2 xy() const;

    float3& operator+=(const float2& v);

private:
    float fX;
    float fY;
    float fZ;
};

class float2 {
public:
    float2() : fX(0.0f), fY(0.0f) {}
    float2(float f) : fX(f), fY(f) {}
    float2(float x, float y) : fX(x), fY(y) {}

    float x() const { return fX; }
    float y() const { return fY; }

    float operator[](int i) const { return i == 0 ? fX : fY; }

    float3 xy1() const { return {fX, fY, 1.0f}; }

    float2 operator-(const float2& other) const { return {fX - other.fX, fY - other.fY}; }

    float2 operator-() const { return {-fX, -fY}; }

    float2 operator+(const float2& other) const { return {fX + other.fX, fY + other.fY}; }

    float2 operator*(float s) const { return {fX * s, fY * s}; }

    float2 operator*(const float2& other) const { return {fX * other.fX, fY * other.fY}; }

    float2 normalize() const {
        float len = std::sqrt(fX * fX + fY * fY);
        return {fX / len, fY / len};
    }

private:
    float fX;
    float fY;
};

float2 operator*(float s, float2 v) { return v * s; }

float2 float3::xy() const { return {fX, fY}; }
float3& float3::operator+=(const float2& v) {
    fX += v.x();
    fY += v.y();
    return *this;
}

bool2 notEqual2(const float2& a, const float2& b) {
    return {!SkScalarNearlyEqual(a[0], b[0]), !SkScalarNearlyEqual(a[1], b[1])};
}

bool any2(const bool2& v) { return v[0] || v[1]; }

float2 perp(const float2& v) { return {-v.y(), v.x()}; }
float2 normalize(float2 s) { return s.normalize(); }
float dot(float2 a, float2 b) { return a.x() * b.x() + a.y() * b.y(); }
float cross_length_2d(float2 a, float2 b) { return a.x() * b.y() - a.y() * b.x(); }

class float2x2 {
public:
    float2x2(const float2& v0, const float2& v1) : fMat{v0, v1} {}
    float2 operator[](int i) const { return fMat[i]; }

private:
    float2 fMat[2];
};

float2 operator* (const float2& v, const float2x2& m) {
    return {v[0] * m[0][0] + v[1] * m[1][0], v[0] * m[0][1] + v[1] * m[1][1]};
}

class float3x3 {
public:
    float3x3() : fMat{} {}

    float3x3(const SkMatrix& skMat) : fMat(skMat) {}

    const SkMatrix& skMatrix() const { return fMat; }

    float3 operator[](int i) const {
        return {fMat.get(i), fMat.get(i + 3), fMat.get(i + 2 * 3)};
    }

    float3 operator*(const float3& v) {
        SkM44 m4{fMat};
        SkV4 v4 = {v.x(), v.y(), 0, v.z()};
        v4 = m4 * v4;
        return {v4.x, v4.y, v4.w};
    }

    float3x3 inverse() const {
        auto inversed = fMat;
        SkASSERT(fMat.invert(&inversed));
        return inversed;
    }

private:
    // row-major
    SkMatrix fMat;
};

float4 per_edge_aa_quad_vertex_fn(  // Vertex Attributes
        uint sk_VertexID,
        const float2& normal,
        // Instance Attributes
        float4 edgeAA,
        const float4& xs,  // ordered TL, TR, BR, BL
        const float4& ys,
        float depth,
        float3x3 localToDevice,
        // Varyings
        float4& edgeDistances,
        // Render Step
        float2& stepLocalCoords) {
    const uint kCornerVertexCount = 4;  // KEEP IN SYNC WITH C++'s
                                        // PerEdgeAAQuadRenderStep::kCornerVertexCount

    // Calculate the local edge vectors, ordered L, T, R, B starting from the bottom left point.
    // For quadrilaterals these are not necessarily axis-aligned, but in all cases they orient
    // the +X/+Y normalized vertex template for each corner.
    float4 dx = xs - xs.wxyz();
    float4 dy = ys - ys.wxyz();
    float4 edgeSquaredLen = dx * dx + dy * dy;

    float4 edgeMask = sign4(edgeSquaredLen);  // 0 for zero-length edge, 1 for non-zero edge.
    if (any4(equal4(edgeMask, float4(0.0)))) {
        // Must clean up (dx,dy) depending on the empty edge configuration
        if (all(equal4(edgeMask, float4(0.0)))) {
            // A point so use the canonical basis
            dx = float4(0.0, 1.0, 0.0, -1.0);
            dy = float4(-1.0, 0.0, 1.0, 0.0);
            edgeSquaredLen = float4(1.0);
        } else {
            // Triangles (3 non-zero edges) copy the adjacent edge. Otherwise it's a line so
            // replace empty edges with the left-hand normal vector of the adjacent edge.
            bool triangle = (edgeMask[0] + edgeMask[1] + edgeMask[2] + edgeMask[3]) > 2.5;
            float4 edgeX = triangle ? dx.yzwx() : dy.yzwx();
            float4 edgeY = triangle ? dy.yzwx() : -dx.yzwx();

            dx = mix(edgeX, dx, edgeMask);
            dy = mix(edgeY, dy, edgeMask);
            edgeSquaredLen = mix(edgeSquaredLen.yzwx(), edgeSquaredLen, edgeMask);
            edgeAA = mix(edgeAA.yzwx(), edgeAA, edgeMask);
        }
    }

    float4 inverseEdgeLen = inversesqrt(edgeSquaredLen);
    dx *= inverseEdgeLen;
    dy *= inverseEdgeLen;

    // Calculate local coordinate for the vertex (relative to xAxis and yAxis at first).
    uint cornerID = uint(sk_VertexID) / kCornerVertexCount;
    float2 xAxis = -float2(dx.yzwx()[cornerID], dy.yzwx()[cornerID]);
    float2 yAxis = float2(dx.xyzw()[cornerID], dy.xyzw()[cornerID]);

    // Vertex is outset from the base shape (and possibly with an additional AA outset later
    // in device space).
    float2 localPos = float2(xs[cornerID], ys[cornerID]);

    // Calculate edge distances and device space coordinate for the vertex
    edgeDistances = dy * (xs - float4(localPos.x())) - dx * (ys - float4(localPos.y()));

    // NOTE: This 3x3 inverse is different than just taking the 1st two columns of the 4x4
    // inverse of the original SkM44 local-to-device matrix. We could calculate the 3x3 inverse
    // and upload it, but it does not seem to be a bottleneck and saves on bandwidth to
    // calculate it here instead.
    float3x3 deviceToLocal = localToDevice.inverse();
    float3 devPos = localToDevice * localPos.xy1();

    // Apply the Jacobian in the vertex shader so any quadrilateral normals do not have to
    // be passed to the fragment shader. However, it's important to use the Jacobian at a
    // vertex on the edge, not the current vertex's Jacobian.
    float4 gx = -dy * (deviceToLocal[0].x() - deviceToLocal[0].z() * xs) +
                dx * (deviceToLocal[0].y() - deviceToLocal[0].z() * ys);
    float4 gy = -dy * (deviceToLocal[1].x() - deviceToLocal[1].z() * xs) +
                dx * (deviceToLocal[1].y() - deviceToLocal[1].z() * ys);
    // NOTE: The gradient is missing a W term so edgeDistances must still be multiplied by
    // 1/w in the fragment shader. The same goes for the encoded coverage scale.
    edgeDistances *= inversesqrt(gx * gx + gy * gy);

    // Bias non-AA edge distances by device W so its coverage contribution is >= 1.0
    // Add additional 1/2 bias here so we don't have to do so in the fragment shader.
    edgeDistances += (1.5 - edgeAA) * abs(devPos.z());

    // Only outset for a vertex that is in front of the w=0 plane to avoid dealing with outset
    // triangles rasterizing differently from the main triangles as w crosses 0.
    if (any2(notEqual2(normal, float2(0.0))) && devPos.z() > 0.0) {
        // Note that when there's no perspective, the jacobian is equivalent to the normal
        // matrix (inverse transpose), but produces correct results when there's perspective
        // because it accounts for the position's influence on a line's projected direction.
        float2x2 J = float2x2(deviceToLocal[0].xy() - deviceToLocal[0].z() * localPos,
                              deviceToLocal[1].xy() - deviceToLocal[1].z() * localPos);

        float2 edgeAANormal = float2(edgeAA[cornerID], edgeAA.yzwx()[cornerID]) * normal;
        float2 nx = edgeAANormal.x() * perp(-yAxis) * J;
        float2 ny = edgeAANormal.y() * perp(xAxis) * J;

        bool isMidVertex = all(notEqual2(edgeAANormal, float2(0)));
        if (isMidVertex) {
            // Produce a bisecting vector in device space.
            nx = normalize(nx);
            ny = normalize(ny);
            if (dot(nx, ny) < -0.8) {
                // Normals are in nearly opposite directions, so adjust to avoid float error.
                float s = sign(cross_length_2d(nx, ny));
                nx = s * perp(nx);
                ny = -s * perp(ny);
            }
        }
        // Adding the normal components together directly results in what we'd have
        // calculated if we'd just transformed 'normal' in one go, assuming they weren't
        // normalized in the if-block above. If they were normalized, the sum equals the
        // bisector between the original nx and ny.
        //
        // We multiply by W so that after perspective division the new point is offset by the
        // now-unit normal.
        // NOTE: (nx + ny) can become the zero vector if the device outset is for an edge
        // marked as non-AA. In this case normalize() could produce the zero vector or NaN.
        // Until a counter-example is found, GPUs seem to discard triangles with NaN vertices,
        // which has the same effect as outsetting by the zero vector with this mesh, so we
        // don't bother guarding the normalize() (yet).
        devPos += devPos.z() * normalize(nx + ny);

        // By construction these points are 1px away from the outer edge in device space.
        // Apply directly to edgeDistances to save work per pixel later on.
        edgeDistances -= devPos.z();
    }

    // Write out final results
    stepLocalCoords = localPos;
    return float4(devPos.x(), devPos.y(), devPos.z() * depth, devPos.z());
}
}
void PerEdgeAAQuadRenderStep::writeVertices(DrawWriter* writer,
                                           const DrawParams& params,
                                           skvx::uint2 ssboIndices) const {
    SkASSERT(params.geometry().isEdgeAAQuad());
    const EdgeAAQuad& quad = params.geometry().edgeAAQuad();

    DrawWriter::Instances instance{*writer, fVertexBuffer, fIndexBuffer, kIndexCount};
    auto vw = instance.append(1);

    // Empty fills should not have been recorded at all.
    SkDEBUGCODE(Rect bounds = params.geometry().bounds());
    SkASSERT(!bounds.isEmptyNegativeOrNaN());

    constexpr uint8_t kAAOn = 255;
    constexpr uint8_t kAAOff = 0;
    auto edgeSigns = skvx::byte4{quad.edgeFlags() & AAFlags::kLeft   ? kAAOn : kAAOff,
                                 quad.edgeFlags() & AAFlags::kTop    ? kAAOn : kAAOff,
                                 quad.edgeFlags() & AAFlags::kRight  ? kAAOn : kAAOff,
                                 quad.edgeFlags() & AAFlags::kBottom ? kAAOn : kAAOff};

    // The vertex shader expects points to be in clockwise order. EdgeAAQuad is the only
    // shape that *might* have counter-clockwise input.
    if (is_clockwise(quad)) {
        vw << edgeSigns << quad.xs() << quad.ys();
    } else {
        vw << skvx::shuffle<2,1,0,3>(edgeSigns)  // swap left and right AA bits
           << skvx::shuffle<1,0,3,2>(quad.xs())  // swap TL with TR, and BL with BR
           << skvx::shuffle<1,0,3,2>(quad.ys()); //   ""
    }

    // All instance types share the remaining instance attribute definitions
    const SkM44& m = params.transform().matrix();

    vw << params.order().depthAsFloat()
       << ssboIndices
       << m.rc(0,0) << m.rc(1,0) << m.rc(3,0)  // mat0
       << m.rc(0,1) << m.rc(1,1) << m.rc(3,1)  // mat1
       << m.rc(0,3) << m.rc(1,3) << m.rc(3,3); // mat2
}

void PerEdgeAAQuadRenderStep::writeUniformsAndTextures(const DrawParams& params,
                                                       PipelineDataGatherer* gatherer) const {
    SkASSERT(params.geometry().isEdgeAAQuad());
    const EdgeAAQuad& quad = params.geometry().edgeAAQuad();

    // Empty fills should not have been recorded at all.
    SkDEBUGCODE(Rect bounds = params.geometry().bounds());
    SkASSERT(!bounds.isEmptyNegativeOrNaN());

    SkDEBUGCODE(UniformExpectationsValidator uev(gatherer, this->uniforms());)

    float4 edgeAA;
    float4 xs;
    float4 ys;
    float depth;
    float3x3 localToDevice;

    constexpr uint8_t kAAOn = 1;
    constexpr uint8_t kAAOff = 0;
    auto edgeSigns = skvx::byte4{quad.edgeFlags() & AAFlags::kLeft   ? kAAOn : kAAOff,
                                 quad.edgeFlags() & AAFlags::kTop    ? kAAOn : kAAOff,
                                 quad.edgeFlags() & AAFlags::kRight  ? kAAOn : kAAOff,
                                 quad.edgeFlags() & AAFlags::kBottom ? kAAOn : kAAOff};

    // The vertex shader expects points to be in clockwise order. EdgeAAQuad is the only
    // shape that *might* have counter-clockwise input.
    if (is_clockwise(quad)) {
        assign(edgeAA, edgeSigns);
        xs = quad.xs();
        ys = quad.ys();
    } else {
        assign(edgeAA, skvx::shuffle<2,1,0,3>(edgeSigns));
        xs = skvx::shuffle<1,0,3,2>(quad.xs());
        ys = skvx::shuffle<1,0,3,2>(quad.ys());
    }

    depth = params.order().depthAsFloat();
    localToDevice = float3x3(params.transform().matrix().asM33());

    float4 devPosArray[kVertexCount];
    float4 edgeDistancesArray[kVertexCount];
    float2 stepLocalCoordsArray[kVertexCount];

    static constexpr float kHR2 = 0.5f * SK_FloatSqrt2; // "half root 2"
    float2 normalArray[kCornerVertexCount] = {
        {1.0f, 0.0f},
        {kHR2, kHR2},
        {0.0f, 1.0f},
        {0.0f, 0.0f},
    };
    for (uint vertex = 0; vertex < kVertexCount; ++vertex) {
        float4& edgeDistances = edgeDistancesArray[vertex];
        float2& stepLocalCoords = stepLocalCoordsArray[vertex];
        float4& devPos = devPosArray[vertex];

        float2 normal = normalArray[vertex % kCornerVertexCount];

        devPos = per_edge_aa_quad_vertex_fn(vertex, normal,
                                                    edgeAA, xs, ys, depth, localToDevice,
                                                    edgeDistances, stepLocalCoords);
    }
    for (uint vertex = 0; vertex < kVertexCount; ++vertex) {
        gatherer->write(fromFloat4(edgeDistancesArray[vertex]));
    }
    for (uint vertex = 0; vertex < kVertexCount; ++vertex) {
        gatherer->write(fromFloat4(devPosArray[vertex]));
    }
}

}  // namespace skgpu::graphite
