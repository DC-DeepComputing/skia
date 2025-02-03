/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_MtlGraphiteTypes_DEFINED
#define skgpu_graphite_MtlGraphiteTypes_DEFINED

#if __OBJC__  // <Metal/Metal.h> only works when compiled for Objective C

#include "include/core/SkTypes.h"

#include "include/gpu/graphite/GraphiteTypes.h"
#include "include/gpu/graphite/TextureInfo.h"
#include "include/private/base/SkAPI.h"

#import <CoreFoundation/CoreFoundation.h>
#import <Metal/Metal.h>
#import <TargetConditionals.h>

namespace skgpu::graphite {

class SK_API MtlTextureInfo final : public TextureInfo::Data {
public:
    static constexpr skgpu::BackendApi kBackendApi = skgpu::BackendApi::kMetal;

    MTLPixelFormat  fFormat = MTLPixelFormatInvalid;
    MTLTextureUsage fUsage = MTLTextureUsageUnknown;
    MTLStorageMode  fStorageMode = MTLStorageModeShared;
    bool fFramebufferOnly = false;

    MtlTextureInfo() = default;
    MtlTextureInfo(const MtlTextureInfo&) = default;

    MtlTextureInfo& operator=(const MtlTextureInfo& info) {
        this->~MtlTextureInfo();
        return *(new (this) MtlTextureInfo(info));
    }

    MtlTextureInfo(CFTypeRef mtlTexture);
    MtlTextureInfo(uint32_t sampleCount,
                   skgpu::Mipmapped mipmapped,
                   MTLPixelFormat format,
                   MTLTextureUsage usage,
                   MTLStorageMode storageMode,
                   bool framebufferOnly)
            : Data(sampleCount, mipmapped)
            , fFormat(format)
            , fUsage(usage)
            , fStorageMode(storageMode)
            , fFramebufferOnly(framebufferOnly) {}

private:
    skgpu::BackendApi type() const override { return kBackendApi; }
    Protected isProtected() const override { return Protected::kNo; }
    uint32_t viewFormat() const override { return (uint32_t) fFormat; }

    size_t bytesPerPixel() const override;
    SkTextureCompressionType compressionType() const override;
    bool isMemoryless() const override;
    SkString toBackendString() const override;

    void copyTo(AnyTextureInfoData& dstData) const override {
        dstData.emplace<MtlTextureInfo>(*this);
    }
    bool equals(const TextureInfo&that) const override;
    bool isCompatible(const TextureInfo& that) const override;
};

}  // namespace skgpu::graphite

#endif  // __OBJC__

#endif // skgpu_graphite_MtlGraphiteTypes_DEFINED
