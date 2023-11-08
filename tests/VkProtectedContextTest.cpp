/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// This is a Vulkan protected memory specific test.

#include "include/core/SkTypes.h"

#if defined(SK_VULKAN)

#include "include/core/SkAlphaType.h"
#include "include/core/SkBlendMode.h"
#include "include/core/SkBlurTypes.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkColorType.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkMaskFilter.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSurface.h"
#include "include/core/SkTypes.h"
#include "include/gpu/GpuTypes.h"
#include "include/gpu/GrBackendSurface.h"
#include "include/gpu/GrDirectContext.h"
#include "include/gpu/GrTypes.h"
#include "tests/CtsEnforcement.h"
#include "tests/Test.h"
#include "tools/gpu/ProtectedUtils.h"
#include "tools/gpu/vk/VkTestHelper.h"

#include <memory>
#include <utility>


#include "include/gpu/graphite/Context.h"
#include "include/gpu/graphite/Recorder.h"

using namespace skgpu;

struct GrContextOptions;

static const int kSize = 8;

static void create_nonprotected_context(skiatest::Reporter* reporter,
                                        skiatest::TestType testType) {
    std::unique_ptr<VkTestHelper> helper = VkTestHelper::Make(testType, /* isProtected= */ false);
    REPORTER_ASSERT(reporter, helper);
}

DEF_GANESH_TEST(VkProtectedContext_CreateNonprotectedContext,
                reporter,
                options,
                CtsEnforcement::kNever) {
    create_nonprotected_context(reporter, skiatest::TestType::kGanesh);
}

DEF_GRAPHITE_TEST(VkProtectedContext_CreateNonprotectedContext_Graphite,
                  reporter,
                  CtsEnforcement::kNextRelease) {
    create_nonprotected_context(reporter, skiatest::TestType::kGraphite);
}

DEF_GANESH_TEST(VkProtectedContext_CreateProtectedContext,
                reporter,
                options,
                CtsEnforcement::kNever) {
    std::unique_ptr<VkTestHelper> helper = VkTestHelper::Make(skiatest::TestType::kGanesh,
                                                              /* isProtected= */ true);
}

DEF_GRAPHITE_TEST(VkProtectedContext_CreateProtectedContext_Graphite,
                  reporter,
                  CtsEnforcement::kNextRelease) {
    std::unique_ptr<VkTestHelper> helper = VkTestHelper::Make(skiatest::TestType::kGraphite,
                                                              /* isProtected= */ true);
}

DEF_GANESH_TEST(VkProtectedContext_CreateProtectedSkSurface,
                reporter,
                options,
                CtsEnforcement::kNever) {
    auto protectedTestHelper = std::make_unique<VkTestHelper>(/* isProtected= */ true);
    if (!protectedTestHelper->init()) {
        SkDebugf("early out of test\n");
        return;
    }

    auto dContext = protectedTestHelper->directContext1();
    REPORTER_ASSERT(reporter, dContext != nullptr);
    ProtectedUtils::CreateProtectedSkSurface(dContext, { kSize, kSize }, /* textureable= */ true);
    ProtectedUtils::CreateProtectedSkSurface(dContext, { kSize, kSize }, /* textureable= */ false);
}

DEF_GANESH_TEST(VkProtectedContext_CreateNonprotectedTextureInProtectedContext,
                reporter,
                options,
                CtsEnforcement::kNever) {
    auto protectedTestHelper = std::make_unique<VkTestHelper>(/* isProtected= */ true);
    if (!protectedTestHelper->init()) {
        SkDebugf("early out of test\n");
        return;
    }
    REPORTER_ASSERT(reporter, protectedTestHelper->directContext1() ||
                              protectedTestHelper->graphiteContext());

    const int kW = 8;
    const int kH = 8;
    GrBackendTexture backendTex =
            protectedTestHelper->directContext1()->createBackendTexture(kW,
                                                                       kH,
                                                                       kRGBA_8888_SkColorType,
                                                                       skgpu::Mipmapped::kNo,
                                                                       GrRenderable::kNo,
                                                                       GrProtected::kNo);
    REPORTER_ASSERT(reporter, !backendTex.isValid());
}

DEF_GANESH_TEST(VkProtectedContext_CreateProtectedTextureInNonprotectedContext,
                reporter,
                options,
                CtsEnforcement::kNever) {
    auto protectedTestHelper = std::make_unique<VkTestHelper>(/* isProtected= */ false);
    if (!protectedTestHelper->init()) {
        SkDebugf("early out of test\n");
        return;
    }
    REPORTER_ASSERT(reporter, protectedTestHelper->directContext1() ||
                              protectedTestHelper->graphiteContext());

    const int kW = 8;
    const int kH = 8;
    GrBackendTexture backendTex =
            protectedTestHelper->directContext1()->createBackendTexture(kW,
                                                                       kH,
                                                                       kRGBA_8888_SkColorType,
                                                                       skgpu::Mipmapped::kNo,
                                                                       GrRenderable::kNo,
                                                                       GrProtected::kYes);
    REPORTER_ASSERT(reporter, !backendTex.isValid());
}

DEF_GANESH_TEST(VkProtectedContext_ReadFromProtectedSurface,
                reporter,
                options,
                CtsEnforcement::kNever) {
    auto protectedTestHelper = std::make_unique<VkTestHelper>(/* isProtected= */ true);
    if (!protectedTestHelper->init()) {
        SkDebugf("early out of test\n");
        return;
    }
    REPORTER_ASSERT(reporter, protectedTestHelper->directContext1() ||
                              protectedTestHelper->graphiteContext());

    auto surface = ProtectedUtils::CreateProtectedSkSurface(protectedTestHelper->directContext1(),
                                                            { kSize, kSize });
    REPORTER_ASSERT(reporter, surface);
    REPORTER_ASSERT(reporter, !surface->readPixels(SkImageInfo(), nullptr, 8, 0, 0));
}

namespace {

struct AsyncContext {
    bool fCalled = false;
    std::unique_ptr<const SkSurface::AsyncReadResult> fResult;
};

static void async_callback(void* c, std::unique_ptr<const SkSurface::AsyncReadResult> result) {
    auto context = static_cast<AsyncContext*>(c);
    context->fResult = std::move(result);
    context->fCalled = true;
}

}  // anonymous namespace

DEF_GANESH_TEST(VkProtectedContext_AsyncReadFromProtectedSurface,
                reporter,
                options,
                CtsEnforcement::kNever) {
    auto protectedTestHelper = std::make_unique<VkTestHelper>(/* isProtected= */ true);
    if (!protectedTestHelper->init()) {
        SkDebugf("early out of test\n");
        return;
    }

    auto dContext = protectedTestHelper->directContext1();

    REPORTER_ASSERT(reporter, dContext != nullptr);

    auto surface = ProtectedUtils::CreateProtectedSkSurface(dContext, { kSize, kSize });
    REPORTER_ASSERT(reporter, surface);
    AsyncContext cbContext;
    const auto image_info = SkImageInfo::Make(10, 10, kRGBA_8888_SkColorType, kPremul_SkAlphaType,
                                      SkColorSpace::MakeSRGB());
    surface->asyncRescaleAndReadPixelsYUV420(kIdentity_SkYUVColorSpace, SkColorSpace::MakeSRGB(),
                                             image_info.bounds(), image_info.dimensions(),
                                             SkSurface::RescaleGamma::kSrc,
                                             SkSurface::RescaleMode::kNearest,
                                             &async_callback, &cbContext);
    dContext->submit();
    while (!cbContext.fCalled) {
        dContext->checkAsyncWorkCompletion();
    }
    REPORTER_ASSERT(reporter, !cbContext.fResult);
}

DEF_GANESH_TEST(VkProtectedContext_DrawRectangle, reporter, options, CtsEnforcement::kNever) {
    auto protectedTestHelper = std::make_unique<VkTestHelper>(/* isProtected= */ true);
    if (!protectedTestHelper->init()) {
        SkDebugf("early out of test\n");
        return;
    }
    REPORTER_ASSERT(reporter, protectedTestHelper->directContext1() ||
                              protectedTestHelper->graphiteContext());

    auto surface = ProtectedUtils::CreateProtectedSkSurface(protectedTestHelper->directContext1(),
                                                            { kSize, kSize });
    REPORTER_ASSERT(reporter, surface);
    SkCanvas* canvas = surface->getCanvas();
    REPORTER_ASSERT(reporter, canvas);
    SkPaint paint;
    paint.setColor(SK_ColorBLACK);
    canvas->drawRect(SkRect::MakeWH(4, 4), paint);
}

DEF_GANESH_TEST(VkProtectedContext_DrawRectangleWithAntiAlias,
                reporter,
                options,
                CtsEnforcement::kNever) {
    auto protectedTestHelper = std::make_unique<VkTestHelper>(/* isProtected= */ true);
    if (!protectedTestHelper->init()) {
        SkDebugf("early out of test\n");
        return;
    }
    REPORTER_ASSERT(reporter, protectedTestHelper->directContext1() ||
                              protectedTestHelper->graphiteContext());

    auto surface = ProtectedUtils::CreateProtectedSkSurface(protectedTestHelper->directContext1(),
                                                            { kSize, kSize });
    REPORTER_ASSERT(reporter, surface);
    SkCanvas* canvas = surface->getCanvas();
    REPORTER_ASSERT(reporter, canvas);
    SkPaint paint;
    paint.setColor(SK_ColorBLACK);
    paint.setAntiAlias(true);
    canvas->drawRect(SkRect::MakeWH(4, 4), paint);
}

DEF_GANESH_TEST(VkProtectedContext_DrawRectangleWithBlendMode,
                reporter,
                options,
                CtsEnforcement::kNever) {
    auto protectedTestHelper = std::make_unique<VkTestHelper>(/* isProtected= */ true);
    if (!protectedTestHelper->init()) {
        SkDebugf("early out of test\n");
        return;
    }
    REPORTER_ASSERT(reporter, protectedTestHelper->directContext1() ||
                              protectedTestHelper->graphiteContext());

    auto surface = ProtectedUtils::CreateProtectedSkSurface(protectedTestHelper->directContext1(),
                                                            { kSize, kSize });
    REPORTER_ASSERT(reporter, surface);
    SkCanvas* canvas = surface->getCanvas();
    REPORTER_ASSERT(reporter, canvas);
    SkPaint paint;
    paint.setColor(SK_ColorBLACK);
    paint.setBlendMode(SkBlendMode::kColorDodge);
    canvas->drawRect(SkRect::MakeWH(4, 4), paint);
}

DEF_GANESH_TEST(VkProtectedContext_DrawRectangleWithFilter,
                reporter,
                options,
                CtsEnforcement::kNever) {
    auto protectedTestHelper = std::make_unique<VkTestHelper>(/* isProtected= */ true);
    if (!protectedTestHelper->init()) {
        SkDebugf("early out of test\n");
        return;
    }
    REPORTER_ASSERT(reporter, protectedTestHelper->directContext1() ||
                              protectedTestHelper->graphiteContext());

    auto surface = ProtectedUtils::CreateProtectedSkSurface(protectedTestHelper->directContext1(),
                                                            { kSize, kSize });
    REPORTER_ASSERT(reporter, surface);
    SkCanvas* canvas = surface->getCanvas();
    REPORTER_ASSERT(reporter, canvas);
    SkPaint paint;
    paint.setColor(SK_ColorBLACK);
    paint.setStyle(SkPaint::kFill_Style);
    paint.setMaskFilter(SkMaskFilter::MakeBlur(
          SkBlurStyle::kOuter_SkBlurStyle, 1.1f));
    canvas->drawRect(SkRect::MakeWH(4, 4), paint);
}

DEF_GANESH_TEST(VkProtectedContext_DrawThinPath, reporter, options, CtsEnforcement::kNever) {
    auto protectedTestHelper = std::make_unique<VkTestHelper>(/* isProtected= */ true);
    if (!protectedTestHelper->init()) {
        SkDebugf("early out of test\n");
        return;
    }
    REPORTER_ASSERT(reporter, protectedTestHelper->directContext1() ||
                              protectedTestHelper->graphiteContext());

    auto surface = ProtectedUtils::CreateProtectedSkSurface(protectedTestHelper->directContext1(),
                                                            { kSize, kSize });
    REPORTER_ASSERT(reporter, surface);
    SkCanvas* canvas = surface->getCanvas();
    REPORTER_ASSERT(reporter, canvas);
    SkPaint paint;
    paint.setColor(SK_ColorBLACK);
    paint.setStyle(SkPaint::kStroke_Style);
    paint.setAntiAlias(true);
    paint.setStrokeWidth(.4f);
    canvas->drawPath(SkPath().moveTo(4, 4).lineTo(6, 6), paint);
}

static void save_layer(skiatest::Reporter* reporter) {
    auto protectedTestHelper = std::make_unique<VkTestHelper>(/* isProtected= */ true);
    if (!protectedTestHelper->init()) {
        SkDebugf("early out of test\n");
        return;
    }
    REPORTER_ASSERT(reporter, protectedTestHelper->directContext1() ||
                              protectedTestHelper->graphiteContext());

    auto surface = ProtectedUtils::CreateProtectedSkSurface(protectedTestHelper->directContext1(),
                                                            { kSize, kSize });
    REPORTER_ASSERT(reporter, surface);
    SkCanvas* canvas = surface->getCanvas();
    REPORTER_ASSERT(reporter, canvas);
    canvas->saveLayer(nullptr, nullptr);
    SkPaint paint;
    paint.setColor(SK_ColorBLACK);
    canvas->drawRect(SkRect::MakeWH(4, 4), paint);
    canvas->restore();
}

DEF_GANESH_TEST(VkProtectedContext_SaveLayer,
                reporter,
                options,
                CtsEnforcement::kNever) {
    save_layer(reporter);
}

DEF_GRAPHITE_TEST(VkProtectedContext_SaveLayer_Graphite,
                  reporter,
                  CtsEnforcement::kNextRelease) {
    save_layer(reporter);
}

static void draw_protected_image_on_protected_surface_test(skiatest::Reporter* reporter,
                                                           skiatest::TestType testType) {
    auto protectedTestHelper = std::make_unique<VkTestHelper>(testType, /* isProtected= */ true);
    if (!protectedTestHelper->init()) {
        SkDebugf("early out of test\n");
        return;
    }
    REPORTER_ASSERT(reporter, protectedTestHelper->directContext1() ||
                              protectedTestHelper->graphiteContext());

    std::unique_ptr<skgpu::graphite::Recorder> recorder;
    if (protectedTestHelper->graphiteContext()) {
        recorder = protectedTestHelper->graphiteContext()->makeRecorder();
    }

    // Create protected image.
    sk_sp<SkSurface> surface1, surface2;

    if (protectedTestHelper->directContext1()) {
        surface1 = ProtectedUtils::CreateProtectedSkSurface(protectedTestHelper->directContext1(),
                                                            {kSize, kSize});
    } else {
        surface1 = ProtectedUtils::CreateProtectedSkSurface(recorder.get(),
                                                            {kSize, kSize},
                                                            Protected::kYes);
    }
    REPORTER_ASSERT(reporter, surface1);
    auto image = surface1->makeImageSnapshot();
    REPORTER_ASSERT(reporter, image);

    // Create protected canvas.
    if (protectedTestHelper->directContext1()) {
        surface2 = ProtectedUtils::CreateProtectedSkSurface(protectedTestHelper->directContext1(),
                                                            {kSize, kSize});
    } else {
        surface2 = ProtectedUtils::CreateProtectedSkSurface(recorder.get(),
                                                            {kSize, kSize},
                                                            Protected::kYes);
    }
    REPORTER_ASSERT(reporter, surface2);
    SkCanvas* canvas = surface2->getCanvas();
    REPORTER_ASSERT(reporter, canvas);

    canvas->drawImage(image, 0, 0);
}

DEF_GANESH_TEST(VkProtectedContext_DrawProtectedImageOnProtectedSurface,
                reporter,
                options,
                CtsEnforcement::kNever) {
    draw_protected_image_on_protected_surface_test(reporter, skiatest::TestType::kGanesh);
}

DEF_GRAPHITE_TEST(VkProtectedContext_DrawProtectedImageOnProtectedSurface_Graphite,
                  reporter,
                  CtsEnforcement::kNextRelease) {
    draw_protected_image_on_protected_surface_test(reporter, skiatest::TestType::kGraphite);
}

#endif  // defined(SK_VULKAN)
