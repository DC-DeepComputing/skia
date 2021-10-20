/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkAlignedAlloc_DEFINED
#define SkAlignedAlloc_DEFINED

#include "include/private/SkTemplates.h"
#include <memory>

// Creates a heap allocation aligned to the specified kByteAlignment. Delete the pointer with a
// matching call to sk_aligned_free<kByteAlignment>().
template <uintptr_t kByteAlignment>
std::enable_if_t<(kByteAlignment > alignof(void*)), void*> sk_aligned_alloc(size_t size) {
    constexpr static uintptr_t kMask = kByteAlignment - 1;
    static_assert((kByteAlignment & kMask) == 0, "kByteAlignment must be a power of 2");
    // Allocate extra space for alignment and for a header containing the original ptr
    // (for sk_aligned_free()).
    size_t totalAllocCount = kMask + sizeof(uintptr_t) + size;
    if (totalAllocCount < size) {  // Detect size_t overflow.
        sk_out_of_memory();
    }
    uintptr_t originalPtr = (uintptr_t)sk_malloc_throw(totalAllocCount);
    char* ptr = (char*)((originalPtr + sizeof(uintptr_t) + kMask) & ~kMask);
    // Store the original ptr for sk_aligned_free().
    memcpy(ptr - sizeof(uintptr_t), &originalPtr, sizeof(uintptr_t));
    return ptr;
}

// Deletes an allocation previously made by sk_aligned_alloc<kByteAlignment>().
template <uintptr_t kByteAlignment>
std::enable_if_t<(kByteAlignment > alignof(void*)), void> sk_aligned_free(void* ptr) {
    if (!ptr) {
        return;
    }
    uintptr_t originalPtr;
    // Retrieve the original ptr.
    memcpy(&originalPtr, (char*)ptr - sizeof(uintptr_t), sizeof(uintptr_t));
    // Free the original ptr.
    sk_free((void*)originalPtr);
}

// Assume malloc() is already aligned to at least alignof(void*). Allocations with this low of an
// alignment requirement can be passed directly malloc().
template <uintptr_t kByteAlignment>
std::enable_if_t<(kByteAlignment <= alignof(void*)), void*> sk_aligned_alloc(size_t size) {
    return sk_malloc_throw(size);
}

template <uintptr_t kByteAlignment>
std::enable_if_t<(kByteAlignment <= alignof(void*)), void> sk_aligned_free(void* ptr) {
    sk_free(ptr);
}

template <uintptr_t kByteAlignment>
void* sk_aligned_alloc(size_t allocCount, size_t size) {
    uint64_t allocSize = (uint64_t)allocCount * (uint64_t)size;
    if constexpr (std::is_same<size_t, uint32_t>::value) {
        if (allocSize >> 32) {  // Detect 32-bit size_t overflow.
            sk_out_of_memory();
        }
    } else if constexpr (std::is_same<size_t, uint64_t>::value) {
        if (std::max(allocCount, size) > (uint64_t)std::numeric_limits<uint32_t>::max()) {
            // Detect size_t overflow (slightly conservatively).
            if (std::numeric_limits<size_t>::max() / allocCount <= size) {
                sk_out_of_memory();
            }
        }
    } else {
        // size_t overflow detection not implemented.
        sk_out_of_memory();
    }
    return sk_aligned_alloc<kByteAlignment>(allocSize);
}

#endif
