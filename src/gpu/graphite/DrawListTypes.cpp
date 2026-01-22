/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/DrawListTypes.h"

namespace skgpu::graphite {

BindingWrapper* Layer::searchBinding(const LayerKey& key) {
    for (BindingWrapper* list : fBindings) {
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
    if (disjointStencil && fDepthOnlyStencil) {
        if (fDepthOnlyBounds.intersects(drawBounds)) {
            return BoundsTest::kIncompatibleOverlap;
        }
    }

    // auto checkIntersection = [&](const auto* typedList) -> bool {
    //     for (const auto* d : typedList->fDraws) {
    //         if (d->fDrawParams->drawBounds().intersects(drawBounds)) {
    //             return true;
    //         }
    //     }
    //     return false;
    // };

    bool foundMatch = false;
    for (BindingWrapper* list : fBindings) {
        if (list->fKey == key) {
            foundMatch = true;
            // If we aren't disjoint stencil, we can overlap with ourselves freely, so we skip the
            // bounds check for this list.
            if (!disjointStencil) continue;
        }

        if (list->fBounds.intersects(drawBounds)) {
            // TODO FINE GRAIN
            //
            // bool overlap = list->fType == BindingListType::kSingle ?
            //                checkIntersection(static_cast<BindingList<SingleDraw>*>(list)) :
            //                checkIntersection(static_cast<BindingList<ChainedDraw>*>(list));
            // if (overlap) {
            //     return BoundsTest::kIncompatibleOverlap;
            // }

            return BoundsTest::kIncompatibleOverlap;
        }
    }

    return foundMatch ? BoundsTest::kCompatibleOverlap : BoundsTest::kDisjoint;
}

BoundsTest Layer::depthOnlyTest(const Rect& drawBounds,
                                const LayerKey& key,
                                bool disjointStencil) {
    if (disjointStencil && fDepthOnlyStencil) {
        // TODO FINE GRAIN
        //
        // for (auto& depthDraw : *fDepthOnlyDraws) {
        //     if (drawBounds.intersects(depthDraw->fDrawParams->drawBounds())) {
        //         return BoundsTest::kIncompatibleOverlap;
        //     }
        // }
        if (fDepthOnlyBounds.intersects(drawBounds)) {
            return BoundsTest::kIncompatibleOverlap;
        }
    }

    // TODO: Understand why depth only draws can't overlap with non-depth draws?
    // breaks ordering in many cases.
    if (true) {  // disjointStencil
        for (BindingWrapper* list : fBindings) {
            if (true) { // list->fType == BindingListType::kChained
                if (list->fBounds.intersects(drawBounds)) {
                    return BoundsTest::kIncompatibleOverlap;
                }
                // TODO FINE GRAIN
                // auto* chainList = static_cast<BindingList<ChainedDraw>*>(list);
                // if (overlap) {
                //     return BoundsTest::kIncompatibleOverlap;
                // }
            }
        }
    }

    return BoundsTest::kCompatibleOverlap;
}

void Layer::addDepthOnlyDraw(SkArenaAllocWithReset* alloc,
                             ChainedDraw* deferredDraw,
                             bool disjointStencil) {
    if (!fDepthOnlyDraws) {
        fDepthOnlyDraws = alloc->make<skia_private::TArray<ChainedDraw*>>();
    }
    fDepthOnlyDraws->push_back(deferredDraw);
    fDepthOnlyBounds.join(deferredDraw->fDrawParams->drawBounds());
    fDepthOnlyStencil |= disjointStencil;
}

} // namespace skgpu::graphite
