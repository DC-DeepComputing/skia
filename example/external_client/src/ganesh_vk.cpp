/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/gpu/GrDirectContext.h"
#include "include/gpu/ganesh/vk/GrVkDirectContext.h"
#include "include/gpu/vk/GrVkBackendContext.h"

#include <dlfcn.h>
#include <cstdio>

#include <vulkan/vulkan_core.h>

// This is a very Linux-specific version of how one would set up a Vulkan context.
static bool load_vk_library(PFN_vkGetInstanceProcAddr* instProc) {
    void* vkLib = dlopen("libvulkan.so");
    if (!vkLib) {
        vkLib = dlopen("libvulkan.so.1");
    }
    if (!vkLib) {
        return false;
    }
    auto proc = (PFN_vkGetInstanceProcAddr)dlsym(vkLib, "vkGetInstanceProcAddr");
    if (!proc) {
        return false;
    }
    *instProc = proc;
    return true;
}

static bool init_vulkan_context(GrVkBackendContext* ctx) {
    PFN_vkGetInstanceProcAddr instProc;
    if (!load_vk_library(&instProc)) {
        return false;
    }
    std::unique_ptr<VkPhysicalDeviceFeatures2> features(new VkPhysicalDeviceFeatures2);
    memset(features.get(), 0, sizeof(VkPhysicalDeviceFeatures2));
    features->sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
    features->pNext = nullptr;

    ctx->fInstance = VK_NULL_HANDLE;
    ctx->fDevice = VK_NULL_HANDLE;

    TODO following along with vulkan basic
}

int main(int argc, char** argv) {
    if (argc != 2) {
        printf("Usage: %s <name.png>\n", argv[0]);
        return 1;
    }

    GrContextOptions opts;
    opts.fSuppressPrints = true;
    GrVkBackendContext vk_ctx;
    if (!init_vulkan_context(&vk_ctx)) {
        printf("Could not make Vulkan context\n");
        return 1;
    }
    sk_sp<GrDirectContext> ctx = GrDirectContexts::MakeVk(vk_ctx, opts);

    if (!ctx) {
        printf("Could not make Vulkan interface\n");
        return 1;
    }
    printf("Context made, now to make the surface\n");

    return 0;
}
