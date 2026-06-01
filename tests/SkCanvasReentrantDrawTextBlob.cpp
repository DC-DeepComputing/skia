
/*
 * Copyright 2026 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "include/core/SkCanvas.h"
#include "include/core/SkData.h"
#include "include/core/SkDrawable.h"
#include "include/core/SkFont.h"
#include "include/core/SkFontMetrics.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "include/core/SkPicture.h"
#include "include/core/SkPictureRecorder.h"
#include "include/core/SkRSXform.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSerialProcs.h"
#include "include/core/SkSurface.h"
#include "include/core/SkTextBlob.h"
#include "include/core/SkTypeface.h"
#include "include/utils/SkCustomTypeface.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkWriteBuffer.h"
#include "tests/Test.h"

static sk_sp<SkTypeface> make_path_typeface() {
    SkCustomTypefaceBuilder builder;
    builder.setGlyph(0, 0.0f, SkPath::Rect(SkRect::MakeLTRB(0, -1, 1, 0)));
    return builder.detach();
}

static sk_sp<SkTypeface> make_drawable_typeface(sk_sp<SkDrawable> drawable) {
    SkCustomTypefaceBuilder builder;
    builder.setGlyph(0, 0.0f, std::move(drawable), SkRect::MakeLTRB(0, -1, 1, 0));
    return builder.detach();
}

static sk_sp<SkTextBlob> make_inner_blob(sk_sp<SkTypeface> tf) {
    SkTextBlobBuilder builder;
    for (int i = 0; i < 64; ++i) {
        SkFont font(tf, static_cast<float>(i));
        const auto& run = builder.allocRunPos(font, 1);
        run.glyphs[0] = 0;
        run.points()[0] = SkPoint::Make(0, 0);
    }
    return builder.make();
}

static sk_sp<SkTextBlob> make_outer_blob(sk_sp<SkTypeface> drawable_tf, sk_sp<SkTypeface> path_tf) {
    SkTextBlobBuilder builder;
    SkFont font0(std::move(drawable_tf), 257.0f);
    {
        const auto& run = builder.allocRunPos(font0, 1);
        run.glyphs[0] = 0;
        run.points()[0] = SkPoint::Make(0, 0);
    }
    SkFont font1(std::move(path_tf), 1.0f);
    {
        const auto& run = builder.allocRunRSXform(font1, 1);
        run.glyphs[0] = 0;
        run.xforms()[0] = SkRSXform::Make(1, 0, 0, 0);
    }
    return builder.make();
}

class ReentrantDrawable final : public SkDrawable {
public:
    explicit ReentrantDrawable(sk_sp<SkTextBlob> inner) : inner_(std::move(inner)) {}
    static sk_sp<SkFlattenable> CreateProc(SkReadBuffer& buffer) {
        auto data = buffer.readByteArrayAsData();
        return sk_make_sp<ReentrantDrawable>(
                SkTextBlob::Deserialize(data->data(), data->size(), {}));
    }
    const char* getTypeName() const override { return "ReentrantDrawable"; }
    void flatten(SkWriteBuffer& buffer) const override {
        auto data = inner_->serialize({});
        buffer.writeByteArray(data->data(), data->size());
    }

protected:
    SkRect onGetBounds() override { return SkRect::MakeLTRB(0, -1, 1, 0); }
    void onDraw(SkCanvas* canvas) override {
        SkPaint paint;
        canvas->drawTextBlob(inner_, 0, 0, paint);
    }

private:
    sk_sp<SkTextBlob> inner_;
};

/**
 * This test revolves around drawing a drawTextBlob with an SkCustomTypeface that consists of an
 * SkDrawable that has another drawTextBlob.
 *
 * The first drawTextBlob would (originally) make use of the SkCanvas->fScratchGlyphRunBuilder, then
 * when the second drawTextBlob begun, fScratchGlyphRunBuilder would clear, before the first
 * drawTextBlob could finish. Thus after exiting out of the second drawTextBlob, we would continue
 * to use the data pointed to by the fScratchGlyphRunBuilder, which would cause a UAF.
 *
 * The setup above is neccessary because, as specified in the bug write up (b/513820666), the path
 * for reentrancy relies on entering simplifyGlyphRunRSXFormAndRedraw, which means that an RSX form
 * must be included and a sufficiently large "inner" blob must be used to force the
 * fGlyphRunListStorage vector to reallocate.
 *
 */
DEF_TEST(SkCanvas_drawTextBlob_b513820666, reporter) {
    SkFlattenable::Register("ReentrantDrawable", ReentrantDrawable::CreateProc);
    sk_sp<SkTypeface> path_tf = make_path_typeface();
    sk_sp<SkTextBlob> inner = make_inner_blob(path_tf);
    sk_sp<SkDrawable> drawable = sk_make_sp<ReentrantDrawable>(std::move(inner));
    sk_sp<SkTypeface> drawable_tf = make_drawable_typeface(std::move(drawable));
    sk_sp<SkTextBlob> outer = make_outer_blob(std::move(drawable_tf), path_tf);
    SkRect bounds = SkRect::MakeWH(1024, 1024);
    SkPictureRecorder recorder;
    SkCanvas* recordingCanvas = recorder.beginRecording(bounds);
    SkPaint paint;
    recordingCanvas->drawTextBlob(outer, 100, 500, paint);
    sk_sp<SkPicture> picture = recorder.finishRecordingAsPicture();
    sk_sp<SkData> data = picture->serialize();
    sk_sp<SkPicture> newPicture = SkPicture::MakeFromData(data.get());
    sk_sp<SkSurface> surface = SkSurfaces::Raster(SkImageInfo::MakeN32Premul(1024, 1024));
    REPORTER_ASSERT(reporter, surface);
    SkCanvas* canvas = surface->getCanvas();
    canvas->drawPicture(newPicture);
}
