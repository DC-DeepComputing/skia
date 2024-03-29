/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "include/gpu/ganesh/mtl/GrMtlBackendSurface.h"

#include "include/gpu/GrTypes.h"
#include "include/private/base/SkAssert.h"
#include "src/gpu/ganesh/GrBackendSurfacePriv.h"
#include "include/gpu/ganesh/mtl/GrMtlTypes.h"
#include "src/gpu/ganesh/mtl/GrMtlUtil.h"
#include "src/gpu/mtl/MtlUtilsPriv.h"

#if !__has_feature(objc_arc)
#error This file must be compiled with Arc. Use -fobjc-arc flag
#endif


class GrMtlBackendFormatData final : public GrBackendFormatData {
public:
    GrMtlBackendFormatData(GrMTLPixelFormat format)
            : fFormat((MTLPixelFormat) format) {}

    GrMTLPixelFormat asVkFormat() const { return (GrMTLPixelFormat) fFormat; }

private:
    SkTextureCompressionType compressionType() const override {
        return GrMtlFormatToCompressionType(fFormat);
    }

    size_t bytesPerBlock() const override {
        return skgpu::MtlFormatBytesPerBlock(fFormat);
    }

    int stencilBits() const override {
        return GrMtlFormatStencilBits(fFormat);
    }

    uint32_t channelMask() const override {
        return skgpu::MtlFormatChannels(fFormat);
    }

    GrColorFormatDesc desc() const override {
        return GrMtlFormatDesc(fFormat);
    }

    bool equal(const GrBackendFormatData* that) const override {
        SkASSERT(!that || that->type() == GrBackendApi::kMetal);
        if (auto otherMtl = static_cast<const GrMtlBackendFormatData*>(that)) {
            return fFormat == otherMtl->fFormat;
        }
        return false;
    }

    std::string toString() const override {
#if defined(SK_DEBUG) || GR_TEST_UTILS
        return skgpu::MtlFormatToString(fFormat);
#else
        return "";
#endif
    }

    void copyTo(AnyFormatData& formatData) const override {
        formatData.emplace<GrMtlBackendFormatData>(fFormat);
    }

#if defined(SK_DEBUG)
    GrBackendApi type() const override { return GrBackendApi::kMetal; }
#endif

    MTLPixelFormat fFormat;
};

static const GrMtlBackendFormatData* get_and_cast_data(const GrBackendFormat& format) {
    auto data = GrBackendSurfacePriv::GetBackendData(format);
    SkASSERT(!data || data->type() == GrBackendApi::kMetal);
    return static_cast<const GrMtlBackendFormatData*>(data);
}

namespace GrBackendFormats {

GrBackendFormat MakeMtl(GrMTLPixelFormat format) {
    return GrBackendSurfacePriv::MakeGrBackendFormat(
            GrTextureType::k2D,
            GrBackendApi::kMetal,
            GrMtlBackendFormatData(format));
}

GrMTLPixelFormat AsMtlFormat(const GrBackendFormat& format) {
    if (format.isValid() && format.backend() == GrBackendApi::kMetal) {
        const GrMtlBackendFormatData* data = get_and_cast_data(format);
        SkASSERT(data);
        return data->asVkFormat();
    }
    // MTLPixelFormatInvalid == 0
    return GrMTLPixelFormat(0);
}

}  // namespace GrBackendFormats
