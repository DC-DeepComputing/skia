/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "include/core/SkPathBuilder.h"
#include "include/core/SkSurface.h"
#include "include/gpu/graphite/Context.h"
#include "include/gpu/graphite/ContextOptions.h"
#include "include/gpu/graphite/Recorder.h"
#include "include/gpu/graphite/Recording.h"
#include "include/gpu/graphite/Surface.h"
#include "src/gpu/graphite/AtlasProvider.h"
#include "src/gpu/graphite/ContextOptionsPriv.h"
#include "src/gpu/graphite/ContextPriv.h"
#include "src/gpu/graphite/RecorderPriv.h"
#include "src/gpu/graphite/RendererProvider.h"
#include "src/gpu/graphite/sparse_strips/AlphaAtlasManager.h"
#include "tests/CtsEnforcement.h"
#include "tests/Test.h"

#include <cmath>
#include <memory>

namespace skgpu::graphite {

namespace {

constexpr int kSize = 512;
constexpr int kPathCount = 24;

void use_sparse_strips(ContextOptions* options) {
    if (options->fOptionsPriv) {
        options->fOptionsPriv->fPathRendererStrategy = PathRendererStrategy::kCPUSparseStripsMSAA8;
    }
}

// A star whose many thin spikes span most of the surface, so that most tiles along its edges are
// EndCap tiles and each draw produces a lot of alpha data. A fresh alpha atlas only holds 96 KB in
// its regular pages, so the scene overflows within the first few draws.
SkPath make_star(int index) {
    constexpr int kPoints = 97;
    const float rotation = index * 0.13f;
    const float cx = kSize * 0.5f + 40.f * std::cos(index * 1.7f);
    const float cy = kSize * 0.5f + 40.f * std::sin(index * 1.3f);
    const float outerRadius = kSize * 0.48f;
    const float innerRadius = kSize * (0.10f + 0.01f * (index % 7));

    SkPathBuilder builder;
    for (int i = 0; i < 2 * kPoints; ++i) {
        const float radius = (i % 2 == 0) ? outerRadius : innerRadius;
        const float theta = rotation + i * SK_ScalarPI / kPoints;
        const SkPoint p = {cx + radius * std::cos(theta), cy + radius * std::sin(theta)};
        if (i == 0) {
            builder.moveTo(p);
        } else {
            builder.lineTo(p);
        }
    }
    builder.close();
    return builder.detach();
}

// Renders the scene with a new Recorder (and so a fresh alpha atlas) and reads it back. With
// `snapEachDraw`, every draw gets its own Recording, so overflowing draws are committed after a full
// flush and may overwrite the overflow page in place. Otherwise all draws share one flush epoch and
// append to, or replace, the overflow page.
bool render_scene(skiatest::Reporter* reporter,
                  Context* context,
                  bool snapEachDraw,
                  SkBitmap* result) {
    std::unique_ptr<Recorder> recorder = context->makeRecorder();
    const SkImageInfo ii =
            SkImageInfo::Make(kSize, kSize, kRGBA_8888_SkColorType, kPremul_SkAlphaType);
    sk_sp<SkSurface> surface = SkSurfaces::RenderTarget(recorder.get(), ii);
    if (!surface) {
        ERRORF(reporter, "Could not create surface");
        return false;
    }

    auto submit = [&]() {
        std::unique_ptr<Recording> recording = recorder->snap();
        return recording && context->insertRecording({recording.get()});
    };

    SkCanvas* canvas = surface->getCanvas();
    canvas->clear(SK_ColorWHITE);
    SkPaint paint;
    paint.setAntiAlias(true);
    for (int i = 0; i < kPathCount; ++i) {
        // Translucent, so that misplaced or missing alphas change the blended result.
        paint.setColor(SkColorSetARGB(0x60, (i * 67) & 0xFF, (i * 131) & 0xFF, (i * 29) & 0xFF));
        canvas->drawPath(make_star(i), paint);
        if (snapEachDraw && !submit()) {
            ERRORF(reporter, "Failed to submit draw %d", i);
            return false;
        }
    }
    if (!submit()) {
        ERRORF(reporter, "Failed to submit scene");
        return false;
    }

    // Make sure the scene actually exercised the overflow page.
    const AlphaAtlasManager* alphaAtlas =
            recorder->priv().getOrCreateAtlasProvider()->alphaAtlasManager();
    REPORTER_ASSERT(reporter,
                    alphaAtlas->getPageProxy(AlphaAtlasManager::kOverflowSlot) != nullptr,
                    "Scene did not overflow the alpha atlas (snapEachDraw=%d)",
                    snapEachDraw);

    result->allocPixels(ii);
    if (!surface->readPixels(*result, 0, 0)) {
        ERRORF(reporter, "readPixels failed");
        return false;
    }
    return true;
}

}  // anonymous namespace

// Renders a scene that overflows the alpha atlas' regular pages, once within a single flush epoch
// (overflow appends and replacements) and once with a flush after every draw (overflow page
// overwrites). Both use the same rasterization, so they must match exactly.
DEF_CONDITIONAL_GRAPHITE_TEST_FOR_CONTEXTS(SparseStrips_OverflowRendering,
                                           skgpu::IsRenderingContext,
                                           reporter,
                                           context,
                                           testContext,
                                           /* anonymous options */,
                                           use_sparse_strips,
                                           /* condition= */ true,
                                           CtsEnforcement::kNever) {
    if (context->priv().rendererProvider()->pathRendererStrategy() !=
        PathRendererStrategy::kCPUSparseStripsMSAA8) {
        ERRORF(reporter, "Context is not using the sparse strips path renderer strategy");
        return;
    }

    SkBitmap batched;
    SkBitmap flushed;
    if (!render_scene(reporter, context, /*snapEachDraw=*/false, &batched) ||
        !render_scene(reporter, context, /*snapEachDraw=*/true, &flushed)) {
        return;
    }

    int mismatches = 0;
    for (int y = 0; y < kSize; ++y) {
        for (int x = 0; x < kSize; ++x) {
            const SkColor batchedColor = batched.getColor(x, y);
            const SkColor flushedColor = flushed.getColor(x, y);
            if (batchedColor != flushedColor) {
                if (mismatches < 8) {
                    ERRORF(reporter,
                           "Mismatch at (%d, %d): batched 0x%08x, flushed 0x%08x",
                           x, y, batchedColor, flushedColor);
                }
                mismatches++;
            }
        }
    }
    REPORTER_ASSERT(reporter, mismatches == 0, "%d mismatched pixels", mismatches);
}

}  // namespace skgpu::graphite
