/*
 * Copyright 2025 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/gpu/vk/VulkanTypes.h"

#include "include/core/SkString.h"

using namespace skgpu;

namespace {

const char* model_str(VkSamplerYcbcrModelConversion c) {
    switch (c) {
        case VK_SAMPLER_YCBCR_MODEL_CONVERSION_RGB_IDENTITY:   return "RGB-I";
        case VK_SAMPLER_YCBCR_MODEL_CONVERSION_YCBCR_IDENTITY: return "YCbCr-I";
        case VK_SAMPLER_YCBCR_MODEL_CONVERSION_YCBCR_709:      return "709";
        case VK_SAMPLER_YCBCR_MODEL_CONVERSION_YCBCR_601:      return "601";
        case VK_SAMPLER_YCBCR_MODEL_CONVERSION_YCBCR_2020:     return "2020";
        default:                                               return "unknown";
    }
    SkUNREACHABLE;
}

const char* range_str(VkSamplerYcbcrRange r) {
    switch (r) {
        case VK_SAMPLER_YCBCR_RANGE_ITU_FULL:   return "full";
        case VK_SAMPLER_YCBCR_RANGE_ITU_NARROW: return "narrow";
        default:                                return "Unknown";
    }
    SkUNREACHABLE;
}

char to_swizzle(VkComponentSwizzle c, char identityAnswer) {
    switch (c) {
        case VK_COMPONENT_SWIZZLE_IDENTITY: return identityAnswer;
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
    SkString result;

    if (fFormat == VK_FORMAT_UNDEFINED) {
        result.appendf("x%llu", (unsigned long long) fExternalFormat);
    } else {
        result.appendf("%d", fFormat);
    }

    result.appendf(" %s+%s %s %s %s %c %c%c%c%c",
                   model_str(fYcbcrModel),
                   range_str(fYcbcrRange),
                   fXChromaOffset ? "mid"  : "cos", // midpoint or cosited-even
                   fYChromaOffset ? "mid"  : "cos", // midpoint or cosited-even
                   fChromaFilter  ? "linear" : "nearest",
                   fForceExplicitReconstruction ? 'T' : 'F',
                   to_swizzle(fComponents.r, 'r'),
                   to_swizzle(fComponents.g, 'g'),
                   to_swizzle(fComponents.b, 'b'),
                   to_swizzle(fComponents.a, 'a'));
    return result;
}
