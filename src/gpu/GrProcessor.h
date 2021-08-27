/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrProcessor_DEFINED
#define GrProcessor_DEFINED

#include "include/core/SkMath.h"
#include "include/core/SkSpan.h"
#include "include/core/SkString.h"
#include "src/gpu/GrColor.h"
#include "src/gpu/GrGpuBuffer.h"
#include "src/gpu/GrProcessorUnitTest.h"
#include "src/gpu/GrProgramDesc.h"
#include "src/gpu/GrSamplerState.h"
#include "src/gpu/GrShaderVar.h"
#include "src/gpu/GrSurfaceProxyPriv.h"
#include "src/gpu/GrTextureProxy.h"

class GrResourceProvider;

/** Provides custom shader code to the Ganesh shading pipeline. GrProcessor objects *must* be
    immutable: after being constructed, their fields may not change.

    Dynamically allocated GrProcessors are managed by a per-thread memory pool. The ref count of an
    processor must reach 0 before the thread terminates and the pool is destroyed.
 */
class GrProcessor {
public:
    enum ClassID {
        kNull_ClassID,  // Reserved ID for missing (null) processors

        kBigKeyProcessor_ClassID,
        kBlendFragmentProcessor_ClassID,
        kBlockInputFragmentProcessor_ClassID,
        kButtCapStrokedCircleGeometryProcessor_ClassID,
        kCircleGeometryProcessor_ClassID,
        kCircularRRectEffect_ClassID,
        kClockwiseTestProcessor_ClassID,
        kColorTableEffect_ClassID,
        kCoverageSetOpXP_ClassID,
        kCustomXP_ClassID,
        kDashingCircleEffect_ClassID,
        kDashingLineEffect_ClassID,
        kDefaultGeoProc_ClassID,
        kDeviceSpace_ClassID,
        kDIEllipseGeometryProcessor_ClassID,
        kDisableColorXP_ClassID,
        kDrawAtlasPathShader_ClassID,
        kEllipseGeometryProcessor_ClassID,
        kEllipticalRRectEffect_ClassID,
        kFwidthSquircleTestProcessor_ClassID,
        kGP_ClassID,
        kGrBicubicEffect_ClassID,
        kGrBitmapTextGeoProc_ClassID,
        kGrColorSpaceXformEffect_ClassID,
        kGrConicEffect_ClassID,
        kGrConvexPolyEffect_ClassID,
        kGrDiffuseLightingEffect_ClassID,
        kGrDisplacementMapEffect_ClassID,
        kGrDistanceFieldA8TextGeoProc_ClassID,
        kGrDistanceFieldLCDTextGeoProc_ClassID,
        kGrDistanceFieldPathGeoProc_ClassID,
        kGrDSLFPTest_DoStatement_ClassID,
        kGrDSLFPTest_ForStatement_ClassID,
        kGrDSLFPTest_IfStatement_ClassID,
        kGrDSLFPTest_SwitchStatement_ClassID,
        kGrDSLFPTest_Swizzle_ClassID,
        kGrDSLFPTest_Ternary_ClassID,
        kGrDSLFPTest_WhileStatement_ClassID,
        kGrFillRRectOp_Processor_ClassID,
        kGrGaussianConvolutionFragmentProcessor_ClassID,
        kGrMatrixConvolutionEffect_ClassID,
        kGrMatrixEffect_ClassID,
        kGrMeshTestProcessor_ClassID,
        kGrMorphologyEffect_ClassID,
        kGrPerlinNoise2Effect_ClassID,
        kGrPipelineDynamicStateTestProcessor_ClassID,
        kGrQuadEffect_ClassID,
        kGrRRectShadowGeoProc_ClassID,
        kGrSkSLFP_ClassID,
        kGrSpecularLightingEffect_ClassID,
        kGrTextureEffect_ClassID,
        kGrUnrolledBinaryGradientColorizer_ClassID,
        kGrYUVtoRGBEffect_ClassID,
        kHighPrecisionFragmentProcessor_ClassID,
        kLatticeGP_ClassID,
        kPDLCDXferProcessor_ClassID,
        kPorterDuffXferProcessor_ClassID,
        kPremulFragmentProcessor_ClassID,
        kQuadEdgeEffect_ClassID,
        kQuadPerEdgeAAGeometryProcessor_ClassID,
        kSeriesFragmentProcessor_ClassID,
        kShaderPDXferProcessor_ClassID,
        kSurfaceColorProcessor_ClassID,
        kSwizzleFragmentProcessor_ClassID,
        kTessellate_BoundingBoxShader_ClassID,
        kTessellate_GrModulateAtlasCoverageEffect_ClassID,
        kTessellate_GrStrokeTessellationShader_ClassID,
        kTessellate_HardwareCurveShader_ClassID,
        kTessellate_HardwareWedgeShader_ClassID,
        kTessellate_HullShader_ClassID,
        kTessellate_MiddleOutShader_ClassID,
        kTessellate_SimpleTriangleShader_ClassID,
        kTessellationTestTriShader_ClassID,
        kTestFP_ClassID,
        kTestRectOp_ClassID,
        kVertexColorSpaceBenchGP_ClassID,
        kVerticesGP_ClassID,
    };

    virtual ~GrProcessor() = default;

    /** Human-meaningful string to identify this processor; may be embedded in generated shader
        code and must be a legal SkSL identifier prefix. */
    virtual const char* name() const = 0;

    /** Human-readable dump of all information */
#if GR_TEST_UTILS
    virtual SkString onDumpInfo() const { return SkString(); }

    SkString dumpInfo() const {
        SkString info(name());
        info.append(this->onDumpInfo());
        return info;
    }
#endif

    void* operator new(size_t size);
    void* operator new(size_t object_size, size_t footer_size);
    void operator delete(void* target);

    void* operator new(size_t size, void* placement) {
        return ::operator new(size, placement);
    }
    void operator delete(void* target, void* placement) {
        ::operator delete(target, placement);
    }

    /** Helper for down-casting to a GrProcessor subclass */
    template <typename T> const T& cast() const { return *static_cast<const T*>(this); }

    ClassID classID() const { return fClassID; }

    /** Describes a uniform. */
    class Uniform {
    public:
        enum class CType : unsigned {
            // Any float/half, vector of floats/half, or matrices of floats/halfs are a tightly
            // packed array of floats. Similarly, any bool/shorts/ints are tightly packed array
            // of int32_t.
            kDefault,
            // Can be used with kFloat3x3 or kHalf3x3
            kSkMatrix,

            kLast = kSkMatrix
        };
        static constexpr int kCTypeCount = static_cast<int>(CType::kLast) + 1;

        constexpr Uniform() :
            fType      (static_cast<unsigned>(kVoid_GrSLType)),
            fCount     (static_cast<unsigned>(GrShaderVar::kNonArray)),
            fVisibility(static_cast<unsigned>(GrShaderFlags::kNone_GrShaderFlags)),
            fCType     (static_cast<unsigned>(CType::kDefault)),
            fOffset    (0) {}

        constexpr Uniform(const char*   name,
                          GrSLType      type,
                          ptrdiff_t     offset,
                          GrShaderFlags visibility = kFragment_GrShaderFlag,
                          CType         ctype      = CType::kDefault) :
                Uniform(name, type, GrShaderVar::kNonArray, offset, visibility, ctype) {}

        constexpr Uniform(const char*   name,
                          GrSLType      type,
                          int           arrayCount,
                          size_t        offset,
                          GrShaderFlags visibility = kFragment_GrShaderFlag,
                          CType         ctype      = CType::kDefault)
            : fName      (name)
            , fType      (static_cast<unsigned>(type      ))
            , fCount     (static_cast<unsigned>(arrayCount))
            , fVisibility(static_cast<unsigned>(visibility))
            , fCType     (static_cast<unsigned>(ctype     ))
            , fOffset    (static_cast<unsigned>(offset    )) {
            SkASSERT(CTypeCompatibleWithType(ctype, type));

            SkASSERT(this->type()       == type      );
            SkASSERT(this->count()      == arrayCount);
            SkASSERT(this->offset()     == offset    );
            SkASSERT(this->visibility() == visibility);
            SkASSERT(this->ctype()      == ctype     );
        }

        constexpr Uniform(const Uniform&) = default;

        Uniform& operator=(const Uniform&) = default;

        constexpr bool isInitialized() const { return this->type() != kVoid_GrSLType; }

        constexpr const char*   name      () const { return fName; }
        constexpr GrSLType      type      () const { return static_cast<GrSLType>     (fType);       }
        constexpr int           count     () const { return static_cast<int>          (fCount);      }
        constexpr GrShaderFlags visibility() const { return static_cast<GrShaderFlags>(fVisibility); }
        constexpr CType         ctype     () const { return static_cast<CType>        (fCType);      }
        constexpr size_t        offset    () const { return static_cast<GrShaderFlags>(fOffset);     }


        GrShaderVar asShaderVar() const {
            return {SkString(fName),
                    this->type(),
                    GrShaderVar::TypeModifier::Uniform,
                    this->count()};
        }

        static constexpr bool CTypeCompatibleWithType(CType, GrSLType);

    private:
        const char* fName       = nullptr;
        unsigned    fType       : 6;
        unsigned    fCount      : 8;
        unsigned    fVisibility : 5;
        unsigned    fCType      : 1;
        unsigned    fOffset     : 32 - (6 + 5 + 8 + 1);

        static_assert(kGrSLTypeCount <= (1 << 6));
    };

    /** Returns the array of uniforms inserted into the program by this processor. */
    SkSpan<const Uniform> uniforms() const { return fUniforms; }

protected:
    GrProcessor(ClassID classID) : fClassID(classID) {}
    GrProcessor(const GrProcessor&) = delete;
    GrProcessor& operator=(const GrProcessor&) = delete;

    /**
     * Specifies the uniforms used by this processor. Should be called when the processor is made
     * (i.e. constructor or factory function). Any uniforms with type void are ignored. This allows
     * a processor to have a contiguous array of data member uniforms where some are conditionally
     * initialized.
     */
    void setUniforms(SkSpan<const Uniform> uniforms) { fUniforms = uniforms; }

    const ClassID fClassID;

private:
    SkSpan<const Uniform> fUniforms;
};

constexpr bool GrProcessor::Uniform::CTypeCompatibleWithType(CType ctype, GrSLType type) {
    switch (ctype) {
        case CType::kDefault:
            return true;
        case CType::kSkMatrix:
            return type == kHalf3x3_GrSLType || type == kFloat3x3_GrSLType;
    }
    SkUNREACHABLE;
}

#endif
