/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "bench/Benchmark.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkImageFilter.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkTileMode.h"
#include "include/effects/SkImageFilters.h"
#include "tools/ToolUtils.h"

#include <algorithm>
#include <iterator>
#include <vector>

namespace {

// The content under blur: a circle with a tall rect through it.
void draw_content(SkCanvas* canvas, const SkRect& bounds, const SkPaint& paint) {
    canvas->drawCircle(bounds.centerX(),
                       bounds.centerY(),
                       0.3f * std::min(bounds.width(), bounds.height()),
                       paint);
    canvas->drawRect(bounds.makeInset(0.4f * bounds.width(), 0.1f * bounds.height()), paint);
}

// Exercises Graphite's layer blurs, to compare the compute blur (GaussianBlurStep) against the
// shader blur by toggling Caps::useComputeBlur(). Each loop draws `count` sibling layers laid out
// in rows across the canvas.
//
// By default each layer blurs its own content through its paint's image filter, and the compute
// blur batches those until the canvas is flushed (or the batch budget is hit). Backdrop layers
// instead blur and tint the canvas beneath them (frosted glass), clipped to their bounds. Each
// backdrop reads the canvas, so the canvas's render pass splits at every layer and the blurs can't
// batch.
//
// Names read graphite_blur_layers_<count>x<size>_sigma<sigma>[_<tileMode>][_backdrop], where
// <size> is <w>x<h> for non-square layers and <sigma> is <x>x<y> for asymmetric blurs.
//
// The canvas must be RGBA8 for the compute blur to run (e.g. --config grmtl or grdawn_mtl).
class GraphiteBlurLayersBench final : public Benchmark {
public:
    GraphiteBlurLayersBench(
            int count, SkISize layerSize, SkSize sigma, SkTileMode tileMode, bool backdrop)
            : fCount(count)
            , fLayerSize(layerSize)
            , fSigma(sigma)
            , fTileMode(tileMode)
            , fBackdrop(backdrop) {
        fName.printf("graphite_blur_layers_%d", count);
        if (layerSize.width() == layerSize.height()) {
            fName.appendf("x%d", layerSize.width());
        } else {
            fName.appendf("x%dx%d", layerSize.width(), layerSize.height());
        }
        if (sigma.width() == sigma.height()) {
            fName.appendf("_sigma%g", sigma.width());
        } else {
            fName.appendf("_sigma%gx%g", sigma.width(), sigma.height());
        }
        if (tileMode != SkTileMode::kDecal) {
            fName.appendf("_%s", ToolUtils::tilemode_name(tileMode));
        }
        if (backdrop) {
            fName.append("_backdrop");
        }
    }

private:
    static constexpr int kCanvasSize = 1024;

    const char* onGetName() override { return fName.c_str(); }
    bool isSuitableFor(Backend backend) override { return backend == Backend::kGraphite; }
    SkISize onGetSize() override { return {kCanvasSize, kCanvasSize}; }

    // Layers fill the canvas in rows, wrapping back to the top if they don't fit.
    SkRect layerBounds(int index) const {
        const int columns = std::max(1, kCanvasSize / fLayerSize.width());
        const int rows = std::max(1, kCanvasSize / fLayerSize.height());
        return SkRect::Make(SkIRect::MakeXYWH((index % columns) * fLayerSize.width(),
                                              (index / columns % rows) * fLayerSize.height(),
                                              fLayerSize.width(),
                                              fLayerSize.height()));
    }

    void onDelayedSetup() override {
        // Tiling needs a crop to tile against, so each layer gets a filter cropped to its own
        // bounds. Decal layers share one uncropped filter, which blurs past the layer's bounds.
        const bool cropped = fTileMode != SkTileMode::kDecal;
        fFilters.clear();
        for (int i = 0; i < (cropped ? fCount : 1); ++i) {
            fFilters.push_back(SkImageFilters::Blur(
                    fSigma.width(),
                    fSigma.height(),
                    fTileMode,
                    nullptr,
                    cropped ? SkImageFilters::CropRect(this->layerBounds(i))
                            : SkImageFilters::CropRect()));
        }
    }

    void onDraw(int loops, SkCanvas* canvas) override {
        constexpr SkColor kColors[] = {SK_ColorRED, SK_ColorGREEN, SK_ColorBLUE, SK_ColorMAGENTA};
        SkPaint paint;
        SkPaint layerPaint;
        SkPaint tintPaint;
        tintPaint.setColor(SkColorSetARGB(0x40, 0xFF, 0xFF, 0xFF));
        for (int i = 0; i < loops; ++i) {
            for (int j = 0; j < fCount; ++j) {
                const SkRect bounds = this->layerBounds(j);
                const sk_sp<SkImageFilter>& filter = fFilters[fFilters.size() > 1 ? j : 0];
                paint.setColor(kColors[j % std::size(kColors)]);
                if (fBackdrop) {
                    draw_content(canvas, bounds, paint);
                    // A backdrop layer ignores its bounds and fills the clip, so clip first.
                    canvas->save();
                    canvas->clipRect(bounds);
                    canvas->saveLayer(SkCanvas::SaveLayerRec(&bounds, nullptr, filter.get(), 0));
                    canvas->drawRect(bounds, tintPaint);
                    canvas->restore();
                    canvas->restore();
                } else {
                    layerPaint.setImageFilter(filter);
                    canvas->saveLayer(&bounds, &layerPaint);
                    draw_content(canvas, bounds, paint);
                    canvas->restore();
                }
            }
        }
    }

    const int fCount;
    const SkISize fLayerSize;
    const SkSize fSigma;
    const SkTileMode fTileMode;
    const bool fBackdrop;
    SkString fName;
    std::vector<sk_sp<SkImageFilter>> fFilters;
};

// Layers that blur their own content.
Benchmark* layer_blurs(int count,
                       SkISize layerSize,
                       SkSize sigma,
                       SkTileMode tileMode = SkTileMode::kDecal) {
    return new GraphiteBlurLayersBench(count, layerSize, sigma, tileMode, /*backdrop=*/false);
}

// Layers that blur the canvas beneath them.
Benchmark* backdrop_blurs(int count, SkISize layerSize, SkSize sigma) {
    return new GraphiteBlurLayersBench(
            count, layerSize, sigma, SkTileMode::kDecal, /*backdrop=*/true);
}

}  // namespace

// Sigmas 0.5/2/4 have radii 2/6/12, each of which exactly fills a compute radius bucket.
// Many small cards, which batch the most.
DEF_BENCH(return layer_blurs(16, {128, 128}, {0.5f, 0.5f});)
DEF_BENCH(return layer_blurs(16, {128, 128}, {2, 2});)
DEF_BENCH(return layer_blurs(16, {128, 128}, {4, 4});)

// A few medium layers.
DEF_BENCH(return layer_blurs(4, {512, 512}, {0.5f, 0.5f});)
DEF_BENCH(return layer_blurs(4, {512, 512}, {2, 2});)
DEF_BENCH(return layer_blurs(4, {512, 512}, {4, 4});)

// One full-canvas layer, which can't batch.
DEF_BENCH(return layer_blurs(1, {1024, 1024}, {0.5f, 0.5f});)
DEF_BENCH(return layer_blurs(1, {1024, 1024}, {2, 2});)
DEF_BENCH(return layer_blurs(1, {1024, 1024}, {4, 4});)

// Fewer cards of the same size, to separate batching from the blur itself.
DEF_BENCH(return layer_blurs(1, {128, 128}, {0.5f, 0.5f});)
DEF_BENCH(return layer_blurs(1, {128, 128}, {2, 2});)
DEF_BENCH(return layer_blurs(1, {128, 128}, {4, 4});)
DEF_BENCH(return layer_blurs(4, {128, 128}, {0.5f, 0.5f});)
DEF_BENCH(return layer_blurs(4, {128, 128}, {2, 2});)
DEF_BENCH(return layer_blurs(4, {128, 128}, {4, 4});)

// Off-bucket sigmas: radius 3 runs the radius 4 bucket, and radius 9 runs the radius 10 bucket.
DEF_BENCH(return layer_blurs(16, {128, 128}, {1, 1});)
DEF_BENCH(return layer_blurs(16, {128, 128}, {3, 3});)
DEF_BENCH(return layer_blurs(4, {512, 512}, {1, 1});)
DEF_BENCH(return layer_blurs(4, {512, 512}, {3, 3});)
DEF_BENCH(return layer_blurs(1, {1024, 1024}, {1, 1});)
DEF_BENCH(return layer_blurs(1, {1024, 1024}, {3, 3});)

// Asymmetric sigmas. With one sigma at 0 the shader blur does a single 1D pass, while the compute
// blur runs both passes at the larger radius.
DEF_BENCH(return layer_blurs(16, {128, 128}, {4, 0});)
DEF_BENCH(return layer_blurs(16, {128, 128}, {0, 4});)
DEF_BENCH(return layer_blurs(16, {128, 128}, {4, 1});)
DEF_BENCH(return layer_blurs(4, {512, 512}, {4, 0});)
DEF_BENCH(return layer_blurs(4, {512, 512}, {0, 4});)
DEF_BENCH(return layer_blurs(4, {512, 512}, {4, 1});)
DEF_BENCH(return layer_blurs(1, {1024, 1024}, {4, 0});)
DEF_BENCH(return layer_blurs(1, {1024, 1024}, {0, 4});)
DEF_BENCH(return layer_blurs(1, {1024, 1024}, {4, 1});)

// Sigmas above 4, which both blurs handle by downscaling and then blurring at sigma <= 4.
DEF_BENCH(return layer_blurs(16, {128, 128}, {8, 8});)
DEF_BENCH(return layer_blurs(16, {128, 128}, {16, 16});)
DEF_BENCH(return layer_blurs(4, {512, 512}, {8, 8});)
DEF_BENCH(return layer_blurs(4, {512, 512}, {16, 16});)
DEF_BENCH(return layer_blurs(1, {1024, 1024}, {8, 8});)
DEF_BENCH(return layer_blurs(1, {1024, 1024}, {16, 16});)

// Small and odd sizes, which leave the compute blur's 64x16 tiles partly empty.
DEF_BENCH(return layer_blurs(64, {32, 32}, {0.5f, 0.5f});)
DEF_BENCH(return layer_blurs(64, {32, 32}, {4, 4});)
DEF_BENCH(return layer_blurs(16, {100, 60}, {0.5f, 0.5f});)
DEF_BENCH(return layer_blurs(16, {100, 60}, {4, 4});)
DEF_BENCH(return layer_blurs(4, {333, 251}, {0.5f, 0.5f});)
DEF_BENCH(return layer_blurs(4, {333, 251}, {4, 4});)

// Clamp tiling against each layer's bounds.
DEF_BENCH(return layer_blurs(16, {128, 128}, {4, 4}, SkTileMode::kClamp);)
DEF_BENCH(return layer_blurs(4, {512, 512}, {4, 4}, SkTileMode::kClamp);)
DEF_BENCH(return layer_blurs(1, {1024, 1024}, {4, 4}, SkTileMode::kClamp);)

// Backdrop blurs (frosted glass), including a heavy blur that downscales.
DEF_BENCH(return backdrop_blurs(16, {128, 128}, {4, 4});)
DEF_BENCH(return backdrop_blurs(16, {128, 128}, {16, 16});)
DEF_BENCH(return backdrop_blurs(4, {512, 512}, {4, 4});)
DEF_BENCH(return backdrop_blurs(4, {512, 512}, {16, 16});)
DEF_BENCH(return backdrop_blurs(1, {1024, 1024}, {4, 4});)
DEF_BENCH(return backdrop_blurs(1, {1024, 1024}, {16, 16});)
