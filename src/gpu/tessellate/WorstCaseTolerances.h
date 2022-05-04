/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_tessellate_WorstCaseTolerances_DEFINED
#define skgpu_tessellate_WorstCaseTolerances_DEFINED

#include "src/gpu/tessellate/Tessellation.h"
#include "src/gpu/tessellate/WangsFormula.h"

namespace skgpu::tess {

// These tolerances determine the worst-case number of parametric and radial segments required to
// accurately linearize curves. Nomenclature:
// - segments = a linear subsection on the curve, either defined as parametric (linear in t) or
//   radial (linear in curve's internal rotation).
// - edges = orthogonal geometry to segments, used in stroking to offset from the central curve by
//   half the stroke width, or to construct the join geometry.
//
// The tolerance values and decisions are estimated in the local path space, along with a 2x2
// approximation matrix that encodes the scale/skew impact (as-best-as-possible) of the full
// local-to-device transform applied in the vertex shader.
class WorstCaseTolerances {
public:
    void setApproximateShaderTransform(const wangs_formula::VectorXform& xform) {
        fCurrentApproxTransform = xform;
    }

    int accountForCubic(float2 p0, float2 p1, float2 p2, float2 p3) {
        float n4 = wangs_formula::cubic_pow4(kPrecision, p0, p1, p2, p3, fCurrentApproxTransform);
        return this->accountForCurve(n4);
    }

    float accountForQuadratic(float2 p0, float2 p1, float2 p2) {
        float n4 = wangs_formula::quadratic_pow4(kPrecision, p0, p1, p2, fCurrentApproxTransform);
        return this->accountForCurve(n4);
    }

    float accountForConic(float2 p0, float2 p1, float2 p2, float w) {
        float n2 = wangs_formula::conic_pow2(kPrecision, p0, p1, p2, w, fCurrentApproxTransform);
        return this->accountForCurve(n2 * n2);
    }

    void accountForStroke(const StrokeParams& strokeParams) {
        float approxDeviceStrokeWidth;
        if (strokeParams.fRadius == 0.f) {
            // Hairlines are always 1 px
            approxDeviceStrokeWidth = 1.f;
        } else {
            // Approximate max scale * local stroke width / 2
            approxDeviceStrokeWidth = strokeParams.fRadius * fCurrentMaxScale;
        }

        float numRadialSegmentsPerRadian = CalcNumRadialSegmentsPerRadian(approxDeviceStrokeWidth);
        float maxEdgesInJoin = WorstCaseEdgesInJoin(strokeParams, numRadialSegmentsPerRadian);

        fMinRequiredRadialSegmentsPerRadian = std::max(fMinRequiredRadialSegmentsPerRadian,
                                                       numRadialSegmentsPerRadian);
        fMinRequiredEdgesInJoins = std::max(fMinRequiredEdgesInJoins, maxEdgesInJoin);
    }

    // Fast log2 of minimum required # of segments per tracked Wang's formula calculations.
    int requiredResolveLevel() const {
        return wangs_formula::nextlog16(fMinRequiredParametricSegments_pow4); // log16(n^4)==log2(n)
    }

    int requiredStrokeEdges() const {
        // The maximum rotation we can have in a stroke is 180 degrees (SK_ScalarPI radians).
        int maxRadialSegmentsInStroke =
                std::max(SkScalarCeilToInt(fMinRequiredRadialSegmentsPerRadian * SK_ScalarPI), 1);

        int maxParametricSegmentsInStroke =
                SkScalarCeilToInt(wangs_formula::root4(fMinRequiredParametricSegments_pow4));
        SkASSERT(maxParametricSegmentsInStroke >= 1);

        // Now calculate the maximum number of edges we will need in the stroke portion of the
        // instance. The first and last edges in a stroke are shared by both the parametric and
        // radial sets of edges, so the total number of edges is:
        //
        //   numCombinedEdges = numParametricEdges + numRadialEdges - 2
        //
        // It's also important to differentiate between the number of edges and segments in a strip:
        //
        //   numSegments = numEdges - 1
        //
        // So the total number of combined edges in the stroke is:
        //
        //   numEdgesInStroke = numParametricSegments + 1 + numRadialSegments + 1 - 2
        //                    = numParametricSegments + numRadialSegments
        //
        int maxEdgesInStroke = maxRadialSegmentsInStroke + maxParametricSegmentsInStroke;

        // Each triangle strip has two sections: It starts with a join then transitions to a stroke.
        // The number of edges in an instance is the sum of edges from the join and stroke sections
        // both.
        // NOTE: The final join edge and the first stroke edge are co-located, however we still need
        // to emit both because the join's edge is half-width and the stroke's is full-width.
        return fMinRequiredEdgesInJoins + maxEdgesInStroke;
    }

private:
    int accountForCurve(float parametricSegments_pow4) {
        if (parametricSegments_pow4 <= 1.f) {
            // The curve is a line, which only requires one segment (which is always the minimum
            // required). Return a value other than 1 to differentiate curve-as-a-line vs.
            // curve-without-chopping to the caller.
            return 0;
        } else if (parametricSegments_pow4 <= pow4(kMaxParametricSegments)) {
            // The curve does not need to be chopped, so can be represented as a single patch.
            fMinRequiredParametricSegments_pow4 = std::max(fMinRequiredParametricSegments_pow4,
                                                           parametricSegments_pow4);
            return 1;
        } else {
            // The curve needs to be chopped
            fMinRequiredParametricSegments_pow4 = pow4(kMaxParametricSegments);
            return SkScalarCeilToInt(wangs_formula::root4(std::min(parametricSegments_pow4,
                                                                   pow4(kMaxSegmentsPerCurve)) /
                                                          pow4(kMaxParametricSegments)));
        }
    }

    // Returns the worst-case number of edges we will need in order to draw a join of the given type.
    static constexpr int WorstCaseEdgesInJoin(const StrokeParams& strokeParams,
                                              float numRadialSegmentsPerRadian) {
        int numEdges = NumFixedEdgesInJoin(strokeParams);
        if (strokeParams.fJoinType < 0.f && numRadialSegmentsPerRadian > 0.f) {
            // For round joins we need to count the radial edges on our own. Account for a
            // worst-case join of 180 degrees (SK_ScalarPI radians).
            numEdges += SkScalarCeilToInt(numRadialSegmentsPerRadian * SK_ScalarPI) - 1;
        }
        return numEdges;
    }

    // The 2x2 approximation of the local-to-device transform that will affect subsequently
    // recorded curves (when fully transformed in the vertex shader).
    wangs_formula::VectorXform fCurrentApproxTransform;
    // A maximum scale factor extracted from the current approximate transform.
    float fCurrentMaxScale;

    // Accumulated minimum requirements to preserve tessellation precision in the worst case over
    // all curves and batched paths.

    // Used for both fills and strokes
    float fMinRequiredParametricSegments_pow4;

    // Used for strokes, adding additional segments along the curve to account for its rotation
    // TODO: Currently we assume the worst case 180 degree rotation for any curve, but tracking
    // max(radialSegments * patch curvature) would be tighter. This would require computing rotation
    // per patch, which could be approximated by tracking min of the tangent dot products, but then
    // we'd be left with the slightly less accurate max(radialSegments) * acos(min(tan dot product))
    // It is also unknown if requesting tighter bounds pays off with less GPU work for more CPU work
    float fMinRequiredRadialSegmentsPerRadian;

    // Used for strokes, tracking the number of additional vertices required to handle joins based
    // on the join type and stroke width.
    // TODO: For round joins, we could also track the rotation angle of the join, instead of
    // assuming 180 degrees. PatchWriter has all necessary control points to do so, but runs into
    // similar trade offs between CPU vs GPU work, and accuracy vs. reducing calls to acos.
    float fMinRequiredEdgesInJoins;
};

}  // namespace skgpu::tess

#endif // skgpu_tessellate_WorstCaseTolerances_DEFINED
