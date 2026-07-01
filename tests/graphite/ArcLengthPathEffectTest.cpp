/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/Test.h"

#include "include/core/SkCanvas.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "include/core/SkPathBuilder.h"
#include "include/gpu/graphite/Context.h"
#include "include/gpu/graphite/Recorder.h"
#include "include/gpu/graphite/Recording.h"
#include "include/gpu/graphite/Surface.h"
#include "src/core/SkPathEffectBase.h"

namespace skgpu::graphite {

class TestArcLengthPathEffect : public SkPathEffectBase {
public:
    static sk_sp<SkPathEffect> Make() {
        return sk_sp<SkPathEffect>(new TestArcLengthPathEffect());
    }

    bool isArcLengthParameterized() const override { return true; }

    bool onFilterPath(SkPathBuilder* dst, const SkPath& src, SkStrokeRec* rec,
                      const SkRect* cullRect, const SkMatrix& ctm) const override {
        return false;
    }

    bool computeFastBounds(SkRect* bounds) const override {
        return false;
    }

    Factory getFactory() const override { return nullptr; }
    const char* getTypeName() const override { return "TestArcLengthPathEffect"; }
};

DEF_GRAPHITE_TEST_FOR_RENDERING_CONTEXTS(ArcLengthPathEffectTest, reporter, context,
                                         CtsEnforcement::kNever) {
    const SkImageInfo surfaceImageInfo = SkImageInfo::Make(
            32, 32, SkColorType::kRGBA_8888_SkColorType, SkAlphaType::kPremul_SkAlphaType);

    std::unique_ptr<Recorder> recorder = context->makeRecorder();
    sk_sp<SkSurface> surface = SkSurfaces::RenderTarget(recorder.get(), surfaceImageInfo);
    if (!surface) {
        return;
    }
    SkCanvas* canvas = surface->getCanvas();

    SkPaint paint;
    paint.setStrokeWidth(2.0f);
    paint.setStyle(SkPaint::kStroke_Style);
    paint.setPathEffect(TestArcLengthPathEffect::Make());

    SkPathBuilder builder;
    builder.moveTo(0, 0);
    builder.quadTo(10, 20, 30, 40);
    builder.cubicTo(40, 50, 60, 70, 80, 90);
    SkPath path = builder.detach();

    canvas->drawPath(path, paint);

    std::unique_ptr<Recording> recording = recorder->snap();
    if (recording) {
        context->insertRecording({recording.get()});
        context->submit(SyncToCpu::kYes);
    }
}

}  // namespace skgpu::graphite
