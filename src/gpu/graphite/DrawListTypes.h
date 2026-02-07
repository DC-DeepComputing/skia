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
    kSingle,
    kStencil,
};

struct LayerKey {
    GraphicsPipelineCache::Index fPipelineIndex;
    TextureDataCache::Index fTextureIndex;

    static constexpr LayerKey None() {
        constexpr LayerKey kInvalid = {GraphicsPipelineCache::kInvalidIndex,
                                       TextureDataCache::kInvalidIndex};
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
            : fDrawParams(params), fUniformIndex(uniformIndex) {}

    const DrawParams* fDrawParams;
    const UniformDataCache::Index fUniformIndex;

    SK_DECLARE_INTERNAL_LLIST_INTERFACE(SingleDraw);
};

struct StencilDraws {
    StencilDraws(const LayerKey& key, const RenderStep* step) : fKey(key), fStep(step) {}

    SkTInternalLList<SingleDraw> fDraws;
    const LayerKey fKey;
    const RenderStep* fStep;
    SK_DECLARE_INTERNAL_LLIST_INTERFACE(StencilDraws);
};

struct DepthDraw {
    DepthDraw(const LayerKey& key,
              const RenderStep* step,
              const DrawParams* drawParams,
              const UniformDataCache::Index uniformIndex)
            : fKey(key), fStep(step), fDrawParams(drawParams), fUniformIndex(uniformIndex) {}

    const LayerKey fKey;
    const RenderStep* fStep;
    const DrawParams* fDrawParams;
    const UniformDataCache::Index fUniformIndex;

    SK_DECLARE_INTERNAL_LLIST_INTERFACE(DepthDraw);
};

struct BindingWrapper {
    BindingWrapper(BindingListType type) : fType(type) {}
    const BindingListType fType;
    Rect fBounds = Rect::InfiniteInverted();
    SK_DECLARE_INTERNAL_LLIST_INTERFACE(BindingWrapper);
};

struct SingleDrawList : public BindingWrapper {
    SingleDrawList(BindingListType type) : BindingWrapper(type) {}
    SkTInternalLList<SingleDraw> fDraws;
    LayerKey fKey;
    RenderStep* fStep;
};

struct StencilDrawList : public BindingWrapper {
    StencilDrawList(BindingListType type) : BindingWrapper(type) {}
    SkTInternalLList<StencilDraws> fStencilDraws;
};

struct DepthDrawList {
    Rect fBounds = Rect::InfiniteInverted();
    bool fIsStencil = false;
    SkTInternalLList<DepthDraw> fDraws;
};

struct Layer {
    DepthDrawList* fDepthInfo = nullptr;
    SkTInternalLList<BindingWrapper> fBindings;
    SK_DECLARE_INTERNAL_LLIST_INTERFACE(Layer);

    BindingWrapper* searchBinding(const LayerKey& key, bool disjointStencil) {
        for (BindingWrapper* list : fBindings) {
            if (disjointStencil) {
                if (list->fType == BindingListType::kStencil) {
                    StencilDrawList* stencil = static_cast<StencilDrawList*>(list);
                    for (StencilDraws* s = stencil->fStencilDraws.head(); s; s = s->fNext) {
                        if (s->fKey == key) {
                            return list;
                        }
                    }
                }
            } else {
                if (list->fType == BindingListType::kSingle) {
                    SingleDrawList* single = static_cast<SingleDrawList*>(list);
                    if (single->fKey == key) {
                        return list;
                    }
                }
            }
        }
        return nullptr;
    }

    BoundsTest depthOnlyTest(const Rect& drawBounds, const LayerKey& key, bool disjointStencil) {
        if (disjointStencil && fDepthInfo && fDepthInfo->fIsStencil) {
            if (fDepthInfo->fBounds.intersects(drawBounds)) {
                return BoundsTest::kIncompatibleOverlap;
            }
        }

        for (BindingWrapper* list : fBindings) {
            if (list->fBounds.intersects(drawBounds)) {
                return BoundsTest::kIncompatibleOverlap;
            }
        }

        return BoundsTest::kCompatibleOverlap;
    }

    template <bool kIsStencil>
    std::pair<BoundsTest, BindingWrapper*> test(const Rect& drawBounds, const LayerKey& key) {
        if constexpr (!kIsStencil) {
            if (fBindings.head() && fBindings.head() == fBindings.tail()) {
                if (fBindings.head()->fType == BindingListType::kSingle) {
                    SingleDrawList* single = static_cast<SingleDrawList*>(fBindings.head());
                    if (single->fKey == key) {
                        return {BoundsTest::kCompatibleOverlap, fBindings.head()};
                    }
                }
            }
        }

        BindingWrapper* foundMatch = nullptr;
        if constexpr (kIsStencil) {
            if (fDepthInfo && fDepthInfo->fIsStencil) {
                if (fDepthInfo->fBounds.intersects(drawBounds)) {
                    return {BoundsTest::kIncompatibleOverlap, foundMatch};
                }
            }

            for (BindingWrapper* list : fBindings) {
                if (list->fType == BindingListType::kStencil) {
                    StencilDrawList* stencil = static_cast<StencilDrawList*>(list);
                    for (StencilDraws* s = stencil->fStencilDraws.head(); s; s = s->fNext) {
                        if (s->fKey == key) {
                            foundMatch = list;
                            break;
                        }
                    }
                }
                if (list->fBounds.intersects(drawBounds)) {
                    return {BoundsTest::kIncompatibleOverlap, nullptr};
                }
            }
        } else {
            for (BindingWrapper* list : fBindings) {
                if (list->fType == BindingListType::kSingle) {
                    SingleDrawList* single = static_cast<SingleDrawList*>(list);
                    if (single->fKey == key) {
                        foundMatch = list;
                        continue;
                    }
                }
                if (list->fBounds.intersects(drawBounds)) {
                    return {BoundsTest::kIncompatibleOverlap, nullptr};
                }
            }
        }

        // Note, !foundMatch, but kDisjoint is functionally the same as a kCompatibleOverlap
        return {foundMatch ? BoundsTest::kCompatibleOverlap : BoundsTest::kDisjoint, foundMatch};
    }

    void addDepthOnlyDraw(SkArenaAllocWithReset* alloc,
                          DepthDraw* deferredDraw,
                          bool disjointStencil) {
        if (!fDepthInfo) {
            fDepthInfo = alloc->make<DepthDrawList>();
        }
        fDepthInfo->fDraws.addToTail(deferredDraw);
        fDepthInfo->fBounds.join(deferredDraw->fDrawParams->drawBounds());
    }

    void add(SkArenaAllocWithReset* alloc,
             BindingWrapper* match,
             const LayerKey& key,
             SingleDraw* draw,
             const RenderStep* step,
             bool dependsOnDst) {
        SingleDrawList* single;
        if (match) {
            single = static_cast<SingleDrawList*>(match);
            single->fBounds.join(draw->fDrawParams->drawBounds());
        } else {
            single = alloc->make<SingleDrawList>(BindingListType::kSingle);
            single->fKey = key;
            single->fStep = const_cast<RenderStep*>(step);
            single->fBounds = draw->fDrawParams->drawBounds();
            fBindings.addToTail(single);
        }

        if (!dependsOnDst) {
            single->fDraws.addToHead(draw);
        } else {
            single->fDraws.addToTail(draw);
        }
    }

    void addStencil(SkArenaAllocWithReset* alloc,
                    BindingWrapper*& match,
                    const LayerKey& key,
                    SingleDraw* draw,
                    const RenderStep* step,
                    StencilDraws** startList) {
        StencilDrawList* stencil;
        StencilDraws* searchStart = nullptr;

        if (match) {
            stencil = static_cast<StencilDrawList*>(match);
            stencil->fBounds.join(draw->fDrawParams->drawBounds());
            searchStart = (*startList) ? (*startList)->fNext : stencil->fStencilDraws.head();
        } else {
            stencil = alloc->make<StencilDrawList>(BindingListType::kStencil);
            stencil->fBounds = draw->fDrawParams->drawBounds();
            fBindings.addToTail(stencil);
            match = stencil;
        }

        StencilDraws* sd = nullptr;
        for (StencilDraws* curr = searchStart; curr; curr = curr->fNext) {
            if (curr->fKey == key) {
                sd = curr;
                break;
            }
        }

        if (!sd) {
            sd = alloc->make<StencilDraws>(key, step);
            stencil->fStencilDraws.addToTail(sd);
        }

        sd->fDraws.addToTail(draw);
        *startList = sd;
    }
};

}  // namespace skgpu::graphite

#endif  // skgpu_graphite_DrawListTypes_DEFINED
