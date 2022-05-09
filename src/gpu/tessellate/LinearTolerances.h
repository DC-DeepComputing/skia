/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_tessellate_LinearTolerances_DEFINED
#define skgpu_tessellate_LinearTolerances_DEFINED

#include "src/gpu/tessellate/Tessellation.h"
#include "src/gpu/tessellate/WangsFormula.h"

namespace skgpu::tess {

/**
 * LinearTolerances stores state to approximate the final device-space transform applied
 * to curves, and uses that to calculate segmentation levels for both the parametric curves and
 * radial components (when stroking, where you have to represent the offset of a curve).
 * These tolerances determine the worst-case number of parametric and radial segments required to
 * accurately linearize curves.
 * - segments = a linear subsection on the curve, either defined as parametric (linear in t) or
 *   radial (linear in curve's internal rotation).
 * - edges = orthogonal geometry to segments, used in stroking to offset from the central curve by
 *   half the stroke width, or to construct the join geometry.
 *
 * The tolerance values and decisions are estimated in the local path space, along with a 2x2
 * approximation matrix that encodes the scale/skew impact (as-best-as-possible) of the full
 * local-to-device transform applied in the vertex shader.
 *
 * The Results object only exposes the final segmentation factor for filled paths (the resolve
 * level) or stroked paths (the number of edges). Internally, however, it accumulates cheaper
 * intermediate values that can be used to compute the final factor when an index count is actually
 * needed to execute a draw function. It is kept separate so that Results can be preserved more
 * efficiently than the entire LinearTolerances object.
 */
class LinearTolerances {
public:
    class Results {
    public:
        // Fast log2 of minimum required # of segments per tracked Wang's formula calculations.
        int requiredResolveLevel() const {
            // log16(n^4) == log2(n)
            return wangs_formula::nextlog16(fMinRequiredParametricSegments_pow4);
        }

        int requiredStrokeEdges() const {
            // The maximum rotation we can have in a stroke is 180 degrees (SK_ScalarPI radians).
            int maxRadialSegmentsInStroke =
                    std::max(SkScalarCeilToInt(fMinRequiredRadialSegmentsPerRadian*SK_ScalarPI), 1);

            int maxParametricSegmentsInStroke =
                    SkScalarCeilToInt(wangs_formula::root4(fMinRequiredParametricSegments_pow4));
            SkASSERT(maxParametricSegmentsInStroke >= 1);

            // Now calculate the maximum number of edges we will need in the stroke portion of the
            // instance. The first and last edges in a stroke are shared by both the parametric and
            // radial sets of edges, so the total number of edges is:
            //
            //   numCombinedEdges = numParametricEdges + numRadialEdges - 2
            //
            // It's important to differentiate between the number of edges and segments in a strip:
            //
            //   numSegments = numEdges - 1
            //
            // So the total number of combined edges in the stroke is:
            //
            //   numEdgesInStroke = numParametricSegments + 1 + numRadialSegments + 1 - 2
            //                    = numParametricSegments + numRadialSegments
            //
            int maxEdgesInStroke = maxRadialSegmentsInStroke + maxParametricSegmentsInStroke;

            // Each triangle strip has two sections: It starts with a join then transitions to a
            // stroke. The number of edges in an instance is the sum of edges from the join and
            // stroke sections both.
            // NOTE: The final join edge and the first stroke edge are co-located, however we still
            // need to emit both because the join's edge is half-width and the stroke is full-width.
            return fMinRequiredEdgesInJoins + maxEdgesInStroke;
        }

    private:
        friend class LinearTolerances;

        // Accumulated minimum requirements to preserve tessellation precision in the worst case
        // over all curves and batched paths.

        // Used for both fills and strokes, always at least one parametric segment
        float fMinRequiredParametricSegments_pow4 = 1.f;

        // Used for strokes, adding additional segments along the curve to account for its rotation
        // TODO: Currently we assume the worst case 180 degree rotation for any curve, but tracking
        // max(radialSegments * patch curvature) would be tighter. This would require computing
        // rotation per patch, which could be approximated by tracking min of the tangent dot
        // products, but then we'd be left with the slightly less accurate
        // "max(radialSegments) * acos(min(tan dot product))". It is also unknown if requesting
        // tighter bounds pays off with less GPU work for more CPU work
        float fMinRequiredRadialSegmentsPerRadian = 0.f;

        // Used for strokes, tracking the number of additional vertices required to handle joins
        // based on the join type and stroke width.
        // TODO: For round joins, we could also track the rotation angle of the join, instead of
        // assuming 180 degrees. PatchWriter has all necessary control points to do so, but runs
        // into similar trade offs between CPU vs GPU work, and accuracy vs. reducing calls to acos.
        int fMinRequiredEdgesInJoins = 0.f;
    };

    const Results& current() const { return fResults; }

    // The max scale factor should be derived from the same matrix that 'xform' was. It's only used
    // in stroking calculations, so can be ignored for path filling.
    void setApproximateShaderTransform(const wangs_formula::VectorXform& xform,
                                       float maxScale = 1.f) {
        fCurrentApproxTransform = xform;
        fCurrentMaxScale = maxScale;
    }

    // Update parametric tolerances to account for a curve (cubic, quadratic, or conic) with the
    // given local-space control points, assuming it will be transformed by the last specified
    // shader transform.
    //
    // Returns the number of curve patch instances (see PatchWriter) that the curve must be uploaded
    // to the GPU. A value of 1 means it can be uploaded as-is; a value of 0 means it can be
    // uploaded as-is, but is visually equivalent to a line; a value greater than 1 is the number
    // of patches the curve must be split into.
    int accountForCubic(float2 p0, float2 p1, float2 p2, float2 p3) {
        float n4 = wangs_formula::cubic_pow4(kPrecision, p0, p1, p2, p3, fCurrentApproxTransform);
        return this->accountForCurve(n4);
    }
    int accountForQuadratic(float2 p0, float2 p1, float2 p2) {
        float n4 = wangs_formula::quadratic_pow4(kPrecision, p0, p1, p2, fCurrentApproxTransform);
        return this->accountForCurve(n4);
    }
    int accountForConic(float2 p0, float2 p1, float2 p2, float w) {
        float n2 = wangs_formula::conic_pow2(kPrecision, p0, p1, p2, w, fCurrentApproxTransform);
        return this->accountForCurve(n2 * n2);
    }

    void accountForTriangle() {
        // No chopping needed, the max supported segment count should always support 2 lines
        // (which form a triangle when implicitly closed).
        static constexpr float kTriangleSegments_pow4 = 2.f * 2.f * 2.f * 2.f;
        this->updateParametricSegments(kTriangleSegments_pow4);
    }

    void accountForStroke(const StrokeParams& strokeParams) {
        float approxDeviceStrokeRadius;
        if (strokeParams.fRadius == 0.f) {
            // Hairlines are always 1 px wide
            approxDeviceStrokeRadius = 0.5f;
        } else {
            // Approximate max scale * local stroke width / 2
            approxDeviceStrokeRadius = strokeParams.fRadius * fCurrentMaxScale;
        }

        fCurrentNumRadialSegmentsPerRadian =
                CalcNumRadialSegmentsPerRadian(approxDeviceStrokeRadius);

        fCurrentEdgesInJoins = NumFixedEdgesInJoin(strokeParams);
        if (strokeParams.fJoinType < 0.f && fCurrentNumRadialSegmentsPerRadian > 0.f) {
            // For round joins we need to count the radial edges on our own. Account for a
            // worst-case join of 180 degrees (SK_ScalarPI radians).
            fCurrentEdgesInJoins +=
                    SkScalarCeilToInt(fCurrentNumRadialSegmentsPerRadian * SK_ScalarPI) - 1;
        }

        this->updateRadialSegments(fCurrentNumRadialSegmentsPerRadian, fCurrentEdgesInJoins);
    }

    // Reset the accumulated Tolerances to just the current state of the tracker. This is useful
    // if a path or list of paths had to be split into multiple draw calls, at which point the
    // tolerances for later draws do not need to depend on the earlier paths.
    void resetTolerances() {
        fResults.fMinRequiredParametricSegments_pow4 = 1.0f;
        // Setting the min required to the current values preserves the last updated stroke params,
        // which we want if the reset came while in the middle of writing a path.
        fResults.fMinRequiredRadialSegmentsPerRadian = fCurrentNumRadialSegmentsPerRadian;
        fResults.fMinRequiredEdgesInJoins = fCurrentEdgesInJoins;
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
            this->updateParametricSegments(parametricSegments_pow4);
            return 1;
        } else {
            // The curve needs to be chopped
            this->updateParametricSegments(pow4(kMaxParametricSegments));
            return SkScalarCeilToInt(wangs_formula::root4(std::min(parametricSegments_pow4,
                                                                   pow4(kMaxSegmentsPerCurve)) /
                                                          pow4(kMaxParametricSegments)));
        }
    }

    void updateParametricSegments(float numParametricSegments_pow4) {
        if (numParametricSegments_pow4 > fResults.fMinRequiredParametricSegments_pow4) {
            fResults.fMinRequiredParametricSegments_pow4 = numParametricSegments_pow4;
        }
    }

    void updateRadialSegments(float numRadialSegmentsPerRadian, int numEdgesInJoin) {
        if (numRadialSegmentsPerRadian > fResults.fMinRequiredRadialSegmentsPerRadian) {
            fResults.fMinRequiredRadialSegmentsPerRadian = numRadialSegmentsPerRadian;
        }
        if (numEdgesInJoin > fResults.fMinRequiredEdgesInJoins) {
            fResults.fMinRequiredEdgesInJoins = numEdgesInJoin;
        }
    }

    // The 2x2 approximation of the local-to-device transform that will affect subsequently
    // recorded curves (when fully transformed in the vertex shader).
    wangs_formula::VectorXform fCurrentApproxTransform = {};
    // A maximum scale factor extracted from the current approximate transform.
    float fCurrentMaxScale = 1.0f;

    float fCurrentNumRadialSegmentsPerRadian = 0.f;
    int fCurrentEdgesInJoins = 0;

    Results fResults;
};

}  // namespace skgpu::tess

#endif // skgpu_tessellate_LinearTolerances_DEFINED
