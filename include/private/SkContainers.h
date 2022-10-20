// Copyright 2022 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#ifndef SkContainers_DEFINED
#define SkContainers_DEFINED

#include "include/core/SkSpan.h"

class SkContainerAllocator {
public:
    SkContainerAllocator(size_t sizeOfT, int maxCapacity)
            : fSizeOfT{sizeOfT}
            , fMaxCapacity{maxCapacity} {}

    // allocate will abort on failure. It will return the empty span given a capacity of 0.
    SkSpan<std::byte> allocate(int capacity, double growthFactor = 1.0);

private:
    friend struct SkContainerAllocatorTestingPeer;
    // All capacity counts will be rounded up to kCapacityMultiple.
    // TODO: this is a constant from the original SkTArray code. This should be checked some how.
    static constexpr int64_t kCapacityMultiple = 8;

    // Round up capacity to next multiple of kCapacityMultiple and pin to fMaxCapacity.
    size_t roundUpCapacity(int64_t capacity) const;

    // Grow the capacity by growthFactor being sure to stay with in kMinBytes and fMaxCapacity.
    size_t growthFactorCapacity(int capacity, double growthFactor) const;

    const size_t fSizeOfT;
    const int64_t fMaxCapacity;
};

// sk_allocate_canfail will return the empty span on failure. Parameter size must be > 0.
SkSpan<std::byte> sk_allocate_canfail(size_t size);

// sk_allocate_throw will abort on failure. It will return the empty span given 0.
SkSpan<std::byte> sk_allocate_throw(size_t size);
#endif  // SkContainers_DEFINED
