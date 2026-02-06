/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/DrawListTypes.h"

namespace skgpu::graphite {

namespace {

}

BindingWrapper* Layer::searchBinding(const LayerKey& key, bool disjointStencil) {
    if (fLastMatch && *fLastKey == key) {
        return fLastMatch;
    }

    if (disjointStencil) {
        for (BindingWrapper* list : fBindings) {
            if (list->fType == BindingListType::kStencil) {
                StencilDrawList* stencil = static_cast<StencilDrawList*>(list);
                for (auto s : stencil->fStencilDraws) {
                    if (s && s->fKey == key) {
                        fLastMatch = list;
                        fLastKey = const_cast<LayerKey*>(&s->fKey);
                        return list;
                    }
                }
            }
        }
    } else {
        for (BindingWrapper* list : fBindings) {
            if (list->fType == BindingListType::kSingle) {
                SingleDrawList* single = static_cast<SingleDrawList*>(list);
                if (single->fKey == key) {
                    fLastMatch = list;
                    fLastKey = &single->fKey;
                    return list;
                }
            }
        }
    }
    return nullptr;
}

// TODO: (thomsmit) This may be sufficiently divergent that templating out might be called for.
BoundsTest Layer::test(const Rect& drawBounds, const LayerKey& key, bool disjointStencil) {

    if (!disjointStencil && fBindings.head() && fBindings.head() == fBindings.tail()) {
        if (fBindings.head()->fType != BindingListType::kStencil) {
            SingleDrawList* single = static_cast<SingleDrawList*>(fBindings.head());
            if (single->fKey == key) {
                fLastMatch = fBindings.head();
                fLastKey = &single->fKey;
                return BoundsTest::kCompatibleOverlap;
            }
        }
    }

    bool foundMatch = false;
    if (disjointStencil) {
        // If this is a stencil, then we must check if the deferred draws are stencil, and if so,
        // check the bounds as well.
        if (fDepthInfo && fDepthInfo->fIsStencil) {
            if (fDepthInfo->fBounds.intersects(drawBounds)) {
                return BoundsTest::kIncompatibleOverlap;
            }
        }

        for (BindingWrapper* list : fBindings) {
            if (list->fType == BindingListType::kStencil) {
                StencilDrawList* stencil = static_cast<StencilDrawList*>(list);
                for (auto s : stencil->fStencilDraws) {
                    if (s && s->fKey == key) {
                        fLastMatch = list;
                        fLastKey = const_cast<LayerKey*>(&s->fKey);
                        foundMatch = true;
                    }
                }
            }
            if (list->fBounds.intersects(drawBounds)) {
                return BoundsTest::kIncompatibleOverlap;
            }
        }
    } else {
        for (BindingWrapper* list : fBindings) {
            if (list->fType == BindingListType::kSingle) {
                SingleDrawList* single = static_cast<SingleDrawList*>(list);
                fLastMatch = list;
                fLastKey = &single->fKey;
                foundMatch = true;
                continue;
            }
            if (list->fBounds.intersects(drawBounds)) {
                return BoundsTest::kIncompatibleOverlap;
            }
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

// And possibly template this as well?
void Layer::add(SkArenaAllocWithReset* alloc, BindingWrapper*& match, const LayerKey& key,
                SingleDraw* draw, const RenderStep* step, bool dependsOnDst, bool disjointStencil, int stepIndex) {
    if (match) {
        if (disjointStencil) {
            StencilDrawList* stencil = static_cast<StencilDrawList*>(match);
            StencilDraws* sd = stencil->fStencilDraws[stepIndex];
            if (sd) {
                sd->fDraws.push_back(draw);
            } else {
                // FIXED: Removed 'alloc' argument
                sd = alloc->make<StencilDraws>(key, step);
                sd->fDraws.push_back(draw);
                stencil->fStencilDraws[stepIndex] = sd;
            }
        } else {
            SingleDrawList* single = static_cast<SingleDrawList*>(match);
            if (!dependsOnDst) {
                single->fDraws.addToHead(draw);
            } else {
                single->fDraws.addToTail(draw);
            }
        }
        match->fBounds.join(draw->fDrawParams->drawBounds());
    } else {
        if (disjointStencil) {
            StencilDrawList* list = alloc->make<StencilDrawList>(BindingListType::kStencil);
            list->fStencilDraws.fill(nullptr); // Important: std::array pointers are not zero-init by default

            StencilDraws* sd = alloc->make<StencilDraws>(key, step);
            sd->fDraws.push_back(draw);
            list->fStencilDraws[stepIndex] = sd;

            list->fBounds = draw->fDrawParams->drawBounds();
            fBindings.addToTail(list);
            match = list;
        } else {
            SingleDrawList* list = alloc->make<SingleDrawList>(BindingListType::kSingle);
            list->fKey = key;
            list->fStep = const_cast<RenderStep*>(step);
            list->fBounds = draw->fDrawParams->drawBounds();

            list->fDraws.addToHead(draw);
            fBindings.addToTail(list);
        }
    }
}

} // namespace skgpu::graphite
