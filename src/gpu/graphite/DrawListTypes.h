/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_DrawListTypes_DEFINED
#define skgpu_graphite_DrawListTypes_DEFINED

#include "include/private/base/SkDebug.h"
#include "src/base/SkBlockAllocator.h"
#include "src/base/SkEnumBitMask.h"
#include "src/base/SkTBlockList.h"
#include "src/gpu/graphite/DrawOrder.h"
#include "src/gpu/graphite/DrawParams.h"
#include "src/gpu/graphite/PaintParams.h"
#include "src/gpu/graphite/PipelineData.h"
#include "src/gpu/graphite/geom/Rect.h"
#include "src/gpu/graphite/geom/Transform.h"

#include <cstdint>
#include <functional>
#include <optional>

namespace skgpu::graphite {
    struct LayerKey {
        GraphicsPipelineCache::Index fPipelineIndex;
        UniformDataCache::Index fUniformIndex;
        TextureDataCache::Index fTextureIndex;
        uint32_t fPadding = 0;

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
        RecordedDraw(const RenderStep* step, const DrawParams& params,
                     const BarrierType& barrierBeforeDraws)
            : fStep(step)
            , fDrawParams(params)
            , fBarrierBeforeDraws(barrierBeforeDraws) {}

        const RenderStep* fStep;
        const DrawParams fDrawParams;
        const BarrierType fBarrierBeforeDraws;

        SK_DECLARE_INTERNAL_LLIST_INTERFACE(RecordedDraw);
    };

    struct DeferredDraw {
        DeferredDraw(const LayerKey& key,
                     const RenderStep* step,
                     const DrawParams* drawParams,
                     const BarrierType& barrierBeforeDraws)
                : fKey(key)
                , fStep(step)
                , fDrawParams(drawParams)
                , fBarrierBeforeDraws(barrierBeforeDraws)
                , fChildDraw(nullptr) {
        }

        const LayerKey fKey;
        const RenderStep* fStep;
        const DrawParams* fDrawParams;
        const BarrierType fBarrierBeforeDraws;
        DeferredDraw* fChildDraw;
    };

    struct BindingList {
        LayerKey fKey;
        Rect fBounds = Rect::InfiniteInverted();
        SkTInternalLList<RecordedDraw> fDraws;

        SK_DECLARE_INTERNAL_LLIST_INTERFACE(BindingList);
    };

    struct Layer {
        Rect fDepthOnlyBounds = Rect::InfiniteInverted();
        skia_private::TArray<DeferredDraw*>* fDepthOnlyDraws = nullptr;
        bool fDepthOnlyIsStencil = false;

        SkTInternalLList<BindingList> fBindings;

        SK_DECLARE_INTERNAL_LLIST_INTERFACE(Layer);

        BindingList* searchBinding(const LayerKey& key);

        BoundsTest test(const Rect& drawBounds, const LayerKey& key, bool disjointStencil);
        BoundsTest depthOnlyTest(const Rect& drawBounds, const LayerKey& key, bool disjointStencil);

        void add(SkArenaAllocWithReset* alloc,
                 BindingList* match,
                 const LayerKey& key,
                 RecordedDraw* draw,
                 bool dependsOnDst);

        void addDepthOnlyDraw(SkArenaAllocWithReset* alloc, DeferredDraw* deferredDraw,
                              bool disjointStencil);

#if defined(GPU_TEST_UTILS)
        RecordedDraw* lastDraw() const { return fLastDraw; }
        LayerKey lastLayerKey() const { return fLastKey; }
        RecordedDraw* fLastDraw = nullptr;
        LayerKey fLastKey;
#endif
    };
}

#endif // skgpu_graphite_DrawListTypes_DEFINED
