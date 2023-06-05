/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkCoordClampShader_DEFINED
#define SkCoordClampShader_DEFINED

#include "include/core/SkFlattenable.h"
#include "src/base/SkArenaAlloc.h"
#include "src/core/SkRasterPipeline.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkRuntimeEffectPriv.h"
#include "src/core/SkVM.h"
#include "src/core/SkWriteBuffer.h"
#include "src/shaders/SkShaderBase.h"

#if defined(SK_GRAPHITE)
#include "src/gpu/graphite/KeyHelpers.h"
#include "src/gpu/graphite/PaintParamsKey.h"
#endif // SK_GRAPHITE

class SkCoordClampShader final : public SkShaderBase {
public:
    SkCoordClampShader(sk_sp<SkShader> shader, const SkRect& subset)
            : fShader(std::move(shader)), fSubset(subset) {}

    ShaderType type() const override { return ShaderType::kCoordClamp; }


#if defined(SK_GRAPHITE)
    void addToKey(const skgpu::graphite::KeyContext&,
                  skgpu::graphite::PaintParamsKeyBuilder*,
                  skgpu::graphite::PipelineDataGatherer*) const override;
#endif

    sk_sp<SkShader> shader() const { return fShader; }
    SkRect subset() const { return fSubset; }

protected:
    SkCoordClampShader(SkReadBuffer&);
    void flatten(SkWriteBuffer&) const override;
    bool appendStages(const SkStageRec&, const SkShaders::MatrixRec&) const override;
#if defined(SK_ENABLE_SKVM)
    skvm::Color program(skvm::Builder*,
                        skvm::Coord device,
                        skvm::Coord local,
                        skvm::Color paint,
                        const SkShaders::MatrixRec&,
                        const SkColorInfo& dst,
                        skvm::Uniforms*,
                        SkArenaAlloc*) const override;
#endif

private:
    friend void ::SkRegisterCoordClampShaderFlattenable();
    SK_FLATTENABLE_HOOKS(SkCoordClampShader)

    sk_sp<SkShader> fShader;
    SkRect fSubset;
};

#endif
