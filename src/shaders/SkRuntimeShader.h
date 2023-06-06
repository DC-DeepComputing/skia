/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkRuntimeShader_DEFINED
#define SkRuntimeShader_DEFINED

#include "include/core/SkAlphaType.h"
#include "include/core/SkBlendMode.h"
#include "include/core/SkBlender.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkCapabilities.h"
#include "include/core/SkColorFilter.h"
#include "include/core/SkData.h"
#include "include/core/SkImage.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkPaint.h"
#include "include/core/SkSurface.h"
#include "include/effects/SkRuntimeEffect.h"
#include "include/private/base/SkAlign.h"
#include "include/private/base/SkDebug.h"
#include "include/private/base/SkMutex.h"
#include "include/private/base/SkOnce.h"
#include "include/private/base/SkTArray.h"
#include "src/base/SkArenaAlloc.h"
#include "src/base/SkNoDestructor.h"
#include "src/base/SkTLazy.h"
#include "src/core/SkBlenderBase.h"
#include "src/core/SkChecksum.h"
#include "src/core/SkColorSpacePriv.h"
#include "src/core/SkColorSpaceXformSteps.h"
#include "src/core/SkEffectPriv.h"
#include "src/core/SkFilterColorProgram.h"
#include "src/core/SkLRUCache.h"
#include "src/core/SkPicturePriv.h"
#include "src/core/SkRasterPipeline.h"
#include "src/core/SkRasterPipelineOpList.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkRuntimeBlender.h"
#include "src/core/SkRuntimeEffectPriv.h"
#include "src/core/SkStreamPriv.h"
#include "src/core/SkWriteBuffer.h"
#include "src/effects/colorfilters/SkColorFilterBase.h"
#include "src/effects/colorfilters/SkRuntimeColorFilter.h"
#include "src/shaders/SkLocalMatrixShader.h"
#include "src/shaders/SkShaderBase.h"
#include "src/sksl/SkSLAnalysis.h"
#include "src/sksl/SkSLBuiltinTypes.h"
#include "src/sksl/SkSLCompiler.h"
#include "src/sksl/SkSLContext.h"
#include "src/sksl/SkSLProgramKind.h"
#include "src/sksl/SkSLProgramSettings.h"
#include "src/sksl/SkSLUtil.h"
#include "src/sksl/analysis/SkSLProgramUsage.h"
#include "src/sksl/codegen/SkSLRasterPipelineBuilder.h"
#include "src/sksl/ir/SkSLFunctionDeclaration.h"
#include "src/sksl/ir/SkSLLayout.h"
#include "src/sksl/ir/SkSLModifiers.h"
#include "src/sksl/ir/SkSLProgram.h"
#include "src/sksl/ir/SkSLProgramElement.h"
#include "src/sksl/ir/SkSLStatement.h"
#include "src/sksl/ir/SkSLType.h"
#include "src/sksl/ir/SkSLVarDeclarations.h"
#include "src/sksl/ir/SkSLVariable.h"
#include "src/sksl/tracing/SkSLDebugTracePriv.h"

#include <algorithm>
#include <tuple>

class SkColorSpace;
struct SkIPoint;

#if defined(SK_GANESH)
#include "include/gpu/GpuTypes.h"
#include "include/gpu/GrRecordingContext.h"
#include "include/gpu/GrTypes.h"
#include "include/gpu/ganesh/SkSurfaceGanesh.h"
#include "src/gpu/ganesh/GrCaps.h"
#include "src/gpu/ganesh/GrColorInfo.h"
#include "src/gpu/ganesh/GrFPArgs.h"
#include "src/gpu/ganesh/GrFragmentProcessor.h"
#include "src/gpu/ganesh/GrFragmentProcessors.h"
#include "src/gpu/ganesh/GrRecordingContextPriv.h"
#endif

#if defined(SK_GRAPHITE)
#include "src/gpu/graphite/KeyContext.h"
#include "src/gpu/graphite/KeyHelpers.h"
#include "src/gpu/graphite/PaintParamsKey.h"
#endif

using UniformsCallback = SkRuntimeEffectPriv::UniformsCallback;

class SkRuntimeShader : public SkShaderBase {
public:
    SkRuntimeShader(sk_sp<SkRuntimeEffect> effect,
               sk_sp<SkSL::DebugTracePriv> debugTrace,
               sk_sp<const SkData> uniforms,
               SkSpan<SkRuntimeEffect::ChildPtr> children);

    SkRuntimeShader(sk_sp<SkRuntimeEffect> effect,
               sk_sp<SkSL::DebugTracePriv> debugTrace,
               UniformsCallback uniformsCallback,
               SkSpan<SkRuntimeEffect::ChildPtr> children);

    SkRuntimeEffect::TracedShader makeTracedClone(const SkIPoint& coord);

    bool isOpaque() const override { return fEffect->alwaysOpaque(); }

    ShaderType type() const override { return ShaderType::kRuntime; }

#if defined(SK_GRAPHITE)
    void addToKey(const skgpu::graphite::KeyContext& keyContext,
                  skgpu::graphite::PaintParamsKeyBuilder* builder,
                  skgpu::graphite::PipelineDataGatherer* gatherer) const override;
#endif

    bool appendStages(const SkStageRec& rec, const SkShaders::MatrixRec& mRec) const override;

#if defined(SK_ENABLE_SKVM)
    skvm::Color program(skvm::Builder* p,
                        skvm::Coord device,
                        skvm::Coord local,
                        skvm::Color paint,
                        const SkShaders::MatrixRec& mRec,
                        const SkColorInfo& colorInfo,
                        skvm::Uniforms* uniforms,
                        SkArenaAlloc* alloc) const override;
#endif

    void flatten(SkWriteBuffer& buffer) const override;

    SkRuntimeEffect* asRuntimeEffect() const override { return fEffect.get(); }

    sk_sp<SkRuntimeEffect> effect() const { return fEffect; }
    std::vector<SkRuntimeEffect::ChildPtr> children() const { return fChildren; }

    sk_sp<const SkData> uniformData(const SkColorSpace* dstCS) const;

    SK_FLATTENABLE_HOOKS(SkRuntimeShader)

private:
    enum Flags {
        kHasLegacyLocalMatrix_Flag = 1 << 1,
    };

    sk_sp<SkRuntimeEffect> fEffect;
    sk_sp<SkSL::DebugTracePriv> fDebugTrace;
    sk_sp<const SkData> fUniformData;
    UniformsCallback fUniformsCallback;
    std::vector<SkRuntimeEffect::ChildPtr> fChildren;
};

#endif
