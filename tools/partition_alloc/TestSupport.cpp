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
#include <sys/utsname.h>
#endif

namespace skiatest {

namespace {

/**
 * Utility to read the first line or word from a sysfs file.
 */
std::string ReadSysfsFile(const std::filesystem::path& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        return "error";
    }
    std::string content;
    if (std::getline(file, content)) {
        // Remove trailing whitespace/newlines
        content.erase(content.find_last_not_of(" \n\r\t") + 1);
        return content;
    }
    return "unknown";
}

/**
 * Attempts to find the driver version for a specific driver name.
 */
std::string GetVersionForDriver(const std::string& driver_name) {
    // 1. Primary NVIDIA check (via /proc)
    if (driver_name == "nvidia" || driver_name == "none") {
        std::ifstream nv_file("/proc/driver/nvidia/version");
        std::string line;
        if (std::getline(nv_file, line)) {
            return line;
        }
    }

    // 2. Nouveau specific check
    if (driver_name == "nouveau" || driver_name == "none") {
        std::string module_ver_path = "/sys/module/nouveau/version";
        if (std::filesystem::exists(module_ver_path)) {
            return "Nouveau " + ReadSysfsFile(module_ver_path);
        }
        // If the module is loaded but no version file exists, report kernel version
        std::ifstream modules("/proc/modules");
        std::string mod_line;
        bool nouveau_loaded = false;
        while (std::getline(modules, mod_line)) {
            if (mod_line.compare(0, 7, "nouveau") == 0) {
                nouveau_loaded = true;
                break;
            }
        }

        if (nouveau_loaded) {
#ifdef __linux__
            struct utsname buffer;
            if (uname(&buffer) == 0) {
                return "Nouveau (Kernel: " + std::string(buffer.release) + ")";
            }
#endif
            return "Nouveau (version unknown)";
        }
    }

    // 3. Check /sys/module for the specific driver or the nvidia/nouveau modules specifically
    for (const std::string& mod : {driver_name, std::string("nvidia"), std::string("nouveau")}) {
        if (mod == "none" || mod.empty()) continue;
        std::string module_ver_path = "/sys/module/" + mod + "/version";
        if (std::filesystem::exists(module_ver_path)) {
            return mod + ": " + ReadSysfsFile(module_ver_path);
        }
    }

    return "version unavailable (driver may not be loaded)";
}

}  // namespace

/**
 * Scans all PCI devices and prints their details.
 */
void PrintPciDevices() {
    namespace fs = std::filesystem;
    const std::string pci_path = "/sys/bus/pci/devices/";
    std::error_code ec;

    if (!fs::exists(pci_path, ec) || ec) {
        std::printf("Error: PCI bus path %s not found.\n", pci_path.c_str());
        return;
    }

    std::cout << "Listing all detected PCI devices for debugging...\n";
    std::printf("%-15s %-10s %-10s %-12s %-10s %-12s\n",
                "BDF Address", "Vendor", "Device", "Class", "Driver", "Note");
    std::printf("--------------------------------------------------------------------------------------\n");

    size_t devices_found = 0;
    size_t gpus_found = 0;

    for (const auto& entry : fs::directory_iterator(pci_path, ec)) {
        if (ec) break;
        devices_found++;

        std::string bdf = entry.path().filename().string();
        std::string vendor = ReadSysfsFile(entry.path() / "vendor");
        std::string device = ReadSysfsFile(entry.path() / "device");
        std::string class_id = ReadSysfsFile(entry.path() / "class");

        // Resolve Driver
        std::string driver_name = "none";
        fs::path driver_link = entry.path() / "driver";
        if (fs::exists(driver_link)) {
            driver_name = fs::canonical(driver_link).filename().string();
        }

        bool is_gpu = (class_id.compare(0, 4, "0x03") == 0);
        if (is_gpu) {
            gpus_found++;
            std::string version = GetVersionForDriver(driver_name);
            std::printf("%-15s %-10s %-10s %-12s %-10s [GPU] %s\n",
                        bdf.c_str(), vendor.c_str(), device.c_str(), class_id.c_str(),
                        driver_name.c_str(), version.c_str());
        } else {
            std::printf("%-15s %-10s %-10s %-12s %-10s\n",
                        bdf.c_str(), vendor.c_str(), device.c_str(), class_id.c_str(),
                        driver_name.c_str());
        }
    }

    if (gpus_found == 0) {
        std::printf("\nNo standard GPU (0x03xxxx) found among %zu devices.\n", devices_found);
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

    // Check for both proprietary and open-source (nouveau) drivers as both can
    // affect system stability or performance in different ways.
    std::string nvidia_ver = GetVersionForDriver("nvidia");
    std::string nouveau_ver = GetVersionForDriver("nouveau");

    if (nvidia_ver.find("version unavailable") == std::string::npos) {
        std::cout << "Detected nVidia proprietary driver: " << nvidia_ver << "\n";
        if (bad_nvidia_drivers_versions.count(nvidia_ver)) return true;
    }

    if (nouveau_ver.find("version unavailable") == std::string::npos) {
        std::cout << "Detected Nouveau driver: " << nouveau_ver << "\n";
        // Currently, we don't have specific bad nouveau versions, but we detect it.
    }

    return false;
}
}  // namespace
#endif

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
