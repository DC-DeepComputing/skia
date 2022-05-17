/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ganesh/effects/GrConvexPolyEffect.h"

#include "src/core/SkPathPriv.h"
#include "src/core/SkRuntimeEffectPriv.h"
#include "src/gpu/KeyBuilder.h"
#include "src/gpu/ganesh/effects/GrSkSLFP.h"
#include "src/gpu/ganesh/glsl/GrGLSLFragmentShaderBuilder.h"
#include "src/gpu/ganesh/glsl/GrGLSLProgramDataManager.h"
#include "src/gpu/ganesh/glsl/GrGLSLUniformHandler.h"

/**
 * edges is a set of n edge equations where n is limited to kMaxEdges. It contains 3*n values.
 * The edges should form a convex polygon. The positive half-plane is considered to be the
 * inside. The equations should be normalized such that the first two coefficients are a unit
 * 2d vector.
 *
 * Currently the edges are specified in device space. In the future we may prefer to specify
 * them in src space. There are a number of ways this could be accomplished but we'd probably
 * have to modify the effect/shaderbuilder interface to make it possible (e.g. give access
 * to the view matrix or untransformed positions in the fragment shader).
 */
static GrFPResult make_convex_poly_effect(std::unique_ptr<GrFragmentProcessor> inputFP,
                                          GrClipEdgeType edgeType,
                                          int n,
                                          float edges[]) {
    if (n <= 0 || n > GrConvexPolyEffect::kMaxEdges) {
        return GrFPFailure(std::move(inputFP));
    }

    // Outset the edges by 0.5 so that a pixel with center on an edge is 50% covered in the AA case
    // and 100% covered in the non-AA case.
    for (int i = 0; i < n; ++i) {
        edges[3 * i + 2] += SK_ScalarHalf;
    }

    static_assert(GrConvexPolyEffect::kMaxEdges == 8);
    static sk_sp<SkRuntimeEffect> effect = SkMakeRuntimeEffect(SkRuntimeEffect::MakeForShader, R"(
        uniform shader child;
        uniform half3 EdgeArray[8];  // kMaxEdges
        uniform int EdgeCount;
        uniform int Antialias;
        uniform int IsInverseFill;

        half4 main(float2 coords) {
            half alpha = 1.0;
            half edge;
            for (int i = 0; i < EdgeCount; ++i) {
                edge = dot(EdgeArray[i], half3(sk_FragCoord.xy1));
                alpha *= (Antialias != 0) ? saturate(edge) : step(0.5, edge);
            }
            if (IsInverseFill != 0) {
                alpha = 1.0 - alpha;
            }
            return child.eval(coords) * alpha;
        }
    )",
    // This shader won't require ES3 after specializations are applied; however, during the initial
    // parse, `EdgeCount` isn't known yet, so the loop initially appears to be unrollable.
    SkRuntimeEffectPriv::ES3Options());

    return GrFPSuccess(GrSkSLFP::Make(
               effect, "ConvexPolyEffect", /*inputFP=*/nullptr,
               GrSkSLFP::OptFlags::kCompatibleWithCoverageAsAlpha,
               "child", std::move(inputFP),
               "EdgeArray", SkMakeSpan((SkV3*)edges, GrConvexPolyEffect::kMaxEdges),
               "EdgeCount", GrSkSLFP::Specialize<int>(n),
               "Antialias", GrSkSLFP::Specialize<int>(GrClipEdgeTypeIsAA(edgeType)),
               "IsInverseFill", GrSkSLFP::Specialize<int>(GrClipEdgeTypeIsInverseFill(edgeType))));
}

GrFPResult GrConvexPolyEffect::Make(std::unique_ptr<GrFragmentProcessor> inputFP,
                                    GrClipEdgeType type, const SkPath& path) {
    if (path.getSegmentMasks() != SkPath::kLine_SegmentMask || !path.isConvex()) {
        return GrFPFailure(std::move(inputFP));
    }

    SkPathFirstDirection dir = SkPathPriv::ComputeFirstDirection(path);
    // The only way this should fail is if the clip is effectively a infinitely thin line. In that
    // case nothing is inside the clip. It'd be nice to detect this at a higher level and either
    // skip the draw or omit the clip element.
    if (dir == SkPathFirstDirection::kUnknown) {
        if (GrClipEdgeTypeIsInverseFill(type)) {
            return GrFPSuccess(
                    GrFragmentProcessor::ModulateRGBA(std::move(inputFP), SK_PMColor4fWHITE));
        }
        // This could use ConstColor instead of ModulateRGBA but it would trigger a debug print
        // about a coverage processor not being compatible with the alpha-as-coverage optimization.
        // We don't really care about this unlikely case so we just use ModulateRGBA to suppress
        // the print.
        return GrFPSuccess(
                GrFragmentProcessor::ModulateRGBA(std::move(inputFP), SK_PMColor4fTRANSPARENT));
    }

    SkScalar        edges[3 * kMaxEdges];
    SkPoint         pts[4];
    SkPath::Verb    verb;
    SkPath::Iter    iter(path, true);

    // SkPath considers itself convex so long as there is a convex contour within it,
    // regardless of any degenerate contours such as a string of moveTos before it.
    // Iterate here to consume any degenerate contours and only process the points
    // on the actual convex contour.
    int n = 0;
    while ((verb = iter.next(pts)) != SkPath::kDone_Verb) {
        switch (verb) {
            case SkPath::kMove_Verb:
            case SkPath::kClose_Verb:
                break;
            case SkPath::kLine_Verb: {
                if (n >= kMaxEdges) {
                    return GrFPFailure(std::move(inputFP));
                }
                if (pts[0] != pts[1]) {
                    SkVector v = pts[1] - pts[0];
                    v.normalize();
                    if (SkPathFirstDirection::kCCW == dir) {
                        edges[3 * n] = v.fY;
                        edges[3 * n + 1] = -v.fX;
                    } else {
                        edges[3 * n] = -v.fY;
                        edges[3 * n + 1] = v.fX;
                    }
                    edges[3 * n + 2] = -(edges[3 * n] * pts[1].fX + edges[3 * n + 1] * pts[1].fY);
                    ++n;
                }
                break;
            }
            default:
                // Non-linear segment so not a polygon.
                return GrFPFailure(std::move(inputFP));
        }
    }

    if (path.isInverseFillType()) {
        type = GrInvertClipEdgeType(type);
    }

    return make_convex_poly_effect(std::move(inputFP), type, n, edges);
}
