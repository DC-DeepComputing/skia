/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_MEMORYPOOL
#define SKSL_MEMORYPOOL

#include <memory>

#include "include/core/SkTypes.h"
#include "src/base/SkArenaAlloc.h"

namespace SkSL {

class MemoryPool {
public:
    static std::unique_ptr<MemoryPool> Make() {
        return std::unique_ptr<MemoryPool>(new MemoryPool);
    }
    void* allocate(size_t size) {
        return fArena.makeBytesAlignedTo(size, sizeof(void*));
    }
    void release(void*) {
        // SkArenaAlloc does not ever attempt to reclaim space.
    }

private:
    MemoryPool() : fArena(fBlock, sizeof(fBlock), /*firstHeapAllocation=*/sizeof(fBlock) / 2) {}

    SkArenaAllocWithReset fArena;
    char fBlock[65536];
};

}  // namespace SkSL

#endif // SKSL_MEMORYPOOL
