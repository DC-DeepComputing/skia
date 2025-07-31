/*
 * Copyright 2025 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/effects/SkStrokeAndFillPathEffect.h"

#include "include/core/SkFlattenable.h"
#include "include/core/SkPath.h"
#include "include/core/SkPathBuilder.h"
#include "include/core/SkPathEffect.h"
#include "include/core/SkPathUtils.h"
#include "include/core/SkRefCnt.h"

#include "src/core/SkPaintDefaults.h"
#include "src/core/SkPathEffectBase.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkWriteBuffer.h"

namespace {

class StrokeAndFillImpl : public SkPathEffectBase {
public:
    StrokeAndFillImpl(float width, SkPaint::Cap cap, SkPaint::Join join, float limit)
    : fWidth(width), fLimit(limit), fCap(cap), fJoin(join)
    {
        SkASSERT(width >= 0);   // zero means we'll just return the fill
        SkASSERT(limit >= 0);
    }

    bool onFilterPath(SkPathBuilder* dst, const SkPath& src, SkStrokeRec*, const SkRect*,
                      const SkMatrix& matrix) const override {
        if (fWidth == 0) {
            dst->addPath(src);
            return true;
        }

        SkPaint paint;
        paint.setStrokeWidth(fWidth);
        paint.setStrokeCap(fCap);
        paint.setStrokeJoin(fJoin);
        paint.setStrokeMiter(fLimit);

        skpathutils::FillPathWithPaint(src, paint, dst, nullptr, matrix);
        dst->addPath(src);

        return true;
    }

    bool computeFastBounds(SkRect* bounds) const override {
        if (bounds) {
            const float radius = fWidth * 0.5f;
            bounds->outset(radius, radius);
        }
        return true;
    }

    static sk_sp<SkFlattenable> CreateProc(SkReadBuffer& buffer) {
        float width   = buffer.readScalar();
        float limit   = buffer.readScalar();
        unsigned cap  = buffer.readUInt();
        unsigned join = buffer.readUInt();

        if (cap <= SkPaint::kLast_Cap && join <= SkPaint::kLast_Join) {
            return SkStrokeAndFillPathEffect::Make(width,
                                                   (SkPaint::Cap)cap,
                                                   (SkPaint::Join)join,
                                                   limit);
        }
        return nullptr;
    }

    void flatten(SkWriteBuffer& buffer) const override {
        buffer.writeScalar(fWidth);
        buffer.writeScalar(fLimit);
        buffer.writeUInt((unsigned)fCap);
        buffer.writeUInt((unsigned)fJoin);
    }

    Factory getFactory() const override { return CreateProc; }
    const char* getTypeName() const override { return "SkStrokeAndFillPathEffect"; }

private:
    const float fWidth, fLimit;
    const SkPaint::Cap  fCap;
    const SkPaint::Join fJoin;

    using INHERITED = SkPathEffectBase;
};

} // namespace

//////////////////////////////////////////////////////////////////////////////////////////////////

sk_sp<SkPathEffect> SkStrokeAndFillPathEffect::Make(float width,
                                                    SkPaint::Cap cap,
                                                    SkPaint::Join join,
                                                    float limit) {
    if (!SkIsFinite(width) || !SkIsFinite(limit)) {
        return nullptr;
    }
    if (width < 0) {
        return nullptr;
    }

    if (limit < 0) {    // signal to use default
        limit = SkPaintDefaults_MiterLimit;
    }
    return sk_sp<SkPathEffect>(new StrokeAndFillImpl(width, cap, join, limit));
}

void SkStrokeAndFillPathEffect::RegisterFlattenables() {
    SkFlattenable::Register("SkStrokeAndFillPathEffect", StrokeAndFillImpl::CreateProc);
}
