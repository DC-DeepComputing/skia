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
#include "include/gpu/graphite/dawn/DawnTypes.h"
#include "src/gpu/graphite/ContextPriv.h"
#include "src/gpu/graphite/Image_Graphite.h"
#include "src/gpu/graphite/Surface_Graphite.h"

#include "webgpu/webgpu_cpp.h"

using namespace skgpu::graphite;

namespace {
const SkISize kSize = {16, 16};

SkBitmap read_image_pixels(skiatest::Reporter* reporter,
                           Context* context,
                           const sk_sp<SkImage>& image) {
    Image* graphiteImage = static_cast<Image*>(image.get());

    SkBitmap bitmap;
    SkPixmap pixmap;
    bitmap.allocPixels(image->imageInfo());
    SkAssertResult(bitmap.peekPixels(&pixmap));
    if (!context->priv().readPixels(
                pixmap, graphiteImage->textureProxyView().proxy(), image->imageInfo(), 0, 0)) {
        ERRORF(reporter, "readPixels failed");
        return {};
    }

    return bitmap;
}

void do_texture_copy_test(skiatest::Reporter* reporter,
                          Context* context,
                          Recorder* surfaceRecorder,
                          BackendTexture& beTexture) {
    sk_sp<SkSurface> surface = SkSurface::MakeGraphiteFromBackendTexture(surfaceRecorder,
                                                                         beTexture,
                                                                         kRGBA_8888_SkColorType,
                                                                         SkColorSpace::MakeSRGB(),
                                                                         /*props=*/nullptr);

    Surface* graphiteSurface = static_cast<Surface*>(surface.get());
    const TextureInfo& textureInfo = graphiteSurface->backingTextureProxy()->textureInfo();

    // Flush the initial clear added by MakeGraphite.
    std::unique_ptr<skgpu::graphite::Recording> surfaceRecording = surfaceRecorder->snap();
    context->insertRecording({surfaceRecording.get()});

    // Clear the whole surface
    const SkImageInfo recordingWholeImageInfo = surface->imageInfo();
    std::unique_ptr<Recorder> fullClearRecorder = context->makeRecorder();
    SkCanvas* fullCanvas =
            fullClearRecorder->makeDeferredCanvas(recordingWholeImageInfo, textureInfo);
    fullCanvas->clear(SkColors::kRed);
    std::unique_ptr<Recording> fullClearRecording = fullClearRecorder->snap();

    // Clear a subregion
    const SkImageInfo recordingSubImageInfo =
            recordingWholeImageInfo.makeDimensions(SkISize::Make(8, 16));
    std::unique_ptr<Recorder> subClearRecorder = context->makeRecorder();
    SkCanvas* subCanvas = subClearRecorder->makeDeferredCanvas(recordingSubImageInfo, textureInfo);
    subCanvas->clear(SkColors::kBlue);
    std::unique_ptr<Recording> subClearRecording = subClearRecorder->snap();

    // Play back recordings.
    context->insertRecording({fullClearRecording.get(), surface.get()});
    context->insertRecording({subClearRecording.get(), surface.get()});
    context->submit();

    // Take a snapshot, the underlying layer should issue a copy task.
    auto snapshotImage = surface->makeImageSnapshot(SkIRect::MakeXYWH(4, 4, 8, 8));
    std::unique_ptr<Recording> snapshotRecording = surface->recorder()->snap();
    context->insertRecording({snapshotRecording.get()});

    SkBitmap bitmap = read_image_pixels(reporter, context, snapshotImage);
    SkPixmap pixmap;
    SkAssertResult(bitmap.peekPixels(&pixmap));

    // Verify that left region is blue, right region is red
    REPORTER_ASSERT(reporter, pixmap.getColor4f(3, 0) == SkColors::kBlue);
    REPORTER_ASSERT(reporter, pixmap.getColor4f(4, 0) == SkColors::kRed);

    surfaceRecorder->deleteBackendTexture(beTexture);
}
}  // namespace

// Test that Dawn backend's texture copy works
DEF_GRAPHITE_TEST_FOR_DAWN_CONTEXT(DawnBackendTextureCopyTest, reporter, context) {
    std::unique_ptr<Recorder> surfaceRecorder = context->makeRecorder();

    DawnTextureInfo textureInfo;
    textureInfo.fSampleCount = 1;
    textureInfo.fMipmapped = Mipmapped::kNo;
    textureInfo.fFormat = wgpu::TextureFormat::RGBA8Unorm;
    textureInfo.fUsage = wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::CopySrc |
                         wgpu::TextureUsage::TextureBinding;

    auto beTexture = surfaceRecorder->createBackendTexture(kSize, textureInfo);
    REPORTER_ASSERT(reporter, beTexture.isValid());

    do_texture_copy_test(reporter, context, surfaceRecorder.get(), beTexture);
}

// Test that Dawn backend's texture copy works with wgpu::TextureView only.
// This should force Dawn backend to use fullscreen drawing to do the copy.
DEF_GRAPHITE_TEST_FOR_DAWN_CONTEXT(DawnBackendTextureViewCopyTest, reporter, context) {
    std::unique_ptr<Recorder> surfaceRecorder = context->makeRecorder();

    DawnTextureInfo textureInfo;
    textureInfo.fSampleCount = 1;
    textureInfo.fMipmapped = Mipmapped::kNo;
    textureInfo.fFormat = wgpu::TextureFormat::RGBA8Unorm;
    textureInfo.fUsage = wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::CopySrc |
                         wgpu::TextureUsage::TextureBinding;

    auto beTexture = surfaceRecorder->createBackendTexture(kSize, textureInfo);
    REPORTER_ASSERT(reporter, beTexture.isValid());
    REPORTER_ASSERT(reporter, beTexture.getDawnTexture());

    wgpu::TextureView textureView = beTexture.getDawnTexture().CreateView();

    // Create backend texture with view only.
    skgpu::graphite::BackendTexture beTextureView(kSize, textureInfo, textureView);

    do_texture_copy_test(reporter, context, surfaceRecorder.get(), beTextureView);

    surfaceRecorder->deleteBackendTexture(beTextureView);
}
