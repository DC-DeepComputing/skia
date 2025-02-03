/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "include/core/SkString.h"
#include "include/gpu/graphite/mtl/MtlGraphiteTypes.h"
#include "include/gpu/graphite/mtl/MtlGraphiteTypesUtils.h"
#include "src/gpu/graphite/TextureInfoPriv.h"
#include "src/gpu/graphite/mtl/MtlGraphiteTypesPriv.h"
#include "src/gpu/mtl/MtlUtilsPriv.h"

#include <cstdint>

#import <Metal/Metal.h>

namespace skgpu::graphite {

MtlTextureInfo::MtlTextureInfo(CFTypeRef texture) {
    SkASSERT(texture);
    id<MTLTexture> mtlTex = (id<MTLTexture>)texture;

    fSampleCount = mtlTex.sampleCount;
    fMipmapped = mtlTex.mipmapLevelCount > 1 ? Mipmapped::kYes : Mipmapped::kNo;

    fFormat = mtlTex.pixelFormat;
    fUsage = mtlTex.usage;
    fStorageMode = mtlTex.storageMode;
    fFramebufferOnly = mtlTex.framebufferOnly;
}

size_t MtlTextureInfo::bytesPerPixel() const {
    return MtlFormatBytesPerBlock(fFormat);
}

SkTextureCompressionType MtlTextureInfo::compressionType() const {
    return MtlFormatToCompressionType(fFormat);
}

bool MtlTextureInfo::isMemoryless() const {
    if (@available(macOS 11.0, iOS 10.0, tvOS 10.0, *)) {
        return fStorageMode == MTLStorageModeMemoryless;
    }
    return false;
}

SkString MtlTextureInfo::toBackendString() const {
    return SkStringPrintf("format=%u,usage=0x%04X,storageMode=%u,fbOnly=%d)",
                          (uint32_t)fFormat,
                          (uint32_t)fUsage,
                          (uint32_t)fStorageMode,
                          fFramebufferOnly);
}

bool MtlTextureInfo::equals(const TextureInfo& that) const {
    const auto& mt = TextureInfoPriv::Get<MtlTextureInfo>(that);
    return fFormat == mt.fFormat && fUsage == mt.fUsage &&
            fStorageMode == mt.fStorageMode && fFramebufferOnly == mt.fFramebufferOnly;
}

bool MtlTextureInfo::isCompatible(const TextureInfo& that) const {
    const auto& mt = TextureInfoPriv::Get<MtlTextureInfo>(that);
    // The usages may match or the usage passed in may be a superset of the usage stored within.
    return fFormat == mt.fFormat && fStorageMode == mt.fStorageMode &&
            fFramebufferOnly == mt.fFramebufferOnly && (fUsage & mt.fUsage) == fUsage;
}

namespace TextureInfos {

skgpu::graphite::TextureInfo MakeMetal(CFTypeRef mtlTexture) {
    return MakeMetal(MtlTextureInfo(mtlTexture));
}

skgpu::graphite::TextureInfo MakeMetal(const MtlTextureInfo& mtlInfo) {
    return TextureInfoPriv::Make(mtlInfo);
}

bool GetMtlTextureInfo(const TextureInfo& info, MtlTextureInfo* out) {
    return TextureInfoPriv::Copy(info, out);
}

}  // namespace TextureInfos

}  // namespace skgpu::graphite
