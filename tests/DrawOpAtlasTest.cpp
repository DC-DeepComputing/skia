/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkBitmap.h"
#include "include/core/SkColor.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkFont.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSize.h"
#include "include/core/SkTypes.h"
#include "include/gpu/GrBackendSurface.h"
#include "include/gpu/GrDirectContext.h"
#include "include/private/gpu/ganesh/GrTypesPriv.h"
#include "src/core/SkIPoint16.h"
#include "src/gpu/ganesh/GrCaps.h"
#include "src/gpu/ganesh/GrDeferredUpload.h"
#include "src/gpu/ganesh/GrDirectContextPriv.h"
#include "src/gpu/ganesh/GrDrawOpAtlas.h"
#include "src/gpu/ganesh/GrDrawingManager.h"
#include "src/gpu/ganesh/GrMemoryPool.h"
#include "src/gpu/ganesh/GrOnFlushResourceProvider.h"
#include "src/gpu/ganesh/GrOpFlushState.h"
#include "src/gpu/ganesh/GrTextureProxy.h"
#include "src/gpu/ganesh/GrXferProcessor.h"
#include "src/gpu/ganesh/ops/AtlasTextOp.h"
#include "src/gpu/ganesh/ops/GrDrawOp.h"
#include "src/gpu/ganesh/ops/GrOp.h"
#include "src/gpu/ganesh/text/GrAtlasManager.h"
#include "tests/Test.h"
#include "tools/gpu/GrContextFactory.h"

#include <memory>
#include <utility>
#include "src/core/SkStrikeCache.h"
#include "src/text/gpu/Glyph.h"

using Glyph = sktext::gpu::Glyph;
using MaskFormat = skgpu::MaskFormat;

class GrResourceProvider;

static const int kNumPlots = 2;
static const int kPlotSize = 32;
static const int kAtlasSize = kNumPlots * kPlotSize;

int GrDrawOpAtlas::numAllocated_TestingOnly() const {
    int count = 0;
    for (uint32_t i = 0; i < this->maxPages(); ++i) {
        if (fViews[i].proxy()->isInstantiated()) {
            ++count;
        }
    }

    return count;
}

void GrAtlasManager::setMaxPages_TestingOnly(uint32_t maxPages) {
    for (int i = 0; i < skgpu::kMaskFormatCount; i++) {
        if (fAtlases[i]) {
            fAtlases[i]->setMaxPages_TestingOnly(maxPages);
        }
    }
}

void GrDrawOpAtlas::setMaxPages_TestingOnly(uint32_t maxPages) {
    SkASSERT(!fNumActivePages);

    fMaxPages = maxPages;
}

class AssertOnEvict : public skgpu::PlotEvictionCallback {
public:
    void evict(skgpu::PlotLocator) override {
        SkASSERT(0); // The unit test shouldn't exercise this code path
    }
};

static void check(skiatest::Reporter* r, GrDrawOpAtlas* atlas,
                  uint32_t expectedActive, uint32_t expectedMax, int expectedAlloced) {
    REPORTER_ASSERT(r, expectedActive == atlas->numActivePages());
    REPORTER_ASSERT(r, expectedMax == atlas->maxPages());
    REPORTER_ASSERT(r, expectedAlloced == atlas->numAllocated_TestingOnly());
}

class TestingUploadTarget : public GrDeferredUploadTarget {
public:
    TestingUploadTarget() { }

    const GrTokenTracker* tokenTracker() final { return &fTokenTracker; }
    GrTokenTracker* writeableTokenTracker() { return &fTokenTracker; }

    GrDeferredUploadToken addInlineUpload(GrDeferredTextureUploadFn&&) final {
        SkASSERT(0); // this test shouldn't invoke this code path
        return fTokenTracker.nextDrawToken();
    }

    GrDeferredUploadToken addASAPUpload(GrDeferredTextureUploadFn&& upload) final {
        return fTokenTracker.nextTokenToFlush();
    }

    void issueDrawToken() { fTokenTracker.issueDrawToken(); }
    void flushToken() { fTokenTracker.flushToken(); }

private:
    GrTokenTracker fTokenTracker;

    using INHERITED = GrDeferredUploadTarget;
};

static bool fill_plot(GrDrawOpAtlas* atlas,
                      GrResourceProvider* resourceProvider,
                      GrDeferredUploadTarget* target,
                      skgpu::AtlasLocator* atlasLocator,
                      int alpha,
                      int plotSize = kPlotSize,
                      void** plot = nullptr) {
    SkImageInfo ii = SkImageInfo::MakeA8(plotSize, plotSize);

    SkBitmap data;
    data.allocPixels(ii);
    data.eraseARGB(alpha, 0, 0, 0);
    if (plot != nullptr) {
        *plot = data.getAddr(0, 0);
    }

    GrDrawOpAtlas::ErrorCode code;
    code = atlas->addToAtlas(resourceProvider, target, plotSize, plotSize,
                             data.getAddr(0, 0), atlasLocator);
    return GrDrawOpAtlas::ErrorCode::kSucceeded == code;
}


// This is a basic DrawOpAtlas test. It simply verifies that multitexture atlases correctly
// add and remove pages. Note that this is simulating flush-time behavior.
DEF_GPUTEST_FOR_RENDERING_CONTEXTS(BasicDrawOpAtlas, reporter, ctxInfo) {
    auto context = ctxInfo.directContext();
    auto proxyProvider = context->priv().proxyProvider();
    auto resourceProvider = context->priv().resourceProvider();
    auto drawingManager = context->priv().drawingManager();
    const GrCaps* caps = context->priv().caps();

    GrOnFlushResourceProvider onFlushResourceProvider(drawingManager);
    TestingUploadTarget uploadTarget;

    GrColorType atlasColorType = GrColorType::kAlpha_8;
    GrBackendFormat format = caps->getDefaultBackendFormat(atlasColorType,
                                                           GrRenderable::kNo);

    AssertOnEvict evictor;
    skgpu::AtlasGenerationCounter counter;

    std::unique_ptr<GrDrawOpAtlas> atlas = GrDrawOpAtlas::Make(
                                                proxyProvider,
                                                format,
                                                GrColorTypeToSkColorType(atlasColorType),
                                                GrColorTypeBytesPerPixel(atlasColorType),
                                                kAtlasSize, kAtlasSize,
                                                kAtlasSize/kNumPlots, kAtlasSize/kNumPlots,
                                                &counter,
                                                GrDrawOpAtlas::AllowMultitexturing::kYes,
                                                &evictor);
    check(reporter, atlas.get(), 0, 4, 0);

    // Fill up the first level
    skgpu::AtlasLocator atlasLocators[kNumPlots * kNumPlots];
    for (int i = 0; i < kNumPlots * kNumPlots; ++i) {
        bool result = fill_plot(
                atlas.get(), resourceProvider, &uploadTarget, &atlasLocators[i], i * 32);
        REPORTER_ASSERT(reporter, result);
        check(reporter, atlas.get(), 1, 4, 1);
    }

    atlas->instantiate(&onFlushResourceProvider);
    check(reporter, atlas.get(), 1, 4, 1);

    // Force allocation of a second level
    skgpu::AtlasLocator atlasLocator;
    bool result = fill_plot(atlas.get(), resourceProvider, &uploadTarget, &atlasLocator, 4 * 32);
    REPORTER_ASSERT(reporter, result);
    check(reporter, atlas.get(), 2, 4, 2);

    // Simulate a lot of draws using only the first plot. The last texture should be compacted.
    for (int i = 0; i < 512; ++i) {
        atlas->setLastUseToken(atlasLocators[0], uploadTarget.tokenTracker()->nextDrawToken());
        uploadTarget.issueDrawToken();
        uploadTarget.flushToken();
        atlas->compact(uploadTarget.tokenTracker()->nextTokenToFlush());
    }

    check(reporter, atlas.get(), 1, 4, 1);
}

#if SK_GPU_V1
#include "src/gpu/ganesh/v1/SurfaceDrawContext_v1.h"

// This test verifies that the AtlasTextOp::onPrepare method correctly handles a failure
// when allocating an atlas page.
DEF_GPUTEST_FOR_RENDERING_CONTEXTS(GrAtlasTextOpPreparation, reporter, ctxInfo) {

    auto dContext = ctxInfo.directContext();

    auto gpu = dContext->priv().getGpu();
    auto resourceProvider = dContext->priv().resourceProvider();

    auto sdc = skgpu::v1::SurfaceDrawContext::Make(dContext, GrColorType::kRGBA_8888, nullptr,
                                                   SkBackingFit::kApprox, {32, 32},
                                                   SkSurfaceProps());

    SkPaint paint;
    paint.setColor(SK_ColorRED);

    SkFont font;
    font.setEdging(SkFont::Edging::kAlias);

    const char* text = "a";
    SkMatrixProvider matrixProvider(SkMatrix::I());

    GrOp::Owner op = skgpu::v1::AtlasTextOp::CreateOpTestingOnly(sdc.get(), paint,
                                                                 font, matrixProvider,
                                                                 text, 16, 16);
    if (!op) {
        return;
    }

    auto atlasTextOp = (skgpu::v1::AtlasTextOp*)op.get();
    atlasTextOp->finalize(*dContext->priv().caps(), nullptr, GrClampType::kAuto);

    TestingUploadTarget uploadTarget;

    GrOpFlushState flushState(gpu, resourceProvider, uploadTarget.writeableTokenTracker());

    GrSurfaceProxyView surfaceView = sdc->writeSurfaceView();
    GrOpFlushState::OpArgs opArgs(op.get(),
                                  surfaceView,
                                  false /*usesMSAASurface*/,
                                  nullptr,
                                  GrDstProxyView(),
                                  GrXferBarrierFlags::kNone,
                                  GrLoadOp::kLoad);

    // Modify the atlas manager so it can't allocate any pages. This will force a failure
    // in the preparation of the text op
    auto atlasManager = dContext->priv().getAtlasManager();
    unsigned int numProxies;
    atlasManager->getViews(MaskFormat::kA8, &numProxies);
    atlasManager->setMaxPages_TestingOnly(0);

    flushState.setOpArgs(&opArgs);
    op->prepare(&flushState);
    flushState.setOpArgs(nullptr);
}
#endif // SK_GPU_V1

void test_atlas_config(skiatest::Reporter* reporter, int maxTextureSize, size_t maxBytes,
                       MaskFormat maskFormat, SkISize expectedDimensions,
                       SkISize expectedPlotDimensions) {
    GrDrawOpAtlasConfig config(maxTextureSize, maxBytes);
    REPORTER_ASSERT(reporter, config.atlasDimensions(maskFormat) == expectedDimensions);
    REPORTER_ASSERT(reporter, config.plotDimensions(maskFormat) == expectedPlotDimensions);
}

DEF_GPUTEST(GrDrawOpAtlasConfig_Basic, reporter, options) {
    // 1/4 MB
    test_atlas_config(reporter, 65536, 256 * 1024, MaskFormat::kARGB,
                      { 256, 256 }, { 256, 256 });
    test_atlas_config(reporter, 65536, 256 * 1024, MaskFormat::kA8,
                      { 512, 512 }, { 256, 256 });
    // 1/2 MB
    test_atlas_config(reporter, 65536, 512 * 1024, MaskFormat::kARGB,
                      { 512, 256 }, { 256, 256 });
    test_atlas_config(reporter, 65536, 512 * 1024, MaskFormat::kA8,
                      { 1024, 512 }, { 256, 256 });
    // 1 MB
    test_atlas_config(reporter, 65536, 1024 * 1024, MaskFormat::kARGB,
                      { 512, 512 }, { 256, 256 });
    test_atlas_config(reporter, 65536, 1024 * 1024, MaskFormat::kA8,
                      { 1024, 1024 }, { 256, 256 });
    // 2 MB
    test_atlas_config(reporter, 65536, 2 * 1024 * 1024, MaskFormat::kARGB,
                      { 1024, 512 }, { 256, 256 });
    test_atlas_config(reporter, 65536, 2 * 1024 * 1024, MaskFormat::kA8,
                      { 2048, 1024 }, { 512, 256 });
    // 4 MB
    test_atlas_config(reporter, 65536, 4 * 1024 * 1024, MaskFormat::kARGB,
                      { 1024, 1024 }, { 256, 256 });
    test_atlas_config(reporter, 65536, 4 * 1024 * 1024, MaskFormat::kA8,
                      { 2048, 2048 }, { 512, 512 });
    // 8 MB
    test_atlas_config(reporter, 65536, 8 * 1024 * 1024, MaskFormat::kARGB,
                      { 2048, 1024 }, { 256, 256 });
    test_atlas_config(reporter, 65536, 8 * 1024 * 1024, MaskFormat::kA8,
                      { 2048, 2048 }, { 512, 512 });
    // 16 MB (should be same as 8 MB)
    test_atlas_config(reporter, 65536, 16 * 1024 * 1024, MaskFormat::kARGB,
                      { 2048, 1024 }, { 256, 256 });
    test_atlas_config(reporter, 65536, 16 * 1024 * 1024, MaskFormat::kA8,
                      { 2048, 2048 }, { 512, 512 });

    // 4MB, restricted texture size
    test_atlas_config(reporter, 1024, 8 * 1024 * 1024, MaskFormat::kARGB,
                      { 1024, 1024 }, { 256, 256 });
    test_atlas_config(reporter, 1024, 8 * 1024 * 1024, MaskFormat::kA8,
                      { 1024, 1024 }, { 256, 256 });

    // 3 MB (should be same as 2 MB)
    test_atlas_config(reporter, 65536, 3 * 1024 * 1024, MaskFormat::kARGB,
                      { 1024, 512 }, { 256, 256 });
    test_atlas_config(reporter, 65536, 3 * 1024 * 1024, MaskFormat::kA8,
                      { 2048, 1024 }, { 512, 256 });

    // minimum size
    test_atlas_config(reporter, 65536, 0, MaskFormat::kARGB,
                      { 256, 256 }, { 256, 256 });
    test_atlas_config(reporter, 65536, 0, MaskFormat::kA8,
                      { 512, 512 }, { 256, 256 });
}

static void use_padding_options(GrContextOptions* options) {
    options->fSupportBilerpFromGlyphAtlas = true;
}

static void do_not_use_padding_options(GrContextOptions* options) {
    options->fSupportBilerpFromGlyphAtlas = false;
}

bool checkPaddingForGlyphs(std::vector<SkGlyph> skGlyphs,
                           std::vector<Glyph> grGlyphs,
                           std::vector<int> padding,
                           int insetPadding) {
    // grGlyphs should be placed in 2 rows and 2 columns (all 4 of them)
    // grGlyph size should match skGlyph size
    // distance between grGlyph1 and grGlyph2 should be paddingExpected1 + paddingExpected2
    // distance between grGlyph and the plot side should be paddingExpected on that size
    int i = 0;
    for (auto& grGlyph : grGlyphs) {
        // Check all glyph sizes
        auto& skGlyph = skGlyphs[i++];
        if (skGlyph.width() != grGlyph.fAtlasLocator.width() + insetPadding * 2 ||
            skGlyph.height() != grGlyph.fAtlasLocator.height() + insetPadding * 2) {
            return false;
        }
    }

    if (skGlyphs.size() != 4) {
        return false;
    }
    auto& s00 = skGlyphs[0];
    auto& s01 = skGlyphs[1];
    auto& s10 = skGlyphs[2];
    // auto& s11 = skGlyphs[3];

    if (grGlyphs.size() != 4) {
        return false;
    }
    auto& g00 = grGlyphs[0].fAtlasLocator;
    auto& g01 = grGlyphs[1].fAtlasLocator;
    auto& g10 = grGlyphs[2].fAtlasLocator;
    auto& g11 = grGlyphs[3].fAtlasLocator;

    // Check the top left glyph placement
    if (g00.topLeft().fX != padding[0] || g00.topLeft().fY != padding[1]) {
        return false;
    }

    // Check the top right glyph placement
    if (g01.topLeft().fX != padding[0] * 2 + padding[2] + s00.width() - insetPadding * 2 ||
        g01.topLeft().fY != padding[1]) {
        return false;
    }

    // Check the bottom left glyph placement
    if (g10.topLeft().fX != padding[0] ||
        g10.topLeft().fY != padding[1] * 2 + padding[3] + s00.height() - insetPadding * 2) {
        return false;
    }

    // Check the bottom left glyph placement
    if (g11.topLeft().fX != padding[0] * 2 + padding[2] + s10.width() - insetPadding * 2 ||
        g11.topLeft().fY != padding[1] * 2 + padding[3] + s01.height() - insetPadding * 2) {
        return false;
    }

    return true;
}

DEF_GPUTEST_FOR_CONTEXTS(GrAtlasManager_withoutSlug,
                         sk_gpu_test::GrContextFactory::IsRenderingContext,
                         reporter,
                         ctxInfo,
                         do_not_use_padding_options) {
    TestingUploadTarget uploadTarget;
    auto dContext = ctxInfo.directContext();
    auto atlasManager = dContext->priv().getAtlasManager();
    auto resourceProvider = dContext->priv().resourceProvider();

    atlasManager->setAtlasDimensionsToMinimum_ForTesting();

    SkFont defaultFont(SkTypeface::MakeFromName("Roboto-Regular", SkFontStyle()), 152);
    SkStrikeSpec strikeSpec = SkStrikeSpec::MakeWithNoDevice(defaultFont);
    sk_sp<SkStrike> strike = strikeSpec.findOrCreateStrike();
    SkArenaAlloc alloc(1 << 12);

    std::vector<SkGlyph> skGlyphs;
    std::vector<Glyph> grGlyphs;
    auto draw2x2 = [&](int srcPadding) {
        atlasManager->freeAll();
        skGlyphs.clear();
        grGlyphs.clear();
        unsigned int numProxies;
        atlasManager->getViews(MaskFormat::kA8, &numProxies);
        atlasManager->setMaxPages_TestingOnly(1);

        skgpu::PlotLocator plotLocator;
        for (int i = 0; i < 4; ++i) {
            char c = "ABCDEFGHIJKLMNOPQRSTUVWXYZ01234567890"[i];
            SkPackedGlyphID id(defaultFont.unicharToGlyph(c));
            SkGlyph skGlyph = strike->getScalerContext()->makeGlyph(id, &alloc);
            skGlyph.setImage(&alloc, strike->getScalerContext());
            SkPackedGlyphID glyphID;
            Glyph grGlyph(glyphID);
            auto errorCode = atlasManager->addGlyphToAtlas(
                    skGlyph, &grGlyph, srcPadding, resourceProvider, &uploadTarget);
            REPORTER_ASSERT(reporter, errorCode == GrDrawOpAtlas::ErrorCode::kSucceeded);
            skGlyphs.emplace_back(skGlyph);
            grGlyphs.emplace_back(grGlyph);
            if (i == 0) {
                plotLocator = grGlyph.fAtlasLocator.plotLocator();
            } else {
                REPORTER_ASSERT(reporter, plotLocator == grGlyph.fAtlasLocator.plotLocator());
            }
        }
    };

    // Direct mask: no padding at all
    draw2x2(/*srcPadding=*/0);
    REPORTER_ASSERT(reporter,
                    checkPaddingForGlyphs(skGlyphs,
                                          grGlyphs,
                                          {0, 0, 0, 0},
                                          /*insetPadding=*/0));
    // Transformed: 1 pixel zero-padding paddings between glyphs and on the sides
    draw2x2(/*srcPadding=*/1);
    REPORTER_ASSERT(reporter,
                    checkPaddingForGlyphs(skGlyphs,
                                          grGlyphs,
                                          {1, 1, 1, 1},
                                          /*insetPadding=*/0));
    // SDFT: 2 pixel zero-paddings between glyphs and on the sides
    draw2x2(2);
    REPORTER_ASSERT(reporter,
                    checkPaddingForGlyphs(skGlyphs,
                                          grGlyphs,
                                          {2, 2, 2, 2},
                                          /*insetPadding=*/2));
}

DEF_GPUTEST_FOR_CONTEXTS(GrAtlasManager_withSlug,
                         sk_gpu_test::GrContextFactory::IsRenderingContext,
                         reporter,
                         ctxInfo,
                         use_padding_options) {
    TestingUploadTarget uploadTarget;
    auto dContext = ctxInfo.directContext();
    auto atlasManager = dContext->priv().getAtlasManager();
    auto resourceProvider = dContext->priv().resourceProvider();

    atlasManager->setAtlasDimensionsToMinimum_ForTesting();

    SkFont defaultFont(SkTypeface::MakeFromName("Roboto-Regular", SkFontStyle()), 152);
    SkStrikeSpec strikeSpec = SkStrikeSpec::MakeWithNoDevice(defaultFont);
    sk_sp<SkStrike> strike = strikeSpec.findOrCreateStrike();
    SkArenaAlloc alloc(1 << 12);

    std::vector<SkGlyph> skGlyphs;
    std::vector<Glyph> grGlyphs;
    auto draw2x2 = [&](int srcPadding) {
        atlasManager->freeAll();
        skGlyphs.clear();
        grGlyphs.clear();
        unsigned int numProxies;
        atlasManager->getViews(MaskFormat::kA8, &numProxies);
        atlasManager->setMaxPages_TestingOnly(1);

        skgpu::PlotLocator plotLocator;
        for (int i = 0; i < 4; ++i) {
            char c = "ABCDEFGHIJKLMNOPQRSTUVWXYZ01234567890"[i];
            SkPackedGlyphID id(defaultFont.unicharToGlyph(c));
            SkGlyph skGlyph = strike->getScalerContext()->makeGlyph(id, &alloc);
            skGlyph.setImage(&alloc, strike->getScalerContext());
            SkPackedGlyphID glyphID;
            Glyph grGlyph(glyphID);
            auto errorCode = atlasManager->addGlyphToAtlas(
                    skGlyph, &grGlyph, srcPadding, resourceProvider, &uploadTarget);
            REPORTER_ASSERT(reporter, errorCode == GrDrawOpAtlas::ErrorCode::kSucceeded);
            skGlyphs.emplace_back(skGlyph);
            grGlyphs.emplace_back(grGlyph);
            if (i == 0) {
                plotLocator = grGlyph.fAtlasLocator.plotLocator();
            } else {
                REPORTER_ASSERT(reporter, plotLocator == grGlyph.fAtlasLocator.plotLocator());
            }
        }
    };

    // Direct mask: left & top padding == 1
    draw2x2(0);
    REPORTER_ASSERT(reporter,
                    checkPaddingForGlyphs(skGlyphs,
                                          grGlyphs,
                                          {1, 1, 0, 0},
                                          /*insetPadding=*/0));
    // Transformed: left & top padding == 1
    draw2x2(1);
    REPORTER_ASSERT(reporter,
                    checkPaddingForGlyphs(skGlyphs,
                                          grGlyphs,
                                          {1, 1, 0, 0},
                                          /*insetPadding=*/0));
    // SDFT: 2 pixel zero-paddings between glyphs and on the sides
    draw2x2(2);
    REPORTER_ASSERT(reporter,
                    checkPaddingForGlyphs(skGlyphs,
                                          grGlyphs,
                                          {2, 2, 2, 2},
                                          /*insetPadding=*/2));
}
