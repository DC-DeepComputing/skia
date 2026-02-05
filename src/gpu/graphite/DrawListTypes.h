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
#include "src/gpu/graphite/DrawTypes.h"
#include "src/gpu/graphite/PaintParams.h"
#include "src/gpu/graphite/PipelineData.h"
#include "src/gpu/graphite/geom/Rect.h"
#include "src/gpu/graphite/geom/Transform.h"

#include <cstdint>
#include <functional>
#include <optional>

namespace skgpu::graphite {
    enum class BoundsTest {
        kDisjoint,
        kCompatibleOverlap,
        kIncompatibleOverlap,
    };

    enum class BindingListType {
        kChained,
        kSingle,
    };

    struct LayerKey {
        GraphicsPipelineCache::Index fPipelineIndex;
        UniformDataCache::Index fUniformIndex;
        TextureDataCache::Index fTextureIndex;
        uint32_t fPadding = 0;

        static constexpr LayerKey None() {
            constexpr LayerKey kInvalid = {
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

    struct SingleDraw {
        SingleDraw(const DrawParams* params)
            : fDrawParams(params) {}

        const DrawParams*   fDrawParams;

        SK_DECLARE_INTERNAL_LLIST_INTERFACE(SingleDraw);
    };

    struct ChainedDraw {
        ChainedDraw(const LayerKey& key, const RenderStep* step, const DrawParams* drawParams)
                : fKey(key)
                , fStep(step)
                , fDrawParams(drawParams)
                , fChildDraw(nullptr) {}

        const LayerKey    fKey;
        const RenderStep* fStep;
        const DrawParams* fDrawParams;
        ChainedDraw*     fChildDraw;

        SK_DECLARE_INTERNAL_LLIST_INTERFACE(ChainedDraw);
    };

    struct BindingWrapper {
        BindingWrapper(BindingListType type) : fType(type) {}

        const BindingListType fType;
        LayerKey fKey;        // Duplicated for chained, but saves static casting otherwise?
        RenderStep* fStep;    // ``
        Rect fBounds = Rect::InfiniteInverted();
        bool fContainsStencil = false;

        SK_DECLARE_INTERNAL_LLIST_INTERFACE(BindingWrapper);
    };

    template <typename T>
    struct BindingList : public BindingWrapper {
        using DrawType = T;
        SkTInternalLList<T> fDraws;
        BindingList(BindingListType type) : BindingWrapper(type) {}
    };

    struct Layer {
        // TODO Replace with BindingList? Likely a little more overhead.
        Rect fDepthOnlyBounds = Rect::InfiniteInverted();
        skia_private::TArray<ChainedDraw*>* fDepthOnlyDraws = nullptr;
        bool fDepthOnlyStencil = false;

        SkTInternalLList<BindingWrapper> fBindings;

        SK_DECLARE_INTERNAL_LLIST_INTERFACE(Layer);

        BindingWrapper* searchBinding(const LayerKey& key);

        BoundsTest test(const Rect& drawBounds, const LayerKey& key, bool disjointStencil);
        BoundsTest depthOnlyTest(const Rect& drawBounds, const LayerKey& key, bool disjointStencil);

        void addDepthOnlyDraw(SkArenaAllocWithReset* alloc, ChainedDraw* deferredDraw,
                              bool disjointStencil);

        template<typename T>
        void add(SkArenaAllocWithReset* alloc, BindingWrapper* match, const LayerKey& key, T* draw,
                 const RenderStep* step, bool dependsOnDst, bool disjointStencil) {
            if (match) {
                auto* typedMatch = static_cast<BindingList<T>*>(match);
                if (!dependsOnDst) {
                    typedMatch->fDraws.addToHead(draw);
                } else {
                    typedMatch->fDraws.addToTail(draw);
                }
                typedMatch->fBounds.join(draw->fDrawParams->drawBounds());
                typedMatch->fContainsStencil |= disjointStencil;
            } else {
                bool isSingle = std::is_same_v<T, SingleDraw>;
                BindingListType type = isSingle ? BindingListType::kSingle
                                                : BindingListType::kChained;
                BindingList<T>* list = alloc->make<BindingList<T>>(type);
                list->fKey = key;
                if (isSingle) {
                    list->fStep = const_cast<RenderStep*>(step); //TODO
                }
                list->fBounds = draw->fDrawParams->drawBounds();
                list->fDraws.addToHead(draw);
                list->fContainsStencil = disjointStencil;
                fBindings.addToTail(list);
            }
        }
    };
} // namespace skgpu::graphite

#endif // skgpu_graphite_DrawListTypes_DEFINED
