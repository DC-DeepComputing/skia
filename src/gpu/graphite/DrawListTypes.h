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

struct Draw {
    Draw(const DrawParams* params, const UniformDataCache::Index uniformIndex)
            : fDrawParams(params), fUniformIndex(uniformIndex) {}
    const DrawParams* fDrawParams;
    const UniformDataCache::Index fUniformIndex;
    SK_DECLARE_INTERNAL_LLIST_INTERFACE(Draw);
};

struct BindingList {
    BindingList(const CompressedPaintersOrder& order, bool isDepthOnly)
            : fOrder(order), fIsDepthOnly(isDepthOnly) {}
    static constexpr uint32_t kCoarseBoundsThreshold = 32;

    CompressedPaintersOrder fOrder;
    const bool fIsDepthOnly;
    LayerKey fKey;
    RenderStep* fStep;
    uint32_t fDrawCount = 0;
    Rect fBounds = Rect::InfiniteInverted();
    SkTInternalLList<Draw> fDraws;
    SK_DECLARE_INTERNAL_LLIST_INTERFACE(BindingList);

    bool intersects(const Rect& drawBounds) const {
        if (!fBounds.intersects(drawBounds)) {
            return false;
        }
        if (fDrawCount > kCoarseBoundsThreshold) {
            return true;
        }
        for (const Draw* d = fDraws.head(); d; d = d->fNext) {
            if (d->fDrawParams->drawBounds().intersects(drawBounds)) {
                return true;
            }
        }
        return false;
    }
};

struct Layer {
    Layer(const CompressedPaintersOrder& order) : fOrder(order) {}

    const CompressedPaintersOrder fOrder;
    CompressedPaintersOrder fListOrder = CompressedPaintersOrder::First();
    SkTInternalLList<BindingList> fBindings;
    SK_DECLARE_INTERNAL_LLIST_INTERFACE(Layer);

    // Search backwards and towards the start, inclusive. Matching on the start is valid, as
    // the insertion is guaranteed to be appendTail
    BindingList* searchBinding(const LayerKey& key, BindingList* startList) {
        BindingList* end = startList ? startList->fPrev : nullptr;
        for (BindingList* list = fBindings.tail(); list != end; list = list->fPrev) {
            if (list->fKey == key) {
                return list;
            }
        }
        return nullptr;
    }

    // Note, for the purposes of allowing intersections with non-shading draws, we only delineate
    // between depthOnlyDraws and nonDepthOnly draws. Although the stencil part of stencil renderers
    // are also non-shading, and thus could be passed through by shading draws, in practice there
    // are very few scenarios where this increases batching/performance. This is because---regardless
    // of the direction of the traversal---the shading part of the stencil renderer is 1) likely
    // very close by 2) will stop any dependsOnDst draw anyways.
    //
    // This was implemented in https://skia-review.git.corp.google.com/c/skia/+/1171836 and slightly
    // regresses performance due to the overhead it introduces.
    template <bool kIsStencil, bool kForwards>
    SK_ALWAYS_INLINE std::pair<BoundsTest, BindingList*> test(const Rect& drawBounds,
                                                                 const LayerKey& key,
                                                                 bool isDepthOnly,
                                                                 bool requiresBarrier,
                                                                 BindingList* startList) {
        BindingList* foundMatch = nullptr;
        BindingList* list;
        BindingList* end;
        if constexpr (kForwards) {
            list = startList ? startList : fBindings.head();
            end = nullptr;
        } else {
            list = fBindings.tail();
            end = startList ? startList->fPrev : nullptr;
        }
        // Advancement is also constexpr
        for (; list != end; list = kForwards ? list->fNext : list->fPrev) {
            if (list->fKey == key) {
                foundMatch = list;
                // If it's a depth only draw, it's ok to overlap with itself, and will ignore
                // the below intersection check. If it's a stencil draw it is never ok to overlap
                // with itself.
                if (!isDepthOnly && !kIsStencil) {
                    if (!requiresBarrier) continue;
                }
            }
            // Stencil draws always check for intersection.
            if constexpr (!kIsStencil) {
                // If it's not a stencil draw, it is either a shading or depth-only draw. Both are
                // allowed to intersect freely with existing depth-only draws. Clipped shading draws
                // are guaranteed to be stopped from passing their parent depth-only draws by the
                // insertion mechanism, not by intersection testing. However, depth-only draws may
                // not bypass shading draws they intersect with, because modifying the depth buffer
                // out of order would corrupt the occlusion and depth-test results for those shading
                // draws, breaking the painter's algorithm.
                if (!list->fIsDepthOnly && list->intersects(drawBounds)) {
                    return {BoundsTest::kIncompatibleOverlap, foundMatch};
                }
            } else {
                if (list->intersects(drawBounds)) {
                    return {BoundsTest::kIncompatibleOverlap, foundMatch};
                }
            }
        }

        // Note, !foundMatch, but kDisjoint is functionally the same as a kCompatibleOverlap
        return {foundMatch ? BoundsTest::kCompatibleOverlap : BoundsTest::kDisjoint, foundMatch};
    }

    SK_ALWAYS_INLINE BindingList* add(SkArenaAllocWithReset* alloc,
                                      BindingList* list,
                                      const LayerKey& key,
                                      Draw* draw,
                                      const RenderStep* step,
                                      bool isDepthOnly) {
        if (list) {
            list->fBounds.join(draw->fDrawParams->drawBounds());
        } else {
            fListOrder = fListOrder.next();
            list = alloc->make<BindingList>(fListOrder, isDepthOnly);
            list->fKey = key;
            list->fStep = const_cast<RenderStep*>(step);
            list->fBounds = draw->fDrawParams->drawBounds();
            fBindings.addToTail(list);
        }

        list->fDraws.addToTail(draw);
        list->fDrawCount++;

        return list;
    }
};

struct Insertion {
    Layer* layer;
    BindingList* list;
};

}  // namespace skgpu::graphite

#endif  // skgpu_graphite_DrawListTypes_DEFINED
