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

    void recordStep(const RenderStep* step,
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

        // Uses SkTInternalLList for compatibility with SkArenaAlloc pointers
        SK_DECLARE_INTERNAL_LLIST_INTERFACE(RecordedDraw);
    };

    struct BindingList {
        LayerKey fKey;
        Rect fBounds = Rect::InfiniteInverted();

        // The specific draws for this binding key
        SkTInternalLList<RecordedDraw> fDraws;

        SK_DECLARE_INTERNAL_LLIST_INTERFACE(BindingList);
    };

    struct Layer {
        Rect fBounds = Rect::InfiniteInverted();

        // The list of bindings (batches) within this Z-layer
        SkTInternalLList<BindingList> fBindings;

        SK_DECLARE_INTERNAL_LLIST_INTERFACE(Layer);

        std::pair<BoundsTest, BindingList*> test(const Rect& drawBounds,
                                                 const LayerKey& key,
                                                 bool disjointStencil);

        void add(SkArenaAllocWithReset* alloc,
                 BindingList* match,
                 const LayerKey& key,
                 const RenderStep* step,
                 const DrawParams& params,
                 bool dependsOnDst);

        bool canExtractBindingList(const BindingList* candidate);
    };

    void lookForBindingMatch(LayerKey binding);

    const Transform& deduplicateTransform(const Transform&);

    SkTBlockList<Transform, 16> fTransforms{SkBlockAllocator::GrowthPolicy::kFibonacci};

    SkArenaAllocWithReset fStorage{kDefaultAllocation};
    SkTInternalLList<Layer> fLayers;

    int fRenderStepCount = 0;

#if defined(SK_DEBUG)
    int fCoverageMaskShapeDrawCount = 0;
#endif

    Rect fDstReadBounds = Rect::InfiniteInverted();
    Rect fPassBounds = Rect::InfiniteInverted();
    bool fRequiresMSAA = false;
    SkEnumBitMask<DepthStencilFlags> fDepthStencilFlags = DepthStencilFlags::kNone;

    UniformDataCache fUniformDataCache;
    TextureDataCache fTextureDataCache;
    GraphicsPipelineCache fPipelineCache;

    LoadOp fLoadOp = LoadOp::kLoad;
    std::array<float, 4> fClearColor = {0.f, 0.f, 0.f, 0.f};
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_DrawListLayer_DEFINED
