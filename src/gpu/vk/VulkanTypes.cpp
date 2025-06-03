/*
 * Copyright 2025 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/gpu/vk/VulkanTypes.h"

#include "include/core/SkString.h"

namespace {

const char* range_str(VkSamplerYcbcrRange r) {
    switch (r) {
        case VK_SAMPLER_YCBCR_RANGE_ITU_FULL:   return "full";
        case VK_SAMPLER_YCBCR_RANGE_ITU_NARROW: return "narrow";
        default:                                return "unk";
    }
    SkUNREACHABLE;
}

char to_swizzle(VkComponentSwizzle c) {
    switch (c) {
        case VK_COMPONENT_SWIZZLE_IDENTITY: return 'I';
        case VK_COMPONENT_SWIZZLE_ZERO:     return '0';
        case VK_COMPONENT_SWIZZLE_ONE:      return '1';
        case VK_COMPONENT_SWIZZLE_R:        return 'r';
        case VK_COMPONENT_SWIZZLE_G:        return 'g';
        case VK_COMPONENT_SWIZZLE_B:        return 'b';
        case VK_COMPONENT_SWIZZLE_A:        return 'a';
        default:                            return '?';
    }
    SkUNREACHABLE;
}

} // anonymous namespace

SkString VulkanYcbcrConversionInfo::toString() const{
    return SkStringPrintf("format %d ext %llu model %d %s x %s y %s %s"
                          "recon %c feat %u swiz %c%c%c%c",
                          fFormat,
                          (unsigned long long) fExternalFormat,
                          fYcbcrModel,
                          range_str(fYcbcrRange),
                          fXChromaOffset ? "mid"  : "cos-even", // midpoint or cosited-even
                          fYChromaOffset ? "mid"  : "cos-even", // midpoint or cosited-even
                          fChromaFilter  ? "linear" : "nearest",
                          fForceExplicitReconstruction ? 'T' : 'F',
                          fFormatFeatures,
                          to_swizzle(fComponents.r),
                          to_swizzle(fComponents.g),
                          to_swizzle(fComponents.b),
                          to_swizzle(fComponents.a));
}
