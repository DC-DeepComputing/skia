/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef skgpu_graphite_DrawListLayer_DEFINED
#define skgpu_graphite_DrawListLayer_DEFINED

#include "src/gpu/graphite/DrawListBase.h"

#include "include/private/SkDebug.h"
#include "include/private/SkEnumBitMask.h"
#include "src/core/SkBlockAllocator.h"
#include "src/core/SkTBlockList.h"
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
            Layer* lastInsertion) override;

    std::unique_ptr<DrawPass> snapDrawPass(Recorder* recorder,
                                           sk_sp<TextureProxy> target,
                                           const SkImageInfo& targetInfo,
                                           const DstReadStrategy dstReadStrategy) override;

    // Discard all previously recorded draws and set to the requested load op (with optional clear
    // color).
    void reset(LoadOp op, SkColor4f clearColor = {0.f, 0.f, 0.f, 0.f}) override;

private:
#if defined(USE_BINDING_TREE2)

#if defined(BULK_BINDING_BOUNDS)
    std::pair<Layer*, PipelineNode*> searchBackwards(SkEnumBitMask<BoundsFlags> testMask,
                           const DrawParams* drawParams,
                           const LayerKey& key,
                           CompressedPaintersOrder stop);
#else
    Layer* searchBackwards(SkEnumBitMask<BoundsFlags> testMask,
                           const DrawParams* drawParams,
                           CompressedPaintersOrder stop);
#endif

#else
    std::pair<Layer*, BindingList*> searchBackwards(const RenderStep* step,
                                                    const LayerKey& key,
                                                    SkEnumBitMask<BoundsFlags> testMask,
                                                    const DrawParams* drawParams,
                                                    CompressedPaintersOrder stop);

    BindingList* findOrCreateBindingInLayer(Layer* layer,
                                            BindingList* parent,
                                            const RenderStep* step,
                                            const LayerKey& key);
#endif

    int fLayerSearchCount = 0;
    int fSearchCount = 0;
    int fMaxLayerSearchCount = 0;

    struct Stats {
        int fTotalLayerCount = 0;
        int fTotalBindingListCount = 0;
        int fTotalDrawCount = 0;

        int fMaxBindingListsPerLayer = 0;
        float fAvgBindingListsPerLayer = 0;

        int fMaxDrawsPerLayer = 0;
        int fMaxDrawsPerBindingList = 0;
        float fAvgDrawsPerLayer = 0;
        float fAvgDrawsPerBindingList = 0;

        int fMaxTestDepth = 0;
        float fAvgTestDepth = 0.f;

#if defined(USE_BINDING_TREE2)
        int fTotalPipelineCount = 0;

        int fMaxPipelinesPerLayer = 0;
        float fAvgPipelinesPerLayer = 0;

        int fMaxDrawsPerPipeline = 0;
        float fAvgDrawsPerPipeline = 0;

        int fMaxBindingsPerPipeline = 0;
        float fAvgBindingsPerPipeline =0 ;
#endif
    };

    void dumpStats() {
        Stats stats;
        for (const Layer* layer : fLayers) {
            Layer::Stats l = layer->collectStats();
            stats.fTotalLayerCount++;
            stats.fTotalBindingListCount += l.fBindingListCount;
            stats.fTotalDrawCount += l.fTotalDrawCount;

            stats.fMaxBindingListsPerLayer = std::max(stats.fMaxBindingListsPerLayer, l.fBindingListCount);
            stats.fMaxDrawsPerLayer = std::max(stats.fMaxDrawsPerLayer, l.fTotalDrawCount);
            stats.fMaxDrawsPerBindingList = std::max(stats.fMaxDrawsPerBindingList, l.fMaxDrawsPerBindingList);
            stats.fAvgDrawsPerBindingList += l.fAvgDrawsPerBindingList;

            stats.fMaxTestDepth = std::max(stats.fMaxTestDepth, l.fMaxTestDepth);
            stats.fAvgTestDepth += l.fAvgTestDepth;

#if defined(USE_BINDING_TREE2)
            stats.fTotalPipelineCount += l.fPipelineCount;
            stats.fMaxPipelinesPerLayer = std::max(stats.fMaxPipelinesPerLayer, l.fPipelineCount);

            stats.fMaxDrawsPerPipeline = std::max(stats.fMaxDrawsPerPipeline, l.fMaxDrawsPerPipeline);
            stats.fAvgDrawsPerPipeline += l.fAvgDrawsPerPipeline;

            stats.fMaxBindingsPerPipeline = std::max(stats.fMaxBindingsPerPipeline, l.fMaxBindingsPerPipeline);
            stats.fAvgBindingsPerPipeline += l.fAvgBindingsPerPipeline;
#endif
        }
        stats.fAvgBindingListsPerLayer = stats.fTotalBindingListCount / (float) stats.fTotalLayerCount;
        stats.fAvgDrawsPerLayer = stats.fTotalDrawCount / (float) stats.fTotalLayerCount;
        stats.fAvgDrawsPerBindingList /= stats.fTotalLayerCount;
        stats.fAvgTestDepth /= stats.fTotalLayerCount;

#if defined(USE_BINDING_TREE2)
        stats.fAvgPipelinesPerLayer = stats.fTotalPipelineCount / (float) stats.fTotalLayerCount;
        stats.fAvgDrawsPerPipeline /= stats.fTotalLayerCount;
        stats.fAvgBindingsPerPipeline /= stats.fTotalLayerCount;
#endif

        SkDebugf("Layer stats:\n"
                 " - Total layers = %d, total binding lists = %d, total draws = %d\n"
                 " - Max binding lists / layer = %d, max draws / layer = %d, max draws / binding list = %d, max test depth = %d\n"
                 " - Avg binding lists / layer = %.2f, avg draws / layer = %.2f, avg draws / binding list = %.2f, avg test depth = %.2f\n"
                 " - Max layer search / draw = %d, avg layer search / draw = %.2f\n",
                stats.fTotalLayerCount, stats.fTotalBindingListCount, stats.fTotalDrawCount,
                stats.fMaxBindingListsPerLayer, stats.fMaxDrawsPerLayer, stats.fMaxDrawsPerBindingList, stats.fMaxTestDepth,
                stats.fAvgBindingListsPerLayer, stats.fAvgDrawsPerLayer, stats.fAvgDrawsPerBindingList, stats.fAvgTestDepth,
                fMaxLayerSearchCount, fLayerSearchCount / (float) fSearchCount);

#if defined(USE_BINDING_TREE2)
        SkDebugf(" - Total pipelines = %d, max pipelines / layer = %d, avg pipelines / layer = %.2f\n"
                 " - Max bindings / pipeline = %d, avg bindings / pipeline = %.2f\n"
                 " - Max draws / pipeline = %d, avg draws / pipeline = %.2f\n",
                stats.fTotalPipelineCount, stats.fMaxPipelinesPerLayer, stats.fAvgPipelinesPerLayer,
                stats.fMaxBindingsPerPipeline, stats.fAvgBindingsPerPipeline,
                stats.fMaxDrawsPerPipeline, stats.fAvgDrawsPerPipeline);
#endif

        fLayerSearchCount = 0;
        fSearchCount = 0;
        fMaxLayerSearchCount = 0;
    }

    friend class DrawPass;

    SkSTArenaAllocWithReset<256> fStorage;
    SkTInternalLList<Layer> fLayers;

    int fDrawCount = 0;
    CompressedPaintersOrder fOrderCounter = CompressedPaintersOrder::First();

    const bool fStorageBufferSupport;
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_DrawListLayer_DEFINED
