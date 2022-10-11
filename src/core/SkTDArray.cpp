/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/private/SkMalloc.h"
#include "include/private/SkTDArray.h"
#include "include/private/SkTo.h"

#include <utility>

int CalculateStorageReserve(int newReserve, int sizeOfT) {
    // Establish the maximum number of elements that includes a valid count for end. In the
    // largest case end() = &fArray[INT_MAX] which is 1 after the last indexable element.
    static constexpr int kMaxCount = INT_MAX;

    // Assume that the array will max out.
    int expandedReserve = kMaxCount;
    if (kMaxCount - newReserve > 4) {
        // Add 1/4 more than we need. Add 4 to ensure this grows by at least 1. Pin to
        // kMaxCount if no room for 1/4 growth.
        int growth = 4 + ((newReserve + 4) >> 2);
        // Read this line as: if (count + growth < kMaxCount) { ... }
        // It's rewritten to avoid signed integer overflow.
        if (kMaxCount - newReserve > growth) {
            return newReserve + growth;
        }
    }


    // With a T size of 1, the above allocator produces the progression of 7, 15, ... Since,
    // the sizeof max_align_t is often 16, there is no reason to allocate anything less than
    // 16 bytes. This eliminates a realloc when pushing back bytes to an SkTDArray.
    if (sizeOfT == 1) {
        // Round up to the multiple of 16.
        return (expandedReserve + 15) & ~15;
    }
    return expandedReserve;
}
