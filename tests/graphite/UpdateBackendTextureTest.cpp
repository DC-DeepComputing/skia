/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/Test.h"

#include "include/core/SkColorSpace.h"
#include "include/gpu/graphite/BackendTexture.h"
#include "include/gpu/graphite/Context.h"
#include "include/gpu/graphite/Recorder.h"
#include "src/core/SkAutoPixmapStorage.h"
#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/ContextPriv.h"
#include "src/gpu/graphite/Surface_Graphite.h"
#include "tests/TestUtils.h"
#include "tools/ToolUtils.h"

using namespace skgpu;
using namespace skgpu::graphite;

namespace {
const SkISize kSize = {32, 32};
constexpr int kNumMipLevels = 6;

void check_solid_pixmap(skiatest::Reporter* reporter,
                        const SkColor4f& expected,
                        const SkPixmap& actual,
                        SkColorType ct,
                        const char* label) {
    const float tols[4] = { 0.01f, 0.01f, 0.01f, 0.01f };

    auto error = std::function<ComparePixmapsErrorReporter>(
            [reporter, ct, label, expected](int x, int y, const float diffs[4]) {
                SkASSERT(x >= 0 && y >= 0);
                ERRORF(reporter, "%s %s - expected: (%.2f, %.2f, %.2f, %.2f) actual: (%.2f, %.2f, %.2f, %.2f)",
                       ToolUtils::colortype_name(ct),
                       label,
                       expected.fR, expected.fG, expected.fB, expected.fA,
                       expected.fR-diffs[0], expected.fG-diffs[1],
                       expected.fB-diffs[2], expected.fA-diffs[3]);
            });

    CheckSolidPixels(expected, actual, tols, error);
}

BackendTexture create_backend_texture(skiatest::Reporter* reporter,
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

    return backendTex;
}

sk_sp<SkImage> wrap_backend_texture(skiatest::Reporter* reporter,
                                    Recorder* recorder,
                                    const skgpu::graphite::BackendTexture& backendTex,
                                    SkColorType ct,
                                    bool withMips) {
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
                  Context* context,
                  Recorder* recorder,
                  SkImage* image,
                  bool withMips,
                  const SkColor4f colors[6]) {
    int numLevels = withMips ? kNumMipLevels : 1;

    int drawSize = kSize.width();

    SkSamplingOptions sampling = withMips
                                 ? SkSamplingOptions(SkFilterMode::kLinear, SkMipmapMode::kNearest)
                                 : SkSamplingOptions(SkFilterMode::kLinear);

    for (int i = 0; i < numLevels; ++i) {
        SkImageInfo ii = SkImageInfo::Make({drawSize, drawSize}, kRGBA_8888_SkColorType,
                                           kPremul_SkAlphaType);

        SkAutoPixmapStorage actual;
        SkAssertResult(actual.tryAlloc(ii));
        actual.erase(SkColors::kTransparent);

        sk_sp<SkSurface> surf = SkSurface::MakeGraphite(recorder, ii, Mipmapped::kNo);
        SkCanvas* canvas = surf->getCanvas();

        canvas->drawImageRect(image, SkRect::MakeWH(drawSize, drawSize), sampling);

        // For now, we cast and call directly into Surface. Once we have a better idea of
        // what the public API for synchronous graphite readPixels we can update this call to use
        // that instead.
        if (!static_cast<skgpu::graphite::Surface*>(surf.get())->onReadPixels(context,
                                                                              recorder,
                                                                              actual,
                                                                              0,
                                                                              0)) {
            ERRORF(reporter, "readPixels failed");
            return;
        }

        GrColorType grCT = SkColorTypeToGrColorType(image->colorType());

        SkColor4f expected = ToolUtils::get_expected_color(colors[i], grCT);

        SkString str;
        str.appendf("mip-level %d", i);

        check_solid_pixmap(reporter, expected, actual, image->colorType(), str.c_str());

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

                BackendTexture backendTex = create_backend_texture(reporter, caps, recorder.get(),
                                                                   ct, withMips, renderable,
                                                                   kColors);

                sk_sp<SkImage> image = wrap_backend_texture(reporter, recorder.get(), backendTex,
                                                            ct, withMips);

                check_levels(reporter, context, recorder.get(), image.get(), withMips, kColors);

                // Still need to update backendTextures here and check again

                recorder->deleteBackendTexture(backendTex);
            }
        }
    }
}
