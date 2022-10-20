// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#include "src/core/SkContainers.h"

#include "include/private/SkMalloc.h"
#include "include/core/SkSpan.h"
#include "include/core/SkTypes.h"
#include "include/private/SkTArray.h"

#include <algorithm>
#include <cstddef>

#if defined(SK_BUILD_FOR_MAC) || defined(SK_BUILD_FOR_IOS)
#include <malloc/malloc.h>
#elif defined(SK_BUILD_FOR_ANDROID) || defined(SK_BUILD_FOR_UNIX)
#include <malloc.h>
#elif defined(SK_BUILD_FOR_WIN)
#include <malloc.h>
#endif

SkSpan<std::byte> sk_allocate(SkAllocationCapacity allocationCapacity) {
    void* data = nullptr;
    size_t size = 0;

    auto [leastCapacity, growthCapacity] = allocationCapacity;

    if (leastCapacity == 0 && growthCapacity == 0) { return {nullptr, 0}; }

    if (growthCapacity > 0) {
        // Ask for the growth amount.
        data = sk_malloc_canfail(growthCapacity);
        size = growthCapacity;
    }

    if (data == nullptr) {
        // Try the atLeast amount.
        data = sk_malloc_throw(leastCapacity);
        size = leastCapacity;
    }

    // Check Skia's standard hook for usable size. If it returns 0 then it's not implemented.
    if (size_t allocatedSize = sk_malloc_usable_size(data)) {
        size = allocatedSize;
    } else {
        // Use the OS specific calls to find the actual capacity.
        #if defined(SK_BUILD_FOR_MAC) || defined(SK_BUILD_FOR_IOS)
            size = malloc_size(data);
        #elif defined(SK_BUILD_FOR_ANDROID) || defined(SK_BUILD_FOR_UNIX)
            size = malloc_usable_size(data);
        #elif defined(SK_BUILD_FOR_WIN)
            size = _msize(data);
        #endif
        // If none of the above apply, then size is just leastCapacity, or growthCapacity.
    }

    SkASSERT(data != nullptr && size >= leastCapacity);
    return {static_cast<std::byte*>(data), size};
}
