/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkComposeColorFilter_DEFINED
#define SkComposeColorFilter_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/core/SkString.h"
#include "include/core/SkUnPreMultiply.h"
#include "include/effects/SkRuntimeEffect.h"
#include "include/private/base/SkTDArray.h"
#include "modules/skcms/skcms.h"
#include "src/base/SkArenaAlloc.h"
#include "src/base/SkNoDestructor.h"
#include "src/core/SkColorFilterBase.h"
#include "src/core/SkColorFilterPriv.h"
#include "src/core/SkColorSpacePriv.h"
#include "src/core/SkColorSpaceXformSteps.h"
#include "src/core/SkMatrixProvider.h"
#include "src/core/SkRasterPipeline.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkRuntimeEffectPriv.h"
#include "src/core/SkVM.h"
#include "src/core/SkWriteBuffer.h"

class SkComposeColorFilter final : public SkColorFilterBase {
public:
    bool onIsAlphaUnchanged() const override;

    bool appendStages(const SkStageRec& rec, bool shaderIsOpaque) const override;

    SkColorFilterBase::Type type() const override { return SkColorFilterBase::Type::kCompose; }

#if defined(SK_ENABLE_SKVM)
    skvm::Color onProgram(skvm::Builder* p, skvm::Color c,
                          const SkColorInfo& dst,
                          skvm::Uniforms* uniforms, SkArenaAlloc* alloc) const override;
#endif

#if defined(SK_GRAPHITE)
    void addToKey(const skgpu::graphite::KeyContext& keyContext,
                  skgpu::graphite::PaintParamsKeyBuilder* builder,
                  skgpu::graphite::PipelineDataGatherer* gatherer) const override;
#endif // SK_GRAPHITE

protected:
    void flatten(SkWriteBuffer& buffer) const override;

private:
    friend void ::SkRegisterComposeColorFilterFlattenable();
    SK_FLATTENABLE_HOOKS(SkComposeColorFilter)

    SkComposeColorFilter(sk_sp<SkColorFilter> outer, sk_sp<SkColorFilter> inner);

    sk_sp<SkColorFilterBase> fOuter;
    sk_sp<SkColorFilterBase> fInner;

    friend class SkColorFilter;

    using INHERITED = SkColorFilter;
};

#endif
