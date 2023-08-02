/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/Test.h"

#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkPath.h"
#include "include/core/SkPixmap.h"
#include "include/core/SkSurface.h"
#include "include/gpu/graphite/BackendTexture.h"
#include "include/gpu/graphite/Context.h"
#include "include/gpu/graphite/Recorder.h"
#include "include/gpu/graphite/Surface.h"
#include "include/gpu/graphite/dawn/DawnTypes.h"

#include "webgpu/webgpu_cpp.h"

using namespace skgpu::graphite;

namespace {
const SkISize kSize = {16, 16};
}

DEF_GRAPHITE_TEST_FOR_DAWN_CONTEXT(DawnBackendTextureSimpleCreationTest, reporter, context) {
    auto recorder = context->makeRecorder();

    DawnTextureInfo textureInfo;
    textureInfo.fSampleCount = 1;
    textureInfo.fMipmapped = skgpu::Mipmapped::kNo;
    textureInfo.fFormat = wgpu::TextureFormat::RGBA8Unorm;
    textureInfo.fUsage = wgpu::TextureUsage::TextureBinding;

    auto beTexture = recorder->createBackendTexture(kSize, textureInfo);
    REPORTER_ASSERT(reporter, beTexture.isValid());
    recorder->deleteBackendTexture(beTexture);

    // It should also pass if we set the usage to be a render target
    textureInfo.fUsage |= wgpu::TextureUsage::RenderAttachment;
    beTexture = recorder->createBackendTexture(kSize, textureInfo);
    REPORTER_ASSERT(reporter, beTexture.isValid());
    recorder->deleteBackendTexture(beTexture);
}

// Test that copying BackendTexture variables works.
DEF_GRAPHITE_TEST_FOR_DAWN_CONTEXT(DawnBackendTextureCopyVariableTest, reporter, context) {
    auto recorder = context->makeRecorder();

    DawnTextureInfo textureInfo;
    textureInfo.fSampleCount = 1;
    textureInfo.fMipmapped = skgpu::Mipmapped::kNo;
    textureInfo.fFormat = wgpu::TextureFormat::RGBA8Unorm;
    textureInfo.fUsage = wgpu::TextureUsage::TextureBinding;

    BackendTexture beTexture = recorder->createBackendTexture(kSize, textureInfo);
    REPORTER_ASSERT(reporter, beTexture.isValid());

    BackendTexture beTexture2;
    REPORTER_ASSERT(reporter, beTexture2 != beTexture);
    REPORTER_ASSERT(reporter, beTexture2.getDawnTexturePtr() == nullptr);
    REPORTER_ASSERT(reporter, beTexture2.getDawnTextureViewPtr() == nullptr);

    beTexture2 = beTexture;
    REPORTER_ASSERT(reporter, beTexture2 == beTexture);
    REPORTER_ASSERT(reporter, beTexture2.getDawnTexturePtr() != nullptr);
    REPORTER_ASSERT(reporter, beTexture2.getDawnTexturePtr() == beTexture.getDawnTexturePtr());

    recorder->deleteBackendTexture(beTexture);
}

// Tests that a drawing with MSAA will have contents retained between recordings.
// This is for testing MSAA load from resolve feature.
namespace {
void TestMultisampleRetainData(Recorder* recorder,
                               const skgpu::graphite::BackendTexture& backendTexture,
                               skiatest::Reporter* reporter,
                               skgpu::graphite::Context* context) {
    auto surface = SkSurfaces::WrapBackendTexture(
            recorder, backendTexture, kRGBA_8888_SkColorType, nullptr, nullptr);

    // Clear entire surface to red
    SkCanvas* surfaceCanvas = surface->getCanvas();
    surfaceCanvas->clear(SkColors::kRed);
    std::unique_ptr<Recording> surfaceRecording = recorder->snap();
    // Flush the clearing
    context->insertRecording({surfaceRecording.get()});

    // Draw a blue path. The old red background should be retained between recordings.
    SkPaint paint;
    paint.setStrokeWidth(3);
    paint.setColor(SkColors::kBlue);
    paint.setStyle(SkPaint::Style::kStroke_Style);

    SkPath path;
    constexpr int kPathPoints[][2] = {
            {3, 2},
            {3, 4},
            {6, 8},
            {3, 15},
    };

    for (size_t i = 0; i < std::size(kPathPoints); ++i) {
        path.lineTo(kPathPoints[i][0], kPathPoints[i][1]);
    }

    surfaceCanvas->drawPath(path, paint);

    std::unique_ptr<Recording> surfaceRecording2 = recorder->snap();
    // Play back recording.
    context->insertRecording({surfaceRecording2.get()});

    // Read pixels.
    SkBitmap bitmap;
    bitmap.allocPixels(surface->imageInfo());
    if (!surface->readPixels(bitmap, 0, 0)) {
        ERRORF(reporter, "readPixels failed");
        return;
    }

    // Verify recording was replayed.
    REPORTER_ASSERT(reporter, bitmap.getColor4f(8, 0) == SkColors::kRed);
    REPORTER_ASSERT(reporter, bitmap.getColor4f(0, 8) == SkColors::kRed);
    REPORTER_ASSERT(reporter, bitmap.getColor4f(15, 14) == SkColors::kRed);

    // Verify points on the path have blue color. We don't verify last point because it is on the
    // edge of the path thus might have blurry color.
    for (size_t i = 0; i < std::size(kPathPoints) - 1; ++i) {
        REPORTER_ASSERT(reporter,
                        bitmap.getColor4f(kPathPoints[i][0], kPathPoints[i][1]) == SkColors::kBlue);
    }
}
}  // namespace

DEF_GRAPHITE_TEST_FOR_DAWN_CONTEXT(DawnBackendTextureMultisampleRetainTest, reporter, context) {
    auto recorder = context->makeRecorder();

    // Test a texture with TextureBinding usage.
    DawnTextureInfo textureInfo;
    textureInfo.fSampleCount = 1;
    textureInfo.fMipmapped = skgpu::Mipmapped::kNo;
    textureInfo.fFormat = wgpu::TextureFormat::RGBA8Unorm;
    textureInfo.fUsage = wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::CopySrc |
                         wgpu::TextureUsage::RenderAttachment;

    auto backendTexture1 = recorder->createBackendTexture(kSize, textureInfo);

    TestMultisampleRetainData(recorder.get(), backendTexture1, reporter, context);

    // Test a texture without TextureBinding usage.
    textureInfo.fUsage = wgpu::TextureUsage::CopySrc | wgpu::TextureUsage::RenderAttachment;

    auto backendTexture2 = recorder->createBackendTexture(kSize, textureInfo);

    TestMultisampleRetainData(recorder.get(), backendTexture2, reporter, context);
}
