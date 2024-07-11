/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "include/core/SkString.h"
#include "src/gpu/graphite/TextureInfoPriv.h"
#include "src/gpu/graphite/mtl/MtlGraphiteTypesPriv.h"
#include "src/gpu/mtl/MtlUtilsPriv.h"

#include <cstdint>

#import <Metal/Metal.h>

namespace skgpu::graphite {

class MtlTextureInfoData final : public TextureInfoData {
public:
    MtlTextureInfoData(MtlTextureSpec m) : fMtlSpec(m) {}

#if defined(SK_DEBUG)
    skgpu::BackendApi type() const override { return skgpu::BackendApi::kMetal; }
#endif

private:
    MtlTextureSpec fMtlSpec;

    size_t bytesPerPixel() const override {
        return MtlFormatBytesPerBlock(static_cast<MTLPixelFormat>(fMtlSpec.fFormat));
    }

    SkTextureCompressionType compressionType() const override {
        return MtlFormatToCompressionType(static_cast<MTLPixelFormat>(fMtlSpec.fFormat));
    }

    SkString toString() const override {
        return SkStringPrintf("Metal(%s,", fMtlSpec.toString().c_str());
    }

    SkString toRPAttachmentString(uint32_t sampleCount) const override {
        return SkStringPrintf(
                "Metal(f=%u,s=%u)", static_cast<unsigned int>(fMtlSpec.fFormat), sampleCount);
    }

    void copyTo(AnyTextureInfoData& dstData) const override {
        // Don't assert that dstData is a metal type because it could be
        // uninitialized and that assert would fail.
        dstData.emplace<MtlTextureInfoData>(fMtlSpec);
    }

    bool equal(const TextureInfoData* that) const override {
        SkASSERT(!that || that->type() == skgpu::BackendApi::kMetal);
        if (auto otherMtl = static_cast<const MtlTextureInfoData*>(that)) {
            return fMtlSpec == otherMtl->fMtlSpec;
        }
        return false;
    }

    bool isCompatible(const TextureInfoData* that) const override {
        SkASSERT(!that || that->type() == skgpu::BackendApi::kMetal);
        if (auto otherMtl = static_cast<const MtlTextureInfoData*>(that)) {
            return fMtlSpec.isCompatible(otherMtl->fMtlSpec);
        }
        return false;
    }
};

}  // namespace skgpu::graphite
