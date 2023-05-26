/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkColorSpaceXformColorFilter_DEFINED
#define SkColorSpaceXformColorFilter_DEFINED

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

class SkColorSpaceXformColorFilter final : public SkColorFilterBase {
public:
    SkColorSpaceXformColorFilter(sk_sp<SkColorSpace> src, sk_sp<SkColorSpace> dst);

#if defined(SK_GRAPHITE)
    void addToKey(const skgpu::graphite::KeyContext& keyContext,
                  skgpu::graphite::PaintParamsKeyBuilder* builder,
                  skgpu::graphite::PipelineDataGatherer* gatherer) const override;
#endif

    bool appendStages(const SkStageRec& rec, bool shaderIsOpaque) const override;

#if defined(SK_ENABLE_SKVM)
    skvm::Color onProgram(skvm::Builder* p, skvm::Color c, const SkColorInfo& dst,
                          skvm::Uniforms* uniforms, SkArenaAlloc* alloc) const override;
#endif

    SkColorFilterBase::Type type() const override {
        return SkColorFilterBase::Type::kColorSpaceXform;
    }

protected:
    void flatten(SkWriteBuffer& buffer) const override;

private:
    friend void ::SkRegisterSkColorSpaceXformColorFilterFlattenable();
    SK_FLATTENABLE_HOOKS(SkColorSpaceXformColorFilter)
    static sk_sp<SkFlattenable> LegacyGammaOnlyCreateProc(SkReadBuffer& buffer);

    const sk_sp<SkColorSpace> fSrc;
    const sk_sp<SkColorSpace> fDst;
    SkColorSpaceXformSteps fSteps;

    friend class SkColorFilter;
    using INHERITED = SkColorFilterBase;
};

#endif
