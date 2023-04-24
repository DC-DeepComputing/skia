/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/effects/SkDashingMaskFilter.h"

#include "include/core/SkBitmap.h"
#include "include/core/SkBlendMode.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkFlattenable.h"
#include "include/core/SkMaskFilter.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkShader.h"
#include "src/core/SkMask.h"
#include "src/core/SkMaskFilterBase.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkWriteBuffer.h"
#include "src/shaders/SkShaderBase.h"

#include <cstdint>
#include <cstring>
#include <memory>
#include <utility>

class SkMatrix;

#if defined(SK_GANESH)
#include "src/gpu/ganesh/GrFragmentProcessor.h"
struct GrFPArgs;
#endif

class SkDashingMaskFilter : public SkMaskFilterBase {
public:
    SkDashingMaskFilter(int foo) : fFoo(foo) {}

    SkMask::Format getFormat() const override { return SkMask::kA8_Format; }

    bool filterMask(SkMask* dst, const SkMask& src, const SkMatrix&,
                    SkIPoint* margin) const override;

    void computeFastBounds(const SkRect& src, SkRect* dst) const override {
        *dst = src;
    }

    bool asABlur(BlurRec*) const override { return false; }

protected:

#if defined(SK_GANESH)
    std::unique_ptr<GrFragmentProcessor> onAsFragmentProcessor(const GrFPArgs&,
                                                               const MatrixRec&) const override;
    bool onHasFragmentProcessor() const override;
#endif

private:
//    friend class SkFlattenable;
    static void RegisterFlattenables();

    SK_FLATTENABLE_HOOKS(SkDashingMaskFilter)

    int fFoo;

    SkDashingMaskFilter(SkReadBuffer&);
    void flatten(SkWriteBuffer&) const override;
};

sk_sp<SkFlattenable> SkDashingMaskFilter::CreateProc(SkReadBuffer& buffer) {
    return SkMaskFilters::MakeDashing(buffer.readInt());
}

void SkDashingMaskFilter::flatten(SkWriteBuffer& buffer) const {
    buffer.writeInt(1);
}

bool SkDashingMaskFilter::filterMask(SkMask* dst,
                                     const SkMask& src,
                                     const SkMatrix& ctm,
                                     SkIPoint* margin) const {
    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
#if defined(SK_GANESH)

std::unique_ptr<GrFragmentProcessor>
SkDashingMaskFilter::onAsFragmentProcessor(const GrFPArgs& args, const MatrixRec& mRec) const {
    return nullptr;
//    auto fp = as_SB(fShader)->asFragmentProcessor(args, mRec);
//    return GrFragmentProcessor::MulInputByChildAlpha(std::move(fp));
}

bool SkDashingMaskFilter::onHasFragmentProcessor() const {
    return true;
}

#endif
///////////////////////////////////////////////////////////////////////////////////////////////////

sk_sp<SkMaskFilter> SkMaskFilters::MakeDashing(int foo) {
    return sk_sp<SkMaskFilter>(new SkDashingMaskFilter(foo));
}

void SkDashingMaskFilter::RegisterFlattenables() { SK_REGISTER_FLATTENABLE(SkDashingMaskFilter); }
