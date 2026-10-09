/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_D3D11Caps_DEFINED
#define skgpu_graphite_D3D11Caps_DEFINED

#include "include/gpu/graphite/d3d11/D3D11GraphiteTypes.h"
#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/TextureFormat.h"

#include <d3d11_1.h>
#include <dxgi1_3.h>

namespace skgpu::graphite {

struct ContextOptions;
struct D3D11BackendContext;

TextureFormat D3D11FormatToTextureFormat(DXGI_FORMAT format);
DXGI_FORMAT TextureFormatToD3D11Format(TextureFormat format);

class D3D11Caps final : public Caps {
public:
    D3D11Caps(const D3D11BackendContext&, const ContextOptions&);
    ~D3D11Caps() override;

    UniqueKey makeGraphicsPipelineKey(const GraphicsPipelineDesc&,
                                      const RenderPassDesc&) const override;
    bool extractGraphicsDescs(const UniqueKey&,
                              GraphicsPipelineDesc*,
                              RenderPassDesc*,
                              const RendererProvider*) const override;
    UniqueKey makeComputePipelineKey(const ComputePipelineDesc&) const override;
    ImmutableSamplerInfo getImmutableSamplerInfo(const TextureInfo&) const override { return {}; }
    std::string toString(const ImmutableSamplerInfo&) const override { return ""; }

    void buildKeyForTexture(SkISize dimensions,
                            const TextureInfo&,
                            ResourceType,
                            GraphiteResourceKey*) const override;

    uint32_t getRenderPassDescKeyForPipeline(const RenderPassDesc&) const;

private:
    TextureInfo onGetDefaultTextureInfo(SkEnumBitMask<TextureUsage> usage,
                                        TextureFormat,
                                        SampleCount,
                                        Mipmapped,
                                        Protected,
                                        Discardable) const override;
    std::pair<SkEnumBitMask<TextureUsage>, Tiling> getTextureUsage(
            const TextureInfo&) const override;

    void initCaps(const D3D11BackendContext&, const ContextOptions&);
    void initShaderCaps();
    void initFormatTable(ID3D11Device*);
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_D3D11Caps_DEFINED
