/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkFibBlockSizes_DEFINED
#define SkFibBlockSizes_DEFINED

#include "include/private/base/SkASAN.h"
#include "include/private/base/SkAssert.h"
#include "include/private/base/SkSpan_impl.h"
#include "include/private/base/SkTo.h"

#include <algorithm>
#include <array>

// We found allocating strictly doubling amounts of memory from the heap left too
// much unused slop, particularly on Android.  Instead we'll follow a Fibonacci-like
// progression.

// SkFibonacci47 is the first 47 Fibonacci numbers. Fib(47) is the largest value less than 2 ^ 32.
extern std::array<const uint32_t, 47> SkFibonacci47;
template<uint32_t kMaxSize>
class SkFibBlockSizes {
public:
    // staticBlockSize, and firstAllocationSize are parameters describing the initial memory
    // layout. staticBlockSize describes the size of the inlined memory, and firstAllocationSize
    // describes the size of the first block to be allocated if the static block is exhausted. By
    // convention, firstAllocationSize is the first choice for the block unit size followed by
    // staticBlockSize followed by the default of 1024 bytes.
    SkFibBlockSizes(uint32_t staticBlockSize, uint32_t firstAllocationSize) : fIndex{0} {
        fBlockUnitSize = firstAllocationSize > 0 ? firstAllocationSize :
                         staticBlockSize     > 0 ? staticBlockSize     : 1024;

        SkASSERT_RELEASE(0 < fBlockUnitSize);
        SkASSERT_RELEASE(fBlockUnitSize < std::min(kMaxSize, (1u << 26) - 1));
    }

    uint32_t nextBlockSize() {
        uint32_t result = SkFibonacci47[fIndex] * fBlockUnitSize;

        if (SkTo<size_t>(fIndex + 1) < SkFibonacci47.size() &&
            SkFibonacci47[fIndex + 1] < kMaxSize / fBlockUnitSize)
        {
            fIndex += 1;
        }

        return result;
    }

private:
    uint32_t fIndex : 6;
    uint32_t fBlockUnitSize : 26;
};

#endif  // SkFibBlockSizes_DEFINED
