/*
 * Copyright 2026 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "tools/partition_alloc/TestSupport.h"

#if defined(SKIA_ENABLE_PARTITION_ALLOC)
#include <partition_alloc/buildflags.h>
#include <partition_alloc/dangling_raw_ptr_checks.h>
#include <partition_alloc/shim/allocator_shim.h>
#include <partition_alloc/shim/allocator_shim_default_dispatch_to_partition_alloc.h>
#include <fstream>
#include <iostream>
#include <set>
#include <string>
#endif

namespace skiatest {

#if defined(SKIA_ENABLE_PARTITION_ALLOC)
namespace {
// Some nVidia drivers are bugged, causing extreme slowdowns when using PartitionAlloc.
//
// As of 2026-01-21, there are a partial set of machine under the builder name:
// Test-Ubuntu24.04-Clang-Golo-GPU-QuadroP400-x86_64-Debug-All-Vulkan.
// failing.
// - lin-180-g580
// - lin-181-g580
// - lin-182-g580 (FAIL)
// - lin-183-g580
// - lin-184-g580
// - lin-185-g580
// - lin-186-g580
// - lin-187-g580
// - lin-188-g580 (SUCCESS)
// - lin-190-g580
// - lin-191-g580
// - lin-192-g580
// - lin-193-g580
// - lin-194-g580
// - lin-195-g580
// - lin-196-g580
// - lin-197-g580
// - lin-198-g580
// - lin-199-g580
// - lin-200-g580
// - lin-201-g580
// - lin-202-g580
// - lin-203-g580
// - lin-204-g580
// - lin-205-g580
// - lin-206-g580
// - lin-207-g580
// - lin-208-g580
// - lin-209-g580
bool SkipBadNvidiaDrivers() {
    std::set<std::string> bad_nvidia_drivers_versions = {};

    // Read the driver version from the system file.
    std::ifstream version_file("/sys/module/nvidia/version");
    if (!version_file.is_open()) {
        return false;  // Not an nVidia driver on Linux.
    }

    std::string driver_version;
    std::getline(version_file, driver_version);

    std::cout << "Detected nVidia driver version: " << driver_version << "\n";

    return bad_nvidia_drivers_versions.count(driver_version);
}
}  // namespace
#endif  // SKIA_ENABLE_PARTITION_ALLOC

void InitializePartitionAllocForTesting() {
#if defined(SKIA_ENABLE_PARTITION_ALLOC)
    if (SkipBadNvidiaDrivers()) {
        return;
    }
    allocator_shim::ConfigurePartitionsForTesting();
    allocator_shim::internal::PartitionAllocMalloc::Allocator()->EnableThreadCacheIfSupported();
#endif
}

}  // namespace skiatest
