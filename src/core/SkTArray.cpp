/*
 * Copyright 2022 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/private/SkTArray.h"

void SkTArrayBase::doRealloc(int64_t newCount, ReallocType reallocType, size_t tSize) {
    int64_t newAllocCount = newCount;
    if (reallocType != kExactFit) {
        // Whether we're growing or shrinking, leave at least 50% extra space for future growth.
        newAllocCount += ((newCount + 1) >> 1);
        // Align the new allocation count to kMinHeapAllocCount.
        static_assert(SkIsPow2(kMinHeapAllocCount), "min alloc count not power of two.");
        newAllocCount = (newAllocCount + (kMinHeapAllocCount - 1)) & ~(kMinHeapAllocCount - 1);
    }

    // At small sizes the old and new alloc count can both be kMinHeapAllocCount.
    if (newAllocCount == fAllocCount) {
        return;
    }

    fAllocCount = SkToU32(Sk64_pin_to_s32(newAllocCount));
    SkASSERT(fAllocCount >= newCount);
    void* newItemArray = sk_malloc_throw((size_t)fAllocCount, tSize);
    sk_careful_memcpy(newItemArray, fItemArray, fCount * tSize);
    if (fOwnMemory) {
        sk_free(fItemArray);
    }
    fItemArray = newItemArray;
    fOwnMemory = true;
    fReserved = false;
}
