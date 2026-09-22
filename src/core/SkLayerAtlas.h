/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkLayerAtlas_DEFINED
#define SkLayerAtlas_DEFINED

#include <memory>
#include <vector>

#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkData.h"
#include "include/core/SkImage.h"
#include "include/core/SkImageFilter.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSurface.h"

class SkDevice;

/**
 * SkLayerAtlasManager manages offscreen atlas surfaces for consecutive saveLayer
 * calls that share the same image filter DAG.
 */
class SkLayerAtlasManager {
public:
    SkLayerAtlasManager();
    ~SkLayerAtlasManager();

    // Check if the layer satisfies general atlas criteria (filter, bounds, dimensions)
    bool isEligible(const SkCanvas::SaveLayerRec& rec,
                    const SkDevice* priorDevice,
                    const SkMatrix& ctm) const;

    // Check if the layer can be added to the current active atlas without flushing
    bool canAddLayer(const SkCanvas::SaveLayerRec& rec,
                     const SkDevice* priorDevice,
                     const SkMatrix& ctm) const;

    // Allocate a slot in the atlas and configure the atlas device for drawing
    sk_sp<SkDevice> startOrContinueLayer(SkCanvas* canvas,
                                         const SkCanvas::SaveLayerRec& rec,
                                         const SkDevice* priorDevice,
                                         const SkMatrix& ctm);

    // Record a pending restore without immediately filtering or blitting
    void deferRestore(const SkPaint& restorePaint);

    // Snapshot the atlas, evaluate the filter DAG once, and blit all pending slots
    void flush(SkCanvas* canvas);

    bool hasPendingLayers() const { return !fPendingRestores.empty() && !fIsFlushing; }
    bool hasActiveLayer() const { return fActiveLayer; }
    bool isFlushing() const { return fIsFlushing; }
    bool intersectsPending(const SkRect& devBounds) const;

    void reset();

private:
    struct PendingRestore {
        SkRect  fSrcSlotRect;       // Slot in atlas (including padding)
        SkRect  fDstRect;           // Target rect on destination (including padding)
        SkRect  fContentDevBounds;  // Content device bounds (unpadded)
        SkPaint fPaint;             // Composite paint (alpha, blendMode, colorFilter)
    };

    sk_sp<SkSurface>     fAtlasSurface;
    sk_sp<SkImageFilter> fActiveFilter;
    sk_sp<SkData>        fActiveFilterData;

    std::vector<PendingRestore> fPendingRestores;

    static constexpr int kAtlasDim = 2048;
    static constexpr int kPadding = 16;
    static constexpr int kMaxSlotDim = 1024;

    int fCurX = 0;
    int fCurY = 0;
    int fRowH = 0;

    bool   fActiveLayer = false;
    SkRect fActiveSlotRect;
    SkRect fActiveDstRect;
    SkRect fActiveContentDevBounds;

    bool fIsFlushing = false;
};

#endif // SkLayerAtlas_DEFINED
