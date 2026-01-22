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
#endif
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <set>
#include <string>

#ifdef __linux__
#include <cstdio>
#endif

namespace skiatest {

namespace {

/**
 * Utility to read the first line or word from a sysfs file.
 */
std::string ReadSysfsFile(const std::filesystem::path& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        return "error_opening";
    }
    std::string content;
    if (std::getline(file, content)) {
        return content;
    }
    return "unknown";
}

/**
 * Attempts to find the driver version for a specific driver name by querying
 * /proc or /sys/module.
 */
std::string GetVersionForDriver(const std::string& driver_name) {
    // 1. NVIDIA Proprietary Path (NVIDIA doesn't use standard module version files)
    if (driver_name == "nvidia") {
        std::ifstream nv_file("/proc/driver/nvidia/version");
        std::string line;
        if (std::getline(nv_file, line)) {
            return line;
        }
    }

    // 2. Standard Module Version Path (Common for amdgpu, i915, nouveau)
    std::string module_ver_path = "/sys/module/" + driver_name + "/version";
    if (std::filesystem::exists(module_ver_path)) {
        return ReadSysfsFile(module_ver_path);
    }

    return "version unavailable";
}

}  // namespace

/**
 * Scans PCI devices using C++17 filesystem.
 * Filters for Display Controllers (GPUs) and identifies their active drivers.
 */
void PrintPciDevices() {
    namespace fs = std::filesystem;
    const std::string pci_path = "/sys/bus/pci/devices/";
    std::error_code ec;

    if (!fs::exists(pci_path, ec) || ec) {
        std::printf("Error: PCI bus path %s not found or access denied.\n", pci_path.c_str());
        return;
    }

    std::cout << "Scanning for GPU devices and active drivers...\n";
    std::printf("%-15s %-10s %-10s %-12s %-15s\n", "BDF Address", "Vendor", "Device", "Driver", "Version Info");
    std::printf("--------------------------------------------------------------------------------------\n");

    size_t devices_found = 0;
    size_t gpus_found = 0;

    for (const auto& entry : fs::directory_iterator(pci_path, ec)) {
        if (ec) break;
        devices_found++;

        // PCI Class 0x03xxxx are Display/VGA controllers (GPUs)
        std::string class_id = ReadSysfsFile(entry.path() / "class");

        // Debugging: Print all devices if none are being flagged as GPUs
        bool is_gpu = (class_id.compare(0, 4, "0x03") == 0);
        if (!is_gpu) {
            continue;
        }

        gpus_found++;
        std::string bdf = entry.path().filename().string();
        std::string vendor = ReadSysfsFile(entry.path() / "vendor");
        std::string device = ReadSysfsFile(entry.path() / "device");

        // Find the driver bound to this specific PCI device
        std::string driver_name = "none";
        fs::path driver_link = entry.path() / "driver";
        if (fs::exists(driver_link)) {
            // Resolves the symlink to get the actual driver name (e.g., "nvidia" or "amdgpu")
            driver_name = fs::canonical(driver_link).filename().string();
        }

        std::string version_info = (driver_name != "none") ? GetVersionForDriver(driver_name) : "N/A";

        std::printf("%-15s %-10s %-10s %-12s %-15s\n",
                    bdf.c_str(),
                    vendor.c_str(),
                    device.c_str(),
                    driver_name.c_str(),
                    version_info.c_str());
    }

    if (ec) {
        std::printf("Error during iteration: %s\n", ec.message().c_str());
    }

    if (gpus_found == 0) {
        std::printf("\nNo GPU devices (class 0x03xxxx) detected among %zu total PCI devices.\n", devices_found);
        std::printf("Check if /sys is mounted and accessible.\n");
    }

    std::cout << std::endl;
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
// - lin-182-g580 TIMEOUT.
// - lin-183-g580
// - lin-184-g580
// - lin-185-g580
// - lin-186-g580
// - lin-187-g580
// - lin-188-g580 PASS: 37min 55s
// - lin-190-g580
// - lin-191-g580
// - lin-192-g580 PASS: 3hrs 55min
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

    // Check /proc/driver/nvidia/version first as it is the most detailed
    std::ifstream version_file("/proc/driver/nvidia/version");
    if (!version_file.is_open()) {
        // Fallback to module version if /proc is restricted
        version_file.open("/sys/module/nvidia/version");
    }

    if (!version_file.is_open()) {
        return false;  // No NVIDIA driver detected.
    }

    std::string driver_version;
    std::getline(version_file, driver_version);

    // If version_file was /proc, it contains a long string. If /sys, just the version number.
    std::cout << "Detected nVidia driver: " << driver_version << "\n";

    return bad_nvidia_drivers_versions.count(driver_version);
}
}  // namespace
#endif  // SKIA_ENABLE_PARTITION_ALLOC

void InitializePartitionAllocForTesting() {
    PrintPciDevices();
#if defined(SKIA_ENABLE_PARTITION_ALLOC)
    if (SkipBadNvidiaDrivers()) {
        std::cout << "Skipping PartitionAlloc due to problematic NVIDIA driver.\n";
        return;
    }
    allocator_shim::ConfigurePartitionsForTesting();
    allocator_shim::internal::PartitionAllocMalloc::Allocator()->EnableThreadCacheIfSupported();
#endif
}

}  // namespace skiatest
