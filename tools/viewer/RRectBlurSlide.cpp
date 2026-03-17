/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "imgui.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkBlurTypes.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkFont.h"
#include "include/core/SkImage.h"
#include "include/core/SkMaskFilter.h"
#include "include/core/SkMesh.h"
#include "include/core/SkPath.h"
#include "include/core/SkPathBuilder.h"
#include "include/core/SkPathIter.h"
#include "include/core/SkRRect.h"
#include "include/effects/SkRuntimeEffect.h"
#include "src/core/SkMeshPriv.h"
#include "src/core/SkRuntimeEffectPriv.h"
#include "tools/fonts/FontToolUtils.h"
#include "tools/viewer/Slide.h"

#include <cmath>

static constexpr char kMeshVS[] = R"(
uniform float3x3 ctm;

Varyings main(const Attributes attrs) {
    Varyings v;
    float3 dev = ctm * float3(attrs.pos, 1.0);
    v.position = dev.xy;
    v.localPos = attrs.pos;
    v.flags = attrs.flags;
    return v;
}
)";

static constexpr char kMeshFS[] = R"(
uniform float3x3 ctm;
uniform float4 rect;
uniform float2 radii[4];
uniform half2 one_over_sqrt2_sigma;
uniform int2 radius;

uniform shader cdf_lut;

const half lut_range = 4.0;
const half lut_size = 256.0;
const bool use_cdf_lut = true;     // Choose between approx computation or LUT for Gaussian CDF
const bool use_gauss_lut = false;  // Enable if exp() is slow (e.g. older Mali/Adreno GPUs)

// We use "u" to mean x/(sigma*sqrt(2)) (for a blur, the distribution mean is always at 0)

// R = CDF, G = exp(-u^2)
half2 lut_sample(half u) {
    float coord = float((u + lut_range) * (lut_size / (2.0 * lut_range)));
    return cdf_lut.eval(float2(coord, 0.5)).rg;
}

half cdf_approx(half u) {
    half s = sign(u);
    half a = abs(u);
    half p = half(1.0) + (half(0.278393) + (half(0.230389) + half(0.078108) * (a * a)) * a) * a;
    p *= p;
    half r = half(0.5) / (p * p);
    return half(0.5) + s * (half(0.5) - r);
}

half cdf(half u) {
    if (use_cdf_lut) {
        return lut_sample(u).r;
    }
    return cdf_approx(u);
}

// CDF of the Gaussian at a spatial offset from the pixel center.
half spatial_cdf(float offset, half one_over_sqrt2_sigma) {
    return cdf(offset * one_over_sqrt2_sigma);
}

// Unnormalized Gaussian weight exp(-u^2) at spatial offset from pixel center.
half gauss_weight(half offset, half one_over_sqrt2_sigma) {
    half u = offset * one_over_sqrt2_sigma;
    if (use_gauss_lut) {
        return lut_sample(u).g;
    }
    return exp(-u * u);
}

// Compute the corner correction via Gauss-Legendre 4 point quadrature.
// Returns the amount to add to the rect base blur.
//
// Always evaluates in "TL-equivalent" space: the arc indents in +x from
// left_x, and sweeps in +y (down) from top_y. Callers reflect pos and cdf_left
// so that every corner looks like a TL corner to this function.
//
// The correction is the integral of g(y) * (cdf_left - CDF(edge_x(y))) dy
// where g(y) is the normalized gaussian kernel weight at y.
//
// We call the horizontal offset between the bounding rect's left edge and
// the corner arc at a particular y offset from top_y the "indent".
// indent(dy) = rx at dy=0 and decreases to 0 at dy=ry. This gives
// up to three ranges in y:
//   1. Near y_start (max indent):
//      If pos.x is far enough away from corner in x then
//      CDF(edge) ~= 1 -> correction ~= cdf_left - 1
//   2. Transition: y range where the corner arc passes through pos.x +/- radius.x
//   3. Near y_end (small indent):
//      If pos.x is far enough inside the corner then
//      edge ~= left_x -> CDF ~= cdf_left -> correction ~= 0
// We focus GL4 on the transition region (2) and add region (1) analytically.
half eval_corner(float2 pos, float top_y, float left_x,
                 half rx, half ry, half cdf_left) {
    half y_start = half(top_y - pos.y);
    half y_end = y_start + ry;

    half cdf_y_lo;
    half cdf_y_hi;
    half y_lo;
    half y_hi;
    if (y_start < -half(radius.y)) {
        y_lo = -half(radius.y);
        cdf_y_lo = 0;
    } else {
        y_lo = y_start;
        cdf_y_lo = spatial_cdf(y_lo, one_over_sqrt2_sigma.y);
    }

    if (y_end > half(radius.y)) {
        y_hi = half(radius.y);
        cdf_y_hi = 1;
    } else {
        y_hi = y_end;
        cdf_y_hi = spatial_cdf(y_hi, one_over_sqrt2_sigma.y);
    }

    if (cdf_y_hi - cdf_y_lo < 0.001) return half(0.0);

    half rx_over_ry = rx / ry;

    // Find the y range where the corner edge is within pos.x +/- radius.x.
    // horizontal indent decreases with dy: rx at dy=0, 0 at dy=ry.
    float dist = pos.x - left_x;
    half indent_lo = half(max(0.0, dist - float(radius.x)));
    half indent_hi = half(min(float(rx), dist + float(radius.x)));

    if (indent_lo >= indent_hi) {
        return half(0.0);
    }

    // Convert indent bounds to dy (arc distance from y_start).
    // indent = rx - (rx/ry)*sqrt(dy*(2ry-dy))
    // Solving: t = (rx - indent)/rx, dy = ry*(1 - sqrt(1 - t*t))
    // Larger indent -> smaller dy (closer to y_start).
    half dy_at_indent_hi = half(0.0);  // closer to y_start
    if (indent_hi < rx) {
        half t = (rx - indent_hi) / rx;
        dy_at_indent_hi = ry * (half(1.0) - sqrt(max(half(0.0), half(1.0) - t * t)));
    }

    half dy_at_indent_lo = ry;  // closer to y_end
    if (indent_lo > half(0.0)) {
        half t = (rx - indent_lo) / rx;
        dy_at_indent_lo = ry * (half(1.0) - sqrt(max(half(0.0), half(1.0) - t * t)));
    }

    // Convert dy to pixel-relative y, clamp to visible range, and compute cdfs.
    half gl4_lo = y_start + dy_at_indent_hi;
    half cdf_gl4_lo;
    if (gl4_lo < y_lo) {
        gl4_lo = y_lo;
        cdf_gl4_lo = cdf_y_lo;
    } else {
        cdf_gl4_lo = spatial_cdf(float(gl4_lo), one_over_sqrt2_sigma.y);
    }

    half gl4_hi = y_start + dy_at_indent_lo;
    half cdf_gl4_hi;
    if (gl4_hi > y_hi) {
        gl4_hi = y_hi;
        cdf_gl4_hi = cdf_y_hi;
    } else {
        cdf_gl4_hi = spatial_cdf(float(gl4_hi), one_over_sqrt2_sigma.y);
    }

    half gl4_range = cdf_gl4_hi - cdf_gl4_lo;
    half result = half(0.0);
    // Saturated correction for the region near y_start (large indent).
    // The edge is far to the right of pos.x, so CDF(edge) ~= 1.
    // This region is [y_lo, gl4_lo].
    half beyond_range = cdf_gl4_lo - cdf_y_lo;
    if (beyond_range > 0) {
        result += (cdf_left - half(1.0)) * beyond_range;
    }

    // GL4 quadrature over the transition region.
    if (gl4_range < 0.001) {
        return result;
    }

    // Gaussian-Legendre 4 point constants:
    const half4 qn = half4(-0.8611363, -0.3399810, 0.3399810, 0.8611363);
    const half4 qw = half4( 0.3478548,  0.6521452, 0.6521452, 0.3478548);

    half center   = half(0.5) * (gl4_lo + gl4_hi);
    half half_len = half(0.5) * (gl4_hi - gl4_lo);

    half weighted_edge = half(0.0);
    half total_weight  = half(0.0);
    for (int i = 0; i < 4; i++) {
        half y  = center + half_len * qn[i];
        half dy = y - y_start;
        // Compute in float to avoid half-precision overflow when ry is large
        half indent = rx - half(float(rx_over_ry) *
            sqrt(max(0.0, float(dy) * (2.0 * float(ry) - float(dy)))));
        half edge_cdf = spatial_cdf(left_x + float(indent) - pos.x,
                                    one_over_sqrt2_sigma.x);
        half w = gauss_weight(y, one_over_sqrt2_sigma.y) * qw[i];
        weighted_edge += w * edge_cdf;
        total_weight  += w;
    }
    if (total_weight >= 0.001) {
        half avg_edge = weighted_edge / total_weight;
        result += gl4_range * (cdf_left - avg_edge);
    }
    return result;
}

float2 main(const Varyings varyings, out half4 color) {
    float2 pos = varyings.localPos;
    half4 f = varyings.flags;  // x=doTL, y=doTR, z=doBR, w=doBL
                               // Inside encoded as all channels ~0.5.

    // Fully inside: all CDFs saturated, no corners needed.
    if (f.x > 0.25 && f.x < 0.75) {
        color = half4(0, 0, 0, 1);
        return pos;
    }

    // Rect base: 4 CDF lookups
    half cdf_l = spatial_cdf(rect.x - pos.x, one_over_sqrt2_sigma.x);
    half cdf_r = spatial_cdf(rect.z - pos.x, one_over_sqrt2_sigma.x);
    half cdf_t = spatial_cdf(rect.y - pos.y, one_over_sqrt2_sigma.y);
    half cdf_b = spatial_cdf(rect.w - pos.y, one_over_sqrt2_sigma.y);
    half v = (cdf_r - cdf_l) * (cdf_b - cdf_t);

    // Per-corner corrections, gated by individual corner flags.
    // Each call reflects pos into TL-equivalent space so eval_corner
    // always sees the arc indenting in +x and sweeping in +y.
    // X-reflection: pos.x -> 2*left_x - pos.x, cdf_l -> 1 - cdf_r.
    // Y-reflection: pos.y -> 2*top_y - pos.y (cdf_left unchanged).
    if (f.x > 0.5) {  // TL: no reflection
        v += eval_corner(pos, rect.y, rect.x,
                         half(radii[0].x), half(radii[0].y), cdf_l);
    }
    if (f.y > 0.5) {  // TR: reflect X
        float2 p = float2(2.0 * rect.z - pos.x, pos.y);
        v += eval_corner(p, rect.y, rect.z,
                         half(radii[1].x), half(radii[1].y), half(1.0) - cdf_r);
    }
    if (f.z > 0.5) {  // BR: reflect X and Y
        float2 p = float2(2.0 * rect.z - pos.x, 2.0 * rect.w - pos.y);
        v += eval_corner(p, rect.w, rect.z,
                         half(radii[2].x), half(radii[2].y), half(1.0) - cdf_r);
    }
    if (f.w > 0.5) {  // BL: reflect Y
        float2 p = float2(pos.x, 2.0 * rect.w - pos.y);
        v += eval_corner(p, rect.w, rect.x,
                         half(radii[3].x), half(radii[3].y), cdf_l);
    }

    color = half4(0, 0, 0, v);
    return pos;
}
)";

// Blender used to visually compare pixel differences between techniques.
static constexpr char kAbsDiffBlender[] = R"(
uniform float scale;
half4 main(half4 src, half4 dst) {
    float diff = clamp(scale * (src.a - dst.a), -1, 1);
    if (diff > 0) {
        return half4(diff, 0, 0, diff);
    } else if (diff < 0) {
        return half4(0, 0, -diff, -diff);
    }
    return half4(0);
}
)";

constexpr uint8_t kInside = 128;  // (0.25, 0.75) in shader -> fully inside (set for all corners or none)
constexpr uint8_t kOn     = 255;  // otherwise, > 0.5 in shader -> corner active
struct Vertex {
    SkPoint pos;
    uint8_t flags[4];  // {doTL, doTR, doBR, doBL} -> kUByte4_unorm -> half4
                        // Safely inside encoded as {kInside, kInside, kInside, kInside}.
};

// Types of blur sigma specifications. The first two correspond to the MaskFilter options
// while the latter two do not.
enum SigmaMode {
    kMFDevice_SigmaMode,    // Single sigma, device space (MF respectCTM=false)
    kMFLocal_SigmaMode,     // Single sigma, local space  (MF respectCTM=true)
    kAnisoDev_SigmaMode,    // sigmaX/sigmaY in device space
    kAnisoLocal_SigmaMode,  // sigmaX/sigmaY in local space
};

// Different strategies for tessellating the mesh of a blurred rrect.
enum MeshMode {
    kFull_MeshMode,              // Partition into grid by active corners and fully inside.
    kSimple_MeshMode,            // A single bounding octagon.
    kSimplePlusInside_MeshMode,  // Simple with an irregular carveout for fully inside.
};

template<typename T>
concept Draw = std::invocable<T, SkCanvas*>;

class RRectBlurSlide : public Slide {
    static constexpr float kInitWidth  = 200.f;
    static constexpr float kInitHeight = 400.f;
    static constexpr SkVector kInitRadii[] = {
        {35, 96}, {154, 215}, {60, 95}, {75, 203}
    };
    static constexpr int kRepeatCnt = 100;

    float fWidth = kInitWidth;
    float fHeight = kInitHeight;
    SkVector fRadii[4] = {kInitRadii[0], kInitRadii[1], kInitRadii[2], kInitRadii[3]};

    float fSigmaX = 20.f;
    float fSigmaY = 20.f;

    sk_sp<SkRuntimeEffect> fAbsDiffRTE;
    sk_sp<SkMeshSpecification> fMeshSpec;
    sk_sp<SkShader> fCdfLUT;
    bool fUniformCorners = false;
    float fAbsDiffScale = 5;
    bool fShowDiffs = true;
    bool fShowRRect = true;
    bool fShowPath = true;
    bool fShowShader = true;
    bool fShowOutline = true;
    bool fCircularCorners = false;
    bool fShowShaderWireframe = false;
    int fMeshMode = 0;
    bool fSplitInsideDraw = false;
    bool fRepeatedDraw = false;
    float fExtraScaleX = 1.f;
    float fExtraScaleY = 1.f;
    float fSkewX = 0.f;
    float fSkewY = 0.f;
    float fRotation = 0.f;  // degrees
    int fSigmaMode = kMFLocal_SigmaMode;

    static constexpr float kMinMargin = 50.f;

    float drawPad(float sigma) const { return std::ceil(3.f * sigma); }

    struct MeshData {
        std::vector<Vertex>   vertices;
        std::vector<uint16_t> indices;
        std::vector<uint16_t> insideIndices;  // populated when fSplitInsideDraw is set
    };

    SkRRect rrect() const {
        return SkRRect::MakeRectRadii(SkRect::MakeWH(fWidth, fHeight), fRadii);
    }

    SkAutoCanvasRestore applyExtraTransform(SkCanvas* c, SkPoint center) {
        SkAutoCanvasRestore acr(c, true);
        c->translate(center.fX, center.fY);
        c->rotate(fRotation);
        c->skew(fSkewX, fSkewY);
        c->scale(fExtraScaleX, fExtraScaleY);
        c->translate(-center.fX, -center.fY);
        return acr;
    }

    MeshData generateMesh(float sigmaX, float sigmaY) const {
        SkVector radii[4];
        std::copy_n(this->rrect().radii().data(), 4, radii);
        // Safe offset: the distance from a straight edge at which the 1D CDF
        // saturates. Used for per-corner safe bounds.
        float safeOffsetX = std::ceil(3.f * sigmaX);
        float safeOffsetY = std::ceil(3.f * sigmaY);
        SkRect rect = this->rrect().getBounds();
        float drawPadX = this->drawPad(sigmaX);
        float drawPadY = this->drawPad(sigmaY);
        float xMin = rect.fLeft   - drawPadX;
        float xMax = rect.fRight  + drawPadX;
        float yMin = rect.fTop    - drawPadY;
        float yMax = rect.fBottom + drawPadY;

        // Per-corner safe boundaries. TL=0, TR=1, BR=2, BL=3.
        // Top corners active when y < ySafe, bottom corners when y > ySafe.
        // Left corners active when x < xSafe, right corners when x > xSafe.
        float ySafe[] = {
            rect.fTop    + radii[0].fY + safeOffsetY,  // TL
            rect.fTop    + radii[1].fY + safeOffsetY,  // TR
            rect.fBottom - radii[2].fY - safeOffsetY,  // BR
            rect.fBottom - radii[3].fY - safeOffsetY,  // BL
        };
        float xSafe[] = {
            rect.fLeft  + radii[0].fX + safeOffsetX,   // TL
            rect.fRight - radii[1].fX - safeOffsetX,   // TR
            rect.fRight - radii[2].fX - safeOffsetX,   // BR
            rect.fLeft  + radii[3].fX + safeOffsetX,   // BL
        };

        // Build sorted, unique boundary arrays from per-corner safe values.
        auto makeBounds = [](std::initializer_list<float> vals, float lo, float hi) {
            std::vector<float> v;
            for (float f : vals) {
                v.push_back(std::clamp(f, lo, hi));
            }
            std::sort(v.begin(), v.end());
            v.erase(std::unique(v.begin(), v.end()), v.end());
            return v;
        };

        // Inset rrect for fully-inside detection. Uses 3.5*sigma so that
        // cells near curved corners are correctly classified.
        float insInsetX = std::ceil(3.5f * sigmaX);
        float insInsetY = std::ceil(3.5f * sigmaY);
        SkRRect insideRRect = this->rrect();
        insideRRect.inset(insInsetX, insInsetY);

        // Include inside-saturation bounds in the grid so the inside
        // optimization doesn't require per-cell sub-splitting (which
        // creates T-junctions on shared edges with non-sub-split neighbors).
        SkRect insBounds = insideRRect.getBounds();
        auto yBounds = makeBounds(
                {yMin, ySafe[0], ySafe[1], ySafe[2], ySafe[3],
                 insBounds.fTop, insBounds.fBottom, yMax}, yMin, yMax);
        auto xBounds = makeBounds(
                {xMin, xSafe[0], xSafe[1], xSafe[2], xSafe[3],
                 insBounds.fLeft, insBounds.fRight, xMax}, xMin, xMax);

        // Octagonal bevel: clip the 4 bounding-rect corners. The bevel
        // line is tangent to the offset ellipse (rrect surface + drawPad) at
        // theta=pi/4, giving maximum area cut while maintaining full blur clearance.
        // Bevel amount per axis: (r + pad) * (2 - sqrt(2)).
        constexpr float kBevelFrac = 2.f - float(SK_FloatSqrt2);  // 2-sqrt(2) ~= 0.586
        SkVector bevel[4];
        for (int i = 0; i < 4; i++) {
            bevel[i] = (radii[i] + SkVector{drawPadX, drawPadY}) * kBevelFrac;
        }

        std::vector<Vertex> vertices;
        std::vector<uint16_t> indices;
        std::vector<uint16_t> insideIndices;

        if (fMeshMode != kFull_MeshMode) {
            uint8_t cF[4] = {  // per-corner flag: only flag non-zero-radius corners
                uint8_t(radii[0].isZero() ? 0 : kOn),
                uint8_t(radii[1].isZero() ? 0 : kOn),
                uint8_t(radii[2].isZero() ? 0 : kOn),
                uint8_t(radii[3].isZero() ? 0 : kOn),
            };
            auto vOn = [&](SkPoint p) {
                vertices.push_back({p, {cF[0], cF[1], cF[2], cF[3]}});
            };
            auto vIn = [&](SkPoint p) {
                vertices.push_back({p, {kInside, kInside, kInside, kInside}});
            };
            auto tri = [&](uint16_t a, uint16_t b, uint16_t c) {
                indices.insert(indices.end(), {a, b, c});
            };

            // Outer octagon (vertices 0-7).
            SkPoint oct[8] = {
                {xMin + bevel[0].fX, yMin              },  // 0: top-left
                {xMax - bevel[1].fX, yMin              },  // 1: top-right
                {xMax,               yMin + bevel[1].fY},  // 2: right-top
                {xMax,               yMax - bevel[2].fY},  // 3: right-bottom
                {xMax - bevel[2].fX, yMax              },  // 4: bottom-right
                {xMin + bevel[3].fX, yMax              },  // 5: bottom-left
                {xMin,               yMax - bevel[3].fY},  // 6: left-bottom
                {xMin,               yMin + bevel[0].fY},  // 7: left-top
            };
            for (auto& p : oct) { vOn(p); }

            // For Simple mode (or if inner rrect is empty), just fan the outer octagon.
            bool fanOnly = (fMeshMode == kSimple_MeshMode);

            if (!fanOnly) {
                // Build inner polygon from a conservative inset of the rrect.
                // Use 3.5*sigma inset which is conservative for corner eccentricity
                // up to ~5.5:1 (verified empirically). SkRRect::inset handles
                // bounds reduction, radius reduction (preserving 0-radius
                // corners), and proportional scaling when radii overlap.
                constexpr float kInvSqrt2 = float(1.f/SK_FloatSqrt2);

                float insetX = std::ceil(3.5f * sigmaX);
                float insetY = std::ceil(3.5f * sigmaY);
                SkRRect innerRRect = this->rrect();
                innerRRect.inset(insetX, insetY);

                if (innerRRect.isEmpty()) {
                    fanOnly = true;
                } else {
                    // Generate inner polygon vertices in CW order.
                    // 1 vertex for 0-radius corners, 3 for non-trivial.
                    std::vector<SkPoint> poly;
                    SkVector ir[4];
                    std::copy_n(innerRRect.radii().data(), 4, ir);
                    SkRect innerRect = innerRRect.getBounds();
                    float iL = innerRect.fLeft, iR = innerRect.fRight;
                    float iT = innerRect.fTop,  iB = innerRect.fBottom;

                    // Arc centers for inner corners.
                    SkPoint arcCenter[] = {
                        {iL + ir[0].fX, iT + ir[0].fY},
                        {iR - ir[1].fX, iT + ir[1].fY},
                        {iR - ir[2].fX, iB - ir[2].fY},
                        {iL + ir[3].fX, iB - ir[3].fY},
                    };

                    // TL: left(0) -> diag(45) -> top(90)
                    if (!ir[0].isZero()) {
                        poly.push_back(arcCenter[0] - SkVector{ir[0].fX, 0});
                        poly.push_back(arcCenter[0] - ir[0] * kInvSqrt2    );
                        poly.push_back(arcCenter[0] - SkVector{0, ir[0].fY});
                    } else {
                        poly.push_back({iL, iT});
                    }
                    // TR: top(0) -> diag(45) -> right(90)
                    if (!ir[1].isZero()) {
                        poly.push_back(arcCenter[1] + SkVector{       0, -ir[1].fY});
                        poly.push_back(arcCenter[1] + SkVector{ir[1].fX, -ir[1].fY} * kInvSqrt2);
                        poly.push_back(arcCenter[1] + SkVector{ir[1].fX,         0});
                    } else {
                        poly.push_back({iR, iT});
                    }
                    // BR: right(0) -> diag(45) -> bottom(90)
                    if (!ir[2].isZero()) {
                        poly.push_back(arcCenter[2] + SkVector{ir[2].fX, 0});
                        poly.push_back(arcCenter[2] + ir[2] * kInvSqrt2    );
                        poly.push_back(arcCenter[2] + SkVector{0, ir[2].fY});
                    } else {
                        poly.push_back({iR, iB});
                    }
                    // BL: bottom(0) -> diag(45) -> left(90)
                    if (!ir[3].isZero()) {
                        poly.push_back(arcCenter[3] + SkVector{        0, ir[3].fY});
                        poly.push_back(arcCenter[3] + SkVector{-ir[3].fX, ir[3].fY} * kInvSqrt2);
                        poly.push_back(arcCenter[3] - SkVector{ ir[3].fX,        0});
                    } else {
                        poly.push_back({iL, iB});
                    }

                    uint16_t insN = static_cast<uint16_t>(poly.size());

                    // Two sets of inner vertices: kInside for interior fill,
                    // kOn for border stitching.
                    uint16_t insBase = static_cast<uint16_t>(vertices.size());
                    for (auto& p : poly) { vIn(p); }

                    uint16_t borderBase = static_cast<uint16_t>(vertices.size());
                    for (auto& p : poly) { vOn(p); }

                    // Fan-triangulate inside polygon (kInside vertices).
                    if (fSplitInsideDraw) {
                        for (uint16_t i = 1; i + 1 < insN; i++) {
                            insideIndices.insert(insideIndices.end(),
                                    {insBase, uint16_t(insBase + i), uint16_t(insBase + i + 1)});
                        }
                    } else {
                        for (uint16_t i = 1; i + 1 < insN; i++) {
                            tri(insBase, uint16_t(insBase + i), uint16_t(insBase + i + 1));
                        }
                    }

                    // Stitch ring between outer octagon (0..7) and inner polygon
                    // (borderBase) using explicit per-section stitching.
                    auto bIdx = [borderBase](uint16_t i) -> uint16_t {
                        return SkToU16(borderBase + i);
                    };

                    // Per-corner inner vertex counts (1 or 3).
                    uint16_t nC[4] = {
                        SkToU16(!ir[0].isZero() ? 3 : 1),
                        SkToU16(!ir[1].isZero() ? 3 : 1),
                        SkToU16(!ir[2].isZero() ? 3 : 1),
                        SkToU16(!ir[3].isZero() ? 3 : 1),
                    };
                    // Starting index of each corner's inner vertices.
                    uint16_t cStart[4] = {0};
                    for (int c = 1; c < 4; c++) {
                        cStart[c] = cStart[c-1] + nC[c-1];
                    }

                    // curInner tracks the last inner vertex used (for bridging).
                    uint16_t curInner = SkToU16(cStart[3] + nC[3] - 1); // last BL vert

                    // Corner section: stitch outer vertices oa/ob to inner verts.
                    // For n=3, the fan is split at the midpoint (45° vertex) to
                    // avoid triangle crossover with eccentric corner radii.
                    auto cornerSection = [&](uint16_t oa, uint16_t ob, int corner) {
                        uint16_t s = cStart[corner];
                        uint16_t n = nC[corner];
                        tri(oa, bIdx(s), bIdx(curInner));
                        if (n == 1) {
                            tri(oa, ob, bIdx(s));
                        } else {
                            tri(oa, bIdx(s + 1), bIdx(s));
                            tri(oa, ob, bIdx(s + 1));
                            tri(ob, bIdx(s + 2), bIdx(s + 1));
                        }
                        curInner = s + n - 1;
                    };

                    auto edgeSection = [&](uint16_t oa, uint16_t ob) {
                        tri(oa, ob, bIdx(curInner));
                    };

                    cornerSection(7, 0, 0);  // TL
                    edgeSection(0, 1);       // Top
                    cornerSection(1, 2, 1);  // TR
                    edgeSection(2, 3);       // Right
                    cornerSection(3, 4, 2);  // BR
                    edgeSection(4, 5);       // Bottom
                    cornerSection(5, 6, 3);  // BL
                    edgeSection(6, 7);       // Left
                }
            }

            if (fanOnly) {
                for (uint16_t i = 1; i < 7; i++) {
                    tri(0, i, uint16_t(i + 1));
                }
            }

            return {std::move(vertices), std::move(indices), std::move(insideIndices)};
        }

        SkASSERT(fMeshMode == kFull_MeshMode);

        auto addQuadTo = [&](std::vector<uint16_t>& idxs,
                             float x0, float y0, float x1, float y1,
                             uint8_t tl, uint8_t tr, uint8_t br, uint8_t bl) {
            uint16_t base = static_cast<uint16_t>(vertices.size());
            uint8_t f[4] = {tl, tr, br, bl};
            vertices.push_back({{x0, y0}, {f[0], f[1], f[2], f[3]}});
            vertices.push_back({{x1, y0}, {f[0], f[1], f[2], f[3]}});
            vertices.push_back({{x1, y1}, {f[0], f[1], f[2], f[3]}});
            vertices.push_back({{x0, y1}, {f[0], f[1], f[2], f[3]}});
            idxs.insert(idxs.end(), {uint16_t(base),     uint16_t(base + 1), uint16_t(base + 3),
                                     uint16_t(base + 1), uint16_t(base + 2), uint16_t(base + 3)});
        };
        auto addQuad = [&](float x0, float y0, float x1, float y1,
                           uint8_t tl, uint8_t tr, uint8_t br, uint8_t bl) {
            addQuadTo(indices, x0, y0, x1, y1, tl, tr, br, bl);
        };

        // Pentagon: quad with one corner beveled. 5 vertices, 3 triangles.
        // corner: 0=TL, 1=TR, 2=BR, 3=BL identifies which corner to clip.
        auto addBeveledQuad = [&](float xlo, float ylo, float xhi, float yhi,
                                    float cx, float cy, int corner,
                                    uint8_t tl, uint8_t tr, uint8_t br, uint8_t bl) {
            uint16_t base = static_cast<uint16_t>(vertices.size());
            uint8_t f[4] = {tl, tr, br, bl};
            auto v = [&](float x, float y) {
                vertices.push_back({{x, y}, {f[0], f[1], f[2], f[3]}});
            };
            auto tri = [&](int a, int b, int c) {
                indices.insert(indices.end(),
                               {uint16_t(base+a), uint16_t(base+b), uint16_t(base+c)});
            };
            // Vertices listed CW (y-down), fan from the vertex opposite the bevel.
            switch (corner) {
                case 0:  // TL
                    v(xlo+cx, ylo); v(xhi, ylo); v(xhi, yhi); v(xlo, yhi); v(xlo, ylo+cy);
                    tri(2,0,1); tri(2,4,0); tri(2,3,4);
                    break;
                case 1:  // TR
                    v(xlo, ylo); v(xhi-cx, ylo); v(xhi, ylo+cy); v(xhi, yhi); v(xlo, yhi);
                    tri(4,0,1); tri(4,1,2); tri(4,2,3);
                    break;
                case 2:  // BR
                    v(xlo, ylo); v(xhi, ylo); v(xhi, yhi-cy); v(xhi-cx, yhi); v(xlo, yhi);
                    tri(0,1,2); tri(0,2,3); tri(0,3,4);
                    break;
                case 3:  // BL
                    v(xlo, ylo); v(xhi, ylo); v(xhi, yhi); v(xlo+cx, yhi); v(xlo, yhi-cy);
                    tri(1,2,3); tri(1,3,4); tri(1,4,0);
                    break;
            }
        };

        size_t xiLast = xBounds.size() - 2;
        size_t yiLast = yBounds.size() - 2;

        for (size_t yi = 0; yi + 1 < yBounds.size(); yi++) {
            float ylo = yBounds[yi], yhi = yBounds[yi + 1];
            if (yhi <= ylo) {
                continue;
            }

            for (size_t xi = 0; xi + 1 < xBounds.size(); xi++) {
                float xlo = xBounds[xi], xhi = xBounds[xi + 1];
                if (xhi <= xlo) {
                    continue;
                }

                // Per-corner active check. Since safe values are grid split points,
                // each cell is entirely inside or outside each corner's region.
                uint8_t doTL = (yhi <= ySafe[0] && xhi <= xSafe[0]) ? kOn : 0;
                uint8_t doTR = (yhi <= ySafe[1] && xlo >= xSafe[1]) ? kOn : 0;
                uint8_t doBR = (ylo >= ySafe[2] && xlo >= xSafe[2]) ? kOn : 0;
                uint8_t doBL = (ylo >= ySafe[3] && xhi <= xSafe[3]) ? kOn : 0;

                // Octagonal bevel on the 4 outer corner cells.
                int bevelCorner = -1;
                float cx = 0, cy = 0;
                if (yi == 0       && xi == 0)      { bevelCorner = 0; }
                if (yi == 0       && xi == xiLast) { bevelCorner = 1; }
                if (yi == yiLast && xi == xiLast)  { bevelCorner = 2; }
                if (yi == yiLast && xi == 0)       { bevelCorner = 3; }
                if (bevelCorner >= 0) {
                    cx = std::min(bevel[bevelCorner].fX, xhi - xlo);
                    cy = std::min(bevel[bevelCorner].fY, yhi - ylo);
                }

                if (bevelCorner >= 0 && cx > 0 && cy > 0) {
                    addBeveledQuad(xlo, ylo, xhi, yhi, cx, cy, bevelCorner,
                                     doTL, doTR, doBR, doBL);
                } else if (insideRRect.contains({xlo, ylo, xhi, yhi})) {
                    // Fully inside the inset rrect: CDF is saturated.
                    auto& dst = fSplitInsideDraw ? insideIndices : indices;
                    addQuadTo(dst, xlo, ylo, xhi, yhi,
                              kInside, kInside, kInside, kInside);
                } else {
                    addQuad(xlo, ylo, xhi, yhi, doTL, doTR, doBR, doBL);
                }
            }
        }
        return {std::move(vertices), std::move(indices), std::move(insideIndices)};
    }

    // Convert UI sigmas to local-space sigmas that produce the correct
    // blur for the given CTM and sigma mode.
    std::tuple<float, float> resolvedLocalSigmas(const SkMatrix& ctm) const {
        switch (fSigmaMode) {
            case kMFDevice_SigmaMode: {
                // fSigmaX is already in device space.  Convert to local
                // sigmas that produce isotropic device-space blur.
                float sx = ctm.mapVector(1.f, 0.f).length();
                float sy = ctm.mapVector(0.f, 1.f).length();
                return {fSigmaX / sx, fSigmaX / sy};
            }
            case kMFLocal_SigmaMode: {
                // MF computes a single device sigma via mapRadius (geometric
                // mean of the two axis scales).  To replicate that isotropic
                // device-space blur we need anisotropic local sigmas.
                float devSigma = ctm.mapRadius(fSigmaX);
                float sx = ctm.mapVector(1.f, 0.f).length();
                float sy = ctm.mapVector(0.f, 1.f).length();
                return {devSigma / sx, devSigma / sy};
            }
            case kAnisoDev_SigmaMode: {
                float sx = ctm.mapVector(1.f, 0.f).length();
                float sy = ctm.mapVector(0.f, 1.f).length();
                return {fSigmaX / sx, fSigmaY / sy};
            }
            case kAnisoLocal_SigmaMode:
                return {fSigmaX, fSigmaY};
        }
        SkUNREACHABLE;
    }

    const char* shaderSkipReason(const SkMatrix& ctm) const {
        if (ctm.hasPerspective()) {
            return "Perspective not supported";
        }
        switch (fSigmaMode) {
            case kMFDevice_SigmaMode:
            case kMFLocal_SigmaMode:
                if (!ctm.preservesRightAngles()) {
                    return "Requires orthogonal CTM (no skew)";
                }
                break;
            case kAnisoDev_SigmaMode:
                if (ctm.preservesAxisAlignment()) {
                    break;
                }
                if (ctm.isSimilarity() && SkScalarNearlyEqual(fSigmaX, fSigmaY)) {
                    break;
                }
                return "Needs axis-aligned CTM (or equal sigmas)";
            case kAnisoLocal_SigmaMode:
                break;
        }
        return nullptr;
    }

    sk_sp<SkMaskFilter> makeBlur() const {
        float sigma = fSigmaX;
        bool respectCTM = (fSigmaMode != kMFDevice_SigmaMode);
        return SkMaskFilter::MakeBlur(kNormal_SkBlurStyle, sigma, respectCTM);
    }

    sk_sp<SkBlender> makeAbsDiff() const {
        SkRuntimeEffectBuilder builder(fAbsDiffRTE);
        builder.uniform("scale") = fAbsDiffScale;
        return builder.makeBlender();
    }

    float margin(float sigma) const {
        return std::max(kMinMargin, 6.f * sigma);
    }

    // Makes a path that fills equivalently to rrect() but is not recognized as a SkRRect.
    SkPath makeEquivPath() const {
        auto basePath = SkPath::RRect(this->rrect());
        auto iter = basePath.iter();

        SkPathBuilder builder;
        while (auto rec = iter.next()) {
            switch (rec->fVerb) {
                case SkPathVerb::kClose:
                    builder.close();
                    break;
                case SkPathVerb::kConic:
                    builder.conicTo(rec->fPoints[1], rec->fPoints[2], rec->fConicWeight);
                    break;
                case SkPathVerb::kMove:
                    builder.moveTo(rec->fPoints[0]);
                    builder.lineTo(this->rrect().getBounds().center());
                    builder.lineTo(rec->fPoints[0]);
                    break;
                case SkPathVerb::kLine:
                    builder.lineTo(rec->fPoints[1]);
                    break;
                case SkPathVerb::kQuad:
                    builder.quadTo(rec->fPoints[1], rec->fPoints[2]);
                    break;
                case SkPathVerb::kCubic:
                    builder.cubicTo(rec->fPoints[1], rec->fPoints[2], rec->fPoints[3]);
                    break;
            }
        }
        return builder.detach();
    }

public:
    RRectBlurSlide() {
        fName = "rrect_blur_playground";

        auto result = SkRuntimeEffect::MakeForBlender(SkString(kAbsDiffBlender));
        SkASSERT_RELEASE(result.effect);
        if (!result.effect) {
            SkDebugf("%s", result.errorText.c_str());
        }
        fAbsDiffRTE = result.effect;
        SkASSERT_RELEASE(fAbsDiffRTE);

        using Attribute = SkMeshSpecification::Attribute;
        using Varying = SkMeshSpecification::Varying;

        static const Attribute kAttributes[]{
            {Attribute::Type::kFloat2,       0, SkString{"pos"}},
            {Attribute::Type::kUByte4_unorm, 8, SkString{"flags"}},
        };
        static const Varying kVaryings[]{
            {Varying::Type::kFloat2, SkString{"localPos"}},
            {Varying::Type::kHalf4,  SkString{"flags"}},
        };

        SkMeshSpecificationPriv::Options options;
        options.maxVersionAllowed = SkSL::Version::k300;
        auto meshResult = SkMeshSpecificationPriv::Make(
                kAttributes,
                sizeof(Vertex),
                kVaryings,
                SkString(kMeshVS),
                SkString(kMeshFS),
                options);
        if (!meshResult.specification) {
            SkDebugf("Mesh spec error: %s\n", meshResult.error.c_str());
        }
        SkASSERT_RELEASE(meshResult.specification);
        fMeshSpec = meshResult.specification;

        static constexpr int kCdfLutSize = 256;
        static constexpr float kCdfLutRange = 4.0f;
        SkBitmap bmp;
        bmp.allocPixels(SkImageInfo::Make(kCdfLutSize, 1,
                                          kR8G8_unorm_SkColorType,
                                          kOpaque_SkAlphaType));
        uint8_t* pixels = static_cast<uint8_t*>(bmp.getPixels());
        for (int i = 0; i < kCdfLutSize; ++i) {
            float x = -kCdfLutRange + 2.0f * kCdfLutRange * (i + 0.5f) / kCdfLutSize;
            float cdf = 0.5f + 0.5f * std::erff(x);
            float gauss = std::exp(-x * x);
            pixels[2 * i    ] = static_cast<uint8_t>(std::round(cdf * 255.0f));
            pixels[2 * i + 1] = static_cast<uint8_t>(std::round(gauss * 255.0f));
        }
        fCdfLUT = bmp.asImage()->makeRawShader(SkTileMode::kClamp, SkTileMode::kClamp,
                                               SkSamplingOptions(SkFilterMode::kLinear));
    }

    void drawSkipText(SkCanvas* canvas, const char* reason) {
        SkFont font = ToolUtils::DefaultPortableFont();
        font.setSize(14);
        SkPaint paint;
        paint.setColor(SK_ColorRED);
        canvas->drawString(reason, this->rrect().getBounds().centerX() - 60,
                           this->rrect().getBounds().centerY(), font, paint);
    }

    void draw(SkCanvas* canvas, Draw auto drawBlur, int repeatCnt) {
        SkPaint outlinePaint;
        outlinePaint.setColor(SK_ColorBLUE);
        outlinePaint.setAntiAlias(true);
        outlinePaint.setStroke(true);
        outlinePaint.setStrokeWidth(0.f);

        for (int i = 0; i < repeatCnt; ++i) {
            drawBlur(canvas);
        }
        if (fShowOutline) {
            canvas->drawRRect(this->rrect(), outlinePaint);
        }

        if (fShowDiffs && fSigmaMode <= kMFLocal_SigmaMode) {
            canvas->save();
            canvas->translate(0, this->rrect().height() + this->margin(fSigmaY));
            this->diffWithPath(canvas, drawBlur);
            if (fShowOutline) {
                canvas->drawRRect(this->rrect(), outlinePaint);
            }
            canvas->restore();
        }
    }

    void diffWithPath(SkCanvas* canvas, Draw auto drawBlur) {
        SkRect bounds;
        bounds = this->rrect().getBounds().makeOutset(this->margin(fSigmaX), this->margin(fSigmaY));

        canvas->saveLayerAlpha(&bounds, 255);
        drawBlur(canvas);
        SkPaint diffPaint;
        diffPaint.setBlender(this->makeAbsDiff());

        canvas->saveLayer(&bounds, &diffPaint);
        SkPaint mfPaint;
        mfPaint.setMaskFilter(this->makeBlur());
        canvas->drawPath(this->makeEquivPath(), mfPaint);
        canvas->restore();

        canvas->restore();
    }

    void draw(SkCanvas* canvas) override {
        this->drawUI();
        SkPaint mfPaint;
        mfPaint.setMaskFilter(this->makeBlur());

        auto drawRRectBlur = [this, mfPaint](SkCanvas* canvas) {
            canvas->drawRRect(this->rrect(), mfPaint);
        };

        auto drawPathBlur = [this, mfPaint](SkCanvas* canvas) {
            canvas->drawPath(this->makeEquivPath(), mfPaint);
        };

        auto drawShaderBlur = [this](SkCanvas* canvas) {
            SkMatrix ctm = canvas->getTotalMatrix();

            SkASSERT(!this->shaderSkipReason(ctm));

            auto [sigmaX, sigmaY] = this->resolvedLocalSigmas(ctm);
            auto [verts, idxs, insideIdxs] = this->generateMesh(sigmaX, sigmaY);

            SkVector radii[4];
            std::copy_n(this->rrect().radii().data(), 4, radii);
            float sqrtHalfOverSigma[2] = { float{1.f/SK_FloatSqrt2} / sigmaX,
                                           float{1.f/SK_FloatSqrt2} / sigmaY };
            int radius[2] = { static_cast<int>(ceil(6 * sigmaX)) / 2,
                              static_cast<int>(ceil(6 * sigmaY)) / 2 };
            SkRect rect = this->rrect().getBounds();

            auto vb = SkMeshes::MakeVertexBuffer(
                    verts.data(), verts.size() * sizeof(Vertex));

            size_t uniformSize = fMeshSpec->uniformSize();
            auto uniformData = SkData::MakeUninitialized(uniformSize);
            void* ptr = uniformData->writable_data();
            auto setUniform = [&](const char* name, const void* data, size_t size) {
                auto* u = fMeshSpec->findUniform(name);
                SkASSERT(u);
                memcpy(SkTAddOffset<void>(ptr, u->offset), data, size);
            };

            // CTM uniform: transpose SkMatrix (row-major) to column-major for SkSL float3x3
            SkScalar m[9];
            ctm.get9(m);
            float ctmCols[9] = {m[0], m[3], m[6],   // col 0
                                m[1], m[4], m[7],   // col 1
                                m[2], m[5], m[8]};  // col 2
            setUniform("ctm",                   ctmCols,                sizeof(ctmCols));
            setUniform("rect",                  &rect,                  sizeof(rect));
            setUniform("radii",                 radii,                  sizeof(radii));
            setUniform("one_over_sqrt2_sigma",  sqrtHalfOverSigma,      sizeof(sqrtHalfOverSigma));
            setUniform("radius",                radius,                 sizeof(radius));

            SkMesh::ChildPtr children[] = {fCdfLUT};

            float drawPadX = this->drawPad(sigmaX), drawPadY = this->drawPad(sigmaY);
            float xMin = rect.fLeft - drawPadX, xMax = rect.fRight + drawPadX;
            float yMin = rect.fTop  - drawPadY, yMax = rect.fBottom + drawPadY;

            // Mesh bounds in device space (since canvas will be identity)
            SkRect localBounds = SkRect::MakeLTRB(xMin, yMin, xMax, yMax);
            SkRect devBounds;
            ctm.mapRect(&devBounds, localBounds);

            // Draw with identity canvas -- VS handles the transform
            canvas->save();
            canvas->resetMatrix();
            auto drawMesh = [&](const auto& indices) {
                auto ib = SkMeshes::MakeIndexBuffer(indices.data(),
                                                    indices.size() * sizeof(uint16_t));
                auto [mesh, error] = SkMesh::MakeIndexed(
                        fMeshSpec,
                        SkMesh::Mode::kTriangles,
                        vb,
                        verts.size(),
                        /*vertexOffset=*/0,
                        std::move(ib),
                        indices.size(),
                        /*indexOffset=*/0,
                        uniformData,
                        children,
                        devBounds);
                if (!mesh.isValid()) {
                    SkDebugf("Mesh error: %s\n", error.c_str());
                    return;
                }

                SkPaint paint;
                canvas->drawMesh(mesh, SkBlender::Mode(SkBlendMode::kDst), paint);
            };
            drawMesh(idxs);
            // Optional separate draw for fully-inside triangles to see if it reduces
            // thread divergence with the blur shader (could use a simpler sampler).
            if (!insideIdxs.empty()) {
                drawMesh(insideIdxs);
            }

            canvas->restore();
        };

        int repeatCnt = fRepeatedDraw ? kRepeatCnt : 1;
        canvas->translate(kMinMargin, kMinMargin);

        // Apply extra transform around the rrect center for testing CTM handling.
        SkPoint center = {this->rrect().getBounds().centerX(), this->rrect().getBounds().centerY()};

        bool mfDraws = (fSigmaMode <= kMFLocal_SigmaMode);

        if (fShowRRect) {
            if (mfDraws) {
                auto acr = this->applyExtraTransform(canvas, center);
                this->draw(canvas, drawRRectBlur, repeatCnt);
            } else {
                this->drawSkipText(canvas, "MF: aniso not supported");
            }
        }
        canvas->translate(this->rrect().width() + this->margin(fSigmaX), 0);
        if (fShowPath) {
            if (mfDraws) {
                auto acr = this->applyExtraTransform(canvas, center);
                this->draw(canvas, drawPathBlur, repeatCnt);
            } else {
                this->drawSkipText(canvas, "MF: aniso not supported");
            }
        }
        canvas->translate(this->rrect().width() + this->margin(fSigmaX), 0);
        if (fShowShader) {
            auto acr = this->applyExtraTransform(canvas, center);
            if (auto* skip = this->shaderSkipReason(canvas->getTotalMatrix())) {
                acr.restore();
                this->drawSkipText(canvas, skip);
            } else {
                this->draw(canvas, drawShaderBlur, repeatCnt);
                if (fShowShaderWireframe) {
                    auto [sigmaX, sigmaY] = this->resolvedLocalSigmas(
                            canvas->getTotalMatrix());
                    auto [verts, idxs, insideIdxs] = this->generateMesh(sigmaX, sigmaY);
                    auto drawWire = [&](const std::vector<uint16_t>& indices,
                                        SkColor color) {
                        SkPaint wirePaint;
                        wirePaint.setColor(color);
                        wirePaint.setAntiAlias(true);
                        wirePaint.setStroke(true);
                        wirePaint.setStrokeWidth(0.f);
                        for (size_t i = 0; i + 2 < indices.size(); i += 3) {
                            SkPoint tri[3] = {
                                verts[indices[i + 0]].pos,
                                verts[indices[i + 1]].pos,
                                verts[indices[i + 2]].pos,
                            };
                            canvas->drawLine(tri[0], tri[1], wirePaint);
                            canvas->drawLine(tri[1], tri[2], wirePaint);
                            canvas->drawLine(tri[2], tri[0], wirePaint);
                        }
                    };
                    drawWire(idxs, SK_ColorMAGENTA);
                    drawWire(insideIdxs, SK_ColorCYAN);
                }
            }
        }
    }

    void drawUI() {
        float wh[2] = {fWidth, fHeight};
        ImGui::Begin("RRect");

        ImGui::Separator();
        ImGui::Text("Show");                     ImGui::SameLine();
        ImGui::Checkbox("RRect" , &fShowRRect ); ImGui::SameLine();
        ImGui::Checkbox("Path"  , &fShowPath  ); ImGui::SameLine();
        ImGui::Checkbox("Shader", &fShowShader);
        ImGui::Checkbox("Show Diff with Path", &fShowDiffs);
        ImGui::Checkbox("Show Outline", &fShowOutline);
        ImGui::BeginDisabled(!fShowShader);
        ImGui::Checkbox("Show Shader Wireframe", &fShowShaderWireframe);
        ImGui::RadioButton("Full Mesh",          &fMeshMode, kFull_MeshMode); ImGui::SameLine();
        ImGui::RadioButton("Simple Mesh",        &fMeshMode, kSimple_MeshMode); ImGui::SameLine();
        ImGui::RadioButton("Simple+Inside Mesh", &fMeshMode, kSimplePlusInside_MeshMode);
        ImGui::BeginDisabled(fMeshMode == kSimple_MeshMode);
        ImGui::Checkbox("Split Inside Draw", &fSplitInsideDraw);
        ImGui::EndDisabled();
        ImGui::EndDisabled();
        ImGui::Checkbox("Repeat", &fRepeatedDraw);
        ImGui::BeginDisabled(!fShowDiffs);
        ImGui::SliderFloat("Diff Scale", &fAbsDiffScale, 1.f, 100.f);
        ImGui::EndDisabled();

        ImGui::Separator();
        ImGui::SliderFloat2("size", wh, 20, 400);
        ImGui::Checkbox("Uniform Corners", &fUniformCorners);
        ImGui::SameLine();
        ImGui::Checkbox("Circular Corners", &fCircularCorners);

        SkVector newRadii[4];

        // Clamp adjacent corner radii so they don't exceed the shared edge length.
        auto clampPair = [](float& a, float& b, float limit) {
            if (a + b > limit) {
                float s = limit / (a + b);
                a *= s;
                b = limit - a;
            }
        };

        if (fCircularCorners) {
            // Circular: control pixel radius directly (rx == ry).
            float pr[4];
            for (int i = 0; i < 4; ++i) {
                int j = fUniformCorners ? 0 : i;
                pr[i] = std::min(fRadii[j].fX, fRadii[j].fY);
            }

            float maxSlider = std::max(wh[0], wh[1]);
            ImGui::SliderFloat("TL Radius", &pr[0], 0.f, maxSlider);
            ImGui::BeginDisabled(fUniformCorners);
            ImGui::SliderFloat("TR Radius", &pr[1], 0.f, maxSlider);
            ImGui::SliderFloat("LL Radius", &pr[3], 0.f, maxSlider);
            ImGui::SliderFloat("LR Radius", &pr[2], 0.f, maxSlider);
            ImGui::EndDisabled();

            if (fUniformCorners) {
                for (int i = 1; i < 4; ++i) {
                    pr[i] = pr[0];
                }
            }

            clampPair(pr[0], pr[1], wh[0]);  // TL + TR <= width  (top)
            clampPair(pr[3], pr[2], wh[0]);  // BL + BR <= width  (bottom)
            clampPair(pr[0], pr[3], wh[1]);  // TL + BL <= height (left)
            clampPair(pr[1], pr[2], wh[1]);  // TR + BR <= height (right)

            for (int i = 0; i < 4; ++i) {
                newRadii[i].fX = newRadii[i].fY = pr[i];
            }
        } else {
            // General: per-corner rx, ry as percentages of width/height.
            float rxy[4][2];
            for (int i = 0; i < 4; ++i) {
                int j = fUniformCorners ? 0 : i;
                rxy[i][0] = 100.f * fRadii[j].fX / wh[0];
                rxy[i][1] = 100.f * fRadii[j].fY / wh[1];
            }

            auto radiusSlider = [&](const char* label, float* rxy) {
                ImGui::SliderFloat2(SkStringPrintf("%s Radii", label).c_str(),
                                    rxy, 0.f, 100.f);
                ImGui::SameLine();
                if (ImGui::Button(SkStringPrintf("Make Circular##%s", label).c_str())) {
                    // Average the pixel radii, convert back to percentages.
                    float avgR = 0.5f * (rxy[0] / 100.f * wh[0] + rxy[1] / 100.f * wh[1]);
                    avgR = std::min(avgR, std::min(wh[0], wh[1]) * 0.5f);
                    rxy[0] = avgR / wh[0] * 100.f;
                    rxy[1] = avgR / wh[1] * 100.f;
                }
            };

            radiusSlider("TL", rxy[0]);
            ImGui::BeginDisabled(fUniformCorners);
            radiusSlider("TR", rxy[1]);
            radiusSlider("LL", rxy[3]);
            radiusSlider("LR", rxy[2]);
            ImGui::EndDisabled();

            if (fUniformCorners) {
                for (int i = 1; i < 4; ++i) {
                    rxy[i][0] = rxy[0][0];
                    rxy[i][1] = rxy[0][1];
                }
            }

            static constexpr float thresh = 100.f;
            clampPair(rxy[0][1], rxy[3][1], thresh);  // TL.y + BL.y <= 100%
            clampPair(rxy[1][1], rxy[2][1], thresh);  // TR.y + BR.y <= 100%
            clampPair(rxy[0][0], rxy[1][0], thresh);  // TL.x + TR.x <= 100%
            clampPair(rxy[3][0], rxy[2][0], thresh);  // BL.x + BR.x <= 100%

            for (int i = 0; i < 4; ++i) {
                newRadii[i].fX = rxy[i][0] / 100.f * wh[0];
                newRadii[i].fY = rxy[i][1] / 100.f * wh[1];
            }
        }

        if (ImGui::Button("Reset Geometry")) {
            wh[0] = kInitWidth;
            wh[1] = kInitHeight;
            std::copy_n(kInitRadii, std::size(kInitRadii), newRadii);
            fUniformCorners = false;
            fCircularCorners = false;
        }

        ImGui::Separator();
        ImGui::Text("Sigma Mode");
        ImGui::RadioButton("MF Device",    &fSigmaMode, kMFDevice_SigmaMode);  ImGui::SameLine();
        ImGui::RadioButton("MF Local",     &fSigmaMode, kMFLocal_SigmaMode);   ImGui::SameLine();
        ImGui::RadioButton("Aniso Device", &fSigmaMode, kAnisoDev_SigmaMode);  ImGui::SameLine();
        ImGui::RadioButton("Aniso Local",  &fSigmaMode, kAnisoLocal_SigmaMode);

        bool isAniso = (fSigmaMode >= kAnisoDev_SigmaMode);
        if (isAniso) {
            ImGui::SliderFloat("Sigma X", &fSigmaX, 1.f, 128.f, "%f");
            ImGui::SliderFloat("Sigma Y", &fSigmaY, 1.f, 128.f, "%f");
        } else {
            ImGui::SliderFloat("Sigma", &fSigmaX, 1.f, 128.f, "%f");
            fSigmaY = fSigmaX;
        }

        ImGui::Separator();
        ImGui::SliderFloat("Extra Scale X", &fExtraScaleX, 0.1f, 4.f);
        ImGui::SliderFloat("Extra Scale Y", &fExtraScaleY, 0.1f, 4.f);
        ImGui::SliderFloat("Skew X", &fSkewX, -1.f, 1.f);
        ImGui::SliderFloat("Skew Y", &fSkewY, -1.f, 1.f);
        ImGui::SliderFloat("Rotation", &fRotation, -180.f, 180.f);
        if (ImGui::Button("Reset Transform")) {
            fExtraScaleX = 1.f;
            fExtraScaleY = 1.f;
            fSkewX = 0.f;
            fSkewY = 0.f;
            fRotation = 0.f;
        }

        ImGui::End();

        fWidth = wh[0];
        fHeight = wh[1];
        std::copy_n(newRadii, std::size(newRadii), fRadii);
    }
};

//////////////////////////////////////////////////////////////////////////////

DEF_SLIDE( return new RRectBlurSlide(); )
