/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_MtlCaps_DEFINED
#define skgpu_graphite_MtlCaps_DEFINED

#include <vector>

#import <Metal/Metal.h>

#include "src/gpu/graphite/Caps.h"

namespace skgpu::graphite {
struct ContextOptions;

class MtlCaps final : public Caps {
public:
    MtlCaps(const id<MTLDevice>, const ContextOptions&);
    ~MtlCaps() override {}


    UniqueKey makeGraphicsPipelineKey(const GraphicsPipelineDesc&,
                                      const RenderPassDesc&) const override;
    UniqueKey makeComputePipelineKey(const ComputePipelineDesc&) const override;

    bool extractGraphicsDescs(const UniqueKey&,
                              GraphicsPipelineDesc*,
                              RenderPassDesc*,
                              const RendererProvider*) const override;

    // Get a sufficiently unique bit representation for the RenderPassDesc to be embedded in other
    // UniqueKeys (e.g. makeGraphicsPipelineKey).
    uint32_t getRenderPassDescKey(const RenderPassDesc&) const;

    bool isMac() const   { return fGPUFamily == GPUFamily::kMac ||
                                  fGPUFamily == GPUFamily::kMacIntel; }
    bool isApple() const { return fGPUFamily == GPUFamily::kApple;    }
    bool isIntel() const { return fGPUFamily == GPUFamily::kMacIntel; }

    void buildKeyForTexture(SkISize dimensions,
                            const TextureInfo&,
                            ResourceType,
                            GraphiteResourceKey*) const override;

private:
    TextureFormatInfo getFormatInfo(TextureFormat) const override;
    SkEnumBitMask<FormatSupport> supportedOperations(const TextureInfo&) const override;

    void initGPUFamily(const id<MTLDevice>);

    void initCaps(const id<MTLDevice>);
    void initShaderCaps();

    enum class GPUFamily {
        kApple,
        kMac,
        kMacIntel,
    };
    static std::optional<MTLGPUFamily> GetGPUFamily(id<MTLDevice>);

    MTLGPUFamily fGUFamily;
    SkEnumBitMask<SampleCount> fSupportedSampleCounts;
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_MtlCaps_DEFINED
