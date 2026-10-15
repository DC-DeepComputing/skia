/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_ScratchDeviceAtlas_DEFINED
#define skgpu_graphite_ScratchDeviceAtlas_DEFINED

#include "include/core/SkImageInfo.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSize.h"
#include "include/core/SkSurfaceProps.h"
#include "src/gpu/RectanizerSkyline.h"
#include "src/gpu/graphite/DrawOrder.h"
#include "src/gpu/graphite/ResourceTypes.h"

#include <memory>
#include <vector>

struct SkImageInfo;

namespace skgpu::graphite {

class BoundsManager;
class Device;
class DrawContext;
class Recorder;
class TextureProxy;

/**
 * AtlasPage represents a single backing texture page of a ScratchDeviceAtlas.
 * During recording, slots are registered with their dimensions while their coordinates remain
 * local [0, 0, w, h]. When the page is flushed or sampled (ensurePlotted()), all slots on the
 * page are sorted by decreasing height and packed into a tightly sized approx-fit texture.
 */
class AtlasPage : public SkRefCnt {
public:
    AtlasPage(sk_sp<TextureProxy> proxy,
              sk_sp<DrawContext> dc,
              SkISize maxDimensions);
    ~AtlasPage() override;

    bool canFit(SkISize slotSize, int padding) const;
    int addSlot(SkISize slotSize, int padding);

    void ensurePlotted();

    SkIPoint getSlotOffset(int slotIndex) {
        this->ensurePlotted();
        SkASSERT(slotIndex >= 0 && slotIndex < static_cast<int>(fSlots.size()));
        return fSlots[slotIndex].fOffset;
    }

    SkISize plottedDimensions() {
        this->ensurePlotted();
        return fPlottedDimensions;
    }

    bool isPlotted() const { return fIsPlotted; }
    int numSlots() const { return static_cast<int>(fSlots.size()); }

    TextureProxy* target() const { return fAtlasProxy.get(); }
    DrawContext* drawContext() const { return fAtlasDC.get(); }
    sk_sp<DrawContext> refDrawContext() const;

private:
    struct Slot {
        SkISize fSize;
        SkIPoint fOffset = {0, 0};
    };

    sk_sp<TextureProxy> fAtlasProxy;
    sk_sp<DrawContext> fAtlasDC;
    SkISize fMaxDimensions;
    SkISize fPlottedDimensions;
    std::vector<Slot> fSlots;
    int64_t fTotalPaddedArea = 0;
    int fPadding = 2;
    bool fIsPlotted = false;
};

/**
 * ScratchDeviceAtlas manages plotting small scratch devices into a shared texture atlas.
 * This batches multiple independent small layers or image filter passes together into a single
 * render pass, dramatically reducing render pass setup overhead.
 */
class ScratchDeviceAtlas {
public:
    explicit ScratchDeviceAtlas(Recorder* recorder);
    ~ScratchDeviceAtlas();

    ScratchDeviceAtlas(const ScratchDeviceAtlas&) = delete;
    ScratchDeviceAtlas& operator=(const ScratchDeviceAtlas&) = delete;

    const SkColorInfo& colorInfo() const { return fColorInfo; }

    // Checks if the given info is eligible for plotting into this atlas.
    bool isEligible(const SkImageInfo& info) const;

    // Checks if a slot of the given size can fit without flushing.
    bool canFit(SkISize size) const;

    // Allocates a slot and returns an atlased Device that records draws into the shared atlas
    // DrawContext.
    sk_sp<Device> createDevice(const SkImageInfo& info, const SkSurfaceProps& props);

    // Flushes pending work from the atlas DrawContext as a DrawTask to the recorder.
    void flush(Recorder* recorder);

    // Resets the atlas state, allocating a fresh atlas texture on next use.
    void reset();

    TextureProxy* target() const;
    DrawContext* drawContext() const;
    AtlasPage* currentPage() const { return fCurrentPage.get(); }
    bool hasPendingWork() const;

    PaintersDepth& currentDepth() { return fCurrentDepth; }

    static constexpr int kMaxSlotDim = 512;
    static constexpr int kSlotPadding = 2;

private:
    bool initAtlas(const SkColorInfo& colorInfo, const SkSurfaceProps& props);

    Recorder* fRecorder;
    SkColorInfo fColorInfo;
    SkSurfaceProps fSurfaceProps;

    sk_sp<AtlasPage> fCurrentPage;
    PaintersDepth fCurrentDepth;
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_ScratchDeviceAtlas_DEFINED
