/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkModeColorFilter_DEFINED
#define SkModeColorFilter_DEFINED

#include "src/core/SkColorFilterBase.h"

class SkModeColorFilter : public SkColorFilterBase {
public:
    static sk_sp<SkColorFilter> Make(const SkColor4f& color,
                                     sk_sp<SkColorSpace> colorSpace,
                                     SkBlendMode mode) {
        return sk_sp<SkColorFilter>(new SkModeColorFilter(color, std::move(colorSpace), mode));
    }

    bool onIsAlphaUnchanged() const override;

#if SK_SUPPORT_GPU
    GrFPResult asFragmentProcessor(std::unique_ptr<GrFragmentProcessor> inputFP,
                                   GrRecordingContext*, const GrColorInfo&,
                                   const SkSurfaceProps& props) const override;
#endif
#ifdef SK_ENABLE_SKSL
    void addToKey(const SkKeyContext& keyContext,
                  SkPaintParamsKeyBuilder* builder,
                  SkPipelineDataGatherer* gatherer) const override;
#endif

    SK_FLATTENABLE_HOOKS(SkModeColorFilter)

protected:
    SkModeColorFilter(const SkColor4f& color, sk_sp<SkColorSpace>, SkBlendMode mode);

    void flatten(SkWriteBuffer&) const override;
    bool onAsAColorMode(SkColor*, SkBlendMode*) const override;

    bool onAppendStages(const SkStageRec& rec, bool shaderIsOpaque) const override;
    skvm::Color onProgram(skvm::Builder*, skvm::Color,
                          const SkColorInfo&, skvm::Uniforms*, SkArenaAlloc*) const override;

private:
    sk_sp<SkColorSpace> fColorSpace;
    SkColor4f           fColor;
    SkBlendMode         fMode;

    friend class SkColorFilter;

    using INHERITED = SkColorFilterBase;
};

#endif
