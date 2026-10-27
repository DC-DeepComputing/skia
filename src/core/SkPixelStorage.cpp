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

SkPixelStorage::SkPixelStorage(Type type)
        : fType(type)
        , fID(NextId()) {}

SkPixelStorage::~SkPixelStorage() {
    if (SkWeakRefCnt* token = fWeakToken.load(std::memory_order_acquire)) {
        token->unref();
    }
}

const SkWeakRefCnt* SkPixelStorage::weakToken() const {
    SkWeakRefCnt* token = fWeakToken.load(std::memory_order_acquire);
    if (!token) {
        SkWeakRefCnt* created = new SkWeakRefCnt();
        if (fWeakToken.compare_exchange_strong(token, created,
                                               std::memory_order_acq_rel,
                                               std::memory_order_acquire)) {
            token = created;
        } else {
            created->unref();
        }
    }
    return token;
}
