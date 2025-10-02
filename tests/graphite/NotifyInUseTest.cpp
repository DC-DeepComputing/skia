/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkImage.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRect.h"
#include "include/core/SkSurface.h"
#include "include/effects/SkBlurMaskFilter.h"
#include "include/gpu/graphite/Context.h"
#include "include/gpu/graphite/Recorder.h"
#include "include/gpu/graphite/Surface.h"
#include "tests/Test.h"

#include "src/gpu/graphite/Surface_Graphite.h"

using namespace skgpu::graphite;

namespace {

    bool colors_are_similar(SkColor c1, SkColor c2, int tolerance) {
        if (std::abs((int)SkColorGetA(c1) - (int)SkColorGetA(c2)) > tolerance) return false;
        if (std::abs((int)SkColorGetR(c1) - (int)SkColorGetR(c2)) > tolerance) return false;
        if (std::abs((int)SkColorGetG(c1) - (int)SkColorGetG(c2)) > tolerance) return false;
        if (std::abs((int)SkColorGetB(c1) - (int)SkColorGetB(c2)) > tolerance) return false;
        return true;
    }

} // namespace anonymous

/*
    Force a Device::makeImageCopy by calling makeImageSnapshot(). This in turn calls
    flushPendingWork(nullptr), then Image::Copy. *However* because flushPendingWork only flushes
    the tasks from mainSurface, when Image::Copy adds the copy task directly to the recorder's
    root task list, the scratchSurface is not yet flushed! So when the recorder snaps in
    readPixels, the initial draw which mainSurface depends on is recorded *after* the draw in
    mainSurface, resulting in an incorrect image:

    =========== RECORDING ===========
    **** FLUSH TOKEN 1 Snap ****
    0: Draw Task (target=0x120e84570)   <-- mainSurface's drawTask
    └── RenderPass Task
    1: Copy TtoT Task                   <-- the copy task from mainSurface to finalSurface
    2: Draw Task (target=0x120e834a0)   <-- scratchSuface's drawTask
    └── RenderPass Task
    3: Draw Task (target=0x120e85690)   <-- finalSurfaces's drawTask
    └── RenderPass Task
    -------------- END --------------

    The correct ordering:
    =========== RECORDING ===========
    **** FLUSH TOKEN 1 Snap ****
    0: Draw Task (target=0x120e834a0)   <-- scratchSuface's drawTask
    └── RenderPass Task
    1: Draw Task (target=0x120e84570)   <-- mainSurface's drawTask
    └── RenderPass Task
    2: Copy TtoT Task                   <-- the copy task from mainSurface to finalSurface
    3: Draw Task (target=0x120e85690)   <-- finalSurfaces's drawContext
    └── RenderPass Task
    -------------- END --------------
*/
DEF_GRAPHITE_TEST_FOR_CONTEXTS(NotifyInUseTestSnapshot, /*filter=*/nullptr, reporter, context,
                               testContext, CtsEnforcement::kApiLevel_202404) {
    auto recorder = context->makeRecorder();
    SkImageInfo info = SkImageInfo::Make(10, 10, kRGBA_8888_SkColorType, kPremul_SkAlphaType);

    sk_sp<SkSurface> scratchSurface = SkSurfaces::RenderTarget(recorder.get(), info);
    SkCanvas* scratchCanvas = scratchSurface->getCanvas();
    SkPaint greenPaint;
    greenPaint.setColor(SK_ColorGREEN);
    scratchCanvas->drawRect(SkRect::Make(info.bounds()), greenPaint);

    sk_sp<SkImage> scratchImage = SkSurfaces::AsImage(scratchSurface);
    REPORTER_ASSERT(reporter, scratchImage);

    sk_sp<SkSurface> mainSurface = SkSurfaces::RenderTarget(recorder.get(), info);
    SkCanvas* mainCanvas = mainSurface->getCanvas();
    mainCanvas->clear(SK_ColorBLACK);

    mainCanvas->drawImage(scratchImage, 0, 0);

    sk_sp<SkSurface> finalSurface = SkSurfaces::RenderTarget(recorder.get(), info);
    SkCanvas* finalCanvas = finalSurface->getCanvas();
    finalCanvas->clear(SK_ColorBLACK);

    finalCanvas->drawImage(mainSurface->makeImageSnapshot(), 0, 0);

    SkBitmap bitmap;
    bitmap.allocPixels(info);
    REPORTER_ASSERT(reporter, finalSurface->readPixels(bitmap, 0, 0));

    SkColor topLeft = bitmap.getColor(0, 0);
    REPORTER_ASSERT(reporter, topLeft == SK_ColorGREEN,
                    "Expected green pixel from scratch surface, got 0x%08X", topLeft);
}

/*
    These tests replicate the compositing behavior in blink's canvas2d. Due to the use of save
    layers for compositing, the final order of draw tasks on the root task list does not match the
    ordering in the GM. At a high level the layer tests look like this:

    DrawBlue
        DrawGeometry DrawContext0
    SaveLayer SkBlendMode::kMultiply

    DrawBlur
        DrawGeometry DrawContext1
    Restore0
        SetImmutable FlushPendingWork
        DrawSpecial DrawGeometry DrawContext0
    SaveLayer SkBlendMode::kMultiply

    DrawRed
        DrawGeometry DrawContext2
    Restore1
        SetImmutable FlushPendingWork
        DrawSpecial DrawGeometry DrawContext0

    Snap()
        FlushTrackedDevices

    ------------------------------------------------------------------------------------------------

    Some blends, like multiply, depend on prior draws. However, on devices without frame buffer
    fetch, this creates a dependency issue because the destination surface must be copied to be
    read in for the blending. This means flushing prior draws to the root task list prior to this
    draw---flushBeforeDraw. Dependency on prior draws is communicated through
    Image_Base::notifyInUse, but correct dependency ordering is contingent on the ordering of
    flushBeforeDraw and notifyInUse relative to each other.

    Calling notifyInUse prior to flushBeforeDraw causes the dependent draw task to be added as a
    child task of the current draw instead of flushing to the root task list. In many cases, adding
    the draw as a child of the dependent draw task is ok because child tasks execute prior to their
    parent task. However, if a later draw *also* depends on the now child draw, an incorrect draw
    order may arise.

    Using the SkBlendMode::Multiply version of the test as an example:

    Incorrect ordering
    1) Blur flushed to the root task list by restore0 setImmutable
    2) Blur gets added as child of Blue
    3) Blue+Blur gets flushed as flushBeforeDraw restore0 drawSpecial
    4) Red flushed to the root task list by restore1 setImmutable
    5) Red gets added as child of Blue+Blur+Red+Blend
    6) Blue+Blur+Red+Blend gets flushed as flushBeforeDraw restore1 drawSpecial
    7) Malformed gets flushed as flushTrackedDevices snap

    Correct ordering
    1) Blur flushed to the root task list by restore0 setImmutable
    2) Blue gets flushed as flushBeforeDraw restore0 drawSpecial
    3) Blur added as child of Blue+Blur+Blend
    3) Red flushed to the root task list by restore1 setImmutable
    5) Blue+Blur+Blend flushed as flushBeforeDraw restore1 drawSpecial
    6) Red added as child of Blue+Blur+Red+Blend
    6) Blue+Blur+Red+Blend flushed as flushTrackedDevices snap

    SkBlendMode::Multiply incorrect ordering:
    =========== RECORDING 1 ===========
    **** FLUSH TOKEN 154 Recorder::Snap ****
    0: Draw Task (target=0x17252f6c0) (DC=0x17253b7d0)  <-- Draw blur into layer
    └── RenderPass Task
    1: Draw Task (target=0x172544bf0) (DC=0x1725290d0)  <-- Draw blue into main layer
    │   Draw Task (target=0x17252f6c0) (DC=0x17253b7d0) <-- Blur is added as child of blue
    │   └── RenderPass Task
    └── RenderPass Task
    2: Draw Task (target=0x17254c3d0) (DC=0x172530cc0)  <-- Draw red into layer
    └── RenderPass Task
    3: Draw Task (target=0x172544bf0) (DC=0x1725290d0)  <-- Blend blue with red!
    │   Draw Task (target=0x17254c3d0) (DC=0x172530cc0) <-- Red is added as child of blend
    │   └── RenderPass Task
    │   Copy TtoT Task: Src=0x172544bf0 Dst=0x172547880 <-- Texture Copy because no FbFetch
    └── RenderPass Task
    4: Draw Task (target=0x172544bf0) (DC=0x1725290d0)  <-- ATTEMPT to blend blue+red with blur!
    │   Copy TtoT Task: Src=0x172544bf0 Dst=0x17254c130 <-- Copy but there's no lastDrawTask !
    └── RenderPass Task
    --------------- END ---------------

    The correct ordering:
    =========== RECORDING 1 ===========
    **** FLUSH TOKEN 181 Recorder::Snap ****
    0: Draw Task (target=0x349db7690) (DC=0x349d856b0)  <-- Draw blur into layer
    └── RenderPass Task
    1: Draw Task (target=0x349db7010) (DC=0x349d98a80)  <-- Draw blue into main layer
    └── RenderPass Task
    2: Draw Task (target=0x349d99f90) (DC=0x349d981d0)  <-- Draw red into layer
    └── RenderPass Task
    3: Draw Task (target=0x349db7010) (DC=0x349d98a80)  <-- Blend blue with blur
    │   Draw Task (target=0x349db7690) (DC=0x349d856b0) <-- Blur is correctly added as child
    │   └── RenderPass Task
    │   Copy TtoT Task: Src=0x349db7010 Dst=0x349d96b90 <-- Texture Copy because no FbFetch
    └── RenderPass Task
    4: Draw Task (target=0x349db7010) (DC=0x349d98a80)  <-- Blend blue+blur with red
    │   Draw Task (target=0x349d99f90) (DC=0x349d981d0) <-- Red is correctly added as child
    │   └── RenderPass Task
    │   Copy TtoT Task: Src=0x349db7010 Dst=0x349d988a0 <-- Texture Copy because no FbFetch
    └── RenderPass Task
    --------------- END ---------------
*/

#define DEFINE_LAYER_BLEND_TEST(TestName, BlendMode, ExpectedBlueOnly, ExpectedBlurOnly,         \
                                ExpectedBlueAndBlur, ExpectedRedOnly, ExpectedAllOverlap)        \
DEF_GRAPHITE_TEST_FOR_CONTEXTS(NotifyInUseTestLayer ## TestName, /*filter=*/nullptr, reporter,   \
                               context, testContext, CtsEnforcement::kApiLevel_202404) {         \
    auto recorder = context->makeRecorder();                                                     \
    SkImageInfo info = SkImageInfo::Make(256, 256, kRGBA_8888_SkColorType, kPremul_SkAlphaType); \
    SkBitmap bitmap;                                                                             \
                                                                                                 \
    {                                                                                            \
        sk_sp<SkSurface> surface = SkSurfaces::RenderTarget(recorder.get(), info);               \
        SkCanvas* canvas = surface->getCanvas();                                                 \
                                                                                                 \
        SkPaint blendPaint;                                                                      \
        blendPaint.setBlendMode(BlendMode);                                                      \
                                                                                                 \
        SkPaint bluePaint;                                                                       \
        bluePaint.setColor(0xFF0099FF);                                                          \
        canvas->clear(SK_ColorTRANSPARENT);                                                      \
        canvas->drawRect(SkRect::MakeXYWH(5, 5, 45, 45), bluePaint);                             \
                                                                                                 \
        canvas->saveLayer(nullptr, &blendPaint);                                                 \
        SkPaint shadowOnlyPaint;                                                                 \
        shadowOnlyPaint.setColor(SK_ColorBLUE);                                                  \
        shadowOnlyPaint.setMaskFilter(SkMaskFilter::MakeBlur(kNormal_SkBlurStyle, 5.0f));        \
        canvas->clear(SK_ColorTRANSPARENT);                                                      \
        canvas->drawRect(SkRect::MakeXYWH(30, 15, 45, 45), shadowOnlyPaint);                     \
        canvas->restore();                                                                       \
                                                                                                 \
        canvas->saveLayer(nullptr, &blendPaint);                                                 \
        SkPaint redPaint;                                                                        \
        redPaint.setColor(0xFFFF3300);                                                           \
        canvas->clear(SK_ColorTRANSPARENT);                                                      \
        canvas->drawRect(SkRect::MakeXYWH(20, 25, 45, 45), redPaint);                            \
        canvas->restore();                                                                       \
                                                                                                 \
        bitmap.allocPixels(info);                                                                \
        REPORTER_ASSERT(reporter, canvas->readPixels(bitmap, 0, 0),                              \
                        "Failed to read pixels for BlendMode '%s'",                              \
                        SkBlendMode_Name(BlendMode));                                            \
    }                                                                                            \
                                                                                                 \
    constexpr int kTolerance = 2;                                                                \
                                                                                                 \
    SkColor blueOnly = bitmap.getColor(10, 10);                                                  \
    REPORTER_ASSERT(reporter, colors_are_similar(blueOnly, ExpectedBlueOnly, kTolerance),        \
                    "Pt(10,10) BlendMode '%s': Expected blue-only ~0x%08X, got 0x%08X",          \
                    SkBlendMode_Name(BlendMode), (uint32_t)ExpectedBlueOnly, blueOnly);          \
                                                                                                 \
    SkColor blurOnly = bitmap.getColor(70, 30);                                                  \
    REPORTER_ASSERT(reporter, colors_are_similar(blurOnly, ExpectedBlurOnly, kTolerance),        \
                    "Pt(70,30) BlendMode '%s': Expected blur-only ~0x%08X, got 0x%08X",          \
                    SkBlendMode_Name(BlendMode), (uint32_t)ExpectedBlurOnly, blurOnly);          \
                                                                                                 \
    SkColor blueAndBlur = bitmap.getColor(40, 20);                                               \
    REPORTER_ASSERT(reporter, colors_are_similar(blueAndBlur, ExpectedBlueAndBlur, kTolerance),  \
                    "Pt(40,20) BlendMode '%s': Expected blue/blur ~0x%08X, got 0x%08X",          \
                    SkBlendMode_Name(BlendMode), (uint32_t)ExpectedBlueAndBlur, blueAndBlur);    \
                                                                                                 \
    SkColor redOnly = bitmap.getColor(25, 68);                                                   \
    REPORTER_ASSERT(reporter, colors_are_similar(redOnly, ExpectedRedOnly, kTolerance),          \
                    "Pt(25,68) BlendMode '%s': Expected red-only ~0x%08X, got 0x%08X",           \
                    SkBlendMode_Name(BlendMode), (uint32_t)ExpectedRedOnly, redOnly);            \
                                                                                                 \
    SkColor allOverlap = bitmap.getColor(40, 40);                                                \
    REPORTER_ASSERT(reporter, colors_are_similar(allOverlap, ExpectedAllOverlap, kTolerance),    \
                    "Pt(40,40) BlendMode '%s': Expected all-overlap ~0x%08X, got 0x%08X",        \
                    SkBlendMode_Name(BlendMode), (uint32_t)ExpectedAllOverlap, allOverlap);      \
}

// Basic Porter-Duff blend modes
DEFINE_LAYER_BLEND_TEST(Clear, SkBlendMode::kClear, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
                                                    0x00000000)
DEFINE_LAYER_BLEND_TEST(Src, SkBlendMode::kSrc, 0x00000000, 0x00000000, 0x00000000, 0xFFFF3300,
                                                0xFFFF3300)
DEFINE_LAYER_BLEND_TEST(Dst, SkBlendMode::kDst, 0xFF0099FF, 0x00000000, 0xFF0099FF, 0x00000000,
                                                0xFF0099FF)
DEFINE_LAYER_BLEND_TEST(SrcOver, SkBlendMode::kSrcOver, 0xFF0099FF, 0xD00000FF, 0xFF0017FF,
                                                        0xFFFF3300, 0xFFFF3300)
DEFINE_LAYER_BLEND_TEST(DstOver, SkBlendMode::kDstOver, 0xFF0099FF, 0xD00000FF, 0xFF0099FF,
                                                        0xFFFD3302, 0xFF0099FF)
DEFINE_LAYER_BLEND_TEST(SrcIn, SkBlendMode::kSrcIn, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
                                                    0xFBFF3300)
DEFINE_LAYER_BLEND_TEST(DstIn, SkBlendMode::kDstIn, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
                                                    0xFB0099FF)
DEFINE_LAYER_BLEND_TEST(SrcOut, SkBlendMode::kSrcOut, 0x00000000, 0x00000000, 0x00000000,
                                                      0xFDFF3300, 0xFFFF3300)
DEFINE_LAYER_BLEND_TEST(DstOut, SkBlendMode::kDstOut, 0xFF0099FF, 0x00000000, 0x26009AFF,
                                                      0x00000000, 0x00000000)
DEFINE_LAYER_BLEND_TEST(SrcATop, SkBlendMode::kSrcATop, 0xFF0099FF, 0x00000000, 0xFF0017FF,
                                                        0x00000000, 0xFFFF3300)
DEFINE_LAYER_BLEND_TEST(DstATop, SkBlendMode::kDstATop, 0x00000000, 0x00000000, 0x00000000,
                                                        0xFFFD3302, 0xFF0498FB)
DEFINE_LAYER_BLEND_TEST(Xor, SkBlendMode::kXor, 0xFF0099FF, 0xD00000FF, 0x26009AFF, 0xFDFF3300,
                                                0xFBFF3300)
DEFINE_LAYER_BLEND_TEST(Plus, SkBlendMode::kPlus, 0xFF0099FF, 0xD00000FF, 0xFF0099FF, 0xFFFF3302,
                                                  0xFFFFCCFF)
DEFINE_LAYER_BLEND_TEST(Modulate, SkBlendMode::kModulate, 0x00000000, 0x00000000, 0x00000000,
                                                          0x00000000, 0xFB000000)
DEFINE_LAYER_BLEND_TEST(Screen, SkBlendMode::kScreen, 0xFF0099FF, 0xD00000FF, 0xFF0099FF,
                                                      0xFFFF3302, 0xFFFFADFF)

// Advanced, non-separable blend modes
DEFINE_LAYER_BLEND_TEST(Overlay, SkBlendMode::kOverlay, 0xFF0099FF, 0xD00000FF, 0xFF0042FF,
                                                        0xFFFD3302, 0xFF0015FF)
DEFINE_LAYER_BLEND_TEST(Darken, SkBlendMode::kDarken, 0xFF0099FF, 0xD00000FF, 0xFF0017FF,
                                                      0xFFFD3300, 0xFF000200)
DEFINE_LAYER_BLEND_TEST(Lighten, SkBlendMode::kLighten, 0xFF0099FF, 0xD00000FF, 0xFF0099FF,
                                                        0xFFFF3302, 0xFFFF99FF)
DEFINE_LAYER_BLEND_TEST(ColorDodge, SkBlendMode::kColorDodge, 0xFF0099FF, 0xD00000FF, 0xFF0099FF,
                                                              0xFFFD3302, 0xFF00BFFF)
DEFINE_LAYER_BLEND_TEST(ColorBurn, SkBlendMode::kColorBurn, 0xFF0099FF, 0xD00000FF, 0xFF0017FF,
                                                            0xFFFD3302, 0xFF0000FF)
DEFINE_LAYER_BLEND_TEST(HardLight, SkBlendMode::kHardLight, 0xFF0099FF, 0xD00000FF, 0xFF0017FF,
                                                            0xFFFF3300, 0xFFFF0100)
DEFINE_LAYER_BLEND_TEST(SoftLight, SkBlendMode::kSoftLight, 0xFF0099FF, 0xD00000FF, 0xFF0065FF,
                                                            0xFFFD3302, 0xFF003AFF)
DEFINE_LAYER_BLEND_TEST(Difference, SkBlendMode::kDifference, 0xFF0099FF, 0xD00000FF, 0xFF009926,
                                                              0xFFFF3302, 0xFFFF6604)
DEFINE_LAYER_BLEND_TEST(Exclusion, SkBlendMode::kExclusion, 0xFF0099FF, 0xD00000FF, 0xFF009926,
                                                            0xFFFF3302, 0xFFFF8F04)
DEFINE_LAYER_BLEND_TEST(Multiply, SkBlendMode::kMultiply, 0xFF0099FF, 0xD00000FF, 0xFF0017FF,
                                                          0xFFFD3300, 0xFF000000)

// HSL component blend modes
DEFINE_LAYER_BLEND_TEST(Hue, SkBlendMode::kHue, 0xFF0099FF, 0xD00000FF, 0xFF566DFF, 0xFFFE3300,
                                                0xFFD05435)
DEFINE_LAYER_BLEND_TEST(Saturation, SkBlendMode::kSaturation, 0xFF0099FF, 0xD00000FF, 0xFF0099FF,
                                                              0xFFFD3302, 0xFF0099FF)
DEFINE_LAYER_BLEND_TEST(Color, SkBlendMode::kColor, 0xFF0099FF, 0xD00000FF, 0xFF566DFF, 0xFFFE3300,
                                                    0xFFFF4314)
DEFINE_LAYER_BLEND_TEST(Luminosity, SkBlendMode::kLuminosity, 0xFF0099FF, 0xD00000FF, 0xFF003659,
                                                              0xFFFE3302, 0xFF4D738D)
