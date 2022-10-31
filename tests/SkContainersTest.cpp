// Copyright 2022 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#include "include/core/SkTypes.h"
#include "include/private/SkContainers.h"

#include "src/core/SkEnumerate.h"

#include "tests/Test.h"

#include <algorithm>
#include <array>
#include <climits>
#include <cstddef>
#include <cstdint>

struct SkContainerAllocatorTestingPeer {
    static size_t RoundUpCapacity(const SkContainerAllocator& a, int64_t capacity) {
        return a.roundUpCapacity(capacity);
    }

    static size_t GrowthFactorCapacity(
            const SkContainerAllocator& a, int capacity, double growthFactor) {
        return a.growthFactorCapacity(capacity, growthFactor);
    }

    static constexpr int64_t kCapacityMultiple = SkContainerAllocator::kCapacityMultiple;
};

static int max_capacity(size_t sizeOfT) {
    return std::min(SIZE_MAX / sizeOfT, (size_t)INT_MAX);
}

DEF_TEST(SkContainerAllocatorBasic, reporter) {
    /* Comment out until a solution can be found.
    // The following test drive the 32-bit machines out of memory so skip them.
    if constexpr (SIZE_MAX > UINT_MAX) {
        {
            SkContainerAllocator a{1, max_capacity(1)};

            SkSpan<std::byte> span = a.allocate(0, 1.0);
            REPORTER_ASSERT(reporter, span.empty());
            sk_free(span.data());

            span = a.allocate(0, 1.5);
            REPORTER_ASSERT(reporter, span.empty());
            sk_free(span.data());

            span = a.allocate(max_capacity(1), 1.0);
            REPORTER_ASSERT(reporter, span.size() >= SkToSizeT(max_capacity(1)));
            sk_free(span.data());
        }

        {
            SkContainerAllocator a{16, max_capacity(16)};

            SkSpan<std::byte> span = a.allocate(0, 1.0);
            REPORTER_ASSERT(reporter, span.empty());
            sk_free(span.data());

            span = a.allocate(0, 1.5);
            REPORTER_ASSERT(reporter, span.empty());
            sk_free(span.data());

            span = a.allocate(max_capacity(16), 1.0);
            REPORTER_ASSERT(reporter, span.size() >= SkToSizeT(max_capacity(16)) * 16);
            sk_free(span.data());
        }
    }  // end skipped on 32-bit Windows tests.
     */

    for (int i = 1; i < 33; ++i) {
        SkContainerAllocator a{(size_t)i, max_capacity(i)};
        int64_t r = SkContainerAllocatorTestingPeer::RoundUpCapacity(a, 1);
        REPORTER_ASSERT(reporter, r == SkContainerAllocatorTestingPeer::kCapacityMultiple);

        r = SkContainerAllocatorTestingPeer::RoundUpCapacity(a, INT_MAX);
        REPORTER_ASSERT(reporter, r == max_capacity(i));

        r = SkContainerAllocatorTestingPeer::RoundUpCapacity(a, max_capacity(i));
        REPORTER_ASSERT(reporter, r == max_capacity(i));
    }

    for (int i = 1; i < 33; ++i) {
        SkContainerAllocator a{(size_t)i, max_capacity(i)};
        int64_t r = SkContainerAllocatorTestingPeer::GrowthFactorCapacity(a, 1, 1.5);
        REPORTER_ASSERT(reporter, r == SkContainerAllocatorTestingPeer::kCapacityMultiple);

        r = SkContainerAllocatorTestingPeer::GrowthFactorCapacity(a, INT_MAX, 1.5);
        REPORTER_ASSERT(reporter, r == max_capacity(i));

        r = SkContainerAllocatorTestingPeer::GrowthFactorCapacity(a, max_capacity(i), 1.5);
        REPORTER_ASSERT(reporter, r == max_capacity(i));
    }
}

namespace skia::containers {
extern SkSpan<std::byte> complete_size(void* ptr, size_t size);
}

static constexpr bool kLogging = false;

static void test_maximum_bite(bool blockerAntagonist, bool reallocFreeAntagonist) {
    const size_t itemSizes[] = {1, 2, 4, 8, 16, 20, 24, 28, 32, 48, 64};
    static constexpr int kIncreases = 8;
    std::vector<int> reallocIsHelpful(std::size(itemSizes));
    std::vector<int> firstHelpSize(std::size(itemSizes), -1);
    std::vector<void*> toFree;
    for (auto [itemSizeIndex, itemSize] : SkMakeEnumerate(itemSizes)) {
        int items = 1;
        for (int i = 0; i <= kIncreases; ++i) {
            auto a = SkContainerAllocator{itemSize, 65536};

            auto span = a.allocate(items, 1.5);

            // Allocate a block the same size right after to block the end of the heap.
            if (blockerAntagonist) {
                toFree.push_back(sk_malloc_throw(span.size_bytes()));
            }
            const int fittedItems = span.size_bytes() / itemSize;

            // Use the allocation routine to calculate the number of items for the next increase.
            const int64_t nextItems =
                    SkContainerAllocatorTestingPeer::GrowthFactorCapacity(a, fittedItems + 1, 1.5);

            void* const ptr = sk_realloc_throw(span.data(), nextItems * itemSize);
            toFree.push_back(ptr);

            // Suck up the freed memory that was just produced by realloc.
            if (reallocFreeAntagonist) {
                toFree.push_back(sk_malloc_throw(span.size_bytes()));
            }

            if constexpr(kLogging) {
                auto span2 = skia::containers::complete_size(ptr, nextItems * itemSize);
                SkDebugf("%d %zu %p %p %zu %zu\n",
                         items,
                         itemSize,
                         span.data(),
                         ptr,
                         span.size_bytes(),
                         span2.size_bytes());
            }

            reallocIsHelpful[itemSizeIndex] += ptr == span.data();
            if (ptr == span.data() && firstHelpSize[itemSizeIndex] == -1) {
                firstHelpSize[itemSizeIndex] = items;
            }
            items = nextItems;
        }
    }
    for (void* p : toFree) {
        sk_free(p);
    }
    SkDebugf("blocker antagonist: %s realloc free antagonist: %s\n",
             blockerAntagonist?"true":"false", reallocFreeAntagonist?"true":"false");
    for (auto [itemSizeIndex, itemSize] : SkMakeEnumerate(itemSizes)) {
        SkDebugf("T size: %zu realloc is helpful: %d / %d first realloc items: %d\n",
             itemSize, reallocIsHelpful[itemSizeIndex], kIncreases, firstHelpSize[itemSizeIndex]);
    }
}

DEF_TEST(TestMaximumBite, r) {
    test_maximum_bite(false, false);
    test_maximum_bite(true, false);
    test_maximum_bite(true, true);
}

