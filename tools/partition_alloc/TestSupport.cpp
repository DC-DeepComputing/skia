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
#endif
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <set>
#include <string>
#include <system_error>

namespace skiatest {


/**
 * Scans PCI devices using C++17 filesystem.
 * Uses std::error_code to avoid exceptions.
 */
void PrintPciDevices() {
    namespace fs = std::filesystem;

    // Print /proc/driver/nvidia/version is available.
    {
        std::ifstream nvidia_version_file("/proc/driver/nvidia/version");
        if (nvidia_version_file.is_open()) {
            std::string line;
            std::cout << "NVIDIA Driver Version Info:\n";
            while (std::getline(nvidia_version_file, line)) {
                std::cout << line << "\n";
            }
            std::cout << std::endl;
        }
    }

    // To debug, print lspci -k
    {
        std::cout << "lspci -k output:\n";
        FILE* pipe = popen("lspci -k", "r");
        if (pipe) {
            char buffer[256];
            while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
                std::cout << buffer;
            }
            pclose(pipe);
        }
        std::cout << std::endl;
    }

    const std::string pci_path = "/sys/bus/pci/devices/";
    std::error_code ec;

    // Check if directory exists without throwing
    if (!fs::exists(pci_path, ec) || ec) {
        std::printf("Error: PCI bus not found or access denied.\n");
        return;
    }

    // Header
    std::printf("%-15s %-10s %-10s %-10s\n", "BDF Address", "Vendor", "Device", "Revision");
    std::printf("----------------------------------------------------------\n");

    // Use directory_iterator with error_code to disable exceptions
    for (const auto& entry : fs::directory_iterator(pci_path, ec)) {
        if (ec) break;

        // The folder name is the BDF (e.g., 0000:00:02.0)
        std::string bdf = entry.path().filename().string();

        // Helper to read sysfs files using C++ streams
        auto read_id = [&](const std::string& filename) -> std::string {
            std::ifstream file(entry.path() / filename);
            std::string id;
            if (!(file >> id)) return "unknown";
            return id;
        };

        std::string vendor = read_id("vendor");
        std::string device = read_id("device");
        std::string rev = read_id("revision");

        std::printf("%-15s %-10s %-10s %-10s\n",
                    bdf.c_str(),
                    vendor.c_str(),
                    device.c_str(),
                    rev.c_str());
    }

    if (ec) {
        std::printf("Error during iteration: %s\n", ec.message().c_str());
    }
}

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
    PrintPciDevices();
#if defined(SKIA_ENABLE_PARTITION_ALLOC)
    if (SkipBadNvidiaDrivers()) {
        return;
    }
    allocator_shim::ConfigurePartitionsForTesting();
    allocator_shim::internal::PartitionAllocMalloc::Allocator()->EnableThreadCacheIfSupported();
#endif
}

}  // namespace skiatest
