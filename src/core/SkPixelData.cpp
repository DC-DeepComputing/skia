/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkPixelData.h"
#include <atomic>

uint32_t SkPixelData::NextId() {
    static std::atomic<uint32_t> gNextID{1};
    uint32_t id;
    do {
        id = gNextID.fetch_add(1, std::memory_order_relaxed);
    } while (id == 0);
    return id;
}

SkPixelData::SkPixelData() : fID(NextId()) {}
