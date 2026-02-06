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
        TextureDataCache::Index fTextureIndex;

        static constexpr LayerKey None() {
            constexpr LayerKey kInvalid = {
                GraphicsPipelineCache::kInvalidIndex,
                TextureDataCache::kInvalidIndex
            };
            return kInvalid;
        }

        // NOTE: removing the uniform index on this check decreases stencil lists on desk_samoa
        // from 602 -> 69
        bool operator==(const LayerKey& other) const {
            return fPipelineIndex == other.fPipelineIndex && fTextureIndex == other.fTextureIndex;
        }

        bool operator!=(const LayerKey& other) const { return !(*this == other); }
    };

    struct SingleDraw {
        SingleDraw(const DrawParams* params, const UniformDataCache::Index uniformIndex)
            : fDrawParams(params)
            , fUniformIndex(uniformIndex) {}

        const DrawParams*   fDrawParams;
        const UniformDataCache::Index fUniformIndex;

        SK_DECLARE_INTERNAL_LLIST_INTERFACE(SingleDraw);
    };

    struct ChainedDraw {
        ChainedDraw(const LayerKey& key, const RenderStep* step, const DrawParams* drawParams,
                    const UniformDataCache::Index uniformIndex)
                : fKey(key)
                , fStep(step)
                , fDrawParams(drawParams)
                , fChildDraw(nullptr)
                , fUniformIndex(uniformIndex) {}

        const LayerKey    fKey;
        const RenderStep* fStep;
        const DrawParams* fDrawParams;
        ChainedDraw*     fChildDraw;
        const UniformDataCache::Index fUniformIndex;

        SK_DECLARE_INTERNAL_LLIST_INTERFACE(ChainedDraw);
    };

    struct DepthDraw {
        DepthDraw(const LayerKey& key, const RenderStep* step, const DrawParams* drawParams,
                    const UniformDataCache::Index uniformIndex)
                : fKey(key)
                , fStep(step)
                , fDrawParams(drawParams)
                , fUniformIndex(uniformIndex) {}

        const LayerKey    fKey;
        const RenderStep* fStep;
        const DrawParams* fDrawParams;
        const UniformDataCache::Index fUniformIndex;
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

    struct DepthDrawInfo {
        Rect fBounds = Rect::InfiniteInverted();
        skia_private::TArray<DepthDraw*>* fDraws = nullptr;
        bool fStencil = false;
    };

    struct Layer {
        DepthDrawInfo* fDepthInfo = nullptr;

        SkTInternalLList<BindingWrapper> fBindings;

        SK_DECLARE_INTERNAL_LLIST_INTERFACE(Layer);

        BindingWrapper* searchBinding(const LayerKey& key);

        BoundsTest test(const Rect& drawBounds, const LayerKey& key, bool disjointStencil);
        BoundsTest depthOnlyTest(const Rect& drawBounds, const LayerKey& key, bool disjointStencil);

        void addDepthOnlyDraw(SkArenaAllocWithReset* alloc, DepthDraw* deferredDraw,
                              bool disjointStencil);

        template<typename T> SK_ALWAYS_INLINE
        void add(SkArenaAllocWithReset* alloc, BindingWrapper* match, const LayerKey& key, T* draw,
                 const RenderStep* step, bool dependsOnDst, bool disjointStencil) {
            if (match) {
                //printf("match\n");
                auto* typedMatch = static_cast<BindingList<T>*>(match);
                if (!dependsOnDst) {
                    typedMatch->fDraws.addToHead(draw);
                } else {
                    typedMatch->fDraws.addToTail(draw);
                }
                typedMatch->fBounds.join(draw->fDrawParams->drawBounds());
                typedMatch->fContainsStencil |= disjointStencil;
            } else {
                //printf("no match %u %u %u\n", key.fPipelineIndex, key.fTextureIndex, key.fUniformIndex);
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
