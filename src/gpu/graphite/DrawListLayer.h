/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef skgpu_graphite_DrawListLayer_DEFINED
#define skgpu_graphite_DrawListLayer_DEFINED

#include "include/gpu/graphite/GraphiteTypes.h"

#include "include/private/base/SkDebug.h"
#include "src/base/SkBlockAllocator.h"
#include "src/base/SkEnumBitMask.h"
#include "src/base/SkTBlockList.h"
#include "src/gpu/graphite/ContextUtils.h"
#include "src/gpu/graphite/DrawCommands.h"
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

class DrawListLayer {
public:
    // Add a construtor to prevent default zero initialization of SkTBlockList members' storage.
    DrawListLayer() {}

    void recordStep(int stepIndex,
                    const RenderStep* step,
                    GraphicsPipelineCache::Index pipelineIndex,
                    UniformDataCache::Index uniformIndex,
                    TextureDataCache::Index textureIndex,
                    const DrawParams& drawParams,
                    SkEnumBitMask<DstUsage> dstUsage);

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
                    const StrokeStyle* stroke);

    std::unique_ptr<DrawPass> snapDrawPass(Recorder* recorder,
                                           sk_sp<TextureProxy> target,
                                           const SkImageInfo& targetInfo,
                                           const DstReadStrategy dstReadStrategy);

    int renderStepCount() const { return fRenderStepCount; }

    bool modifiesTarget() const {
        return this->renderStepCount() > 0 || fLoadOp == LoadOp::kClear;
    }

    bool samplesTexture(const TextureProxy* texture) const {
        return fTextureDataCache.hasTexture(texture);
    }

    // Discard all previously recorded draws and set to the requested load op (with optional clear
    // color).
    void reset(LoadOp op, SkColor4f clearColor = {0.f, 0.f, 0.f, 0.f});

    // Bounds for a dst read required by this DrawList. These bounds are only valid if drawsReadDst
    // returns true.
    const Rect& dstReadBounds() const { return fDstReadBounds; }
    const Rect& passBounds() const { return fPassBounds; }
    bool drawsReadDst() const { return !fDstReadBounds.isEmptyNegativeOrNaN(); }
    bool drawsRequireMSAA() const { return fRequiresMSAA; }
    SkEnumBitMask<DepthStencilFlags> depthStencilFlags() const { return fDepthStencilFlags; }

    SkDEBUGCODE(bool hasCoverageMaskDraws() const { return fCoverageMaskShapeDrawCount > 0; })

private:
    friend class DrawPass;
    friend class SimpleBoundsLayerSortTest;

    static constexpr uint32_t kMaxSearchLimit = 32;
    static constexpr uint32_t kDefaultAllocation = 4096; //TODO can be moved upstream as an option

    struct Draw {
    public:
        Draw(const Renderer* renderer, const Transform& transform, const Geometry& geometry,
             const Clip& clip, DrawOrder order, BarrierType barrierBeforeDraws,
             const StrokeStyle* stroke)
                : fRenderer(renderer)
                , fDrawParams(transform, geometry, clip, order, stroke)
                , fBarrierBeforeDraws(barrierBeforeDraws) {}

        const Renderer* renderer()                             const { return fRenderer;           }
        const DrawParams& drawParams()                         const { return fDrawParams;         }
        const BarrierType& barrierBeforeDraws()                const { return fBarrierBeforeDraws; }

    private:
        const Renderer* fRenderer; // Owned by SharedContext of Recorder that recorded the draw
        DrawParams fDrawParams; // The DrawParam's transform is owned by fTransforms of the DrawList
        BarrierType fBarrierBeforeDraws;
    };

    struct LayerKey {
        GraphicsPipelineCache::Index fPipelineIndex;
        UniformDataCache::Index fUniformIndex;
        TextureDataCache::Index fTextureIndex;
        uint32_t fPadding = 0;  // Try to enable autovectorization by padding out to 128-bits

        static const LayerKey& kNone() {
            static const LayerKey kInvalid = {
                GraphicsPipelineCache::kInvalidIndex,
                UniformDataCache::kInvalidIndex,
                TextureDataCache::kInvalidIndex
            };
            return kInvalid;
        }

        bool operator==(const LayerKey& other) const {
            return fPipelineIndex == other.fPipelineIndex &&
                fUniformIndex == other.fUniformIndex &&
                fTextureIndex == other.fTextureIndex;
        }

        bool operator!=(const LayerKey& other) const { return !(*this == other); }
    };

    struct RecordedDraw {
        RecordedDraw(const RenderStep* step, const DrawParams& params)
            : fStep(step)
            , fDrawParams(params) {}

        const RenderStep* fStep;
        DrawParams fDrawParams;

        SK_DECLARE_INTERNAL_LLIST_INTERFACE(RecordedDraw);
    };

    // A BindingList is a collection of draws that share the same rendering state.
    // All draws within the same BindingList must share the same LayerKey, allowing
    // them to be batched into a single GPU submission.
    //
    // Unlike the parent Layer (which ensures separate BindingLists are disjoint),
    // draws *within* the same BindingList are allowed to overlap. Because they
    // share a state, their Z-ordering is handled locally by the order of the
    // 'fDraws' linked list (Painter's Algorithm).
    struct BindingList {
        LayerKey fKey;

        Rect fBounds = Rect::InfiniteInverted();

        SkTInternalLList<RecordedDraw> fDraws;

        SK_DECLARE_INTERNAL_LLIST_INTERFACE(BindingList);
    };

    // A Layer is a collection of BindingLists that act as a single logical "Z-plane"
    // in the render sequence.
    //
    // GUARANTEES:
    // 1. Geometric Disjointness: No two BindingLists within the same Layer overlap
    //    spatially. If List A and List B represent different keys in this layer,
    //    their bounds are guaranteed not to intersect.
    // 2. Order Independence: Because of #1, the renderer can execute the BindingLists
    //    in this layer in any order (e.g., to minimize state changes) without violating
    //    Painter's Algorithm correctness.
    //
    // FORWARD MERGING:
    // The disjoint guarantee allows for "Forward Merging" (Promotion). If a new draw
    // overlaps an existing BindingList in an older layer, we can "steal" that existing
    // list and move it up to the new/current layer. This prevents the new draw from
    // breaking the existing batch, allowing us to combine old and new geometry into
    // a single draw call.
    struct Layer {
        // The collection of non-overlapping batches contained in this depth slice.
        SkTInternalLList<BindingList> fBindings;

        SK_DECLARE_INTERNAL_LLIST_INTERFACE(Layer);

        BindingList* searchBinding(const LayerKey& key);

        BoundsTest test(const Rect& drawBounds, const LayerKey& key, bool disjointStencil);

        void add(SkArenaAllocWithReset* alloc,
                 BindingList* match,
                 const LayerKey& key,
                 const RenderStep* step,
                 const DrawParams& params,
                 bool dependsOnDst);

#if defined(GPU_TEST_UTILS)
        RecordedDraw* lastDraw() const { return fLastDraw; }
        LayerKey lastLayerKey() const { return fLastKey; }
        RecordedDraw* fLastDraw = nullptr;
        LayerKey fLastKey;
#endif
    };

    void lookForBindingMatch(LayerKey binding);

    const Transform& deduplicateTransform(const Transform&);

    SkTBlockList<Transform, 16> fTransforms{SkBlockAllocator::GrowthPolicy::kFibonacci};

    SkArenaAllocWithReset fStorage{kDefaultAllocation};
    SkTInternalLList<Layer> fLayers;

    int fRenderStepCount = 0;

    Rect fDstReadBounds = Rect::InfiniteInverted();
    Rect fPassBounds = Rect::InfiniteInverted();
    bool fRequiresMSAA = false;
    SkEnumBitMask<DepthStencilFlags> fDepthStencilFlags = DepthStencilFlags::kNone;

    UniformDataCache fUniformDataCache;
    TextureDataCache fTextureDataCache;
    GraphicsPipelineCache fPipelineCache;

    LoadOp fLoadOp = LoadOp::kLoad;
    std::array<float, 4> fClearColor = {0.f, 0.f, 0.f, 0.f};

    // Some hooks for the layer testing
#if defined(GPU_TEST_UTILS)
    Layer* lastLayer() const { return fLastLayer; }
    Layer* fLastLayer = nullptr;

    using DrawObserver = std::function<void(int, const LayerKey&, const RecordedDraw*)>;
    DrawObserver fDrawObserver;
    void setDrawObserver(DrawObserver observer) {
        fDrawObserver = std::move(observer);
    }
#endif

#if defined(SK_DEBUG)
    int fCoverageMaskShapeDrawCount = 0;
#endif
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_DrawListLayer_DEFINED
