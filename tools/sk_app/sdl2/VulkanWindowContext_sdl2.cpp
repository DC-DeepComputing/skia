
/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/gpu/vk/GrVkVulkan.h"
#include "src/gpu/ganesh/vk/GrVkInterface.h"
#include "src/gpu/ganesh/vk/GrVkUtil.h"
#include "tools/gpu/vk/VkTestUtils.h"
#include "tools/sk_app/VulkanWindowContext.h"
#include "tools/sk_app/sdl2/WindowContextFactory_sdl2.h"

#include <SDL2/SDL_vulkan.h>

namespace sk_app {

namespace window_context_factory {

std::unique_ptr<WindowContext> MakeVulkanForSDL2(const SDL2WindowInfo& info,
                                                 const DisplayParams& displayParams) {
    if (SDL_Vulkan_LoadLibrary(nullptr) != 0) {
        return nullptr;
    }
    auto instProc =
            reinterpret_cast<PFN_vkGetInstanceProcAddr>(SDL_Vulkan_GetVkGetInstanceProcAddr());
    if (!instProc) {
        return nullptr;
    }
    // Out utilities require a vkGetInstanceProcAddr and vkGetDeviceProcAddr while SDL only provides
    // a getter for the former. We should be able to rework our utilities to work with just
    // vkGetInstanceProcAddr because it can be used, once an instance is created, to get
    // vkGetDeviceProcAddr.
    return nullptr;
}

}  // namespace window_context_factory

}  // namespace sk_app
