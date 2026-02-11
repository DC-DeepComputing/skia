/*
 * Copyright 2026 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "tools/partition_alloc/TestSupport.h"

#if defined(SKIA_ENABLE_PARTITION_ALLOC)
#include <partition_alloc/dangling_raw_ptr_checks.h>
#include <partition_alloc/shim/allocator_shim.h>
#include <partition_alloc/shim/allocator_shim_default_dispatch_to_partition_alloc.h>
#endif

#include <iostream>
namespace skiatest {

void InitializePartitionAllocForTesting() {
#if defined(SKIA_ENABLE_PARTITION_ALLOC)
    allocator_shim::ConfigurePartitionsForTesting();
    allocator_shim::internal::PartitionAllocMalloc::Allocator()->EnableThreadCacheIfSupported();
#endif
}

}  // namespace skiatest
