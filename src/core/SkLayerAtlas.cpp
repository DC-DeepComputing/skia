/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkLayerAtlas.h"

#include <algorithm>

#include "include/core/SkBlendMode.h"
#include "include/core/SkColorFilter.h"
#include "include/core/SkSamplingOptions.h"
#include "include/effects/SkImageFilters.h"
#include "src/core/SkDevice.h"

#include <vector>

static sk_sp<SkImageFilter> StripCropFilter(sk_sp<SkImageFilter> filter) {
    if (!filter) {
        return nullptr;
    }
    while (filter && filter->getTypeName() && strcmp(filter->getTypeName(), "SkCropImageFilter") == 0) {
        if (filter->countInputs() > 0) {
            filter = sk_ref_sp(const_cast<SkImageFilter*>(filter->getInput(0)));
        } else {
            return nullptr;
        }
    }
    if (!filter) {
        return nullptr;
    }
    SkColorFilter* cfRaw = nullptr;
    if (filter->isColorFilterNode(&cfRaw)) {
        sk_sp<SkColorFilter> cf(cfRaw);
        sk_sp<SkImageFilter> input = filter->countInputs() > 0
            ? StripCropFilter(sk_ref_sp(const_cast<SkImageFilter*>(filter->getInput(0))))
            : nullptr;
        return SkImageFilters::ColorFilter(std::move(cf), std::move(input));
    }
    if (filter->getTypeName() && strcmp(filter->getTypeName(), "SkMergeImageFilter") == 0) {
        int count = filter->countInputs();
        if (count <= 0) {
            return nullptr;
        }
        std::vector<sk_sp<SkImageFilter>> stripped(count);
        for (int i = 0; i < count; ++i) {
            stripped[i] = StripCropFilter(sk_ref_sp(const_cast<SkImageFilter*>(filter->getInput(i))));
        }
        return SkImageFilters::Merge(stripped.data(), count);
    }
    if (filter->getTypeName() && strcmp(filter->getTypeName(), "SkComposeImageFilter") == 0) {
        if (filter->countInputs() == 2) {
            auto outer = StripCropFilter(sk_ref_sp(const_cast<SkImageFilter*>(filter->getInput(0))));
            auto inner = StripCropFilter(sk_ref_sp(const_cast<SkImageFilter*>(filter->getInput(1))));
            return SkImageFilters::Compose(std::move(outer), std::move(inner));
        }
    }
    return filter;
}

SkLayerAtlasManager::SkLayerAtlasManager() = default;
SkLayerAtlasManager::~SkLayerAtlasManager() = default;

bool SkLayerAtlasManager::isEligible(const SkCanvas::SaveLayerRec& rec,
                                     const SkDevice* priorDevice,
                                     const SkMatrix& ctm) const {
    if (!priorDevice) {
        return false;
    }
    // Only GPU-backed devices (Ganesh / Graphite) are eligible for layer atlas.
    if (!priorDevice->recordingContext() && !priorDevice->recorder()) {
        return false;
    }
    // No backdrop filter or init with previous
    if (rec.fBackdrop || (rec.fSaveLayerFlags & SkCanvas::kInitWithPrevious_SaveLayerFlag)) {
        return false;
    }
    // Single image filter (either on fPaint or in fFilters)
    const SkImageFilter* filter = rec.fPaint ? rec.fPaint->getImageFilter() : nullptr;
    if (!filter) {
        if (rec.fFilters.size() == 1 && rec.fFilters[0]) {
            filter = rec.fFilters[0].get();
        } else {
            return false;
        }
    } else if (!rec.fFilters.empty()) {
        return false;
    }
    sk_sp<SkImageFilter> unwrapped = StripCropFilter(sk_ref_sp(const_cast<SkImageFilter*>(filter)));
    if (!unwrapped) {
        return false;
    }
    // Bounded layer
    if (!rec.fBounds || !rec.fBounds->isFinite() || rec.fBounds->isEmpty()) {
        return false;
    }
    // Transform must be scale-translate (or identity)
    if (!ctm.isScaleTranslate()) {
        return false;
    }

    // Check slot size limit
    SkRect devRect = ctm.mapRect(*rec.fBounds);
    int devFloorLeft = SkScalarFloorToInt(devRect.left());
    int devFloorTop = SkScalarFloorToInt(devRect.top());
    int devCeilRight = SkScalarCeilToInt(devRect.right());
    int devCeilBottom = SkScalarCeilToInt(devRect.bottom());
    int contentW = devCeilRight - devFloorLeft;
    int contentH = devCeilBottom - devFloorTop;
    if (contentW <= 0 || contentH <= 0 || contentW > kMaxSlotDim || contentH > kMaxSlotDim) {
        return false;
    }
    int slotW = contentW + 2 * kPadding;
    int slotH = contentH + 2 * kPadding;
    if (slotW > kAtlasDim || slotH > kAtlasDim) {
        return false;
    }

    return true;
}

bool SkLayerAtlasManager::canAddLayer(const SkCanvas::SaveLayerRec& rec,
                                      const SkDevice* priorDevice,
                                      const SkMatrix& ctm) const {
    if (!this->isEligible(rec, priorDevice, ctm)) {
        return false;
    }
    if (fActiveLayer) {
        return false;
    }
    if (fPendingRestores.empty()) {
        return true;
    }

    // Check filter match
    const SkImageFilter* filter = rec.fPaint ? rec.fPaint->getImageFilter() : nullptr;
    if (!filter && rec.fFilters.size() == 1) {
        filter = rec.fFilters[0].get();
    }
    if (!filter) {
        return false;
    }

    sk_sp<SkImageFilter> unwrapped = StripCropFilter(sk_ref_sp(const_cast<SkImageFilter*>(filter)));
    if (!unwrapped) {
        return false;
    }
    if (fActiveFilter.get() != unwrapped.get()) {
        if (!fActiveFilterData) {
            return false;
        }
        sk_sp<SkData> filterData = unwrapped->serialize();
        if (!filterData || !fActiveFilterData->equals(filterData.get())) {
            return false;
        }
    }

    SkRect devRect = ctm.mapRect(*rec.fBounds);

    // Check atlas capacity
    int devFloorLeft = SkScalarFloorToInt(devRect.left());
    int devFloorTop = SkScalarFloorToInt(devRect.top());
    int devCeilRight = SkScalarCeilToInt(devRect.right());
    int devCeilBottom = SkScalarCeilToInt(devRect.bottom());
    int contentW = devCeilRight - devFloorLeft;
    int contentH = devCeilBottom - devFloorTop;
    int slotW = contentW + 2 * kPadding;
    int slotH = contentH + 2 * kPadding;

    int slotX = fCurX;
    if (((slotX + kPadding) & 1) != (devFloorLeft & 1)) {
        slotX += 1;
    }
    int slotY = fCurY;
    if (((slotY + kPadding) & 1) != (devFloorTop & 1)) {
        slotY += 1;
    }
    if (slotX + slotW > kAtlasDim) {
        int nextY = fCurY + fRowH;
        slotX = 0;
        if (((slotX + kPadding) & 1) != (devFloorLeft & 1)) {
            slotX += 1;
        }
        slotY = nextY;
        if (((slotY + kPadding) & 1) != (devFloorTop & 1)) {
            slotY += 1;
        }
    }
    if (slotY + slotH > kAtlasDim) {
        return false;
    }

    return true;
}

sk_sp<SkDevice> SkLayerAtlasManager::startOrContinueLayer(SkCanvas* canvas,
                                                         const SkCanvas::SaveLayerRec& rec,
                                                         const SkDevice* priorDevice,
                                                         const SkMatrix& ctm) {
    SkASSERT(!fActiveLayer);

    SkRect devRect = ctm.mapRect(*rec.fBounds);
    int devFloorLeft = SkScalarFloorToInt(devRect.left());
    int devFloorTop = SkScalarFloorToInt(devRect.top());
    int devCeilRight = SkScalarCeilToInt(devRect.right());
    int devCeilBottom = SkScalarCeilToInt(devRect.bottom());
    int contentW = devCeilRight - devFloorLeft;
    int contentH = devCeilBottom - devFloorTop;
    int slotW = contentW + 2 * kPadding;
    int slotH = contentH + 2 * kPadding;

    if (!fAtlasSurface) {
        SkImageInfo atlasInfo = priorDevice->imageInfo().makeWH(kAtlasDim, kAtlasDim);
        fAtlasSurface = canvas->makeSurface(atlasInfo);
        if (!fAtlasSurface) {
            return nullptr;
        }
        fAtlasSurface->getCanvas()->clear(SK_ColorTRANSPARENT);
        fCurX = 0;
        fCurY = 0;
        fRowH = 0;

        const SkImageFilter* filter = rec.fPaint ? rec.fPaint->getImageFilter() : nullptr;
        if (!filter && rec.fFilters.size() == 1) {
            filter = rec.fFilters[0].get();
        }
        fActiveFilter = StripCropFilter(sk_ref_sp(const_cast<SkImageFilter*>(filter)));
        fActiveFilterData = fActiveFilter ? fActiveFilter->serialize() : nullptr;
    }

    int slotX = fCurX;
    if (((slotX + kPadding) & 1) != (devFloorLeft & 1)) {
        slotX += 1;
    }
    int slotY = fCurY;
    if (((slotY + kPadding) & 1) != (devFloorTop & 1)) {
        slotY += 1;
    }

    if (slotX + slotW > kAtlasDim) {
        fCurX = 0;
        fCurY += fRowH;
        fRowH = 0;
        slotX = fCurX;
        if (((slotX + kPadding) & 1) != (devFloorLeft & 1)) {
            slotX += 1;
        }
        slotY = fCurY;
        if (((slotY + kPadding) & 1) != (devFloorTop & 1)) {
            slotY += 1;
        }
    }

    if (slotY + slotH > kAtlasDim) {
        return nullptr;
    }

    fCurX = slotX + slotW;
    fRowH = std::max(fRowH, (slotY - fCurY) + slotH);

    fActiveSlotRect = SkRect::MakeXYWH(slotX, slotY, slotW, slotH);
    fActiveDstRect = SkRect::MakeXYWH(devFloorLeft - kPadding, devFloorTop - kPadding, slotW, slotH);
    fActiveContentDevBounds = SkRect::MakeXYWH(devFloorLeft, devFloorTop, contentW, contentH);
    fActiveLayer = true;

    // Configure atlas device
    SkDevice* atlasDevice = fAtlasSurface->getCanvas()->topDevice();

    int dx = (slotX + kPadding) - devFloorLeft;
    int dy = (slotY + kPadding) - devFloorTop;

    // Reset coordinate system to identity before clipping in slot coordinates
    atlasDevice->setDeviceCoordinateSystem(SkM44(), SkM44(), SkM44(), 0, 0);
    atlasDevice->pushClipStack();
    atlasDevice->clipRect(
        SkRect::MakeXYWH(slotX + kPadding, slotY + kPadding, contentW, contentH),
        SkClipOp::kIntersect,
        /*aa=*/false);

    // Now set the layer's localToDevice mapping with slot offset (-dx, -dy).
    // Note: devFloorLeft/devFloorTop was computed from ctm (global canvas space),
    // so deviceToGlobal and globalToDevice are identity transforms offset by (-dx, -dy).
    atlasDevice->setDeviceCoordinateSystem(
        SkM44(),
        SkM44(),
        SkM44(ctm),
        -dx, -dy);

    return sk_ref_sp(atlasDevice);
}

void SkLayerAtlasManager::deferRestore(const SkPaint& restorePaint) {
    if (fAtlasSurface) {
        SkDevice* atlasDevice = fAtlasSurface->getCanvas()->topDevice();
        if (atlasDevice) {
            atlasDevice->popClipStack();
            atlasDevice->setDeviceCoordinateSystem(SkM44(), SkM44(), SkM44(), 0, 0);
        }
    }

    PendingRestore restore;
    restore.fSrcSlotRect = fActiveSlotRect;
    restore.fDstRect = fActiveDstRect;
    restore.fContentDevBounds = fActiveContentDevBounds;
    restore.fPaint = restorePaint;
    fPendingRestores.push_back(restore);

    fActiveLayer = false;
}

void SkLayerAtlasManager::flush(SkCanvas* canvas) {
    if (fIsFlushing || fPendingRestores.empty() || !fAtlasSurface) {
        return;
    }
    fIsFlushing = true;

    SkDebugf("[LayerAtlas] Flush: batched %zu operations\n", fPendingRestores.size());

    sk_sp<SkImage> atlasImage = fAtlasSurface->makeImageSnapshot();
    if (!atlasImage) {
        this->reset();
        fIsFlushing = false;
        return;
    }

    SkImageInfo atlasInfo = fAtlasSurface->imageInfo();
    sk_sp<SkSurface> filteredSurface = canvas->makeSurface(atlasInfo);
    if (!filteredSurface) {
        this->reset();
        fIsFlushing = false;
        return;
    }
    filteredSurface->getCanvas()->clear(SK_ColorTRANSPARENT);

    SkPaint filterPaint;
    filterPaint.setImageFilter(fActiveFilter);
    filteredSurface->getCanvas()->drawImage(atlasImage, 0, 0, SkSamplingOptions(), &filterPaint);

    sk_sp<SkImage> filteredImage = filteredSurface->makeImageSnapshot();
    if (!filteredImage) {
        this->reset();
        fIsFlushing = false;
        return;
    }

    {
        SkAutoCanvasRestore acr(canvas, true);
        canvas->resetMatrix();

        for (const auto& pending : fPendingRestores) {
            SkPaint compositePaint = pending.fPaint;
            compositePaint.setImageFilter(nullptr);
            compositePaint.setAntiAlias(false);
            canvas->drawImageRect(filteredImage,
                                  pending.fSrcSlotRect,
                                  pending.fDstRect,
                                  SkSamplingOptions(),
                                  &compositePaint,
                                  SkCanvas::kStrict_SrcRectConstraint);
        }
    }

    this->reset();
    fIsFlushing = false;
}


bool SkLayerAtlasManager::intersectsPending(const SkRect& devBounds) const {
    for (const auto& pending : fPendingRestores) {
        if (devBounds.intersects(pending.fContentDevBounds)) {
            return true;
        }
    }
    return false;
}

void SkLayerAtlasManager::reset() {
    fAtlasSurface.reset();
    fActiveFilter.reset();
    fActiveFilterData.reset();
    fPendingRestores.clear();
    fCurX = 0;
    fCurY = 0;
    fRowH = 0;
    fActiveLayer = false;
    fActiveSlotRect.setEmpty();
    fActiveDstRect.setEmpty();
    fActiveContentDevBounds.setEmpty();
}
