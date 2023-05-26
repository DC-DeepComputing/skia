/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkWorkingFormatColorFilter_DEFINED
#define SkWorkingFormatColorFilter_DEFINED

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

class SkWorkingFormatColorFilter final : public SkColorFilterBase {
public:
    SkWorkingFormatColorFilter(sk_sp<SkColorFilter>          child,
                               const skcms_TransferFunction* tf,
                               const skcms_Matrix3x3*        gamut,
                               const SkAlphaType*            at);

    sk_sp<SkColorSpace> workingFormat(const sk_sp<SkColorSpace>& dstCS, SkAlphaType* at) const;

    SkColorFilterBase::Type type() const override {
        return SkColorFilterBase::Type::kWorkingFormat;
    }

#if defined(SK_GRAPHITE)
    void addToKey(const skgpu::graphite::KeyContext& keyContext,
                  skgpu::graphite::PaintParamsKeyBuilder* builder,
                  skgpu::graphite::PipelineDataGatherer* gatherer) const override;
#endif

    bool appendStages(const SkStageRec& rec, bool shaderIsOpaque) const override;

#if defined(SK_ENABLE_SKVM)
    skvm::Color onProgram(skvm::Builder* p, skvm::Color c, const SkColorInfo& rawDst,
                          skvm::Uniforms* uniforms, SkArenaAlloc* alloc) const override;
#endif

    SkPMColor4f onFilterColor4f(const SkPMColor4f& origColor,
                                SkColorSpace* rawDstCS) const override;

    bool onIsAlphaUnchanged() const override;

private:
    friend void ::SkRegisterWorkingFormatColorFilterFlattenable();
    SK_FLATTENABLE_HOOKS(SkWorkingFormatColorFilter)

    void flatten(SkWriteBuffer& buffer) const override;

    sk_sp<SkColorFilter>   fChild;
    skcms_TransferFunction fTF;     bool fUseDstTF    = true;
    skcms_Matrix3x3        fGamut;  bool fUseDstGamut = true;
    SkAlphaType            fAT;     bool fUseDstAT    = true;
};

#endif
