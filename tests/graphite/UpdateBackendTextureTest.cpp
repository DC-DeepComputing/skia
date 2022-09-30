/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/Test.h"

#include "include/gpu/graphite/BackendTexture.h"
#include "include/gpu/graphite/Context.h"
#include "include/gpu/graphite/Recorder.h"
#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/ContextPriv.h"
//#include "include/gpu/GpuTypes.h"
//#include "src/gpu/graphite/ResourceTypes.h"

#include "include/core/SkColorSpace.h"
//#include "include/core/SkSurface.h"

#include "tools/ToolUtils.h"

using namespace skgpu;
using namespace skgpu::graphite;

namespace {
const SkISize kSize = {32, 32};
constexpr int kNumMipLevels = 6;

sk_sp<SkImage> create_backend_backed_image(skiatest::Reporter* reporter,
                                           const Caps* caps,
                                           Recorder* recorder,
                                           SkColorType ct,
                                           bool withMips,
                                           Renderable renderable,
                                           const SkColor4f colors[6]) {

    SkPixmap pixmaps[6];
    std::unique_ptr<char[]> memForPixmaps;

    int numMipLevels = ToolUtils::make_pixmaps(ct, withMips, colors, pixmaps,
                                               &memForPixmaps);
    SkASSERT(numMipLevels == 1 || numMipLevels == kNumMipLevels);
    SkASSERT(kSize == pixmaps[0].dimensions());

    TextureInfo info = caps->getDefaultSampledTextureInfo(ct,
                                                          numMipLevels,
                                                          Protected::kNo,
                                                          renderable);

    BackendTexture backendTex = recorder->createBackendTexture(kSize, info);
    REPORTER_ASSERT(reporter, backendTex.isValid());

    recorder->updateBackendTexture(backendTex, pixmaps, numMipLevels);

    sk_sp<SkImage> image = SkImage::MakeGraphiteFromBackendTexture(recorder,
                                                                   backendTex,
                                                                   ct,
                                                                   kPremul_SkAlphaType,
                                                                   /* colorSpace= */ nullptr);
    REPORTER_ASSERT(reporter, image);
    REPORTER_ASSERT(reporter, image->hasMipmaps() == withMips);

    return image;
}

void check_levels(skiatest::Reporter* reporter,
                  Recorder* recorder,
                  SkImage* image,
                  bool withMips,
                  const SkColor4f colors[6]) {
    int numLevels = withMips ? kNumMipLevels : 1;

    int drawSize = kSize.width();

    SkSamplingOptions sampling{SkFilterMode::kNearest, SkMipmapMode::kNearest};

    for (int i = 0; i < numLevels; ++i) {
        SkImageInfo ii = SkImageInfo::Make({drawSize, drawSize}, kRGBA_8888_SkColorType,
                                           kPremul_SkAlphaType);

        sk_sp<SkSurface> surf = SkSurface::MakeGraphite(recorder, ii, Mipmapped::kNo);
        SkCanvas* canvas = surf->getCanvas();

        canvas->drawImageRect(image, SkRect::MakeWH(drawSize, drawSize), sampling);

        drawSize /= 2;
    }
}

} // anonymous namespace

DEF_GRAPHITE_TEST_FOR_CONTEXTS(UpdateImageBackendTextureTest, reporter, context) {
    const Caps* caps = context->priv().caps();
    std::unique_ptr<Recorder> recorder = context->makeRecorder();

    constexpr SkColor4f kColors[6] = {
            { 1.0f, 0.0f, 0.0f, 1.0f }, // R
            { 0.0f, 1.0f, 0.0f, 0.9f }, // G
            { 0.0f, 0.0f, 1.0f, 0.7f }, // B
            { 0.0f, 1.0f, 1.0f, 0.5f }, // C
            { 1.0f, 0.0f, 1.0f, 0.3f }, // M
            { 1.0f, 1.0f, 0.0f, 0.2f }, // Y
    };

    // TODO: test more than just RGBA8
    for (SkColorType ct : { kRGBA_8888_SkColorType }) {
        for (bool withMips : { true, false }) {
            for (Renderable renderable : { Renderable::kYes, Renderable::kNo }) {

                sk_sp<SkImage> image = create_backend_backed_image(reporter, caps, recorder.get(),
                                                                   ct, withMips, renderable,
                                                                   kColors);

                check_levels(reporter, recorder.get(), image.get(), withMips, kColors);

//                recorder->deleteBackendTexture(backendTex);
            }
        }
    }
}
