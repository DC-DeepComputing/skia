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
#include "include/gpu/graphite/mtl/MtlGraphiteTypes_cpp.h"
#include "include/private/base/SkAPI.h"

#import <CoreFoundation/CoreFoundation.h>
#import <Metal/Metal.h>
#import <TargetConditionals.h>

namespace skgpu::graphite {

class SK_API MtlTextureInfo final : public TextureInfo::Data {
public:
    static constexpr skgpu::BackendApi kBackend = skgpu::BackendApi::kMetal;

    MTLPixelFormat fFormat = MTLPixelFormatInvalid;
    MTLTextureUsage fUsage = MTLTextureUsageUnknown;
    MTLStorageMode fStorageMode = MTLStorageModeShared;
    bool fFramebufferOnly = false;

    MtlTextureInfo() = default;
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

    Protected isProtected() const { return Protected::kNo; }
    bool isMemoryless() const {
        // @available(macOS 11.0, iOS 10.0, tvOS 10.0) is assumed
        return fStorageMode == MTLStorageModeMemoryless;
    }

private:
    uint32_t viewFormat() const override { return (uint32_t) fFormat; }

    size_t bytesPerPixel() const override;
    SkTextureCompressionType compressionType() const override;
    SkString toBackendString() const override;

    bool serialize(SkWStream*) const override;
    bool deserialize(SkStream*) override;

    void copyTo(TextureInfo::AnyTextureInfoData& dstData) const override {
        dstData.emplace<MtlTextureInfo>(*this);
    }
    bool equals(const TextureInfo&that) const override;
    bool isCompatible(const TextureInfo& that) const override;
};

}  // namespace skgpu::graphite

#endif  // __OBJC__

#endif // skgpu_graphite_MtlGraphiteTypes_DEFINED
