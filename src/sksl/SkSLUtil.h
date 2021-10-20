/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_UTIL
#define SKSL_UTIL

#include <cstdarg>
#include <memory>
#include "stdlib.h"
#include "string.h"
#include "include/private/SkSLDefines.h"
#include "src/sksl/SkSLLexer.h"

#ifndef SKSL_STANDALONE
#include "include/core/SkTypes.h"
#include "include/private/GrTypesPriv.h"
#if SK_SUPPORT_GPU
#include "include/gpu/GrContextOptions.h"
#endif // SK_SUPPORT_GPU
#endif // SKSL_STANDALONE

//struct GrContextOptions;
class SkJSONWriter;

namespace SkSL {

class Context;
class OutputStream;
class StringStream;
class Type;

// Limited set of GLSL versions we build shaders for. Clients should round down the GLSL version to
// one of these enums.
enum class GLSLGeneration {
    /**
     * Desktop GLSL 1.10 and ES2 shading language (based on desktop GLSL 1.20)
     */
    k110,
    /**
     * Desktop GLSL 1.30
     */
    k130,
    /**
     * Desktop GLSL 1.40
     */
    k140,
    /**
     * Desktop GLSL 1.50
     */
    k150,
    /**
     * Desktop GLSL 3.30, and ES GLSL 3.00
     */
    k330,
    /**
     * Desktop GLSL 4.00
     */
    k400,
    /**
     * Desktop GLSL 4.20
     */
    k420,
    /**
     * ES GLSL 3.10 only TODO Make GLSLCap objects to make this more granular
     */
    k310es,
    /**
     * ES GLSL 3.20
     */
    k320es,
};

}  // namespace SkSL

// we're being compiled standalone, so we don't have access to caps...
class StandaloneShaderCaps {
public:
    SkSL::GLSLGeneration fGLSLGeneration = SkSL::GLSLGeneration::k400;
    SkSL::GLSLGeneration generation() const {
        return fGLSLGeneration;
    }

    bool fAtan2ImplementedAsAtanYOverX = false;
    bool atan2ImplementedAsAtanYOverX() const {
        return fAtan2ImplementedAsAtanYOverX;
    }

    bool fCanUseMinAndAbsTogether = true;
    bool canUseMinAndAbsTogether() const {
        return fCanUseMinAndAbsTogether;
    }

    bool fMustForceNegatedAtanParamToFloat = false;
    bool mustForceNegatedAtanParamToFloat() const {
        return fMustForceNegatedAtanParamToFloat;
    }

    bool fMustForceNegatedLdexpParamToMultiply = false;
    bool mustForceNegatedLdexpParamToMultiply() const {
        return fMustForceNegatedLdexpParamToMultiply;
    }

    bool fShaderDerivativeSupport = true;
    bool shaderDerivativeSupport() const {
        return fShaderDerivativeSupport;
    }

    bool fUsesPrecisionModifiers = false;
    bool usesPrecisionModifiers() const {
        return fUsesPrecisionModifiers;
    }

    bool mustDeclareFragmentShaderOutput() const {
        return fGLSLGeneration > SkSL::GLSLGeneration::k110;
    }

    bool fFBFetchSupport = false;
    bool fbFetchSupport() const {
        return fFBFetchSupport;
    }

    bool fFBFetchNeedsCustomOutput = false;
    bool fbFetchNeedsCustomOutput() const {
        return fFBFetchNeedsCustomOutput;
    }

    bool fFlatInterpolationSupport = true;
    bool flatInterpolationSupport() const {
        return fFlatInterpolationSupport;
    }

    bool fNoperspectiveInterpolationSupport = true;
    bool noperspectiveInterpolationSupport() const {
        return fNoperspectiveInterpolationSupport;
    }

    bool fMultisampleInterpolationSupport = true;
    bool multisampleInterpolationSupport() const {
        return fMultisampleInterpolationSupport;
    }

    bool fSampleMaskSupport = true;
    bool sampleMaskSupport() const {
        return fSampleMaskSupport;
    }

    bool fExternalTextureSupport = true;
    bool externalTextureSupport() const {
        return fExternalTextureSupport;
    }

    bool fMustDoOpBetweenFloorAndAbs = false;
    bool mustDoOpBetweenFloorAndAbs() const {
        return fMustDoOpBetweenFloorAndAbs;
    }

    bool fMustGuardDivisionEvenAfterExplicitZeroCheck = false;
    bool mustGuardDivisionEvenAfterExplicitZeroCheck() const {
        return fMustGuardDivisionEvenAfterExplicitZeroCheck;
    }

    bool fMustEnableAdvBlendEqs = false;
    bool mustEnableAdvBlendEqs() const {
        return fMustEnableAdvBlendEqs;
    }

    bool fCanUseAnyFunctionInShader = true;
    bool canUseAnyFunctionInShader() const {
        return fCanUseAnyFunctionInShader;
    }

    bool fNoDefaultPrecisionForExternalSamplers = false;
    bool noDefaultPrecisionForExternalSamplers() const {
        return fNoDefaultPrecisionForExternalSamplers;
    }

    bool fFloatIs32Bits = true;
    bool floatIs32Bits() const {
        return fFloatIs32Bits;
    }

    bool fIntegerSupport = false;
    bool integerSupport() const {
        return fIntegerSupport;
    }

    bool fNonsquareMatrixSupport = false;
    bool nonsquareMatrixSupport() const {
        return fNonsquareMatrixSupport;
    }

    bool fInverseHyperbolicSupport = false;
    bool inverseHyperbolicSupport() const {
        return fInverseHyperbolicSupport;
    }

    bool fBuiltinFMASupport = false;
    bool builtinFMASupport() const {
        return fBuiltinFMASupport;
    }

    bool fBuiltinDeterminantSupport = false;
    bool builtinDeterminantSupport() const {
        return fBuiltinDeterminantSupport;
    }

    bool fCanUseDoLoops = false;
    bool canUseDoLoops() const {
        // we define this to false in standalone so we don't use do loops while inlining in FP files
        // (which would then, being baked in, end up being used even in contexts where do loops are
        // not allowed)
        return fCanUseDoLoops;
    }

    bool fUseNodePools = true;
    bool useNodePools() const {
        return fUseNodePools;
    }

    const char* fShaderDerivativeExtensionString = nullptr;
    const char* shaderDerivativeExtensionString() const {
        return fShaderDerivativeExtensionString;
    }

    const char* fFragCoordConventionsExtensionString = nullptr;
    const char* fragCoordConventionsExtensionString() const {
        return fFragCoordConventionsExtensionString;
    }

    const char* fExternalTextureExtensionString = nullptr;
    const char* externalTextureExtensionString() const {
        return fExternalTextureExtensionString;
    }

    const char* fSecondExternalTextureExtensionString = nullptr;
    const char* secondExternalTextureExtensionString() const {
        return fSecondExternalTextureExtensionString;
    }

    const char* fVersionDeclString = "";
    const char* versionDeclString() const {
        return fVersionDeclString;
    }

    bool fCanUseFractForNegativeValues = true;
    bool canUseFractForNegativeValues() const {
        return fCanUseFractForNegativeValues;
    }

    bool fCanUseFragCoord = true;
    bool canUseFragCoord() const {
        return fCanUseFragCoord;
    }

    bool fIncompleteShortIntPrecision = false;
    bool incompleteShortIntPrecision() const {
        return fIncompleteShortIntPrecision;
    }

    bool fAddAndTrueToLoopCondition = false;
    bool addAndTrueToLoopCondition() const {
        return fAddAndTrueToLoopCondition;
    }

    bool fUnfoldShortCircuitAsTernary = false;
    bool unfoldShortCircuitAsTernary() const {
        return fUnfoldShortCircuitAsTernary;
    }

    bool fEmulateAbsIntFunction = false;
    bool emulateAbsIntFunction() const {
        return fEmulateAbsIntFunction;
    }

    bool fRewriteDoWhileLoops = false;
    bool rewriteDoWhileLoops() const {
        return fRewriteDoWhileLoops;
    }

    bool fRewriteSwitchStatements = false;
    bool rewriteSwitchStatements() const {
        return fRewriteSwitchStatements;
    }

    bool fRemovePowWithConstantExponent = false;
    bool removePowWithConstantExponent() const {
        return fRemovePowWithConstantExponent;
    }

    const char* fFBFetchColorName = nullptr;
    const char* fbFetchColorName() const {
        return fFBFetchColorName;
    }

    bool fRewriteMatrixVectorMultiply = false;
    bool rewriteMatrixVectorMultiply() const {
        return fRewriteMatrixVectorMultiply;
    }

    bool fRewriteMatrixComparisons = false;
    bool rewriteMatrixComparisons() const {
        return fRewriteMatrixComparisons;
    }
};

namespace SkSL {
class ShaderCapsFactory;
class SharedCompiler;
}

#if !defined(SKSL_STANDALONE) && SK_SUPPORT_GPU

class GrShaderCaps : public SkRefCnt {
public:
    /**
     * Indicates how GLSL must interact with advanced blend equations. The KHR extension requires
     * special layout qualifiers in the fragment shader.
     */
    enum AdvBlendEqInteraction {
        kNotSupported_AdvBlendEqInteraction,     //<! No _blend_equation_advanced extension
        kAutomatic_AdvBlendEqInteraction,        //<! No interaction required
        kGeneralEnable_AdvBlendEqInteraction,    //<! layout(blend_support_all_equations) out

        kLast_AdvBlendEqInteraction = kGeneralEnable_AdvBlendEqInteraction
    };

    GrShaderCaps();

    void dumpJSON(SkJSONWriter*) const;

    bool supportsDistanceFieldText() const { return fShaderDerivativeSupport; }

    bool shaderDerivativeSupport() const { return fShaderDerivativeSupport; }
    bool dstReadInShaderSupport() const { return fDstReadInShaderSupport; }
    bool dualSourceBlendingSupport() const { return fDualSourceBlendingSupport; }
    bool nonsquareMatrixSupport() const { return fNonsquareMatrixSupport; }

    /** Indicates true 32-bit integer support, with unsigned types and bitwise operations */
    bool integerSupport() const { return fIntegerSupport; }

    /** asinh(), acosh(), atanh() */
    bool inverseHyperbolicSupport() const { return fInverseHyperbolicSupport; }

    /**
     * Some helper functions for encapsulating various extensions to read FB Buffer on openglES
     *
     * TODO(joshualitt) On desktop opengl 4.2+ we can achieve something similar to this effect
     */
    bool fbFetchSupport() const { return fFBFetchSupport; }

    bool fbFetchNeedsCustomOutput() const { return fFBFetchNeedsCustomOutput; }

    const char* versionDeclString() const { return fVersionDeclString; }

    const char* fbFetchColorName() const { return fFBFetchColorName; }

    const char* fbFetchExtensionString() const { return fFBFetchExtensionString; }

    bool flatInterpolationSupport() const { return fFlatInterpolationSupport; }

    bool preferFlatInterpolation() const { return fPreferFlatInterpolation; }

    bool noperspectiveInterpolationSupport() const { return fNoPerspectiveInterpolationSupport; }

    bool sampleMaskSupport() const { return fSampleMaskSupport; }

    bool externalTextureSupport() const { return fExternalTextureSupport; }

    bool vertexIDSupport() const { return fVertexIDSupport; }

    // isinf() is defined, and floating point infinities are handled according to IEEE standards.
    bool infinitySupport() const { return fInfinitySupport; }

    // Returns true if `expr` in `myArray[expr]` can be any integer expression. If false, `expr`
    // must be a constant-index-expression as defined in the OpenGL ES2 specification, Appendix A.5.
    bool nonconstantArrayIndexSupport() const {
        return fNonconstantArrayIndexSupport;
    }

    // frexp(), ldexp(), findMSB(), findLSB().
    bool bitManipulationSupport() const { return fBitManipulationSupport; }

    bool floatIs32Bits() const { return fFloatIs32Bits; }

    bool halfIs32Bits() const { return fHalfIs32Bits; }

    bool hasLowFragmentPrecision() const { return fHasLowFragmentPrecision; }

    // Use a reduced set of rendering algorithms or less optimal effects in order to
    // reduce the number of unique shaders generated.
    bool reducedShaderMode() const { return fReducedShaderMode; }

    /**
     * SkSL ES3 requires support for derivatives, nonsquare matrices and bitwise integer operations.
     */
    bool supportsSkSLES3() const {
        return fShaderDerivativeSupport && fNonsquareMatrixSupport && fIntegerSupport &&
               fGLSLGeneration >= SkSL::GLSLGeneration::k330;
    }

    // SkSL only.
    bool builtinFMASupport() const { return fBuiltinFMASupport; }

    bool builtinDeterminantSupport() const { return fBuiltinDeterminantSupport; }

    AdvBlendEqInteraction advBlendEqInteraction() const { return fAdvBlendEqInteraction; }

    bool mustEnableAdvBlendEqs() const {
        return fAdvBlendEqInteraction >= kGeneralEnable_AdvBlendEqInteraction;
    }

    bool mustDeclareFragmentShaderOutput() const {
        return fGLSLGeneration > SkSL::GLSLGeneration::k110;
    }

    bool usesPrecisionModifiers() const { return fUsesPrecisionModifiers; }

    // Returns whether we can use the glsl function any() in our shader code.
    bool canUseAnyFunctionInShader() const { return fCanUseAnyFunctionInShader; }

    bool canUseMinAndAbsTogether() const { return fCanUseMinAndAbsTogether; }

    bool canUseFractForNegativeValues() const { return fCanUseFractForNegativeValues; }

    bool mustForceNegatedAtanParamToFloat() const { return fMustForceNegatedAtanParamToFloat; }

    // http://skbug.com/12076
    bool mustForceNegatedLdexpParamToMultiply() const {
        return fMustForceNegatedLdexpParamToMultiply;
    }

    // Returns whether a device incorrectly implements atan(y,x) as atan(y/x)
    bool atan2ImplementedAsAtanYOverX() const { return fAtan2ImplementedAsAtanYOverX; }

    // If this returns true some operation (could be a no op) must be called between floor and abs
    // to make sure the driver compiler doesn't inline them together which can cause a driver bug in
    // the shader.
    bool mustDoOpBetweenFloorAndAbs() const { return fMustDoOpBetweenFloorAndAbs; }

    // If false, SkSL uses a workaround so that sk_FragCoord doesn't actually query gl_FragCoord
    bool canUseFragCoord() const { return fCanUseFragCoord; }

    // If true, short ints can't represent every integer in the 16-bit two's complement range as
    // required by the spec. SKSL will always emit full ints.
    bool incompleteShortIntPrecision() const { return fIncompleteShortIntPrecision; }

    bool colorSpaceMathNeedsFloat() const { return fColorSpaceMathNeedsFloat; }

    // If true, then conditions in for loops need "&& true" to work around driver bugs.
    bool addAndTrueToLoopCondition() const { return fAddAndTrueToLoopCondition; }

    // If true, then expressions such as "x && y" or "x || y" are rewritten as
    // ternary to work around driver bugs.
    bool unfoldShortCircuitAsTernary() const { return fUnfoldShortCircuitAsTernary; }

    bool emulateAbsIntFunction() const { return fEmulateAbsIntFunction; }

    bool rewriteDoWhileLoops() const { return fRewriteDoWhileLoops; }

    bool rewriteSwitchStatements() const { return fRewriteSwitchStatements; }

    bool removePowWithConstantExponent() const { return fRemovePowWithConstantExponent; }

    bool requiresLocalOutputColorForFBFetch() const { return fRequiresLocalOutputColorForFBFetch; }

    bool mustObfuscateUniformColor() const { return fMustObfuscateUniformColor; }

    // The D3D shader compiler, when targeting PS 3.0 (ie within ANGLE) fails to compile certain
    // constructs. See detailed comments in GrGLCaps.cpp.
    bool mustGuardDivisionEvenAfterExplicitZeroCheck() const {
        return fMustGuardDivisionEvenAfterExplicitZeroCheck;
    }

    // On Nexus 6, the GL context can get lost if a shader does not write a value to gl_FragColor.
    // https://bugs.chromium.org/p/chromium/issues/detail?id=445377
    bool mustWriteToFragColor() const { return fMustWriteToFragColor; }

    // The Android emulator claims samplerExternalOES is an unknown type if a default precision
    // statement is made for the type.
    bool noDefaultPrecisionForExternalSamplers() const {
        return fNoDefaultPrecisionForExternalSamplers;
    }

    // ARM GPUs calculate `matrix * vector` in SPIR-V at full precision, even when the inputs are
    // RelaxedPrecision. Rewriting the multiply as a sum of vector*scalar fixes this. (skia:11769)
    bool rewriteMatrixVectorMultiply() const {
        return fRewriteMatrixVectorMultiply;
    }

    // Rewrites matrix equality comparisons to avoid an Adreno driver bug. (skia:11308)
    bool rewriteMatrixComparisons() const { return fRewriteMatrixComparisons; }

    // ANGLE disallows do loops altogether, and we're seeing crashes on Tegra3 with do loops in at
    // least some cases.
    bool canUseDoLoops() const { return fCanUseDoLoops; }

    // Some GPUs produce poor results when enabling Metal's fastmath option
    bool canUseFastMath() const { return fCanUseFastMath; }

    // By default, SkSL pools IR nodes per-program. To debug memory corruption, it is sometimes
    // helpful to disable that feature.
    bool useNodePools() const { return fUseNodePools; }

    // When we have the option of using either dFdx or dfDy in a shader, this returns whether we
    // should avoid using dFdx. We have found some drivers have bugs or lower precision when using
    // dFdx.
    bool avoidDfDxForGradientsWhenPossible() const { return fAvoidDfDxForGradientsWhenPossible; }

    // Returns the string of an extension that must be enabled in the shader to support
    // derivatives. If nullptr is returned then no extension needs to be enabled. Before calling
    // this function, the caller should check that shaderDerivativeSupport exists.
    const char* shaderDerivativeExtensionString() const {
        SkASSERT(this->shaderDerivativeSupport());
        return fShaderDerivativeExtensionString;
    }

    // This returns the name of an extension that must be enabled in the shader, if such a thing is
    // required in order to use a secondary output in the shader. This returns a nullptr if no such
    // extension is required. However, the return value of this function does not say whether dual
    // source blending is supported.
    const char* secondaryOutputExtensionString() const { return fSecondaryOutputExtensionString; }

    // This returns the name of an extension that must be enabled in the shader to support external
    // textures. In some cases, two extensions must be enabled - the second extension is returned
    // by secondExternalTextureExtensionString(). If that function returns nullptr, then only one
    // extension is required.
    const char* externalTextureExtensionString() const {
        SkASSERT(this->externalTextureSupport());
        return fExternalTextureExtensionString;
    }

    const char* secondExternalTextureExtensionString() const {
        SkASSERT(this->externalTextureSupport());
        return fSecondExternalTextureExtensionString;
    }

    const char* noperspectiveInterpolationExtensionString() const {
        SkASSERT(this->noperspectiveInterpolationSupport());
        return fNoPerspectiveInterpolationExtensionString;
    }

    const char* sampleVariablesExtensionString() const {
        SkASSERT(this->sampleMaskSupport());
        return fSampleVariablesExtensionString;
    }

    const char* tessellationExtensionString() const {
        SkASSERT(this->tessellationSupport());
        return fTessellationExtensionString;
    }

    int maxFragmentSamplers() const { return fMaxFragmentSamplers; }

    // Maximum number of segments a tessellation edge can be divided into.
    int maxTessellationSegments() const { return fMaxTessellationSegments; }

    bool tessellationSupport() const { return SkToBool(fMaxTessellationSegments);}

    SkSL::GLSLGeneration generation() const { return fGLSLGeneration; }

private:
    void applyOptionsOverrides(const GrContextOptions& options);

    SkSL::GLSLGeneration fGLSLGeneration;

    bool fShaderDerivativeSupport           : 1;
    bool fDstReadInShaderSupport            : 1;
    bool fDualSourceBlendingSupport         : 1;
    bool fIntegerSupport                    : 1;
    bool fNonsquareMatrixSupport            : 1;
    bool fInverseHyperbolicSupport          : 1;
    bool fFBFetchSupport                    : 1;
    bool fFBFetchNeedsCustomOutput          : 1;
    bool fUsesPrecisionModifiers            : 1;
    bool fFlatInterpolationSupport          : 1;
    bool fPreferFlatInterpolation           : 1;
    bool fNoPerspectiveInterpolationSupport : 1;
    bool fSampleMaskSupport                 : 1;
    bool fExternalTextureSupport            : 1;
    bool fVertexIDSupport                   : 1;
    bool fInfinitySupport                   : 1;
    bool fNonconstantArrayIndexSupport      : 1;
    bool fBitManipulationSupport            : 1;
    bool fFloatIs32Bits                     : 1;
    bool fHalfIs32Bits                      : 1;
    bool fHasLowFragmentPrecision           : 1;
    bool fReducedShaderMode                 : 1;

    // Used by SkSL to know when to generate polyfills.
    bool fBuiltinFMASupport : 1;
    bool fBuiltinDeterminantSupport : 1;

    // Used for specific driver bug work arounds
    bool fCanUseAnyFunctionInShader                   : 1;
    bool fCanUseMinAndAbsTogether                     : 1;
    bool fCanUseFractForNegativeValues                : 1;
    bool fMustForceNegatedAtanParamToFloat            : 1;
    bool fMustForceNegatedLdexpParamToMultiply        : 1;
    bool fAtan2ImplementedAsAtanYOverX                : 1;
    bool fMustDoOpBetweenFloorAndAbs                  : 1;
    bool fRequiresLocalOutputColorForFBFetch          : 1;
    bool fMustObfuscateUniformColor                   : 1;
    bool fMustGuardDivisionEvenAfterExplicitZeroCheck : 1;
    bool fCanUseFragCoord                             : 1;
    bool fIncompleteShortIntPrecision                 : 1;
    bool fAddAndTrueToLoopCondition                   : 1;
    bool fUnfoldShortCircuitAsTernary                 : 1;
    bool fEmulateAbsIntFunction                       : 1;
    bool fRewriteDoWhileLoops                         : 1;
    bool fRewriteSwitchStatements                     : 1;
    bool fRemovePowWithConstantExponent               : 1;
    bool fMustWriteToFragColor                        : 1;
    bool fNoDefaultPrecisionForExternalSamplers       : 1;
    bool fRewriteMatrixVectorMultiply                 : 1;
    bool fRewriteMatrixComparisons                    : 1;
    bool fColorSpaceMathNeedsFloat                    : 1;
    bool fCanUseDoLoops                               : 1;
    bool fCanUseFastMath                              : 1;
    bool fAvoidDfDxForGradientsWhenPossible           : 1;

    // This controls behavior of the SkSL compiler, not the code we generate
    bool fUseNodePools : 1;

    const char* fVersionDeclString;

    const char* fShaderDerivativeExtensionString;
    const char* fSecondaryOutputExtensionString;
    const char* fExternalTextureExtensionString;
    const char* fSecondExternalTextureExtensionString;
    const char* fNoPerspectiveInterpolationExtensionString;
    const char* fSampleVariablesExtensionString;
    const char* fTessellationExtensionString;

    const char* fFBFetchColorName;
    const char* fFBFetchExtensionString;

    int fMaxFragmentSamplers;
    int fMaxTessellationSegments;

    AdvBlendEqInteraction fAdvBlendEqInteraction;

    friend class GrCaps;  // For initialization.
    friend class GrDawnCaps;
    friend class GrD3DCaps;
    friend class GrGLCaps;
    friend class GrMockCaps;
    friend class GrMtlCaps;
    friend class GrVkCaps;
    friend class SkSL::ShaderCapsFactory;
    friend class SkSL::SharedCompiler;
};
#else
class GrShaderCaps {};
#endif // !defined(SKSL_STANDALONE) && SK_SUPPORT_GPU

namespace SkSL {

#if defined(SKSL_STANDALONE) || !SK_SUPPORT_GPU

using ShaderCapsClass = StandaloneShaderCaps;
using ShaderCapsPointer = std::shared_ptr<StandaloneShaderCaps>;
extern StandaloneShaderCaps standaloneCaps;

#else

using ShaderCapsClass = GrShaderCaps;
using ShaderCapsPointer = sk_sp<GrShaderCaps>;

#endif  // defined(SKSL_STANDALONE) || !SK_SUPPORT_GPU

// Various sets of caps for use in tests
class ShaderCapsFactory {
public:
    static ShaderCapsPointer Default() {
        ShaderCapsPointer result = MakeShaderCaps();
        result->fVersionDeclString = "#version 400";
        result->fShaderDerivativeSupport = true;
        result->fBuiltinDeterminantSupport = true;
        result->fCanUseDoLoops = true;
        return result;
    }

    static ShaderCapsPointer Standalone() {
        return MakeShaderCaps();
    }

    static ShaderCapsPointer AddAndTrueToLoopCondition() {
        ShaderCapsPointer result = MakeShaderCaps();
        result->fVersionDeclString = "#version 400";
        result->fAddAndTrueToLoopCondition = true;
        return result;
    }

    static ShaderCapsPointer CannotUseFractForNegativeValues() {
        ShaderCapsPointer result = MakeShaderCaps();
        result->fVersionDeclString = "#version 400";
        result->fCanUseFractForNegativeValues = false;
        return result;
    }

    static ShaderCapsPointer CannotUseFragCoord() {
        ShaderCapsPointer result = MakeShaderCaps();
        result->fVersionDeclString = "#version 400";
        result->fCanUseFragCoord = false;
        return result;
    }

    static ShaderCapsPointer CannotUseMinAndAbsTogether() {
        ShaderCapsPointer result = MakeShaderCaps();
        result->fVersionDeclString = "#version 400";
        result->fCanUseMinAndAbsTogether = false;
        return result;
    }

    static ShaderCapsPointer EmulateAbsIntFunction() {
        ShaderCapsPointer result = MakeShaderCaps();
        result->fVersionDeclString = "#version 400";
        result->fEmulateAbsIntFunction = true;
        return result;
    }

    static ShaderCapsPointer FramebufferFetchSupport() {
        ShaderCapsPointer result = MakeShaderCaps();
        result->fFBFetchSupport = true;
        result->fFBFetchColorName = "gl_LastFragData[0]";
        return result;
    }

    static ShaderCapsPointer IncompleteShortIntPrecision() {
        ShaderCapsPointer result = MakeShaderCaps();
        result->fVersionDeclString = "#version 310es";
        result->fUsesPrecisionModifiers = true;
        result->fIncompleteShortIntPrecision = true;
        return result;
    }

    static ShaderCapsPointer MustForceNegatedAtanParamToFloat() {
        ShaderCapsPointer result = MakeShaderCaps();
        result->fVersionDeclString = "#version 400";
        result->fMustForceNegatedAtanParamToFloat = true;
        return result;
    }

    static ShaderCapsPointer MustForceNegatedLdexpParamToMultiply() {
        ShaderCapsPointer result = MakeShaderCaps();
        result->fVersionDeclString = "#version 400";
        result->fMustForceNegatedLdexpParamToMultiply = true;
        return result;
    }

    static ShaderCapsPointer MustGuardDivisionEvenAfterExplicitZeroCheck() {
        ShaderCapsPointer result = MakeShaderCaps();
        result->fMustGuardDivisionEvenAfterExplicitZeroCheck = true;
        return result;
    }

    static ShaderCapsPointer RemovePowWithConstantExponent() {
        ShaderCapsPointer result = MakeShaderCaps();
        result->fVersionDeclString = "#version 400";
        result->fRemovePowWithConstantExponent = true;
        return result;
    }

    static ShaderCapsPointer RewriteDoWhileLoops() {
        ShaderCapsPointer result = MakeShaderCaps();
        result->fVersionDeclString = "#version 400";
        result->fRewriteDoWhileLoops = true;
        return result;
    }

    static ShaderCapsPointer RewriteMatrixComparisons() {
        ShaderCapsPointer result = MakeShaderCaps();
        result->fRewriteMatrixComparisons = true;
        result->fUsesPrecisionModifiers = true;
        return result;
    }

    static ShaderCapsPointer RewriteMatrixVectorMultiply() {
        ShaderCapsPointer result = MakeShaderCaps();
        result->fVersionDeclString = "#version 400";
        result->fRewriteMatrixVectorMultiply = true;
        return result;
    }

    static ShaderCapsPointer RewriteSwitchStatements() {
        ShaderCapsPointer result = MakeShaderCaps();
        result->fVersionDeclString = "#version 400";
        result->fRewriteSwitchStatements = true;
        return result;
    }

    static ShaderCapsPointer SampleMaskSupport() {
        ShaderCapsPointer result = Default();
        result->fSampleMaskSupport = true;
        return result;
    }

    static ShaderCapsPointer ShaderDerivativeExtensionString() {
        ShaderCapsPointer result = MakeShaderCaps();
        result->fVersionDeclString = "#version 400";
        result->fShaderDerivativeSupport = true;
        result->fShaderDerivativeExtensionString = "GL_OES_standard_derivatives";
        result->fUsesPrecisionModifiers = true;
        return result;
    }

    static ShaderCapsPointer UnfoldShortCircuitAsTernary() {
        ShaderCapsPointer result = MakeShaderCaps();
        result->fVersionDeclString = "#version 400";
        result->fUnfoldShortCircuitAsTernary = true;
        return result;
    }

    static ShaderCapsPointer UsesPrecisionModifiers() {
        ShaderCapsPointer result = MakeShaderCaps();
        result->fVersionDeclString = "#version 400";
        result->fUsesPrecisionModifiers = true;
        return result;
    }

    static ShaderCapsPointer Version110() {
        ShaderCapsPointer result = MakeShaderCaps();
        result->fVersionDeclString = "#version 110";
        result->fGLSLGeneration = GLSLGeneration::k110;
        return result;
    }

    static ShaderCapsPointer Version450Core() {
        ShaderCapsPointer result = MakeShaderCaps();
        result->fVersionDeclString = "#version 450 core";
        return result;
    }

private:
    static ShaderCapsPointer MakeShaderCaps();
};

#if !defined(SKSL_STANDALONE)
bool type_to_grsltype(const Context& context, const Type& type, GrSLType* outType);
#endif

void write_stringstream(const StringStream& d, OutputStream& out);

}  // namespace SkSL

#endif  // SKSL_UTIL
