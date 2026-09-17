/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_ScratchDeviceAtlas_DEFINED
#define skgpu_graphite_ScratchDeviceAtlas_DEFINED

#include "include/core/SkImageInfo.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSize.h"
#include "include/core/SkSurfaceProps.h"
#include "src/gpu/RectanizerSkyline.h"
#include "src/gpu/graphite/Device.h"
#include "src/gpu/graphite/DrawOrder.h"

#include <memory>

struct SkImageInfo;

namespace skgpu::graphite {

class BoundsManager;
class DrawContext;
class Recorder;
class TextureProxy;

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
    DrawContext* drawContext() const { return fAtlasDC.get(); }
    bool hasPendingWork() const;

    BoundsManager* boundsManager() const { return fColorDepthBoundsManager.get(); }
    Device::IntersectionTreeSet* disjointStencilSet() const { return fDisjointStencilSet.get(); }
    PaintersDepth& currentDepth() { return fCurrentDepth; }

    static constexpr int kMaxSlotDim = 512;
    static constexpr int kSlotPadding = 2;

private:
    bool initAtlas(const SkColorInfo& colorInfo, const SkSurfaceProps& props);

    Recorder* fRecorder;
    SkColorInfo fColorInfo;
    SkSurfaceProps fSurfaceProps;
    SkISize fDimensions;

    sk_sp<TextureProxy> fAtlasProxy;
    sk_sp<DrawContext> fAtlasDC;
    std::unique_ptr<skgpu::RectanizerSkyline> fRectanizer;

    std::unique_ptr<BoundsManager> fColorDepthBoundsManager;
    std::unique_ptr<Device::IntersectionTreeSet> fDisjointStencilSet;
    PaintersDepth fCurrentDepth;
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_ScratchDeviceAtlas_DEFINED
