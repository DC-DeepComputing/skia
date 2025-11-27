/*
 * Copyright 2025 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "tools/partition_alloc/TestSupport.h"

#if defined(SKIA_ENABLE_PARTITION_ALLOC)
#include <partition_alloc/dangling_raw_ptr_checks.h>
#include <partition_alloc/shim/allocator_shim.h>
#include <partition_alloc/shim/allocator_shim_default_dispatch_to_partition_alloc.h>
#include <partition_alloc/buildflags.h>
#endif

namespace skiatest {

void InitializePartitionAllocForTesting() {
#if defined(SKIA_ENABLE_PARTITION_ALLOC)
    allocator_shim::ConfigurePartitionsForTesting();
    allocator_shim::internal::PartitionAllocMalloc::Allocator()->EnableThreadCacheIfSupported();

    // To simulate a chromium realistic environment, act as if skia was in a
    // foreground process and set the largest cached size using the same limit.
    static constexpr size_t kThreadCacheLargeSizeThreshold = 1 << 15;
    ::partition_alloc::ThreadCache::SetLargestCachedSize(kThreadCacheLargeSizeThreshold);
    allocator_shim::AdjustDefaultAllocatorForForeground();
#endif
}

}  // namespace skiatest
