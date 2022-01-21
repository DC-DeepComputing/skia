/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/graphite/src/mtl/MtlSampler.h"

#include "experimental/graphite/src/mtl/MtlCaps.h"
#include "experimental/graphite/src/mtl/MtlGpu.h"
#include "include/core/SkSamplingOptions.h"

namespace skgpu::mtl {

static inline MTLSamplerAddressMode tile_mode_to_mtl_sampler_address(SkTileMode tileMode,
                                                                     const Caps& caps) {
    switch (tileMode) {
        case SkTileMode::kClamp:
            return MTLSamplerAddressModeClampToEdge;
        case SkTileMode::kRepeat:
            return MTLSamplerAddressModeRepeat;
        case SkTileMode::kMirror:
            return MTLSamplerAddressModeMirrorRepeat;
        case SkTileMode::kDecal:
            if (@available(macOS 10.12, iOS 14.0, *)) {
                SkASSERT(caps.clampToBorderSupport());
                return MTLSamplerAddressModeClampToBorderColor;
            } else {
                SkASSERT(false);
                return MTLSamplerAddressModeClampToZero;
            }
    }
    SK_ABORT("Unknown tile mode.");
}

sk_cfp<id<MTLSamplerState>> Sampler::MakeMtlSamplerState(const Gpu* gpu,
                                                         const SkSamplingOptions& samplingOptions,
                                                         SkTileMode xTileMode,
                                                         SkTileMode yTileMode) {
    sk_cfp<MTLSamplerDescriptor*> desc([[MTLSamplerDescriptor alloc] init]);

    MTLSamplerMinMagFilter minMagFilter = [&] {
        switch (samplingOptions.filter) {
            case SkFilterMode::kNearest: return MTLSamplerMinMagFilterNearest;
            case SkFilterMode::kLinear:  return MTLSamplerMinMagFilterLinear;
        }
        SkUNREACHABLE;
    }();

    MTLSamplerMipFilter mipFilter = [&] {
      switch (samplingOptions.mipmap) {
          case SkMipmapMode::kNone:    return MTLSamplerMipFilterNotMipmapped;
          case SkMipmapMode::kNearest: return MTLSamplerMipFilterNearest;
          case SkMipmapMode::kLinear:  return MTLSamplerMipFilterLinear;
      }
      SkUNREACHABLE;
    }();

    auto samplerDesc = [[MTLSamplerDescriptor alloc] init];
    samplerDesc.rAddressMode = MTLSamplerAddressModeClampToEdge;
    samplerDesc.sAddressMode = tile_mode_to_mtl_sampler_address(xTileMode, gpu->mtlCaps());
    samplerDesc.tAddressMode = tile_mode_to_mtl_sampler_address(yTileMode, gpu->mtlCaps());
    samplerDesc.magFilter = minMagFilter;
    samplerDesc.minFilter = minMagFilter;
    samplerDesc.mipFilter = mipFilter;
    samplerDesc.lodMinClamp = 0.0f;
    samplerDesc.lodMaxClamp = FLT_MAX;  // default value according to docs.
    samplerDesc.maxAnisotropy = 1.0f;
    samplerDesc.normalizedCoordinates = true;
    if (@available(macOS 10.11, iOS 9.0, *)) {
        samplerDesc.compareFunction = MTLCompareFunctionNever;
    }
#ifdef SK_ENABLE_MTL_DEBUG_INFO
    // TODO: add label
#endif

    sk_cfp<id<MTLSamplerState>> sampler([gpu->device() newSamplerStateWithDescriptor:desc.get()]);
    return sampler;
}

sk_sp<skgpu::Sampler> Sampler::Make(const Gpu* gpu,
                             const SkSamplingOptions& samplingOptions,
                             SkTileMode xTileMode,
                             SkTileMode yTileMode) {
    sk_cfp<id<MTLSamplerState>> samplerState = Sampler::MakeMtlSamplerState(gpu, samplingOptions,
                                                                   xTileMode, yTileMode);
    if (!samplerState) {
        return nullptr;
    }
    return sk_sp<Sampler>(new Sampler(gpu, std::move(samplerState)));
}

Sampler::Sampler(const Gpu* gpu,
                 sk_cfp<id<MTLSamplerState>> samplerState)
        : skgpu::Sampler(gpu)
        , fSamplerState(std::move(samplerState)) {}

void Sampler::onFreeGpuData() {
    fSamplerState.reset();
}

} // namespace skgpu::mtl

