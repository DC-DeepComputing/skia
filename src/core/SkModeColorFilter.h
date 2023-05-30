/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkModeColorFilter_DEFINED
#define SkModeColorFilter_DEFINED

#include "include/core/SkColor.h"
#include "include/core/SkFlattenable.h"
#include "src/core/SkColorFilterBase.h"

class SkReadBuffer;
class SkWriteBuffer;
enum class SkBlendMode;
struct SkStageRec;

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
    SkColorFilterBase::Type type() const override { return SkColorFilterBase::Type::kMode; }

    SkColor4f color() const { return fColor; }
    SkBlendMode mode() const { return fMode; }

private:
    friend void ::SkRegisterModeColorFilterFlattenable();
    SK_FLATTENABLE_HOOKS(SkModeColorFilter)

    void flatten(SkWriteBuffer&) const override;
    bool onAsAColorMode(SkColor*, SkBlendMode*) const override;

#if defined(SK_ENABLE_SKVM)
    skvm::Color onProgram(skvm::Builder*,
                          skvm::Color,
                          const SkColorInfo&,
                          skvm::Uniforms*,
                          SkArenaAlloc*) const override;
#endif

    SkColor4f fColor;  // always stored in sRGB
    SkBlendMode fMode;
};

#endif
