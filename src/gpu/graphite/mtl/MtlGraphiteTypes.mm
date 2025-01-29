/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkStream.h"
#include "src/gpu/graphite/mtl/MtlGraphiteTypesPriv.h"

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

MtlTextureInfo MtlTextureSpecToTextureInfo(const MtlTextureSpec& mtlSpec,
                                           uint32_t sampleCount,
                                           Mipmapped mipmapped) {
    MtlTextureInfo info;
    // Shared info
    info.fSampleCount = sampleCount;
    info.fMipmapped = mipmapped;

    // Mtl info
    info.fFormat = mtlSpec.fFormat;
    info.fUsage = mtlSpec.fUsage;
    info.fStorageMode = mtlSpec.fStorageMode;
    info.fFramebufferOnly = mtlSpec.fFramebufferOnly;

    return info;
}

bool MtlTextureSpec::serialize(SkWStream* stream) const {
    // TODO(robertphillips): not densely packed
    if (!stream->write32(static_cast<uint32_t>(fFormat)))          { return false; } // 16b
    if (!stream->write32(static_cast<uint32_t>(fUsage)))           { return false; } // 8b
    if (!stream->write32(static_cast<uint32_t>(fStorageMode)))     { return false; } // 4b
    if (!stream->write32(static_cast<uint32_t>(fFramebufferOnly))) { return false; } // 1b
    return true;
}

bool MtlTextureSpec::Deserialize(SkStream* stream, MtlTextureSpec* out) {
    uint32_t tmp;

    if (!stream->readU32(&tmp)) {
        return false;
    }
    out->fFormat = static_cast<MTLPixelFormat>(tmp);

    if (!stream->readU32(&tmp)) {
        return false;
    }
    out->fUsage = static_cast<MTLTextureUsage>(tmp);

    if (!stream->readU32(&tmp)) {
        return false;
    }
    out->fStorageMode = static_cast<MTLStorageMode>(tmp);

    if (!stream->readU32(&tmp)) {
        return false;
    }
    out->fFramebufferOnly = SkToBool(tmp);
    return true;
}

}  // namespace skgpu::graphite
