/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/DrawListTypes.h"

namespace skgpu::graphite {

namespace {}

BindingWrapper* Layer::searchBinding(const LayerKey& key, bool disjointStencil) {
    if (fLastMatch && *fLastKey == key) {
        return fLastMatch;
    }

    auto updateMatch = [&](BindingWrapper* list, const LayerKey* foundKey) {
        fLastMatch = list;
        fLastKey = const_cast<LayerKey*>(foundKey);
        return list;
    };

    for (BindingWrapper* list : fBindings) {
        if (disjointStencil) {
            if (list->fType == BindingListType::kStencil) {
                StencilDrawList* stencil = static_cast<StencilDrawList*>(list);
                for (StencilDraws* s = stencil->fStencilDraws.head(); s; s = s->fNext) {
                    if (s->fKey == key) {
                        return updateMatch(list, &s->fKey);
                    }
                }
            }
        } else {
            if (list->fType == BindingListType::kSingle) {
                SingleDrawList* single = static_cast<SingleDrawList*>(list);
                if (single->fKey == key) {
                    return updateMatch(list, &single->fKey);
                }
            }
        }
    }
    return nullptr;
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

void Layer::addDepthOnlyDraw(SkArenaAllocWithReset* alloc,
                             DepthDraw* deferredDraw,
                             bool disjointStencil) {
    if (!fDepthInfo) {
        fDepthInfo = alloc->make<DepthDrawList>();
    }
    fDepthInfo->fDraws.addToTail(deferredDraw);
    fDepthInfo->fBounds.join(deferredDraw->fDrawParams->drawBounds());
}

void Layer::add(SkArenaAllocWithReset* alloc,
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

void Layer::addStencil(SkArenaAllocWithReset* alloc,
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

}  // namespace skgpu::graphite