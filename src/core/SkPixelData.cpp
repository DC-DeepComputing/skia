/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkPixelData.h"
#include <atomic>
#include <cstdint>

uint32_t SkPixelData::getId() {
    static std::atomic<uint32_t> gNextID{1};
    if (!fId) {
        uint32_t id;
        do {
            id = gNextID.fetch_add(1, std::memory_order_relaxed);
        } while (id == 0);
        fId = id;
    }
    return fId;
}
