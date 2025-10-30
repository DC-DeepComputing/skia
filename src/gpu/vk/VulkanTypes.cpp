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

std::string model_str(VkSamplerYcbcrModelConversion c) {
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

std::string range_str(VkSamplerYcbcrRange r) {
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

std::string VulkanYcbcrConversionInfo::toString() const{
    std::string result;

    if (fFormat == VK_FORMAT_UNDEFINED) {
        result += 'x';
        result += fExternalFormat;
    } else {
        result += fFormat;
    }

    result += " " + model_str(fYcbcrModel) + "+" + range_str(fYcbcrRange);
    result += fXChromaOffset ? " mid"  : " cos"; // midpoint or cosited-even
    result += fYChromaOffset ? " mid"  : " cos"; // midpoint or cosited-even
    result += fChromaFilter  ? " linear" : " nearest";
    result += fForceExplicitReconstruction ? " T " : " F ";
    result += to_swizzle(fComponents.r, 'r');
    result += to_swizzle(fComponents.g, 'g');
    result += to_swizzle(fComponents.b, 'b');
    result += to_swizzle(fComponents.a, 'a');
    return result;
}
