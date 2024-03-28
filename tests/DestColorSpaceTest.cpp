/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/Test.h"

#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkPaint.h"
#include "include/core/SkSurface.h"

#if defined(SK_GANESH)
#include "include/gpu/ganesh/SkSurfaceGanesh.h"
#endif

#if defined(SK_GRAPHITE)
#include "include/gpu/graphite/BackendTexture.h"
#include "include/gpu/graphite/Context.h"
#include "include/gpu/graphite/Surface.h"
#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/ContextPriv.h"
#endif

using namespace skgpu;
using namespace skgpu::graphite;

const SkISize kSize = {16, 16};

namespace {

SkImageInfo make_image_info() {
    sk_sp<SkColorSpace> cs = SkColorSpace::MakeSRGB()->makeColorSpin();

    return SkImageInfo::Make(kSize, kRGBA_8888_SkColorType, kPremul_SkAlphaType, std::move(cs));
}

void run_test(sk_sp<SkSurface> surface, skiatest::Reporter* reporter) {
    REPORTER_ASSERT(reporter, surface);

    SkPaint p;
    p.setColor(SK_ColorRED);

    SkCanvas* c = surface->getCanvas();
    c->drawRect(SkRect::MakeWH(surface->width(), surface->height()), p);

    SkBitmap bitmap;
    SkPixmap pixmap;
    bitmap.allocPixels(surface->imageInfo());
    SkAssertResult(bitmap.peekPixels(&pixmap));
    if (!surface->readPixels(pixmap, 0, 0)) {
        ERRORF(reporter, "readPixels failed");
        return;
    }
    SkColor4f col = bitmap.getColor4f(0, 0);
    SkDebugf("Out: %.2f %.2f %.2f %.2f\n", col.fR, col.fG, col.fB, col.fA);
}

} // anonymous namespace

DEF_TEST(DestColorSpaceTest_Raster, reporter) {
    sk_sp<SkSurface> surface = SkSurfaces::Raster(make_image_info());

    run_test(std::move(surface), reporter);
}

#if defined(SK_GANESH)
DEF_GANESH_TEST_FOR_CONTEXTS(DestColorSpaceTest_Ganesh,
                             skgpu::IsRenderingContext,
                             reporter,
                             ctxInfo,
                             nullptr,
                             CtsEnforcement::kNextRelease) {
    sk_sp<SkSurface> surface = SkSurfaces::RenderTarget(ctxInfo.directContext(),
                                                        Budgeted::kYes,
                                                        make_image_info());

    run_test(std::move(surface), reporter);
}
#endif // SK_GANESH

#if defined(SK_GRAPHITE)
DEF_GRAPHITE_TEST_FOR_ALL_CONTEXTS(DestColorSpaceTest_Graphite,
                                   reporter,
                                   context,
                                   CtsEnforcement::kNextRelease) {
    std::unique_ptr<Recorder> recorder = context->makeRecorder();
    sk_sp<SkSurface> surface = SkSurfaces::RenderTarget(recorder.get(), make_image_info());

    run_test(std::move(surface), reporter);

}

DEF_GRAPHITE_TEST_FOR_ALL_CONTEXTS(DestColorSpaceTest_Wrapped_Graphite,
                                   reporter,
                                   context,
                                   CtsEnforcement::kNextRelease) {
    auto caps = context->priv().caps();

    TextureInfo info = caps->getDefaultSampledTextureInfo(kRGBA_8888_SkColorType,
                                                          /* mipmapped= */ Mipmapped::kNo,
                                                          Protected::kNo,
                                                          Renderable::kYes);

    std::unique_ptr<Recorder> recorder = context->makeRecorder();
    BackendTexture texture = recorder->createBackendTexture(kSize, info);
    REPORTER_ASSERT(reporter, texture.isValid());

    sk_sp<SkColorSpace> cs = SkColorSpace::MakeSRGB()->makeColorSpin();

    sk_sp<SkSurface> surface = SkSurfaces::WrapBackendTexture(recorder.get(),
                                                              texture,
                                                              kRGBA_8888_SkColorType,
                                                              cs,
                                                              /* props= */ nullptr);

    run_test(std::move(surface), reporter);

    recorder->deleteBackendTexture(texture);
}

#endif // SK_GRAPHITE
