/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <atomic>
#include "experimental/graphite/src/UniquePaintParamsID.h"

namespace skgpu {

#if 0
UniquePaintParamsID UniquePaintParamsID::Next() {
    static std::atomic <uint32_t> nextID{1};
    uint32_t id;
    do {
        id = nextID.fetch_add(1, std::memory_order_relaxed);
    } while (id == SK_InvalidUniqueID);

    return UniquePaintParamsID(id);
}
#endif

} // namespace skgpu
