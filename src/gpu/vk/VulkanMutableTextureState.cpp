/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/gpu/vk/VulkanMutableTextureState.h"

namespace skgpu {
    VkImageLayout GetVkImageLayout(const MutableTextureState& state) {
        SkASSERT(state.fBackend == BackendApi::kVulkan);
        return state.fVkState.getImageLayout();
    }

    void setImageLayout(VkImageLayout layout) {
        SkASSERT(state.fBackend == BackendApi::kVulkan);
        state->fVkState.setImageLayout(layout);
    }

    uint32_t getQueueFamilyIndex() const {
        SkASSERT(state.fBackend == BackendApi::kVulkan);
        return state.fVkState.getQueueFamilyIndex();
    }

    void setQueueFamilyIndex(uint32_t queueFamilyIndex) {
        SkASSERT(state.fBackend == BackendApi::kVulkan);
        state.fVkState.setQueueFamilyIndex(queueFamilyIndex);
    }
}
