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

    TextureInfo getDefaultAttachmentTextureInfo(AttachmentDesc,
                                                Protected,
                                                Discardable) const override;

    TextureInfo getDefaultSampledTextureInfo(SkColorType,
                                             Mipmapped mipmapped,
                                             Protected,
                                             Renderable) const override;

    TextureInfo getTextureInfoForSampledCopy(const TextureInfo& textureInfo,
                                             Mipmapped mipmapped) const override;

    TextureInfo getDefaultCompressedTextureInfo(SkTextureCompressionType,
                                                Mipmapped mipmapped,
                                                Protected) const override;

    TextureInfo getDefaultStorageTextureInfo(SkColorType) const override;

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
    void initGPUFamily(const id<MTLDevice>);

    void initCaps(const id<MTLDevice>);
    void initShaderCaps();
    void initFormatTable(const id<MTLDevice>);

    enum class GPUFamily {
        kApple,
        kMac,
        kMacIntel,
    };
    static bool GetGPUFamily(id<MTLDevice> device, GPUFamily* gpuFamily, int* group);

    SkEnumBitMask<TextureFormatInfo::Supports> supportedOperations(const TextureInfo&) const override;

    GPUFamily fGPUFamily;
    int fFamilyGroup;
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_MtlCaps_DEFINED
