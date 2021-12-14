/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/graphite/src/Renderer.h"

#include "experimental/graphite/src/ContextUtils.h"
#include "experimental/graphite/src/DrawWriter.h"
#include "experimental/graphite/src/UniformManager.h"
#include "experimental/graphite/src/geom/Shape.h"
#include "experimental/graphite/src/geom/Transform_graphite.h"
#include "include/core/SkPathTypes.h"
#include "src/gpu/BufferWriter.h"
#include "src/gpu/tessellate/MiddleOutPolygonTriangulator.h"
#include "src/gpu/tessellate/PathCurveTessellator.h"

namespace skgpu {

namespace {

// TODO: These settings are actually shared by tessellating path renderers, so will be
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
        /*depthCompare=*/CompareOp::kAlways, // kGreater once steps know the right depth value
        /*stencilTest=*/ true,
        /*depthTest=*/   true,
        /*depthWrite=*/  false
    };
    constexpr DepthStencilSettings kEvenOddFill = {
        /*frontStencil=*/kToggle,
        /*backStencil=*/ kToggle,
        /*refValue=*/    0,
        /*depthCompare=*/CompareOp::kAlways, // kGreater once steps know the right depth value
        /*stencilTest=*/ true,
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
    // Resets non-zero bits to 0, passes when not zero
    constexpr DepthStencilSettings::Face kNormal = {
        /*stencilFail=*/   StencilOp::kKeep,
        /*depthFail=*/     StencilOp::kKeep,
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
        /*depthCompare=*/CompareOp::kAlways, // kGreater once steps know the right depth value
        /*stencilTest=*/ true,
        /*depthTest=*/   true,
        /*depthWrite=*/  true
    };
    constexpr DepthStencilSettings kInvertedDSS = {
        /*frontStencil=*/kInverted,
        /*backStencil=*/ kInverted,
        /*refValue=*/    0,
        /*depthCompare=*/CompareOp::kAlways, // kGreater once steps know the right depth value
        /*stencilTest=*/ true,
        /*depthTest=*/   true,
        /*depthWrite=*/  true
    };
    return inverse ? kInvertedDSS : kNormalDSS;
}

class StencilFanRenderStep final : public RenderStep {
public:
    StencilFanRenderStep(bool evenOdd)
            : RenderStep(Flags::kRequiresMSAA,
                         /*uniforms=*/{{"localToDevice", SLType::kFloat4x4}},
                         PrimitiveType::kTriangles,
                         fillrule_settings(evenOdd),
                         /*vertexAttrs=*/{{"position", VertexAttribType::kFloat2, SLType::kFloat2}},
                         /*instanceAttrs=*/{}) {}

    ~StencilFanRenderStep() override {}

    const char* name() const override { return "stencil-fan"; }

    const char* vertexSkSL() const override {
        return "     float4 devPosition = localToDevice * float4(position, 0.0, 1.0);\n";
    }

    void writeVertices(DrawWriter* writer, const Transform&, const Shape& shape) const override {
        // TODO: Have Shape provide a path-like iterator so we don't actually have to convert non
        // paths to SkPath just to iterate their pts/verbs
        SkPath path = shape.asPath();
        for (PathMiddleOutFanIter it(path); !it.done();) {
            for (auto [p0, p1, p2] : it.nextStack()) {
                // TODO: Instead do one appendVertices(maxTrianglsInFans*3) and then return vertices at the end
                auto vw = writer->appendVertices(3);
                vw << p0 << p1 << p2;
            }
        }
    }

    sk_sp<UniformData> writeUniforms(Layout layout,
                                     const Transform& localToDevice,
                                     const Shape&) const override {
        // TODO: Given that a RenderStep has its own uniform binding slot, these offsets never
        // change so we could cache them per layout.
        UniformManager mgr(layout);
        size_t dataSize = mgr.writeUniforms(this->uniforms(), nullptr, nullptr, nullptr);
        sk_sp<UniformData> transformData = UniformData::Make((int) this->numUniforms(),
                                                             this->uniforms().data(),
                                                             dataSize);

        const void* transform[1] = {&localToDevice.matrix()};
        mgr.writeUniforms(this->uniforms(),
                          transform,
                          transformData->offsets(),
                          transformData->data());
        return transformData;
    }
};

class StencilCurvesRenderStep final : public RenderStep {
public:
    StencilCurvesRenderStep(bool evenOdd)
            : RenderStep(Flags::kRequiresMSAA,
                         /*uniforms=*/{{"localToDevice", SLType::kFloat4x4}},
                         PrimitiveType::kTriangles,
                         fillrule_settings(evenOdd),
                         /*vertexAttrs=*/{{"position", VertexAttribType::kFloat2, SLType::kFloat2}},
                         /*instanceAttrs=*/{}) {}

    ~StencilCurvesRenderStep() override {}

    const char* name() const override { return "stencil-curves"; }

    const char* vertexSkSL() const override {
        const char* GrPathTessellationShader::Impl::kEvalRationalCubicFn = R"(
float3 safe_mix(float3 a, float3 b, float T, float one_minus_T) {
    return a*one_minus_T + b*T;
}
float2 eval_rational_cubic(float4x3 P, float T) {
    float one_minus_T = 1.0 - T;
    float3 ab = safe_mix(P[0], P[1], T, one_minus_T);
    float3 bc = safe_mix(P[1], P[2], T, one_minus_T);
    float3 cd = safe_mix(P[2], P[3], T, one_minus_T);
    float3 abc = safe_mix(ab, bc, T, one_minus_T);
    float3 bcd = safe_mix(bc, cd, T, one_minus_T);
    float3 abcd = safe_mix(abc, bcd, T, one_minus_T);
    return abcd.xy / abcd.z;
})";

            const auto& shader = args.fGeomProc.cast<GrPathTessellationShader>();
    args.fVaryingHandler->emitAttributes(shader);

    // Vertex shader.
    const char* affineMatrix, *translate;
    fAffineMatrixUniform = args.fUniformHandler->addUniform(nullptr, kVertex_GrShaderFlag,
                                                            kFloat4_GrSLType, "affineMatrix",
                                                            &affineMatrix);
    fTranslateUniform = args.fUniformHandler->addUniform(nullptr, kVertex_GrShaderFlag,
                                                         kFloat2_GrSLType, "translate", &translate);
    args.fVertBuilder->codeAppendf("float2x2 AFFINE_MATRIX = float2x2(%s);", affineMatrix);
    args.fVertBuilder->codeAppendf("float2 TRANSLATE = %s;", translate);
    this->emitVertexCode(*args.fShaderCaps,
                         shader,
                         args.fVertBuilder,
                         args.fVaryingHandler,
                         gpArgs);

    // Fragment shader.
    if (!(shader.fAttribs & PatchAttribs::kColor)) {
        const char* color;
        fColorUniform = args.fUniformHandler->addUniform(nullptr, kFragment_GrShaderFlag,
                                                         kHalf4_GrSLType, "color", &color);
        args.fFragBuilder->codeAppendf("half4 %s = %s;", args.fOutputColor, color);
    } else {
        args.fFragBuilder->codeAppendf("half4 %s = %s;",
                                       args.fOutputColor, fVaryingColorName.c_str());
    }
    args.fFragBuilder->codeAppendf("const half4 %s = half4(1);", args.fOutputCoverage);

        const MiddleOutShader& middleOutShader = shader.cast<MiddleOutShader>();
            v->defineConstant("PRECISION", skgpu::kTessellationPrecision);
            v->defineConstant("MAX_FIXED_RESOLVE_LEVEL",
                              (float)skgpu::PathTessellator::kMaxFixedResolveLevel);
            v->defineConstant("MAX_FIXED_SEGMENTS",
                              (float)(1 << skgpu::PathTessellator::kMaxFixedResolveLevel));
            v->insertFunction(skgpu::wangs_formula::as_sksl().c_str());
            if (middleOutShader.fAttribs & PatchAttribs::kExplicitCurveType) {
                v->insertFunction(SkStringPrintf(R"(
                bool is_conic_curve() {
                    return curveType != %g;
                })", skgpu::kCubicCurveType).c_str());
                v->insertFunction(SkStringPrintf(R"(
                bool is_triangular_conic_curve() {
                    return curveType == %g;
                })", skgpu::kTriangularConicCurveType).c_str());
            } else {
                SkASSERT(shaderCaps.infinitySupport());
                v->insertFunction(R"(
                bool is_conic_curve() { return isinf(p23.w); }
                bool is_triangular_conic_curve() { return isinf(p23.z); })");
            }
            if (shaderCaps.bitManipulationSupport()) {
                v->insertFunction(R"(
                float ldexp_portable(float x, float p) {
                    return ldexp(x, int(p));
                })");
            } else {
                v->insertFunction(R"(
                float ldexp_portable(float x, float p) {
                    return x * exp2(p);
                })");
            }
            v->codeAppend(R"(
            float resolveLevel = resolveLevel_and_idx.x;
            float idxInResolveLevel = resolveLevel_and_idx.y;
            float2 localcoord;)");
            if (middleOutShader.fAttribs & PatchAttribs::kFanPoint) {
                v->codeAppend(R"(
                // A negative resolve level means this is the fan point.
                if (resolveLevel < 0) {
                    localcoord = fanPointAttrib;
                } else)");  // Fall through to next if ().
            }
            v->codeAppend(R"(
            if (is_triangular_conic_curve()) {
                // This patch is an exact triangle.
                localcoord = (resolveLevel != 0)      ? p01.zw
                           : (idxInResolveLevel != 0) ? p23.xy
                                                      : p01.xy;
            } else {
                float2 p0=p01.xy, p1=p01.zw, p2=p23.xy, p3=p23.zw;
                float w = -1;  // w < 0 tells us to treat the instance as an integral cubic.
                float maxResolveLevel;
                if (is_conic_curve()) {
                    // Conics are 3 points, with the weight in p3.
                    w = p3.x;
                    maxResolveLevel = wangs_formula_conic_log2(PRECISION, AFFINE_MATRIX * p0,
                                                                          AFFINE_MATRIX * p1,
                                                                          AFFINE_MATRIX * p2, w);
                    p1 *= w;  // Unproject p1.
                    p3 = p2;  // Duplicate the endpoint for shared code that also runs on cubics.
                } else {
                    // The patch is an integral cubic.
                    maxResolveLevel = wangs_formula_cubic_log2(PRECISION, p0, p1, p2, p3,
                                                               AFFINE_MATRIX);
                }
                if (resolveLevel > maxResolveLevel) {
                    // This vertex is at a higher resolve level than we need. Demote to a lower
                    // resolveLevel, which will produce a degenerate triangle.
                    idxInResolveLevel = floor(ldexp_portable(idxInResolveLevel,
                                                             maxResolveLevel - resolveLevel));
                    resolveLevel = maxResolveLevel;
                }
                // Promote our location to a discrete position in the maximum fixed resolve level.
                // This is extra paranoia to ensure we get the exact same fp32 coordinates for
                // colocated points from different resolve levels (e.g., the vertices T=3/4 and
                // T=6/8 should be exactly colocated).
                float fixedVertexID = floor(.5 + ldexp_portable(
                        idxInResolveLevel, MAX_FIXED_RESOLVE_LEVEL - resolveLevel));
                if (0 < fixedVertexID && fixedVertexID < MAX_FIXED_SEGMENTS) {
                    float T = fixedVertexID * (1 / MAX_FIXED_SEGMENTS);

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
            float2 vertexpos = AFFINE_MATRIX * localcoord + TRANSLATE;)");
            gpArgs->fLocalCoordVar.set(kFloat2_GrSLType, "localcoord");
            gpArgs->fPositionVar.set(kFloat2_GrSLType, "vertexpos");
            if (middleOutShader.fAttribs & PatchAttribs::kColor) {
                GrGLSLVarying colorVarying(GrSLType::kHalf4_GrSLType);
                varyingHandler->addVarying("color",
                                           &colorVarying,
                                           GrGLSLVaryingHandler::Interpolation::kCanBeFlat);
                v->codeAppendf("%s = colorAttrib;", colorVarying.vsOut());
                fVaryingColorName = colorVarying.fsIn();
            }
        return "     float4 devPosition = localToDevice * float4(position, 0.0, 1.0);\n";
    }

    void writeVertices(DrawWriter* writer, const Transform&, const Shape& shape) const override {
        int maxTessellationSegments = 32; // TODO caps check
        PathCurveTessellator tessellator(/*infinitySupport=*/true); // TODO caps check

        SkPath path = shape.asPath(); // Use a path-like iterator directly to avoid path malloc
        // TODO: Check path bounds against max tessellation count and run through PathChopper to get
        // a pre-chopped path so that PatchWriter doesn't assert
        // TODO: Is this the right place for that? hmm, I don't think so, since it would mean that
        // the path used by the middle-out step is different. The pre-chopping needs to happen in
        // DrawList before it records the path.
        PathDrawList list{path, SkMatrix::I(), {1.f, 1.f, 1.f, 1.f}}; // FIXME actually pack color

        int preallocCount = tessellator.patchPreallocCount(path.countVerbs());

        tessellator.prepareFixedCountBuffers(target); // FIXME actually lookup and/or create vertex and index templates
        // FIXME if we tweak the index count for the template based on the resolve level, that needs
        // to be set before we start appending instance data since we need the template. However, if
        // we need to use the patch writer or call writePatches to determine that resolve level,
        // it's a chicken and egg problem.
        // V1 gets around it because we accumulate everything up front into a chunk array and then
        // can choose how to execute all draw calls at the end. In this case I'd need to know the
        // resolve level first so that all auto draws are appropriate.
        // writer->setFixedInstanceTemplate(vertexBuffer, indexBuffer, indexCountForShape);

        int fixedIndexCount = NumCurveTrianglesAtResolveLevel(fFixedResolveLevel) * 3;

        // FIXME we need PatchWriter to operate on DrawWriter or VertexWriter directly instead of
        // trying to use the chunk array system. Although the chunk array system might provide a way
        // to get around the index count, since we could have it collect vertex arrays from the
        // draw writer / DBM and then iterate over them at the end... but that won't auto-merge like
        // we want to be able to since we aren't using the path list feature
        PatchWriter patchWriter(target, &tessellator, patchPreallocCount);
        tessellator.writePatches(patchWriter, maxTessellationSegments, SkMatrix::I(), &list);
        // TODO: PatchWriter should get routed to the DrawWriter, and that's that...

        // TODO: reclaim vertex data that was over allocated by preallocCount and wasn't used
    }

    sk_sp<UniformData> writeUniforms(Layout layout,
                                     const Transform& localToDevice,
                                     const Shape&) const override {
        // TODO: Given that a RenderStep has its own uniform binding slot, these offsets never
        // change so we could cache them per layout.
        UniformManager mgr(layout);
        size_t dataSize = mgr.writeUniforms(this->uniforms(), nullptr, nullptr, nullptr);
        sk_sp<UniformData> transformData = UniformData::Make((int) this->numUniforms(),
                                                             this->uniforms().data(),
                                                             dataSize);

        const void* transform[1] = {&localToDevice.matrix()};
        mgr.writeUniforms(this->uniforms(),
                          transform,
                          transformData->offsets(),
                          transformData->data());
        return transformData;
    }
};

class FillBoundsRenderStep final : public RenderStep {
public:
    FillBoundsRenderStep(bool inverse)
            : RenderStep(Flags::kPerformsShading,
                         /*uniforms=*/{},
                         PrimitiveType::kTriangles,
                          cover_settings(inverse),
                         /*vertexAttrs=*/{{"position", VertexAttribType::kFloat4, SLType::kFloat4}},
                         /*instanceAttrs=*/{}) {}

    ~FillBoundsRenderStep() override {}

    const char* name() const override { return "fill-bounds"; }

    const char* vertexSkSL() const override {
        return "    float4 devPosition = position;\n";//float4(position.xy, 0.0, position.z);\n";
    }

    void writeVertices(DrawWriter* writer,
                       const Transform& localToDevice,
                       const Shape& shape) const override {
        // TODO: Must outset by 1/4px to ensure we fully cover any pixels touched by earlier steps
        // so that the stencil buffer is reset.
        // TODO: Must cover render target bounds instead of shape bounds if it's inverse filled.

        //

        // Transform shape.bounds() by local2Device
        // TODO: optimize this given rect input and const z = 0, w = 1?
        Rect bounds = shape.bounds();
        SkV4 p0 = localToDevice.matrix() * SkV4{bounds.left(), bounds.top(), 0.f, 1.f};
        SkV4 p1 = localToDevice.matrix() * SkV4{bounds.left(), bounds.bot(), 0.f, 1.f};
        SkV4 p2 = localToDevice.matrix() * SkV4{bounds.right(), bounds.top(), 0.f, 1.f};
        SkV4 p3 = localToDevice.matrix() * SkV4{bounds.right(), bounds.bot(), 0.f, 1.f};

        auto vw = writer->appendVertices(6);
        vw << p0 << p1 << p2
           << p2 << p1 << p3;
        // TODO: need to handle local coords as well...
    }

    sk_sp<UniformData> writeUniforms(Layout layout, const Transform&, const Shape&) const override {
        // The fill-bounds step does not need any uniforms, positions are pre-transformed.
        return nullptr;
    }
};

} // anonymous namespace

const Renderer& Renderer::StencilAndFillPath(SkPathFillType fillType) {
    // Because each fill type uses a different stencil settings, there is one Renderer per type.
    // However, at each stage (stencil vs. cover), there are only two RenderSteps to branch on.
    static const StencilFanRenderStep kWindingStencilFan{false};
    static const StencilFanRenderStep kEvenOddStencilFan{true};
    static const FillBoundsRenderStep kFill{false};
    static const FillBoundsRenderStep kInverseFill{true};

    // TODO: Uncomment and include the curve stenciling steps to draw curved paths
    static const Renderer kWindingRenderer{"stencil-and-fill[winding]",
                                           &kWindingStencilFan,
                                           /*&kWindingStencilCurves,*/
                                           &kFill};
    static const Renderer kInverseWindingRenderer{"stencil-and-fill[inverse-winding]",
                                                  &kWindingStencilFan,
                                                  /*&kWindingStencilCurves,*/
                                                  &kInverseFill};
    static const Renderer kEvenOddRenderer{"stencil-and-fill[evenodd]",
                                           &kEvenOddStencilFan,
                                           /*&kEvenOddStencilCurves,*/
                                           &kFill};
    static const Renderer kInverseEvenOddRenderer{"stencil-and-fill[inverse-evenodd]",
                                                  &kEvenOddStencilFan,
                                                  /*&kEvenOddStencilCurves,*/
                                                  &kInverseFill};

    switch(fillType) {
        case SkPathFillType::kWinding: return kWindingRenderer;
        case SkPathFillType::kEvenOdd: return kEvenOddRenderer;
        case SkPathFillType::kInverseWinding: return kInverseWindingRenderer;
        case SkPathFillType::kInverseEvenOdd: return kInverseEvenOddRenderer;
    }
    SkUNREACHABLE;
}

} // namespace skgpu
