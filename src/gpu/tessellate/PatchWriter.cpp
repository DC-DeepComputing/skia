/*
 * Copyright 2021 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/tessellate/PatchWriter.h"

#include "src/gpu/tessellate/WangsFormula.h"

#if SK_GPU_V1
#include "src/gpu/tessellate/PathTessellator.h"
#include "src/gpu/tessellate/StrokeTessellator.h"
#endif

namespace skgpu {

SK_ALWAYS_INLINE SkPoint to_skpoint(float2 p) { return skvx::bit_pun<SkPoint>(p); }

#if SK_GPU_V1
PatchWriter::PatchWriter(GrMeshDrawTarget* target,
                         PathTessellator* tessellator,
                         int maxTessellationSegments,
                         int initialPatchAllocCount)
        : PatchWriter(target,
                      &tessellator->fVertexChunkArray,
                      tessellator->fAttribs,
                      maxTessellationSegments,
                      sizeof(SkPoint) * 4 + PatchAttribsStride(tessellator->fAttribs),
                      initialPatchAllocCount) {
}

PatchWriter::PatchWriter(GrMeshDrawTarget* target,
                         StrokeTessellator* tessellator,
                         int maxTessellationSegments,
                         int initialPatchAllocCount)
        : PatchWriter(target,
                      &tessellator->fVertexChunkArray,
                      tessellator->fAttribs,
                      maxTessellationSegments,
                      sizeof(SkPoint) * 4 + PatchAttribsStride(tessellator->fAttribs),
                      initialPatchAllocCount) {
}
#endif

void PatchWriter::chopAndWriteQuads(float2 p0, float2 p1, float2 p2, int numPatches) {
    // If we're in this function, the curve is being chopped to reach the max segments, so assume
    // that that is what n^4 would evaluate to.
    const float n4 = fMaxSegments_pow4;

    // If we aren't fanning, we need to fill the space between chops with triangles.
    bool needsInnerTriangles = !(fAttribs & PatchAttribs::kFanPoint);
    MiddleOutPolygonTriangulator innerTriangulator(numPatches, to_skpoint(p0));
    for (; numPatches >= 3; numPatches -= 2) {
        // Chop into 3 quads.
        float4 T = float4(1,1,2,2) / numPatches;
        float4 ab = mix(p0.xyxy(), p1.xyxy(), T);
        float4 bc = mix(p1.xyxy(), p2.xyxy(), T);
        float4 abc = mix(ab, bc, T);
        // p1 & p2 of the cubic representation of the middle quad.
        float4 middle = mix(ab, bc, mix(T, T.zwxy(), 2/3.f));

        *this << Quadratic(p0, ab.lo, abc.lo, n4);  // Write the 1st quad.
        if (needsInnerTriangles) {
            *this << Triangle(p0, abc.lo, abc.hi);
        }
        *this << Cubic(abc.lo, middle.lo, middle.hi, abc.hi, n4);  // Write the 2nd quad.
        if (needsInnerTriangles) {
            *this << innerTriangulator.pushVertex(to_skpoint(abc.hi));
        }
        std::tie(p0, p1) = {abc.hi, bc.hi};  // Save the 3rd quad.
    }
    if (numPatches == 2) {
        // Chop into 2 quads.
        float2 ab = (p0 + p1) * .5f;
        float2 bc = (p1 + p2) * .5f;
        float2 abc = (ab + bc) * .5f;

        *this << Quadratic(p0, ab, abc, n4);  // Write the 1st quad.
        if (needsInnerTriangles) {
            *this << Triangle(p0, abc, p2);
        }
        *this << Quadratic(abc, bc, p2, n4);  // Write the 2nd quad.
    } else {
        SkASSERT(numPatches == 1);
        *this << Quadratic(p0, p1, p2, n4);  // Write the single quad.
    }
    if (needsInnerTriangles) {
        *this << innerTriangulator.pushVertex(to_skpoint(p2));
        *this << innerTriangulator.close();
    }
}

void PatchWriter::chopAndWriteConics(float2 p0, float2 p1, float2 p2, float w, int numPatches) {
    const float n4 = fMaxSegments_pow4;

    // If we aren't fanning, we need to fill the space between chops with triangles.
    bool needsInnerTriangles = !(fAttribs & PatchAttribs::kFanPoint);
    MiddleOutPolygonTriangulator innerTriangulator(numPatches, to_skpoint(p0));
    // Load the conic in 3d homogeneous (unprojected) space.
    float4 h0 = float4(p0,1,1);
    float4 h1 = float4(p1,1,1) * w;
    float4 h2 = float4(p2,1,1);
    for (; numPatches >= 2; --numPatches) {
        // Chop in homogeneous space.
        float T = 1.f/numPatches;
        float4 ab = mix(h0, h1, T);
        float4 bc = mix(h1, h2, T);
        float4 abc = mix(ab, bc, T);

        // Project and write the 1st conic.
        float2 midpoint = abc.xy() / abc.w();
        *this << Conic(h0.xy() / h0.w(),
                       ab.xy() / ab.w(),
                       midpoint,
                       ab.w() / sqrtf(h0.w() * abc.w()),
                       n4);

        if (needsInnerTriangles) {
            *this << innerTriangulator.pushVertex(to_skpoint(midpoint));
        }
        std::tie(h0, h1) = {abc, bc};  // Save the 2nd conic (in homogeneous space).
    }
    // Project and write the remaining conic.
    SkASSERT(numPatches == 1);
    *this << Conic(h0.xy() / h0.w(),
                   h1.xy() / h1.w(),
                   h2.xy(), // h2.w == 1
                   h1.w() / sqrtf(h0.w()),
                   n4);
    if (needsInnerTriangles) {
        *this << innerTriangulator.pushVertex(to_skpoint(h2.xy()));
        *this << innerTriangulator.close();
    }
}

void PatchWriter::chopAndWriteCubics(float2 p0, float2 p1, float2 p2, float2 p3, int numPatches) {
    const float n4 = fMaxSegments_pow4;

    // If we aren't fanning, we need to fill the space between chops with triangles.
    bool needsInnerTriangles = !(fAttribs & PatchAttribs::kFanPoint);
    MiddleOutPolygonTriangulator innerTriangulator(numPatches, to_skpoint(p0));
    for (; numPatches >= 3; numPatches -= 2) {
        // Chop into 3 cubics.
        float4 T = float4(1,1,2,2) / numPatches;
        float4 ab = mix(p0.xyxy(), p1.xyxy(), T);
        float4 bc = mix(p1.xyxy(), p2.xyxy(), T);
        float4 cd = mix(p2.xyxy(), p3.xyxy(), T);
        float4 abc = mix(ab, bc, T);
        float4 bcd = mix(bc, cd, T);
        float4 abcd = mix(abc, bcd, T);
        float4 middle = mix(abc, bcd, T.zwxy());  // p1 & p2 of the middle cubic.

        *this << Cubic(p0, ab.lo, abc.lo, abcd.lo, n4);  // Write the 1st cubic.
        if (needsInnerTriangles) {
            *this << Triangle(p0, abcd.lo, abcd.hi);
        }
        *this << Cubic(abcd.lo, middle.lo, middle.hi, abcd.hi, n4);  // Write the 2nd cubic.
        if (needsInnerTriangles) {
            *this << innerTriangulator.pushVertex(to_skpoint(abcd.hi));
        }
        std::tie(p0, p1, p2) = {abcd.hi, bcd.hi, cd.hi};  // Save the 3rd cubic.
    }
    if (numPatches == 2) {
        // Chop into 2 cubics.
        float2 ab = (p0 + p1) * .5f;
        float2 bc = (p1 + p2) * .5f;
        float2 cd = (p2 + p3) * .5f;
        float2 abc = (ab + bc) * .5f;
        float2 bcd = (bc + cd) * .5f;
        float2 abcd = (abc + bcd) * .5f;

        *this << Cubic(p0, ab, abc, abcd, n4);  // Write the 1st cubic.
        if (needsInnerTriangles) {
            *this << Triangle(p0, abcd, p3);
        }
        *this << Cubic(abcd, bcd, cd, p3, n4);  // Write the 2nd cubic.
    } else {
        SkASSERT(numPatches == 1);
        *this << Cubic(p0, p1, p2, p3, n4);  // Write the single cubic.
    }
    if (needsInnerTriangles) {
        *this << innerTriangulator.pushVertex(to_skpoint(p3));
        *this << innerTriangulator.close();
    }
}

// float PatchWriter::chopAndWriteQuads(float2 p0, float2 p1, float2 p2,
//                                      const wangs_formula::VectorXform& shaderXform) {
//     float n4 = wangs_formula::quadratic_pow4(kTessellationPrecision, p0, p1, p2, shaderXform);
//     // TODO: Expand this check somehow for strokes too
//     if (!(fAttribs & PatchAttribs::kFanPoint) && n4 <= 1.f) {
//         // This quad only needs one segment (e.g. line), and we're not filling space with fans so
//         // nothing needs to be written.
//         return 0.f;
//     } else if (n4 <= fMaxSegments_pow4) {
//         // This quad already fits in the max tessellation segments
//         CubicPatch(*this, n4) << QuadToCubic{p0, p1, p2};
//         fStrokeControlPointAttrib = p1;
//         return n4;
//     } else {
//         // The path should have been pre-chopped if needed so that all curves fit within
//         // kMaxTessellationSegmentsPerCurve, but clamp to ensure we don't recurse too much.
//         int numPatches = SkScalarCeilToInt(wangs_formula::root4(
//                 std::min(n4, pow4(kMaxTessellationSegmentsPerCurve)) / fMaxSegments_pow4));
//         this->chopAndWriteQuads(p0, p1, p2, numPatches);
//         // n4 was needed for the unchopped quad; chopAndWriteQuads should have written 'numPatches'
//         // quads that only need fMaxSegments_pow4, so that's the effective wang's result to report.
//         return fMaxSegments_pow4;
//     }
// }

// // 3 challenges with migrating strokes to within these same functions
// // 1. Conic control point representation is different?
// // 2. Where to store the deferred stroke, and does that deferred stroke need to be choppped or
// //    could we allow it to just be the 1st verb before we get to the contour close, even if were
// //    then subdivided?
// // 3. What defines the control points? Is it the first point? Last? 2nd to last regardless of verb?
// // 4. Is recursion splitting at 1/2 better or different than the chopping defined for fills?
// // 5. Do we need to update the end control point for chopped patches?
// // 6. What about strokes that aren't closed? Does that use kMoveWithinContour to set the "last"
// //    control point before we add any patches so there never is a deferred patch?
// // 7. Could the stroke iterator provide the last control point more easily so there's no deferring?
// // 8. Deferring a patch technically moves it to be drawn later in the instance list, but that's fine
// //    since each patch can be ordered arbitrarily

// float PatchWriter::chopAndWriteConics(float2 p0, float2 p1, float2 p2, float w,
//                                       const wangs_formula::VectorXform& shaderXform) {
//     float n2 = wangs_formula::conic_pow2(kTessellationPrecision, p0, p1, p2, w, shaderXform);
//     if (!(fAttribs & PatchAttribs::kFanPoint) && n2 <= 1.f) {
//         return 0.f;
//     } else if (n2 <= fMaxSegments_pow2) {
//         // This conic already fits in the max tessellation segments
//         ConicPatch(*this, n2*n2) << p0 << p1 << p2 << w;
//         fStrokeControlPointAttrib = p1;
//         return n2 * n2;
//     } else {
//         // Like chopAndWriteQuads, compute recursion limit and then chop
//         int numPatches = SkScalarCeilToInt(sqrtf(
//                 std::min(n2, pow2(kMaxTessellationSegmentsPerCurve)) / fMaxSegments_pow2));
//         this->chopAndWriteConics(p0, p1, p2, w, numPatches);
//         return fMaxSegments_pow4;
//     }
// }

// float PatchWriter::chopAndWriteCubics(float2 p0, float2 p1, float2 p2, float2 p3,
//                                       const wangs_formula::VectorXform& shaderXform) {
//     float n4 = wangs_formula::cubic_pow4(kTessellationPrecision, p0, p1, p2, p3, shaderXform);
//     if (!(fAttribs & PatchAttribs::kFanPoint) && n4 <= 1.f) {
//         return 0.f;
//     } else if (n4 <= fMaxSegments_pow4) {
//         // This cubic already fits in the max tessellation segments
//         CubicPatch(*this, n4) << p0 << p1 << p2 << p3;
//         fStrokeControlPointAttrib =
//         return n4;
//     } else {
//         // Like chopAndWriteQuads, compute recursion limit and then chop
//         int numPatches = SkScalarCeilToInt(wangs_formula::root4(
//                 std::min(n4, pow4(kMaxTessellationSegmentsPerCurve)) / fMaxSegments_pow4));
//         this->chopAndWriteCubics(p0, p1, p2, p3, numPatches);
//         return fMaxSegments_pow4;
//     }
// }

}  // namespace skgpu
