/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRect.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "src/base/SkRandom.h"

#include <fcntl.h>

#include "include/gpu/vk/VulkanTypes.h"
#include "include/core/SkFourByteTag.h"

#include "src/gpu/graphite/vk/VulkanYcbcrConversion.h"
#include "src/gpu/graphite/Caps.h"
#include "include/gpu/graphite/Recorder.h"
#include "src/gpu/graphite/RecorderPriv.h"

static void to_buffer(const skgpu::VulkanYcbcrConversionInfo& i, std::array<uint32_t, 14>& out) {
    out[0] = i.format();
    out[1] = i.externalFormat() >> 32;
    out[2] = i.externalFormat() & 0xFFFFFFFF;
    out[3] = i.model();
    out[4] = i.range();
    out[5] = i.xChromaOffset();
    out[6] = i.yChromaOffset();
    out[7] = i.chromaFilter();
    out[8] = i.forceExplicitReconstruction();
    out[9]  = i.components().r;
    out[10] = i.components().g;
    out[11] = i.components().b;
    out[12] = i.components().a;
    VkFormatFeatureFlags features = 0;
    if (i.samplerFilterMustMatchChromaFilter()) {
        features |= VK_FORMAT_FEATURE_SAMPLED_IMAGE_YCBCR_CONVERSION_SEPARATE_RECONSTRUCTION_FILTER_BIT;
    }
    if (i.supportsLinearFilter()) {
        features |= VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT;
    }
    out[13] = features;
}

static void create_file(const skgpu::graphite::Caps* caps) {
    skgpu::VulkanYcbcrConversionInfo tmp((uint64_t) 769,
                                         VK_SAMPLER_YCBCR_MODEL_CONVERSION_RGB_IDENTITY,
                                         VK_SAMPLER_YCBCR_RANGE_ITU_FULL,
                                         VK_CHROMA_LOCATION_COSITED_EVEN,
                                         VK_CHROMA_LOCATION_MIDPOINT,
                                         VK_FILTER_LINEAR,
                                         0,
                                         { VK_COMPONENT_SWIZZLE_IDENTITY,
                                           VK_COMPONENT_SWIZZLE_ZERO,
                                           VK_COMPONENT_SWIZZLE_ONE,
                                           VK_COMPONENT_SWIZZLE_R },
                                         VK_FORMAT_FEATURE_MIDPOINT_CHROMA_SAMPLES_BIT |
                                         VK_FORMAT_FEATURE_COSITED_CHROMA_SAMPLES_BIT |
                                         VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT |
                                         VK_FORMAT_FEATURE_SAMPLED_IMAGE_YCBCR_CONVERSION_LINEAR_FILTER_BIT);
    skgpu::VulkanYcbcrConversionInfo tmp2(VK_FORMAT_R8G8B8A8_SRGB,
                                         VK_SAMPLER_YCBCR_MODEL_CONVERSION_YCBCR_709,
                                         VK_SAMPLER_YCBCR_RANGE_ITU_NARROW,
                                         VK_CHROMA_LOCATION_MIDPOINT,
                                         VK_CHROMA_LOCATION_COSITED_EVEN,
                                         VK_FILTER_NEAREST,
                                         1,
                                         { VK_COMPONENT_SWIZZLE_G,
                                           VK_COMPONENT_SWIZZLE_B,
                                           VK_COMPONENT_SWIZZLE_A,
                                           VK_COMPONENT_SWIZZLE_IDENTITY },
                                         VK_FORMAT_FEATURE_MIDPOINT_CHROMA_SAMPLES_BIT |
                                         VK_FORMAT_FEATURE_COSITED_CHROMA_SAMPLES_BIT |
                                         VK_FORMAT_FEATURE_SAMPLED_IMAGE_YCBCR_CONVERSION_CHROMA_RECONSTRUCTION_EXPLICIT_FORCEABLE_BIT |
                                         VK_FORMAT_FEATURE_SAMPLED_IMAGE_YCBCR_CONVERSION_SEPARATE_RECONSTRUCTION_FILTER_BIT);

    constexpr SkFourByteTag kMagic1  = SkSetFourByteTag('y','c','b','c');
    constexpr SkFourByteTag kMagic2  = SkSetFourByteTag('r','f','i','l');
    constexpr int kNumHeaderUInts = 4;

    std::array<uint32_t, 14> buffer;

    int fd = creat("foo.tmp", O_WRONLY);

    buffer[0] = kMagic1;
    buffer[1] = kMagic2;
    buffer[2] = 1;
    buffer[3] = 0;
    write(fd, buffer.data(), kNumHeaderUInts * sizeof(uint32_t));

    skgpu::graphite::ImmutableSamplerInfo s = skgpu::graphite::VulkanYcbcrConversion::ToImmutableSamplerInfo(tmp);
    std::string str = caps->toString(s);
    SkDebugf("---- %s\n", str.c_str());

    to_buffer(tmp, buffer);
    write(fd, buffer.data(), sizeof(buffer));

    s = skgpu::graphite::VulkanYcbcrConversion::ToImmutableSamplerInfo(tmp2);
    str = caps->toString(s);
    SkDebugf("---- %s\n", str.c_str());

    to_buffer(tmp2, buffer);
    write(fd, buffer.data(), sizeof(buffer));

    close(fd);
}


namespace skiagm {

// This GM draws a lot of arcs in a 'Z' shape. It particularly exercises
// the 'drawArc' code near a singularly of its processing (i.e., near the
// edge of one of its underlying quads).
class ArcOfZorroGM : public GM {
public:
    ArcOfZorroGM() {
        this->setBGColor(0xFFCCCCCC);
    }

protected:
    SkString getName() const override { return SkString("arcofzorro"); }

    SkISize getISize() override { return SkISize::Make(1000, 1000); }

    void onDraw(SkCanvas* canvas) override {
        skgpu::graphite::Recorder* recorder = canvas->recorder();

        SkASSERT_RELEASE(recorder->backend() == skgpu::BackendApi::kVulkan);

        const skgpu::graphite::Caps* caps = recorder->priv().caps();

        create_file(caps);
    }

private:
    using INHERITED = GM;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM(return new ArcOfZorroGM;)
}  // namespace skiagm
