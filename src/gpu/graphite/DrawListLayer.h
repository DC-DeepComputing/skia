/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef skgpu_graphite_DrawListLayer_DEFINED
#define skgpu_graphite_DrawListLayer_DEFINED

#include "src/gpu/graphite/DrawListBase.h"

#include "include/private/base/SkDebug.h"
#include "src/base/SkBlockAllocator.h"
#include "src/base/SkEnumBitMask.h"
#include "src/base/SkTBlockList.h"
#include "src/gpu/graphite/ContextUtils.h"
#include "src/gpu/graphite/DrawCommands.h"
#include "src/gpu/graphite/DrawListTypes.h"
#include "src/gpu/graphite/DrawOrder.h"
#include "src/gpu/graphite/DrawParams.h"
#include "src/gpu/graphite/PaintParams.h"
#include "src/gpu/graphite/PipelineData.h"
#include "src/gpu/graphite/geom/Rect.h"
#include "src/gpu/graphite/geom/Transform.h"

#include <cstdint>
#include <functional>
#include <limits>
#include <optional>

namespace skgpu::graphite {

class DrawPass;
class Geometry;
class Renderer;

class DrawListLayer final : public DrawListBase {
public:
    // Add a construtor to prevent default zero initialization of SkTBlockList members' storage.
    DrawListLayer() : DrawListBase() {}

    // DrawList requires that all Transforms be valid and asserts as much; invalid transforms should
    // be detected at the Device level or similar. The provided Renderer must be compatible with the
    // 'shape' and 'stroke' parameters. If the renderer uses coverage AA, 'ordering' must have a
    // compressed painters order that reflects that. If the renderer uses stencil, the 'ordering'
    // must have a valid stencil index as well.
    void recordDraw(const Renderer* renderer,
                    const Transform& localToDevice,
                    const Geometry& geometry,
                    const Clip& clip,
                    DrawOrder ordering,
                    UniquePaintParamsID paintID,
                    SkEnumBitMask<DstUsage> dstUsage,
                    BarrierType barrierBeforeDraws,
                    PipelineDataGatherer* gatherer,
                    const StrokeStyle* stroke,
                    std::optional<std::reference_wrapper<DrawParams*>> captureParams = {},
                    std::optional<std::reference_wrapper<Layer*>> layer = {}) override;

    std::unique_ptr<DrawPass> snapDrawPass(Recorder* recorder,
                                           sk_sp<TextureProxy> target,
                                           const SkImageInfo& targetInfo,
                                           const DstReadStrategy dstReadStrategy) override;

    // Discard all previously recorded draws and set to the requested load op (with optional clear
    // color).
    void reset(LoadOp op, SkColor4f clearColor = {0.f, 0.f, 0.f, 0.f}) override;

private:
    template <bool kIsStencil>
    void recordBackwards(int stepIndex,
                         bool dependsOnDst,
                         const RenderStep* step,
                         const LayerKey& key,
                         const DrawParams* drawParams,
                         const BarrierType& barrierBeforeDraws);

    void recordStep(int stepIndex,
                    bool dependsOnDst,
                    const RenderStep* step,
                    const LayerKey& key,
                    const DrawParams* drawParams,
                    const BarrierType& barrierBeforeDraws);

    void recordStencil(int stepIndex,
                       bool dependsOnDst,
                       const RenderStep* step,
                       const LayerKey& key,
                       const DrawParams* drawParams,
                       const BarrierType& barrierBeforeDraws);

    void recordClipped(int stepIndex,
                       bool isStencil,
                       bool dependsOnDst,
                       const RenderStep* step,
                       const LayerKey& key,
                       const DrawParams* drawParams,
                       const BarrierType& barrierBeforeDraws,
                       std::optional<std::reference_wrapper<Layer*>> startLayer);

    void recordDepthOnly(int stepIndex,
                         bool isStencil,
                         bool dependsOnDst,
                         const RenderStep* step,
                         const LayerKey& key,
                         const DrawParams* drawParams,
                         const BarrierType& barrierBeforeDraws,
                         std::optional<std::reference_wrapper<Layer*>> captureLayer);

    friend class DrawPass;
    friend class SimpleBoundsLayerSortTest;

    static constexpr uint32_t kMaxSearchLimit = 32;
    static constexpr uint32_t kDefaultAllocation = 4096; //TODO can be moved upstream as an option

    SkArenaAllocWithReset fStorage{kDefaultAllocation};
    SkTInternalLList<Layer> fLayers;

    int fDrawCount = 0;
    ChainedDraw* fLastDeferredDraw = nullptr;
    ChainedDraw* fLastRecordedDraw = nullptr;

    // Some hooks for the layer testing
#if defined(GPU_TEST_UTILS)
    Layer* lastLayer() const { return fLastLayer; }
    Layer* fLastLayer = nullptr;

    using DrawObserver = std::function<void(int, const LayerKey&, const SingleDraw*)>;
    DrawObserver fDrawObserver;
    void setDrawObserver(DrawObserver observer) {
        fDrawObserver = std::move(observer);
    }
#endif
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_DrawListLayer_DEFINED
