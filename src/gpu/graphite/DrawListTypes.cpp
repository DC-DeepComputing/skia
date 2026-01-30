/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/DrawListTypes.h"

#if defined(_MSC_VER)
    #define FORCE_INLINE __forceinline
#else
    #define FORCE_INLINE __attribute__((always_inline))
#endif

namespace skgpu::graphite {

FORCE_INLINE
BindingList* Layer::searchBinding(const LayerKey& key) {
    for (BindingList* list : fBindings) {
        if (list->fKey == key) {
            return list;
        }
    }
    return nullptr;
}

FORCE_INLINE
BoundsTest Layer::test(const Rect& drawBounds,
                       const LayerKey& key,
                       bool disjointStencil) {
    if (!disjointStencil && fBindings.head() &&
        fBindings.head() == fBindings.tail() && fBindings.head()->fKey == key) {
        return BoundsTest::kCompatibleOverlap;
    }

    // If this is a stencil, then we must check if the deferred draws are stencil, and if so, check
    // the bounds as well.
    if (disjointStencil && fDepthOnlyStencil) {
        if (fDepthOnlyBounds.intersects(drawBounds)) {
            return BoundsTest::kIncompatibleOverlap;
        }
    }

    bool foundMatch = false;
    if (disjointStencil) {
        for (BindingList* list : fBindings) {
            if (list->fKey == key) {
                foundMatch = true;
            }

            if (list->fContainsStencil && list->fBounds.intersects(drawBounds)) {
                return BoundsTest::kIncompatibleOverlap;
            }
        }
    } else {
        for (BindingList* list : fBindings) {
            if (list->fKey == key) {
                foundMatch = true;
                // If we aren't disjoint stencil, we can overlap with ourselves freely, so we skip the
                // bounds check for this list.
                continue;
            }

            if (!list->fContainsStencil && list->fBounds.intersects(drawBounds)) {
                return BoundsTest::kIncompatibleOverlap;
            }
        }
    }

    // Note, !foundMatch, but kDisjoint is functionally the same as a kCompatibleOverlap
    return foundMatch ? BoundsTest::kCompatibleOverlap : BoundsTest::kDisjoint;
}

FORCE_INLINE
BoundsTest Layer::depthOnlyTest(const Rect& drawBounds,
                                const LayerKey& key,
                                bool disjointStencil) {
    if (disjointStencil && fDepthOnlyStencil) {
        if (fDepthOnlyBounds.intersects(drawBounds)) {
            return BoundsTest::kIncompatibleOverlap;
        }
    }

    for (BindingList* list : fBindings) {
        if ((disjointStencil ^ list->fContainsStencil) && list->fBounds.intersects(drawBounds)) {
            return BoundsTest::kIncompatibleOverlap;
        }
    }

    return BoundsTest::kCompatibleOverlap;
}

FORCE_INLINE
void Layer::add(SkArenaAllocWithReset* alloc, BindingList* match, const LayerKey& key,
                  RecordedDraw* draw, bool dependsOnDst, bool disjointStencil) {
    if (match) {
        if (!dependsOnDst) {
            match->fDraws.addToHead(draw);
        } else {
            match->fDraws.addToTail(draw);
        }
        match->fBounds.join(draw->fDrawParams->drawBounds());
    } else {
        BindingList* list = alloc->make<BindingList>(disjointStencil);
        list->fKey = key;
        list->fBounds = draw->fDrawParams->drawBounds();
        list->fDraws.addToHead(draw);
        fBindings.addToTail(list);
    }
}

FORCE_INLINE
void Layer::addDepthOnlyDraw(SkArenaAllocWithReset* alloc,
                             DeferredDraw* deferredDraw,
                             bool disjointStencil) {
    if (!fDepthOnlyDraws) {
        fDepthOnlyDraws = alloc->make<skia_private::TArray<DeferredDraw*>>();
    }
    fDepthOnlyDraws->push_back(deferredDraw);
    fDepthOnlyBounds.join(deferredDraw->fDrawParams->drawBounds());
    fDepthOnlyStencil |= disjointStencil;
}

} // namespace skgpu::graphite

#undef FORCE_INLINE
