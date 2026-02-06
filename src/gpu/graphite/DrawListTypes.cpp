/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/DrawListTypes.h"

namespace skgpu::graphite {

// TODO (thomsmit) Test bounded fine grain intersection checking performance
BindingWrapper* Layer::searchBinding(const LayerKey& key) {
    if (fLastMatch && fLastMatch->fKey == key) {
        return fLastMatch;
    }

    for (BindingWrapper* list : fBindings) {
        if (list->fKey == key) {
            fLastMatch = list;
            return list;
        }
    }
    return nullptr;
}

BoundsTest Layer::test(const Rect& drawBounds, const LayerKey& key, bool disjointStencil) {
    if (!disjointStencil && fBindings.head() &&
        fBindings.head() == fBindings.tail() && fBindings.head()->fKey == key) {
        fLastMatch = fBindings.head();
        return BoundsTest::kCompatibleOverlap;
    }

    // If this is a stencil, then we must check if the deferred draws are stencil, and if so, check
    // the bounds as well.
    if (disjointStencil && fDepthInfo && fDepthInfo->fIsStencil) {
        if (fDepthInfo->fBounds.intersects(drawBounds)) {
            return BoundsTest::kIncompatibleOverlap;
        }
    }

    bool foundMatch = false;
    for (BindingWrapper* list : fBindings) {
        if (list->fKey == key) {
            foundMatch = true;
            // If we aren't disjoint stencil, we can overlap with ourselves freely, so we skip the
            // bounds check for this list.
            if (!disjointStencil) continue;
        }

        if (list->fBounds.intersects(drawBounds)) {
            return BoundsTest::kIncompatibleOverlap;
        }
    }

    // Note, !foundMatch, but kDisjoint is functionally the same as a kCompatibleOverlap
    return foundMatch ? BoundsTest::kCompatibleOverlap : BoundsTest::kDisjoint;
}

BoundsTest Layer::depthOnlyTest(const Rect& drawBounds, const LayerKey& key, bool disjointStencil) {
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

void Layer::addDepthOnlyDraw(SkArenaAllocWithReset* alloc, DepthDraw* deferredDraw,
                             bool disjointStencil) {
    if (!fDepthInfo) {
        fDepthInfo = alloc->make<DepthDrawList>();
    }
    fDepthInfo->fDraws.addToTail(deferredDraw);
    fDepthInfo->fBounds.join(deferredDraw->fDrawParams->drawBounds());
}

} // namespace skgpu::graphite
