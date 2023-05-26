/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkModeColorFilter_DEFINED
#define SkModeColorFilter_DEFINED

#include "include/core/SkColorFilter.h"
#include "include/private/SkColorData.h"
#include "src/base/SkArenaAlloc.h"
#include "src/core/SkBlendModePriv.h"
#include "src/core/SkBlitRow.h"
#include "src/core/SkColorFilterBase.h"
#include "src/core/SkColorSpacePriv.h"
#include "src/core/SkColorSpaceXformSteps.h"
#include "src/core/SkRasterPipeline.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkVM.h"
#include "src/core/SkValidationUtils.h"
#include "src/core/SkWriteBuffer.h"

#if defined(SK_GRAPHITE)
#include "src/gpu/graphite/KeyContext.h"
#include "src/gpu/graphite/KeyHelpers.h"
#include "src/gpu/graphite/PaintParamsKey.h"
#endif

class SkModeColorFilter final : public SkColorFilterBase {
public:
    SkModeColorFilter(const SkColor4f& color, SkBlendMode mode);

    bool appendStages(const SkStageRec& rec, bool shaderIsOpaque) const override;

    bool onIsAlphaUnchanged() const override;

#if defined(SK_GRAPHITE)
    void addToKey(const skgpu::graphite::KeyContext&,
                  skgpu::graphite::PaintParamsKeyBuilder*,
                  skgpu::graphite::PipelineDataGatherer*) const override;
#endif

private:
    friend void ::SkRegisterModeColorFilterFlattenable();
    SK_FLATTENABLE_HOOKS(SkModeColorFilter)

    void flatten(SkWriteBuffer&) const override;
    bool onAsAColorMode(SkColor*, SkBlendMode*) const override;

    SkColorFilterBase::Type type() const override { return SkColorFilterBase::Type::kMode; }

#if defined(SK_ENABLE_SKVM)
    skvm::Color onProgram(skvm::Builder*, skvm::Color,
                          const SkColorInfo&, skvm::Uniforms*, SkArenaAlloc*) const override;
#endif

    SkColor4f   fColor; // always stored in sRGB
    SkBlendMode fMode;
};

#endif