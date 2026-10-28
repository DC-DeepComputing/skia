/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/private/SkAssert.h"
#include "include/private/SkPixelStorage.h"
#include <atomic>

uint64_t SkPixelStorage::NextId() {
    static std::atomic<uint64_t> gNextID{1};
    uint64_t id = gNextID.fetch_add(1, std::memory_order_relaxed);
    SkASSERT_RELEASE(id != 0);
    return id;
}

SkPixelStorage::SkPixelStorage(Type type) : fType(type), fID(NextId()) {}
