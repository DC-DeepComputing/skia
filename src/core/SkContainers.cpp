// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#include "src/core/SkContainers.h"

#include "include/private/SkMalloc.h"
#include "include/core/SkSpan.h"
#include "include/core/SkTypes.h"

#include <algorithm>
#include <cstddef>

#if defined(SK_BUILD_FOR_MAC) || defined(SK_BUILD_FOR_IOS)
#include <malloc/malloc.h>
#elif defined(SK_BUILD_FOR_ANDROID) || defined(SK_BUILD_FOR_UNIX)
#include <malloc.h>
#elif defined(SK_BUILD_FOR_WIN)
#include <malloc.h>
#endif

SkSpan<std::byte> sk_allocate(int atLeast, size_t sizeOfT, double growthFactor) {
    SkASSERT(atLeast >= 0);
    SkASSERT(sizeOfT > 0);
    SkASSERT(growthFactor >= 1.0);

    void* data = nullptr;
    size_t size = 0;

    static constexpr size_t kMinSize = sizeof(max_align_t);
    // There is no reason to ask for anything less the alignment of the allocation.
    const size_t leastAllocation = std::max(kMinSize, atLeast * sizeOfT);

    if (growthFactor > 1.0) {
        const int maxCount = SkToInt(std::min(SIZE_MAX / sizeOfT, SkToSizeT(INT_MAX)));
        SkASSERT_RELEASE(atLeast <= maxCount);

        const size_t largestAllocation = maxCount * sizeOfT;

        const size_t allocationWithGrowth =
            std::min(static_cast<size_t>(ceil(leastAllocation * growthFactor)), largestAllocation);

        // Ask for the growth amount.
        data = sk_malloc_canfail(allocationWithGrowth);
        size = allocationWithGrowth;
    }

    if (data == nullptr) {
        // Try the atLeast amount.
        data = sk_malloc_throw(leastAllocation);
        size = atLeast;
    }

    // Check Skia's standard hook for usable size. If it returns 0 then it's not implemented.
    if (size_t allocatedSize = sk_malloc_usable_size(data)) {
        size = allocatedSize;
    } else {
        // Use the OS specific calls to find the actual memory size.
        #if defined(SK_BUILD_FOR_MAC) || defined(SK_BUILD_FOR_IOS)
            size = malloc_size(data);
        #elif defined(SK_BUILD_FOR_ANDROID) || defined(SK_BUILD_FOR_UNIX)
            size = malloc_usable_size(data);
        #elif defined(SK_BUILD_FOR_WIN)
            size = _msize(data);
        #endif
    }

    SkASSERT(data != nullptr && size >= leastAllocation);
    return {static_cast<std::byte*>(data), size};
}
