/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkMatrixColorFilter_DEFINED
#define SkMatrixColorFilter_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/core/SkUnPreMultiply.h"
#include "include/effects/SkColorMatrix.h"
#include "include/effects/SkRuntimeEffect.h"
#include "include/private/SkColorData.h"
#include "src/core/SkColorFilterBase.h"
#include "src/core/SkColorSpacePriv.h"
#include "src/core/SkRasterPipeline.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkRuntimeEffectPriv.h"
#include "src/core/SkVM.h"
#include "src/core/SkWriteBuffer.h"

#if defined(SK_GRAPHITE)
#include "src/gpu/graphite/KeyHelpers.h"
#include "src/gpu/graphite/PaintParamsKey.h"
#endif // SK_GRAPHITE

class SkMatrixColorFilter final : public SkColorFilterBase {
public:
    enum class Domain : uint8_t { kRGBA, kHSLA };

    explicit SkMatrixColorFilter(const float array[20], Domain);

    bool appendStages(const SkStageRec& rec, bool shaderIsOpaque) const override;

    bool onIsAlphaUnchanged() const override { return fAlphaIsUnchanged; }

    SkColorFilterBase::Type type() const override { return SkColorFilterBase::Type::kMatrix; }

#if defined(SK_GRAPHITE)
    void addToKey(const skgpu::graphite::KeyContext&,
                  skgpu::graphite::PaintParamsKeyBuilder*,
                  skgpu::graphite::PipelineDataGatherer*) const override;
#endif

private:
    friend void ::SkRegisterMatrixColorFilterFlattenable();
    SK_FLATTENABLE_HOOKS(SkMatrixColorFilter)

    void flatten(SkWriteBuffer&) const override;
    bool onAsAColorMatrix(float matrix[20]) const override;

#if defined(SK_ENABLE_SKVM)
    skvm::Color onProgram(skvm::Builder*, skvm::Color,
                          const SkColorInfo& dst,
                          skvm::Uniforms* uniforms, SkArenaAlloc*) const override;
#endif

    float  fMatrix[20];
    bool   fAlphaIsUnchanged;
    Domain fDomain;
};

#endif