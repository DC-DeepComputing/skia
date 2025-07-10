/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkPathUtils.h"

#include "include/core/SkMatrix.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "include/core/SkPathBuilder.h"
#include "include/core/SkPathEffect.h"
#include "include/core/SkScalar.h"
#include "include/core/SkStrokeRec.h"
#include "src/core/SkMatrixPriv.h"

namespace skpathutils {

#ifdef SK_SUPPORT_MUTABLE_PATHEFFECT
bool FillPathWithPaint(const SkPath& src, const SkPaint& paint, SkPath* dst) {
    return skpathutils::FillPathWithPaint(src, paint, dst, nullptr, 1);
}

bool FillPathWithPaint(const SkPath& src, const SkPaint& paint, SkPath* dst,
                       const SkRect* cullRect, SkScalar resScale) {
    return skpathutils::FillPathWithPaint(src, paint, dst, cullRect,
                                          SkMatrix::Scale(resScale, resScale));
}

bool FillPathWithPaint(const SkPath& src, const SkPaint& paint, SkPath* dst,
                       const SkRect* cullRect, const SkMatrix& ctm) {
    SkPathBuilder builder;
    auto result = FillPathWithPaint(src, paint, &builder, cullRect, ctm);
    if (result.useDst) {
        *dst = builder.detach();
    }
    return result.doFill;
}
#endif

FillResult FillPathWithPaint(const SkPath& origSrc, const SkPaint& paint, SkPathBuilder* builder,
                             const SkRect* cullRect, const SkMatrix& ctm) {
    builder->reset();
    if (!origSrc.isFinite()) {
        return {false, false};
    }

    const SkScalar resScale = SkMatrixPriv::ComputeResScaleForStroking(ctm);
    SkStrokeRec rec(paint, resScale);

#if defined(SK_BUILD_FOR_FUZZER)
    // Prevent lines with small widths from timing out.
    if (rec.getStyle() == SkStrokeRec::Style::kStroke_Style && rec.getWidth() < 0.001) {
        return {false, false};
    }
#endif

    const SkPath* srcPtr = &origSrc;
    SkPath pathStorage;
    SkPathEffect* pe = paint.getPathEffect();
    if (pe && pe->filterPath(builder, origSrc, &rec, cullRect, ctm)) {
        pathStorage = builder->detach();
        srcPtr = &pathStorage;
    }

    const bool doFill = !rec.isHairlineStyle();

    if (!rec.applyToPath(builder, *srcPtr)) {
        return {false, doFill};
    }

    if (!builder->isFinite()) {
        builder->reset();
    }
    return {true, doFill};
}

FillResult FillPathWithPaint(const SkPath& src, const SkPaint& paint, SkPathBuilder* dst) {
    return FillPathWithPaint(src, paint, dst, nullptr, SkMatrix::I());
}

SkPath FillPathWithPaint(const SkPath& src, const SkPaint& paint, bool* isFillPtr) {
    SkPathBuilder builder;
    auto result = FillPathWithPaint(src, paint, &builder);

    if (isFillPtr) {
        *isFillPtr = result.doFill;
    }
    return result.useDst ? builder.detach() : src;
}


} // namespace skpathutils
