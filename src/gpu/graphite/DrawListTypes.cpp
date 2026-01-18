/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/DrawListTypes.h"

namespace skgpu::graphite {

BindingList* Layer::searchBinding(const LayerKey& key) {
    for (BindingList* list : fBindings) {
        if (list->fKey == key) {
            return list;
        }
    }
    return nullptr;
}

BoundsTest Layer::test(const Rect& drawBounds,
                       const LayerKey& key,
                       bool disjointStencil) {
    if (!disjointStencil && fBindings.head() &&
        fBindings.head() == fBindings.tail() && fBindings.head()->fKey == key) {
        return BoundsTest::kCompatibleOverlap;
    }

    // If this is a stencil, then we must check if the deferred draws are stencil, and if so, check
    // the bounds as well.
    if (disjointStencil && fDepthOnlyIsStencil) {
        if (fDepthOnlyBounds.intersects(drawBounds)) {
            return BoundsTest::kIncompatibleOverlap;
        }
    }

    bool foundMatch = false;
    for (BindingList* list : fBindings) {
        if (list->fKey == key) {
            foundMatch = true;
            // If we aren't disjoint stencil, we can overlap with ourselves freely, so we skip the
            // bounds check for this list.
            if (!disjointStencil) continue;
        }

        if (list->fBounds.intersects(drawBounds)) {
            for (const RecordedDraw* d : list->fDraws) {
                if (d->fDrawParams.drawBounds().intersects(drawBounds)) {
                    return BoundsTest::kIncompatibleOverlap;
                }
            }
        }
    }

    return foundMatch ? BoundsTest::kCompatibleOverlap : BoundsTest::kDisjoint;
}

BoundsTest Layer::depthOnlyTest(const Rect& drawBounds,
                                const LayerKey& key,
                                bool disjointStencil) {
    if (fDepthOnlyDraws) {
        for (auto& depthDraw : *fDepthOnlyDraws) {
            if (drawBounds.intersects(depthDraw->fDrawParams.drawBounds())) {
                return BoundsTest::kIncompatibleOverlap;
            }
        }
    }

    if (disjointStencil) {
         for (BindingList* list : fBindings) {
            if (list->fBounds.intersects(drawBounds)) {
                for (const RecordedDraw* d : list->fDraws) {
                    if (d->fDrawParams.drawBounds().intersects(drawBounds)) {
                        return BoundsTest::kIncompatibleOverlap;
                    }
                }
            }
        }
    }

    return BoundsTest::kCompatibleOverlap;
}

void Layer::add(SkArenaAllocWithReset* alloc,
                BindingList* match,
                const LayerKey& key,
                RecordedDraw* draw,
                bool dependsOnDst) {
    if (match) {
        if (!dependsOnDst) {
            match->fDraws.addToHead(draw);
        } else {
            match->fDraws.addToTail(draw);
        }
        match->fBounds.join(draw->fDrawParams.drawBounds());
    } else {
        BindingList* list = alloc->make<BindingList>();
        list->fKey = key;
        list->fBounds = draw->fDrawParams.drawBounds();
        list->fDraws.addToHead(draw);
        fBindings.addToTail(list);
    }

#ifdef GPU_TEST_UTILS
    this->fLastDraw = draw;
    this->fLastKey = key;
#endif
}

void Layer::addDepthOnlyDraw(SkArenaAllocWithReset* alloc,
                             DeferredDraw* deferredDraw,
                             bool disjointStencil) {
    if (!fDepthOnlyDraws) {
        fDepthOnlyDraws = alloc->make<skia_private::TArray<DeferredDraw*>>();
    }
    fDepthOnlyDraws->push_back(deferredDraw);
    fDepthOnlyBounds.join(deferredDraw->fDrawParams.drawBounds());
    fDepthOnlyIsStencil |= disjointStencil;
}

} // namespace skgpu::graphite
