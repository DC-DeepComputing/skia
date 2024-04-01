/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/Test.h"

#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColorFilter.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkPaint.h"
#include "include/core/SkSurface.h"
#include "include/effects/SkColorMatrix.h"
#include "src/core/SkColorSpacePriv.h"

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

sk_sp<SkColorSpace> get_src_color_space() {
    return SkColorSpace::MakeRGB(SkNamedTransferFn::kLinear, SkNamedGamut::kSRGB);
}

sk_sp<SkColorSpace> get_dst_color_space() {
//    return SkColorSpace::MakeRGB(SkNamedTransferFn::kLinear, SkNamedGamut::kSRGB);

//    BT2020 = STANDARD_BT2020 | TRANSFER_SMPTE_170M | RANGE_FULL
//    R: 187/255
    return SkColorSpace::MakeRGB(SkNamedTransferFn::kRec2020, SkNamedGamut::kRec2020);

//    ADOBE_RGB = STANDARD_ADOBE_RGB | TRANSFER_GAMMA2_2 | RANGE_FULL
//    R: 192/255

//    return SkColorSpace::MakeSRGB()->makeColorSpin();
//    return SkColorSpace::MakeRGB(SkNamedTransferFn::k2Dot2, gNarrow_toXYZD50);
//    return SkColorSpace::MakeRGB(SkNamedTransferFn::kRec2020, SkNamedGamut::kRec2020);
//    return SkColorSpace::MakeRGB(SkNamedTransferFn::kSRGB, SkNamedGamut::kDisplayP3);
    return SkColorSpace::MakeRGB(SkNamedTransferFn::k2Dot2, SkNamedGamut::kAdobeRGB);
}

SkImageInfo make_image_info() {
    return SkImageInfo::Make(kSize, kRGBA_8888_SkColorType, kPremul_SkAlphaType, get_dst_color_space());
}

void run_test(sk_sp<SkSurface> surface, skiatest::Reporter* reporter) {
    REPORTER_ASSERT(reporter, surface);

    sk_sp<SkColorSpace> srcCS = get_src_color_space();
    SkPaint p;
    p.setColor4f({ 0.5f, 0.25f, 0.125f, 1.0f }, srcCS.get());

    SkColorMatrix m;
    m.setScale(0.9f, 0, 0, 1.0f);
//    p.setColorFilter(SkColorFilters::Matrix(m));

    SkColorMatrix m2(.9, 0, 0, 0, 0,
                     0, .9, 0, 0, 0,
                     0, 0, .9, 0, 0,
                     0, 0, 0, 1, 0);
    p.setColorFilter(SkColorFilters::Matrix(m2));

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
    SkColor col = bitmap.getColor(0, 0);
    SkColor4f col4f = bitmap.getColor4f(0, 0);
    SkDebugf("Out: %d %d %d %d | %.2f %.2f %.2f %.2f\n",
             SkColorGetR(col), SkColorGetG(col), SkColorGetB(col), SkColorGetA(col),
             col4f.fR, col4f.fG, col4f.fB, col4f.fA);
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

    sk_sp<SkSurface> surface = SkSurfaces::WrapBackendTexture(recorder.get(),
                                                              texture,
                                                              kRGBA_8888_SkColorType,
                                                              get_dst_color_space(),
                                                              /* props= */ nullptr);

    run_test(std::move(surface), reporter);

    recorder->deleteBackendTexture(texture);
}

#endif // SK_GRAPHITE
