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

    // There is no reason to ask for anything less the alignment of the allocation.
    const size_t leastAllocation = std::max(sizeof(max_align_t), atLeast * sizeOfT);

    if (growthFactor > 1.0) {
        // Calculate the max count given sizeOfT. This will be pinned to INT_MAX. This is mainly
        // a safety measure for 32-bit machines.
        // TODO: maybe maxCount should be passed in.
        const int maxCount = SkToInt(std::min(SIZE_MAX / sizeOfT, SkToSizeT(INT_MAX)));
        SkASSERT_RELEASE(atLeast <= maxCount);

        static constexpr int kMultiple = 8;
        static constexpr int kRounding = kMultiple - 1;

        // Multiply by the growthFactor. Notice that for small values of atLeast, rounding up to
        // kMultiple will provide most of the growth. Pin the highest value so no overflow happens
        // when rounding up below.
        const int growthCount = std::min(atLeast * growthFactor, (double)maxCount - kMultiple);

        // Round up to the next multiple of kMultiple.
        const int roundedGrowthCount = (growthCount + kRounding) & ~kRounding;
        const size_t allocationWithGrowth = roundedGrowthCount * sizeOfT;

        // Ask for the growth amount.
        data = sk_malloc_canfail(allocationWithGrowth);
        size = allocationWithGrowth;
    }

    if (data == nullptr) {
        // Try the atLeast amount.
        data = sk_malloc_throw(leastAllocation);
        size = leastAllocation;
    }

    // Check Skia's standard hook for usable size. If it returns 0 then it's not implemented.
    if (size_t allocatedSize = sk_malloc_usable_size(data)) {
        size = allocatedSize;
    } else {
        // Use the OS specific calls to find the actual allocation size.
        #if defined(SK_BUILD_FOR_MAC) || defined(SK_BUILD_FOR_IOS)
            size = malloc_size(data);
        #elif defined(SK_BUILD_FOR_ANDROID) || defined(SK_BUILD_FOR_UNIX)
            size = malloc_usable_size(data);
        #elif defined(SK_BUILD_FOR_WIN)
            size = _msize(data);
        #endif
        // If none of the above apply, then size is just allocationWithGrowth, or leastAllocation.
    }

    SkASSERT(data != nullptr && size >= leastAllocation);
    return {static_cast<std::byte*>(data), size};
}
