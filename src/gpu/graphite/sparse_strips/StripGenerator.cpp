/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/sparse_strips/StripGenerator.h"

#include "include/core/SkMatrix.h"
#include "include/core/SkPath.h"
#include "src/gpu/graphite/AtlasProvider.h"
#include "src/gpu/graphite/RecorderPriv.h"
#include "src/gpu/graphite/geom/Shape.h"
#include "src/gpu/graphite/sparse_strips/AlphaAtlasManager.h"
#include "src/gpu/graphite/sparse_strips/Flatten.h"
#include "src/gpu/graphite/sparse_strips/MakeStrips.h"
#include "src/gpu/graphite/sparse_strips/Polyline.h"
#include "src/gpu/graphite/sparse_strips/SparseStripsConfig.h"
#include "src/gpu/graphite/sparse_strips/StripCache.h"
#include "src/gpu/graphite/sparse_strips/Tiler.h"

#include <optional>

namespace skgpu::graphite {

namespace {

// Computes the region that the strips of a draw restricted to `clip` are generated within: the
// clip within the device, with its left and top edges snapped down to the tile grid so that the
// first tile row and column are whole. The clip may be off the grid: Device's draw bounds take the
// scissor's edges where it's tighter, and the scissor is only snapped to 4px, or to the edges of a
// device-rect clip. Returns false if it is empty.
bool make_tile_clip(const SkIRect& clip, int width, int height, SkIRect* tileClip) {
    *tileClip = clip;
    if (!tileClip->intersect(SkIRect::MakeWH(width, height))) {
        return false;
    }
    tileClip->fLeft =
            (tileClip->fLeft / SparseStripConfig::kTileWidth) * SparseStripConfig::kTileWidth;
    tileClip->fTop =
            (tileClip->fTop / SparseStripConfig::kTileHeight) * SparseStripConfig::kTileHeight;
    return true;
}

// Returns true if geometry with the device space bounds `devBounds` lies within `tileClip`, so that
// neither flattening nor tiling discards or simplifies any of it, as they only do so for geometry
// strictly outside of the clip. No margin is needed, which matters since Device's clip is the
// shape's bounds snapped to the tile grid, which can be within a pixel of the shape.
bool within_tile_clip(const SkRect& devBounds, const SkIRect& tileClip) {
    return devBounds.isFinite() && SkRect::Make(tileClip).contains(devBounds);
}

}  // anonymous namespace

StripGenerator::DrawScope::DrawScope(StripGenerator* generator)
        : fAtlasScope(generator->atlasManager(), &generator->fEnds) {}

StripGenerator::StripGenerator(int width,
                               int height,
                               const SparseStripConfig::LUTArray& maskLUT,
                               Recorder* recorder)
        : fWidth(width)
        , fHeight(height)
        , fMaskLUT(maskLUT)
        , fRecorder(recorder)
        , fWindingHistogram((height + SparseStripConfig::kTileHeight - 1) /
                            SparseStripConfig::kTileHeight)
        , fCulled(true) {}

StripGenerator::~StripGenerator() = default;

#if defined(GPU_TEST_UTILS)
StripGenerator::DebugCaptureFn& StripGenerator::DebugCaptureCallbackStorage() {
    // Leaked so that there is no exit-time destructor.
    static DebugCaptureFn* gFn = new DebugCaptureFn();
    return *gFn;
}

void StripGenerator::SetDebugCaptureCallback(DebugCaptureFn fn) {
    DebugCaptureCallbackStorage() = std::move(fn);
}

const StripGenerator::DebugCaptureFn& StripGenerator::GetDebugCaptureCallback() {
    return DebugCaptureCallbackStorage();
}
#endif

AlphaAtlasManager* StripGenerator::atlasManager() const {
    return fRecorder->priv().getOrCreateAtlasProvider()->alphaAtlasManager();
}

AlphaAtlasManager* StripGenerator::beginDraw() {
    fEnds.clear();
    fWides.clear();

    AlphaAtlasManager* atlasManager = this->atlasManager();
    if (atlasManager) {
        // Discard any partial atlas state left behind by a previous draw that failed.
        atlasManager->beginDraw();
    }
    return atlasManager;
}

bool StripGenerator::processGeometry(const Shape& shape, const SkMatrix& ctm, const SkIRect& clip) {
#if defined(GPU_TEST_UTILS)
    if (const DebugCaptureFn& captureFn = GetDebugCaptureCallback()) {
        captureFn(shape.asPath(), ctm, clip, SkISize::Make(fWidth, fHeight));
    }
#endif
    AlphaAtlasManager* atlasManager = this->beginDraw();
    if (!atlasManager) {
        return false;
    }
    SkIRect tileClip;
    if (!make_tile_clip(clip, fWidth, fHeight, &tileClip)) {
        return true;
    }

    // Strips that don't depend on the clip can be reused by any draw of the shape with the same
    // transform up to an integer translation, even a clipped one, since the clip still applies when
    // they are drawn.
    SkIPoint origin = {0, 0};
    const std::optional<StripCache::Key> key = StripCache::Key::Make(shape, ctm, &origin);
    if (key && atlasManager->findCachedStrips(*key, origin, &fEnds, &fWides)) {
        atlasManager->bindPages(&fEnds);
        return true;
    }

    // The strips of a shape within the clip don't depend on it. Otherwise the shape is clipped, or
    // inverse filled and so fills the clip, and its strips can only be reused by draws with the
    // same transform and clip.
    const bool clipIndependent =
            !shape.inverted() && within_tile_clip(ctm.mapRect(shape.bounds().asSkRect()), tileClip);
    std::optional<StripCache::Key> clippedKey;
    if (!clipIndependent) {
        clippedKey = StripCache::Key::MakeClipped(shape, ctm, tileClip);
        if (clippedKey && atlasManager->findCachedStrips(*clippedKey, {0, 0}, &fEnds, &fWides)) {
            atlasManager->bindPages(&fEnds);
            return true;
        }
    }

    if (!this->generateStrips(atlasManager, shape.asPath(), ctm, tileClip)) {
        return false;
    }
    // Alphas in the overflow page don't keep their contents, so strips referencing them can't be
    // reused.
    if (fEnds.hasOverflowCaps() || (fEnds.empty() && fWides.empty())) {
        return true;
    }
    if (clipIndependent) {
        // Tiling never culls any of a shape within the clip, but strips are only cached as clip
        // independent if it agrees.
        if (key && !fCulled) {
            atlasManager->cacheStrips(*key, origin, fEnds, fWides);
        }
    } else if (clippedKey) {
        atlasManager->cacheStrips(*clippedKey, {0, 0}, fEnds, fWides);
    }
    return true;
}

bool StripGenerator::processGeometry(const SkPath& path, const SkMatrix& ctm) {
    return this->processGeometry(path, ctm, SkIRect::MakeWH(fWidth, fHeight));
}

bool StripGenerator::processGeometry(const SkPath& path, const SkMatrix& ctm, const SkIRect& clip) {
#if defined(GPU_TEST_UTILS)
    if (const DebugCaptureFn& captureFn = GetDebugCaptureCallback()) {
        captureFn(path, ctm, clip, SkISize::Make(fWidth, fHeight));
    }
#endif
    AlphaAtlasManager* atlasManager = this->beginDraw();
    if (!atlasManager) {
        return false;
    }
    SkIRect tileClip;
    if (!make_tile_clip(clip, fWidth, fHeight, &tileClip)) {
        return true;
    }
    return this->generateStrips(atlasManager, path, ctm, tileClip);
}

bool StripGenerator::generateStrips(AlphaAtlasManager* atlasManager,
                                    const SkPath& path,
                                    const SkMatrix& ctm,
                                    const SkIRect& tileClip) {
    Flatten flattener;
    Polyline polyline;
    flattener.processPaths<FlattenMode::kSimd>(path, ctm, tileClip, &polyline);

    if (polyline.count() == 0 && !path.isInverseFillType()) {
        return true;
    }

    fWindingHistogram.resize((fHeight + SparseStripConfig::kTileHeight - 1) /
                             SparseStripConfig::kTileHeight);
    if (fCulled) {
        fWindingHistogram.clear();
    }
#if defined(SK_DEBUG)
    // Tiler only writes to the histogram when it reports a culling event, so if the previous draw
    // did not cull, the histogram must still be zero from the last time it was cleared.
    for (int16_t winding : fWindingHistogram.data()) {
        SkASSERT(winding == 0);
    }
#endif

    Tiles<SparseStripConfig::kTileWidth, SparseStripConfig::kTileHeight> tiler;
    fCulled = tiler.makeTilesMSAA(polyline, tileClip, &fWindingHistogram);
    tiler.sortTiles();

    bool success =
            MakeStrips::MsaaSimd<SparseStripConfig::kTileWidth, SparseStripConfig::kTileHeight>(
                    tiler,
                    &fWides,
                    &fEnds,
                    atlasManager,
                    path.getFillType(),
                    polyline,
                    fMaskLUT,
                    fCulled,
                    fWindingHistogram,
                    tileClip);

    if (!success) {
        return false;
    }

    if (!fEnds.empty()) {
        atlasManager->bindPages(&fEnds);
    }
    return true;
}

bool StripGenerator::overflowNeedsFlush() const {
    return fEnds.hasPendingOverflow() && this->atlasManager()->overflowCommitNeedsFlush();
}

bool StripGenerator::commitOverflow() {
    return this->atlasManager()->commitOverflow(&fEnds);
}

}  // namespace skgpu::graphite
