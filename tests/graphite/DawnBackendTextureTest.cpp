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
#include "include/core/SkPixmap.h"
#include "include/gpu/graphite/BackendTexture.h"
#include "include/gpu/graphite/Context.h"
#include "include/gpu/graphite/Recorder.h"
#include "include/gpu/graphite/Surface.h"
#include "include/gpu/graphite/dawn/DawnTypes.h"
#include "src/gpu/graphite/Surface_Graphite.h"

#include "webgpu/webgpu_cpp.h"  // NO_G3_REWRITE

using namespace skgpu::graphite;

namespace {
const SkISize kSize = {16, 16};
}

DEF_GRAPHITE_TEST_FOR_DAWN_CONTEXT(DawnBackendTextureSimpleCreationTest,
                                   reporter,
                                   context,
                                   testContext) {
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
DEF_GRAPHITE_TEST_FOR_DAWN_CONTEXT(DawnBackendTextureCopyVariableTest,
                                   reporter,
                                   context,
                                   testContext) {
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

// Test that BackendTexture can be deleted while there are on going gpu works.
DEF_GRAPHITE_TEST_FOR_DAWN_CONTEXT(DawnBackendTextureDeleteBeforePendingGPUWorks,
                                   reporter,
                                   context,
                                   testContext) {
    auto recorder = context->makeRecorder();

    DawnTextureInfo textureInfo;
    textureInfo.fSampleCount = 1;
    textureInfo.fMipmapped = skgpu::Mipmapped::kNo;
    textureInfo.fFormat = wgpu::TextureFormat::RGBA8Unorm;
    textureInfo.fUsage = wgpu::TextureUsage::CopySrc | wgpu::TextureUsage::RenderAttachment;

    BackendTexture beTexture = recorder->createBackendTexture(kSize, textureInfo);
    REPORTER_ASSERT(reporter, beTexture.isValid());

    // Create surface from backend texture.
    const SkImageInfo surfaceImageInfo = SkImageInfo::Make(
            kSize, SkColorType::kRGBA_8888_SkColorType, SkAlphaType::kPremul_SkAlphaType);

    SkSurfaceProps surfaceProps;
    auto surface = SkSurfaces::WrapBackendTexture(recorder.get(),
                                                  beTexture,
                                                  kRGBA_8888_SkColorType,
                                                  surfaceImageInfo.refColorSpace(),
                                                  &surfaceProps);

    // Delete the backend texture.
    recorder->deleteBackendTexture(beTexture);

    // Clear entire surface to red
    SkCanvas* surfaceCanvas = surface->getCanvas();
    surfaceCanvas->clear(SkColors::kRed);
    std::unique_ptr<Recording> recording = recorder->snap();
    // Flush the clearing
    context->insertRecording({recording.get()});

    // Read pixels.
    SkBitmap bitmap;
    bitmap.allocPixels(surfaceImageInfo);
    if (!surface->readPixels(bitmap, 0, 0)) {
        ERRORF(reporter, "readPixels failed");
        return;
    }

    // Verify recording was replayed.
    REPORTER_ASSERT(reporter, bitmap.getColor4f(8, 0) == SkColors::kRed);
    REPORTER_ASSERT(reporter, bitmap.getColor4f(0, 8) == SkColors::kRed);
}
