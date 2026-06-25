/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef skgpu_graphite_DrawListLayer_DEFINED
#define skgpu_graphite_DrawListLayer_DEFINED

#include "src/gpu/graphite/DrawListBase.h"

#include "include/private/SkEnumBitMask.h"
#include "src/gpu/graphite/DrawCommands.h"
#include "src/gpu/graphite/DrawListTypes.h"
#include "src/gpu/graphite/DrawOrder.h"
#include "src/gpu/graphite/DrawParams.h"
#include "src/gpu/graphite/PaintParams.h"
#include "src/gpu/graphite/PipelineData.h"
#include "src/gpu/graphite/geom/Transform.h"

#include <cstdint>

namespace skgpu::graphite {

class DrawPass;
class Geometry;
class Renderer;

class DrawListLayer final : public DrawListBase {
public:
    explicit DrawListLayer(bool storageBufferSupport)
        : DrawListBase()
        , fStorageBufferSupport(storageBufferSupport) {}

    // DrawList requires that all Transforms be valid and asserts as much; invalid transforms should
    // be detected at the Device level or similar. The provided Renderer must be compatible with the
    // 'shape' and 'stroke' parameters.
    std::pair<DrawParams*, Layer*> recordDraw(
            const Renderer* renderer,
            const Transform& localToDevice,
            const Geometry& geometry,
            const Clip& clip,
            DrawOrder ordering,
            UniquePaintParamsID paintID,
            SkEnumBitMask<DstUsage> dstUsage,
            BarrierType barrierBeforeDraws,
            PipelineDataGatherer* gatherer,
            const StrokeStyle* stroke,
            Layer* stopLayer) override;

    std::unique_ptr<DrawPass> snapDrawPass(Recorder* recorder,
                                           sk_sp<TextureProxy> target,
                                           const SkImageInfo& targetInfo,
                                           const DstReadStrategy dstReadStrategy) override;

    // Discard all previously recorded draws and set to the requested load op (with optional clear
    // color).
    void reset(LoadOp op, SkColor4f clearColor = {0.f, 0.f, 0.f, 0.f}) override;

private:
    friend class DrawPass;

    std::pair<Layer*, BindingList*> searchBackwards(const RenderStep* step,
                                                    const LayerKey& key,
                                                    SkEnumBitMask<BoundsFlags> testMask,
                                                    const Draw* draw,
                                                    CompressedPaintersOrder stop);

    BindingList* findOrCreateBindingInLayer(Layer* layer,
                                            LayerKey key,
                                            const RenderStep* step,
                                            BindingList* parent);

    SkSTArenaAllocWithReset<256> fStorage;
    SkTInternalLList<Layer> fLayers;

    int fDrawCount = 0;
    CompressedPaintersOrder fOrderCounter = CompressedPaintersOrder::First();

    const bool fStorageBufferSupport;
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_DrawListLayer_DEFINED
