/*
 * Copyright 2020 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrStrokeTessellationShader_DEFINED
#define GrStrokeTessellationShader_DEFINED

#include "src/gpu/ganesh/tessellate/shaders/GrTessellationShader.h"

#include "include/core/SkStrokeRec.h"
#include "src/gpu/ganesh/GrVx.h"
#include "src/gpu/ganesh/glsl/GrGLSLVarying.h"
#include "src/gpu/tessellate/Tessellation.h"

// Tessellates a batch of stroke patches directly to the canvas. Tessellated stroking works by
// creating stroke-width, orthogonal edges at set locations along the curve and then connecting them
// with a quad strip. These orthogonal edges come from two different sets: "parametric edges" and
// "radial edges". Parametric edges are spaced evenly in the parametric sense, and radial edges
// divide the curve's _rotation_ into even steps. The tessellation shader evaluates both sets of
// edges and sorts them into a single quad strip. With this combined set of edges we can stroke any
// curve, regardless of curvature.
class GrStrokeTessellationShader : public GrTessellationShader {
    using PatchAttribs = skgpu::PatchAttribs;

public:

    // 'viewMatrix' is applied to the geometry post tessellation. It cannot have perspective.
    GrStrokeTessellationShader(const GrShaderCaps&, PatchAttribs, const SkMatrix& viewMatrix,
                               const SkStrokeRec&, SkPMColor4f, int8_t maxParametricSegments_log2);

    PatchAttribs attribs() const { return fPatchAttribs; }
    bool hasDynamicStroke() const { return fPatchAttribs & PatchAttribs::kStrokeParams; }
    bool hasDynamicColor() const { return fPatchAttribs & PatchAttribs::kColor; }
    bool hasExplicitCurveType() const { return fPatchAttribs & PatchAttribs::kExplicitCurveType; }
    const SkStrokeRec& stroke() const { return fStroke;}
    int8_t maxParametricSegments_log2() const { return fMaxParametricSegments_log2; }
    float fixedCountNumTotalEdges() const { return fFixedCountNumTotalEdges;}

    // Used by GrFixedCountTessellator to configure the uniform value that tells the shader how many
    // total edges are in the triangle strip.
    void setFixedCountNumTotalEdges(int value) {
        fFixedCountNumTotalEdges = value;
    }

private:
    const char* name() const override { return "GrStrokeTessellationShader"; }
    void addToKey(const GrShaderCaps&, skgpu::KeyBuilder*) const override;
    std::unique_ptr<ProgramImpl> makeProgramImpl(const GrShaderCaps&) const final;

    const PatchAttribs fPatchAttribs;
    const SkStrokeRec fStroke;
    const int8_t fMaxParametricSegments_log2;

    constexpr static int kMaxAttribCount = 6;
    SkSTArray<kMaxAttribCount, Attribute> fAttribs;

    // This is a uniform value used when fMode is kFixedCount that tells the shader how many total
    // edges are in the triangle strip.
    float fFixedCountNumTotalEdges = 0;

    class Impl;
};

#endif
