/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/graphite/src/Renderer.h"

#include "experimental/graphite/src/ContextUtils.h"
#include "experimental/graphite/src/DrawBufferManager.h"
#include "experimental/graphite/src/DrawWriter.h"
#include "experimental/graphite/src/UniformManager.h"
#include "experimental/graphite/src/geom/Shape.h"
#include "experimental/graphite/src/geom/Transform_graphite.h"
#include "include/core/SkPathTypes.h"
#include "include/core/SkRect.h"
#include "src/gpu/BufferWriter.h"
#include "src/gpu/tessellate/MiddleOutPolygonTriangulator.h"

#include "src/gpu/tessellate/PathCurveTessellator.h"
#include "src/gpu/tessellate/PatchWriter.h"
#include "src/gpu/tessellate/WangsFormula.h"

namespace skgpu {

namespace {

// TODO: These settings are actually shared by tessellating path renderers, so will be exposed later

// Returns the stencil settings to use for a standard Redbook "stencil" pass.
constexpr DepthStencilSettings fillrule_settings(bool evenOdd) {
    // Increments clockwise triangles and decrements counterclockwise. Used for "winding" fill.
    constexpr DepthStencilSettings::Face kIncCW = {
        /*stencilFail=*/   StencilOp::kKeep,
        /*depthFail=*/     StencilOp::kKeep,
        /*dsPass=*/        StencilOp::kIncWrap,
        /*stencilCompare=*/CompareOp::kAlways,
        /*readMask=*/      0xffffffff,
        /*writeMask=*/     0xffffffff
    };
    constexpr DepthStencilSettings::Face kDecCCW = {
        /*stencilFail=*/   StencilOp::kKeep,
        /*depthFail=*/     StencilOp::kKeep,
        /*dsPass=*/        StencilOp::kDecWrap,
        /*stencilCompare=*/CompareOp::kAlways,
        /*readMask=*/      0xffffffff,
        /*writeMask=*/     0xffffffff
    };

    // Toggles the bottom stencil bit. Used for "even-odd" fill.
    constexpr DepthStencilSettings::Face kToggle = {
        /*stencilFail=*/   StencilOp::kKeep,
        /*depthFail=*/     StencilOp::kKeep,
        /*dsPass=*/        StencilOp::kInvert,
        /*stencilCompare=*/CompareOp::kAlways,
        /*readMask=*/      0xffffffff,
        /*writeMask=*/     0x00000001
    };

    // Always use ref = 0, disable depths, but still use greater depth test.
    constexpr DepthStencilSettings kWindingFill = {
        /*frontStencil=*/kIncCW,
        /*backStencil=*/ kDecCCW,
        /*refValue=*/    0,
        /*stencilTest=*/ true,
        /*depthCompare=*/CompareOp::kAlways, // kGreater once steps know the right depth value
        /*depthTest=*/   true,
        /*depthWrite=*/  false
    };
    constexpr DepthStencilSettings kEvenOddFill = {
        /*frontStencil=*/kToggle,
        /*backStencil=*/ kToggle,
        /*refValue=*/    0,
        /*stencilTest=*/ true,
        /*depthCompare=*/CompareOp::kAlways, // kGreater once steps know the right depth value
        /*depthTest=*/   true,
        /*depthWrite=*/  false
    };

    return evenOdd ? kEvenOddFill : kWindingFill;
}

// Returns the stencil settings to use for a standard Redbook "fill" pass. Allows non-zero
// stencil values to pass and write a color, and resets the stencil value back to zero; discards
// immediately on stencil values of zero (or does the inverse of these operations when the path
// requires filling everything else).
constexpr DepthStencilSettings cover_settings(bool inverse) {
    // Resets non-zero bits to 0, passes when not zero. We set depthFail to kZero because if we
    // encounter that case, the kNotEqual=0 stencil test passed, so it does need to be set back to 0
    // and the dsPass op won't be run. In practice, since the stencil steps will fail the same depth
    // test, the stencil value will likely not be non-zero, but best to be explicit.
    constexpr DepthStencilSettings::Face kNormal = {
        /*stencilFail=*/   StencilOp::kKeep,
        /*depthFail=*/     StencilOp::kZero,
        /*dsPass=*/        StencilOp::kZero,
        /*stencilCompare=*/CompareOp::kNotEqual,
        /*readMask=*/      0xffffffff,
        /*writeMask=*/     0xffffffff
    };

    // Resets non-zero bits to 0, passes when zero
    constexpr DepthStencilSettings::Face kInverted = {
        /*stencilFail=*/   StencilOp::kZero,
        /*depthFail=*/     StencilOp::kKeep,
        /*dsPass=*/        StencilOp::kKeep,
        /*stencilCompare=*/CompareOp::kEqual,
        /*readMask=*/      0xffffffff,
        /*writeMask=*/     0xffffffff
    };

    // Always use ref = 0, enabled depth writes, and greater depth test, both
    // front and back use the same stencil settings.
    constexpr DepthStencilSettings kNormalDSS = {
        /*frontStencil=*/kNormal,
        /*frontStencil=*/kNormal,
        /*refValue=*/    0,
        /*stencilTest=*/ true,
        /*depthCompare=*/CompareOp::kAlways, // kGreater once steps know the right depth value
        /*depthTest=*/   true,
        /*depthWrite=*/  true
    };
    constexpr DepthStencilSettings kInvertedDSS = {
        /*frontStencil=*/kInverted,
        /*backStencil=*/ kInverted,
        /*refValue=*/    0,
        /*stencilTest=*/ true,
        /*depthCompare=*/CompareOp::kAlways, // kGreater once steps know the right depth value
        /*depthTest=*/   true,
        /*depthWrite=*/  true
    };
    return inverse ? kInvertedDSS : kNormalDSS;
}

class StencilFanRenderStep final : public RenderStep {
public:
    StencilFanRenderStep(bool evenOdd)
            : RenderStep(Flags::kRequiresMSAA,
                         /*uniforms=*/{},
                         PrimitiveType::kTriangles,
                         fillrule_settings(evenOdd),
                         /*vertexAttrs=*/{{"position", VertexAttribType::kFloat3, SLType::kFloat3}},
                         /*instanceAttrs=*/{}) {}

    ~StencilFanRenderStep() override {}

    const char* name() const override { return "stencil-fan"; }

    const char* vertexSkSL() const override {
        return "     float4 devPosition = float4(position.xy, 0.0, position.z);\n";
    }

    void writeVertices(DrawWriter* writer,
                       const SkIRect& bounds,
                       const Transform& localToDevice,
                       const Shape& shape) const override {
        // TODO: Have Shape provide a path-like iterator so we don't actually have to convert non
        // paths to SkPath just to iterate their pts/verbs
        SkPath path = shape.asPath();
        for (PathMiddleOutFanIter it(path); !it.done();) {
            for (auto [p0, p1, p2] : it.nextStack()) {
                // TODO: PathMiddleOutFanIter should use SkV2 instead of SkPoint?
                SkV2 p[3] = {{p0.fX, p0.fY}, {p1.fX, p1.fY}, {p2.fX, p2.fY}};
                SkV4 devPoints[3];
                localToDevice.mapPoints(p, devPoints, 3);

                // TODO: Instead do one appendVertices(maxTrianglsInFans*3) and then return vertices
                // at the end to avoid redundant bounds/offset checking in the DrawWriter.
                auto vw = writer->appendVertices(3);

                vw << devPoints[0].x << devPoints[0].y << devPoints[0].w  // p0
                   << devPoints[1].x << devPoints[1].y << devPoints[1].w  // p1
                   << devPoints[2].x << devPoints[2].y << devPoints[2].w; // p2
            }
        }
    }

    sk_sp<UniformData> writeUniforms(Layout layout,
                                     const SkIRect&,
                                     const Transform&,
                                     const Shape&) const override {
        // Control points are pre-transformed to device space on the CPU, so no uniforms needed.
        return nullptr;
    }
};

class DrawWriterVertexStore : public VertexStore {
public:
    DrawWriterVertexStore(DrawWriter* writer, size_t stride)
            : fWriter(writer)
            , fStride(stride) {}

    ~DrawWriterVertexStore() override {}

    VertexWriter appendVertex(float fixedSegments_pow4) override {
        int resolveLevel = wangs_formula::nextlog16(fixedSegments_pow4);
        int indexCount = PathTessellator::NumCurveTrianglesAtResolveLevel(resolveLevel) * 3;
        return fWriter->appendDynamicTemplateInstances(indexCount, 1);
        // return fWriter->appendInstances(1);
    }

    size_t stride() const override { return fStride; }

private:
    DrawWriter* fWriter;
    size_t fStride;
};

class StencilCurvesRenderStep : public RenderStep {
    // No fan point, since this step only does curves, not wedges
    // No stroke params, since this step is for fills
    // No packed color, since our hard-coded pipeline doesn't support that yet
    // No explicit curve type, since we assume infinity is supported for now
    // No wide color, since that should be embedded in ShaderCombination to avoid permutations
    static constexpr PatchAttribs kAttribs = PatchAttribs::kNone;

public:
    StencilCurvesRenderStep(bool evenOdd)
            : RenderStep(Flags::kRequiresMSAA,
                         /*uniforms=*/{},
                         PrimitiveType::kTriangles,
                         fillrule_settings(evenOdd),
                         /*vertexAttrs=*/{{"resolveLevel_and_idx",
                                           VertexAttribType::kFloat2, SLType::kFloat2}},
                         /*instanceAttrs=*/{{"p01", VertexAttribType::kFloat4, SLType::kFloat4},
                                            {"p23", VertexAttribType::kFloat4, SLType::kFloat4}}) {
        // Since it is a filled tessellation with no other patch attribs, the instance attribs
        // are just 4 float2 control points
        static_assert(0 == PatchAttribsStride(kAttribs));
        SkASSERT(4 * sizeof(SkV2) == this->instanceStride());
    }

    ~StencilCurvesRenderStep() override {}

    const char* name() const override { return "stencil-curves"; }

    const char* vertexSkSL() const override {
        // TODO: Just stuff this in the vertex shader so that everyone has it?
        // v->insertFunction(skgpu::wangs_formula::as_sksl().c_str());
        return R"(
            float resolveLevel = resolveLevel_and_idx.x;
            float idxInResolveLevel = resolveLevel_and_idx.y;
            float2 localcoord;
            if (isinf(p23.z)) {
                // This patch is an exact triangle.
                localcoord = (resolveLevel != 0)      ? p01.zw
                           : (idxInResolveLevel != 0) ? p23.xy
                                                      : p01.xy;
            } else {
                float2 p0=p01.xy, p1=p01.zw, p2=p23.xy, p3=p23.zw;
                float w = -1;  // w < 0 tells us to treat the instance as an integral cubic.
                float maxResolveLevel;
                if (isinf(p23.w)) {
                    // Conics are 3 points, with the weight in p3.
                    w = p3.x;
                    maxResolveLevel = wangs_formula_conic_log2(4, p0, p1, p2, w);
                    p1 *= w;  // Unproject p1.
                    p3 = p2;  // Duplicate the endpoint for shared code that also runs on cubics.
                } else {
                    // The patch is an integral cubic.
                    maxResolveLevel = wangs_formula_cubic_log2(4, p0, p1, p2, p3, float2x2(1.0, 0.0, 0.0, 1.0));
                }
                if (resolveLevel > maxResolveLevel) {
                    // This vertex is at a higher resolve level than we need. Demote to a lower
                    // resolveLevel, which will produce a degenerate triangle.
                    idxInResolveLevel = floor(ldexp(idxInResolveLevel,
                                                    int(maxResolveLevel - resolveLevel)));
                    resolveLevel = maxResolveLevel;
                }
                // Promote our location to a discrete position in the maximum fixed resolve level.
                // This is extra paranoia to ensure we get the exact same fp32 coordinates for
                // colocated points from different resolve levels (e.g., the vertices T=3/4 and
                // T=6/8 should be exactly colocated).
                float fixedVertexID = floor(.5 + ldexp(
                        idxInResolveLevel, int(5 - resolveLevel)));
                if (0 < fixedVertexID && fixedVertexID < 32) {
                    float T = fixedVertexID * (1 / 32.0);

                    // Evaluate at T. Use De Casteljau's for its accuracy and stability.
                    float2 ab = mix(p0, p1, T);
                    float2 bc = mix(p1, p2, T);
                    float2 cd = mix(p2, p3, T);
                    float2 abc = mix(ab, bc, T);
                    float2 bcd = mix(bc, cd, T);
                    float2 abcd = mix(abc, bcd, T);

                    // Evaluate the conic weight at T.
                    float u = mix(1.0, w, T);
                    float v = w + 1 - u;  // == mix(w, 1, T)
                    float uv = mix(u, v, T);

                    localcoord = (w < 0) ? /*cubic*/ abcd : /*conic*/ abc/uv;
                } else {
                    localcoord = (fixedVertexID == 0) ? p0.xy : p3.xy;
                }
            }
            float4 devPosition = float4(localcoord.xy, 0.0, 1.0);)";
    }

    void writeVertices(DrawWriter* writer,
                       const SkIRect& bounds,
                       const Transform& localToDevice,
                       const Shape& shape) const override {
        // TODO caps check
        int maxTessellationSegments = 32;

        // TODO: Support SkM44 in PatchWriter and be able to write homogenous coords; this falls
        // over if localToDevice has perspective.
        SkMatrix m = localToDevice.matrix().asM33();

        SkPath path = shape.asPath(); // TODO: Use path iterator on shape directly
        PathTessellator::PathDrawList list{m, path, {1.f, 1.f, 1.f, 1.f}};

        // Dynamic templates have a flexible index count that can be increased by subsequent calls
        // to writeVertices. The minimum index count must be set for each patch that is recorded.
        BindBufferInfo vertices = writer->bufferManager()->getStaticBuffer(
                BufferType::kVertex,
                PathCurveTessellator::WriteFixedVertexBuffer,
                FixedVertexBufferSize);
        BindBufferInfo indices = writer->bufferManager()->getStaticBuffer(
                BufferType::kIndex,
                PathCurveTessellator::WriteFixedIndexBuffer,
                FixedIndexBufferSize);

        writer->setDynamicInstanceTemplate(vertices, indices);
        DrawWriterVertexStore vertexStore(writer, this->instanceStride());
        PatchWriter patchWriter(&vertexStore, kAttribs);

        // TODO: Probably write this loop for DrawWriter/PatchWriter and homogenous coords w/o
        // the path curve tessellator baggage.
        // TODO: Would be nice to figure out a better way to get the resolve level, although the
        // dynamic template business isn't sooo awkward, so long as it has the appendDynamicInstances()
        // take both the desired count and number of instances to add that would use that count.
        // -- but maybe that really does mean we write to chunks first and then push all at once?
        // -- could be solved if we always used max resolve level index count but uploaded the resolve level
        //    needed per patch, then we could compare the patch resolve level to the vtx resolve level
        //    and automatically emit degenerate tris. May make things simpler?
        PathCurveTessellator::WritePatches(patchWriter, maxTessellationSegments, kAttribs, SkMatrix::I(), list);
    }

    sk_sp<UniformData> writeUniforms(Layout layout,
                                     const SkIRect&,
                                     const Transform&,
                                     const Shape&) const override {
        // Control points are pre-transformed to device space on the CPU, so no uniforms needed.
        return nullptr;
    }

    static size_t FixedVertexBufferSize() {
        return PathCurveTessellator::FixedVertexBufferSize(PathTessellator::kMaxFixedResolveLevel);
    }
    static size_t FixedIndexBufferSize() {
        return PathCurveTessellator::FixedIndexBufferSize(PathTessellator::kMaxFixedResolveLevel);
    }
};

class FillBoundsRenderStep final : public RenderStep {
public:
    // TODO: Will need to add kRequiresStencil when we support specifying stencil settings and
    // the Renderer includes the stenciling step first.
    FillBoundsRenderStep(bool inverseFill)
            : RenderStep(Flags::kPerformsShading,
                         /*uniforms=*/{},
                         PrimitiveType::kTriangles,
                         cover_settings(inverseFill),
                         /*vertexAttrs=*/{{"position", VertexAttribType::kFloat3, SLType::kFloat3}},
                         /*instanceAttrs=*/{})
            , fInverseFill(inverseFill) {}

    ~FillBoundsRenderStep() override {}

    const char* name() const override { return "fill-bounds"; }

    const char* vertexSkSL() const override {
        return "     float4 devPosition = float4(position.xy, 0.0, position.z);\n";
    }

    void writeVertices(DrawWriter* writer,
                       const SkIRect& bounds,
                       const Transform& localToDevice,
                       const Shape& shape) const override {
        SkV4 devPoints[4]; // ordered TL, TR, BR, BL

        if (fInverseFill) {
            // TODO: When we handle local coords, we'd need to map these corners by the inverse.
            devPoints[0] = {(float) bounds.fLeft,  (float) bounds.fTop,    0.f, 1.f};
            devPoints[1] = {(float) bounds.fRight, (float) bounds.fTop,    0.f, 1.f};
            devPoints[2] = {(float) bounds.fRight, (float) bounds.fBottom, 0.f, 1.f};
            devPoints[3] = {(float) bounds.fLeft,  (float) bounds.fBottom, 0.f, 1.f};
        } else {
            localToDevice.mapPoints(shape.bounds(), devPoints);
        }

        auto vw = writer->appendVertices(6);
        vw << devPoints[0].x << devPoints[0].y << devPoints[0].w // TL
           << devPoints[3].x << devPoints[3].y << devPoints[3].w // BL
           << devPoints[1].x << devPoints[1].y << devPoints[1].w // TR
           << devPoints[1].x << devPoints[1].y << devPoints[1].w // TR
           << devPoints[3].x << devPoints[3].y << devPoints[3].w // BL
           << devPoints[2].x << devPoints[2].y << devPoints[2].w;// BR
    }

    sk_sp<UniformData> writeUniforms(Layout layout,
                                     const SkIRect&,
                                     const Transform& localToDevice,
                                     const Shape&) const override {
        // Positions are pre-transformed on the CPU so no uniforms needed
        return nullptr;
    }

private:
    const bool fInverseFill;
};

class ConvexRenderStep final : public RenderStep {
public:
    ConvexRenderStep()
            : RenderStep(Flags::kPerformsShading,
                         /*uniforms=*/{},
                         PrimitiveType::kTriangles,
                         DepthStencilSettings{
                            /*frontStencil=*/{},
                            /*backStencil=*/ {},
                            /*refValue=*/    0,
                            /*stencilTest=*/ true,
                            /*depthCompare=*/CompareOp::kAlways, // kGreater once steps know the right depth value
                            /*depthTest=*/   true,
                            /*depthWrite=*/  true
                        },
                         /*vertexAttrs=*/{{"position", VertexAttribType::kFloat3, SLType::kFloat3}},
                         /*instanceAttrs=*/{}) {}

    ~ConvexRenderStep() override {}

    const char* name() const override { return "convex"; }

    const char* vertexSkSL() const override {
        return "     float4 devPosition = float4(position.xy, 0.0, position.z);\n";
    }

    void writeVertices(DrawWriter* writer,
                       const SkIRect& bounds,
                       const Transform& localToDevice,
                       const Shape& shape) const override {
        SkV4 devPoints[4]; // ordered TL, TR, BR, BL

        localToDevice.mapPoints(shape.bounds(), devPoints);

        auto vw = writer->appendVertices(6);
        vw << devPoints[0].x << devPoints[0].y << devPoints[0].w // TL
           << devPoints[3].x << devPoints[3].y << devPoints[3].w // BL
           << devPoints[1].x << devPoints[1].y << devPoints[1].w // TR
           << devPoints[1].x << devPoints[1].y << devPoints[1].w // TR
           << devPoints[3].x << devPoints[3].y << devPoints[3].w // BL
           << devPoints[2].x << devPoints[2].y << devPoints[2].w;// BR
    }

    sk_sp<UniformData> writeUniforms(Layout layout,
                                     const SkIRect&,
                                     const Transform& localToDevice,
                                     const Shape&) const override {
        // Positions are pre-transformed on the CPU so no uniforms needed
        return nullptr;
    }

private:
};

} // anonymous namespace

const Renderer& Renderer::StencilAndFillPath(SkPathFillType fillType) {
    // Because each fill type uses a different stencil settings, there is one Renderer per type.
    // However, at each stage (stencil vs. cover), there are only two RenderSteps to branch on.
    static const StencilFanRenderStep kWindingStencilFan{false};
    static const StencilFanRenderStep kEvenOddStencilFan{true};
    static const StencilCurvesRenderStep kWindingStencilCurves{false};
    static const StencilCurvesRenderStep kEvenOddStencilCurves{true};
    static const FillBoundsRenderStep kFill{false};
    static const FillBoundsRenderStep kInverseFill{true};

    static const Renderer kWindingRenderer{"stencil-and-fill[winding]",
                                           &kWindingStencilFan,
                                           &kWindingStencilCurves,
                                           &kFill};
    static const Renderer kInverseWindingRenderer{"stencil-and-fill[inverse-winding]",
                                                  &kWindingStencilFan,
                                                  &kWindingStencilCurves,
                                                  &kInverseFill};
    static const Renderer kEvenOddRenderer{"stencil-and-fill[evenodd]",
                                           &kEvenOddStencilFan,
                                           &kEvenOddStencilCurves,
                                           &kFill};
    static const Renderer kInverseEvenOddRenderer{"stencil-and-fill[inverse-evenodd]",
                                                  &kEvenOddStencilFan,
                                                  &kEvenOddStencilCurves,
                                                  &kInverseFill};

    switch(fillType) {
        case SkPathFillType::kWinding: return kWindingRenderer;
        case SkPathFillType::kEvenOdd: return kEvenOddRenderer;
        case SkPathFillType::kInverseWinding: return kInverseWindingRenderer;
        case SkPathFillType::kInverseEvenOdd: return kInverseEvenOddRenderer;
    }
    SkUNREACHABLE;
}

const Renderer& Renderer::Convex() {
    static const ConvexRenderStep kConvex{};
    static const Renderer kConvexRenderer{"convex", &kConvex};
    return kConvexRenderer;
}

} // namespace skgpu
