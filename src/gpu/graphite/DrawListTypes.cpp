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

}  // namespace skgpu::graphite
