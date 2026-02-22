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

struct LayerKey {
    static constexpr uint32_t kPipelineMatchBit = 4;
    static constexpr uint32_t kTextureMatchBit  = 2;
    static constexpr uint32_t kExactMatchMask   = kPipelineMatchBit | kTextureMatchBit;

    GraphicsPipelineCache::Index fPipelineIndex;
    TextureDataCache::Index fTextureIndex;

    static constexpr LayerKey None() {
        return {GraphicsPipelineCache::kInvalidIndex, TextureDataCache::kInvalidIndex};
    }

    uint8_t matchMask(const LayerKey& other) const {
        return (fPipelineIndex == other.fPipelineIndex ? kPipelineMatchBit : 0) |
               (fTextureIndex  == other.fTextureIndex  ? kTextureMatchBit  : 0);
    }

    // NOTE: removing the uniform index on this check decreases stencil lists on desk_samoa
    // from 602 -> 69
    bool operator==(const LayerKey& other) const {
        return fPipelineIndex == other.fPipelineIndex && fTextureIndex == other.fTextureIndex;
    }

    bool operator!=(const LayerKey& other) const { return !(*this == other); }

    static bool isExactMatch(uint8_t mask) {
        return mask == kExactMatchMask;
    }
};

struct Draw {
    Draw(const DrawParams* params, const UniformDataCache::Index uniformIndex)
            : fDrawParams(params), fUniformIndex(uniformIndex) {}
    const DrawParams* fDrawParams;
    const UniformDataCache::Index fUniformIndex;
    SK_DECLARE_INTERNAL_LLIST_INTERFACE(Draw);
};

struct BindingList {
    BindingList(const CompressedPaintersOrder& order, bool isShading, bool isStencil)
            : fOrder(order), fIsShading(isShading), fIsStencil(isStencil) {}
    static constexpr uint32_t kCoarseBoundsThreshold = 32;

    CompressedPaintersOrder fOrder;
    const bool fIsShading; // TODO (thomsmit): an extra byte but saves masking?
    const bool fIsStencil; // worth?
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

    template <bool kIsStencil, bool kIsDepthOnly, bool kForwards>
    SK_ALWAYS_INLINE std::tuple<BoundsTest, BindingList*, uint8_t> test(const Rect& drawBounds,
                                                                        const LayerKey& key,
                                                                        bool requiresBarrier,
                                                                        BindingList* startList) {
        BindingList* bestMatch = nullptr;
        uint8_t bestMatchMask = 0;

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
            uint8_t matchMask = key.matchMask(list->fKey);
            if (LayerKey::isExactMatch(matchMask)) {
                bestMatchMask = matchMask;
                bestMatch = list;
                // If it's a depth only draw, it's ok to overlap with itself, and will ignore
                // the below intersection check. If it's a stencil draw it is never ok to overlap
                // with itself.
                if constexpr (!kIsDepthOnly && !kIsStencil) {
                    if (!requiresBarrier) continue;
                }
            }

            if constexpr (!kIsStencil) {
                // If it's not a stencil draw, it is either a shading or depth-only draw. Both are
                // allowed to intersect freely with existing depth-only draws. Clipped shading draws
                // are guaranteed to be stopped from passing their parent depth-only draws by the
                // insertion mechanism, not by intersection testing. However, depth-only draws may
                // not bypass shading draws they intersect with, because modifying the depth buffer
                // out of order would corrupt the occlusion and depth-test results for those shading
                // draws, breaking the painter's algorithm.
                if (list->fIsShading && list->intersects(drawBounds)) {
                    return {BoundsTest::kIncompatibleOverlap, bestMatch, 0};
                }
            } else {
                // If it is a stencil draw, it can still be shading or non-shading, this is because
                // we treat both the stencil and the cover step as shading. Furthermore, this
                // could also be the shading step of a depth only draw.
                if (key.fTextureIndex == TextureDataCache::kInvalidIndex) { // Implies non-shading
                    // If the draw is non-shading, it is free to intersect with any non-stencil draw
                    if (list->fIsStencil && list->intersects(drawBounds)) {
                        return {BoundsTest::kIncompatibleOverlap, bestMatch, 0};
                    }
                } else {
                    // If the draw is shading, it may intersect with any non-shading and non-stencil
                    // draw.
                    if ((list->fIsShading || list->fIsStencil) && list->intersects(drawBounds)) {
                        return {BoundsTest::kIncompatibleOverlap, bestMatch, 0};
                    }
                }
            }

            if (matchMask > bestMatchMask) {
                bestMatchMask = matchMask;
                bestMatch = list;
            }
        }

        // Note, !foundMatch, but kDisjoint is functionally the same as a kCompatibleOverlap
        return {bestMatchMask == LayerKey::kExactMatchMask ? BoundsTest::kCompatibleOverlap
                                                           : BoundsTest::kDisjoint,
                bestMatch,
                bestMatchMask};
    }

    template <bool kIsDepthOnly>
    SK_ALWAYS_INLINE BindingList* add(SkArenaAllocWithReset* alloc,
                                      BindingList* match,
                                      const LayerKey& key,
                                      Draw* draw,
                                      const RenderStep* step,
                                      bool insertBefore,
                                      bool isStencil,
                                      uint8_t matchMask) {
        if (!LayerKey::isExactMatch(matchMask)) {
            fListOrder = fListOrder.next();
            BindingList* list = alloc->make<BindingList>(fListOrder,
                                            key.fTextureIndex != TextureDataCache::kInvalidIndex,
                                            isStencil);
            list->fKey = key;
            list->fStep = const_cast<RenderStep*>(step);
            list->fBounds = draw->fDrawParams->drawBounds();
            if (matchMask != 0) {
                SkASSERT(match);
                fBindings.addAfter(list, match);
            } else {
                SkASSERT(!match);
                if constexpr (kIsDepthOnly) {
                    fBindings.addToHead(list);
                } else {
                    fBindings.addToTail(list);
                }
            }
            match = list;
        }

        match->fBounds.join(draw->fDrawParams->drawBounds());

        if (insertBefore) {
            match->fDraws.addToHead(draw);
        } else {
            match->fDraws.addToTail(draw);
        }

        match->fDrawCount++;

        return match;
    }
};

struct Insertion {
    Layer* layer;
    BindingList* list;
};

}  // namespace skgpu::graphite

#endif  // skgpu_graphite_DrawListTypes_DEFINED
