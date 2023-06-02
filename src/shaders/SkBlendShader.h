/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkBlendShader_DEFINED
#define SkBlendShader_DEFINED

#include "include/core/SkColorFilter.h"
#include "include/core/SkFlattenable.h"
#include "include/core/SkString.h"
#include "include/effects/SkRuntimeEffect.h"
#include "include/private/SkColorData.h"
#include "src/base/SkArenaAlloc.h"
#include "src/core/SkBlendModePriv.h"
#include "src/core/SkBlenderBase.h"
#include "src/core/SkRasterPipeline.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkRuntimeEffectPriv.h"
#include "src/core/SkVM.h"
#include "src/core/SkWriteBuffer.h"
#include "src/shaders/SkShaderBase.h"

#if defined(SK_GRAPHITE)
#include "src/gpu/Blend.h"
#include "src/gpu/graphite/KeyHelpers.h"
#include "src/gpu/graphite/PaintParamsKey.h"
#endif


class SkBlendShader final : public SkShaderBase {
public:
    SkBlendShader(SkBlendMode mode, sk_sp<SkShader> dst, sk_sp<SkShader> src)
            : fDst(std::move(dst))
            , fSrc(std::move(src))
            , fMode(mode) {}

    ShaderType type() const override { return ShaderType::kBlend; }


#if defined(SK_GRAPHITE)
    void addToKey(const skgpu::graphite::KeyContext&,
                  skgpu::graphite::PaintParamsKeyBuilder*,
                  skgpu::graphite::PipelineDataGatherer*) const override;
#endif

protected:
    SkBlendShader(SkReadBuffer&);
    void flatten(SkWriteBuffer&) const override;
    bool appendStages(const SkStageRec&, const SkShaders::MatrixRec&) const override;

#if defined(SK_ENABLE_SKVM)
    skvm::Color program(skvm::Builder*,
                        skvm::Coord device,
                        skvm::Coord local,
                        skvm::Color paint,
                        const SkShaders::MatrixRec& mRec,
                        const SkColorInfo& dst,
                        skvm::Uniforms*,
                        SkArenaAlloc*) const override;
#endif  // defined(SK_ENABLE_SKVM)

private:
    friend void ::SkRegisterBlendShaderFlattenable();
    SK_FLATTENABLE_HOOKS(SkBlendShader)

    sk_sp<SkShader>     fDst;
    sk_sp<SkShader>     fSrc;
    SkBlendMode         fMode;
};

#endif
