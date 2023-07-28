/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "include/gpu/ganesh/gl/GrGLBackendSurface.h"

#include "include/core/SkRefCnt.h"
#include "include/core/SkTextureCompressionType.h"
#include "include/core/SkTypes.h"
#include "include/gpu/GrBackendSurface.h"
#include "include/gpu/GrTypes.h"
#include "include/gpu/gl/GrGLTypes.h"
#include "include/private/base/SkTo.h"
#include "include/private/gpu/ganesh/GrGLTypesPriv.h"
#include "include/private/gpu/ganesh/GrTypesPriv.h"
#include "src/gpu/ganesh/gl/GrGLBackendSurfacePriv.h"
#include "src/gpu/ganesh/gl/GrGLDefines.h"
#include "src/gpu/ganesh/gl/GrGLUtil.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <string>
#include <utility>

class GrGLBackendFormatData final : public GrBackendFormatData {
public:
    GrGLBackendFormatData(GrGLenum format): fGLFormat(format) {}

    GrGLenum asEnum() const { return fGLFormat; }
private:
    SkTextureCompressionType compressionType() const override {
        switch (GrGLFormatFromGLEnum(fGLFormat)) {
            case GrGLFormat::kCOMPRESSED_ETC1_RGB8:
            case GrGLFormat::kCOMPRESSED_RGB8_ETC2:
                return SkTextureCompressionType::kETC2_RGB8_UNORM;
            case GrGLFormat::kCOMPRESSED_RGB8_BC1:
                return SkTextureCompressionType::kBC1_RGB8_UNORM;
            case GrGLFormat::kCOMPRESSED_RGBA8_BC1:
                return SkTextureCompressionType::kBC1_RGBA8_UNORM;
            default:
                return SkTextureCompressionType::kNone;
        }
    }

    size_t bytesPerBlock() const override {
        return GrGLFormatBytesPerBlock(GrGLFormatFromGLEnum(fGLFormat));
    }

    int stencilBits() const override {
        return GrGLFormatStencilBits(GrGLFormatFromGLEnum(fGLFormat));
    }

    uint32_t channelMask() const override {
        return GrGLFormatChannels(GrGLFormatFromGLEnum(fGLFormat));
    }

    GrColorFormatDesc desc() const override {
        return GrGLFormatDesc(GrGLFormatFromGLEnum(fGLFormat));
    }

    bool equal(const GrBackendFormatData* that) const override {
        if (auto otherGL = static_cast<const GrGLBackendFormatData*>(that)) {
            return fGLFormat == otherGL->fGLFormat;
        }
        return false;
    }

    std::string toString() const override {
        return GrGLFormatToStr(fGLFormat);
    }

    GrGLenum fGLFormat; // the sized, internal format of the GL resource
};

static GrTextureType gl_target_to_gr_target(GrGLenum target) {
    switch (target) {
        case GR_GL_TEXTURE_NONE:
            return GrTextureType::kNone;
        case GR_GL_TEXTURE_2D:
            return  GrTextureType::k2D;
        case GR_GL_TEXTURE_RECTANGLE:
            return GrTextureType::kRectangle;
        case GR_GL_TEXTURE_EXTERNAL:
            return GrTextureType::kExternal;
        default:
            SkUNREACHABLE;
    }
}

namespace GrBackendFormats {
GrBackendFormat MakeGL(GrGLenum format, GrGLenum target) {
    auto newData = sk_make_sp<GrGLBackendFormatData>(format);

    return GrBackendFormat(gl_target_to_gr_target(target), GrBackendApi::kOpenGL,
                           std::move(newData));
}

GrGLFormat AsGLFormat(const GrBackendFormat& format) {
    if (format.isValid() && format.backend() == GrBackendApi::kOpenGL) {
        auto data = static_cast<const GrGLBackendFormatData*>(format.backendData().get());
        SkASSERT(data);
        return GrGLFormatFromGLEnum(data->asEnum());
    }
    return GrGLFormat::kUnknown;
}

GrGLenum AsGLFormatEnum(const GrBackendFormat& format) {
    if (format.isValid() && format.backend() == GrBackendApi::kOpenGL) {
        auto data = static_cast<const GrGLBackendFormatData*>(format.backendData().get());
        SkASSERT(data);
        return data->asEnum();
    }
    return 0;
}
}  // namespace GrBackendFormats


GrGLBackendTextureData::GrGLBackendTextureData(const GrGLTextureInfo& info,
                                               GrGLTextureParameters* params):
    fGLInfo(info, params) {}

void GrGLBackendTextureData::cleanup() {
    fGLInfo.cleanup();
}

void GrGLBackendTextureData::assign(const GrBackendTextureData* that, bool cleanupPrevData) {
    if (auto otherGL = static_cast<const GrGLBackendTextureData*>(that)) {
        fGLInfo.assign(otherGL->fGLInfo, cleanupPrevData);
    }
}

bool GrGLBackendTextureData::isProtected() const {
    return fGLInfo.isProtected();
}

bool GrGLBackendTextureData::equal(const GrBackendTextureData* that) const {
    if (auto otherGL = static_cast<const GrGLBackendTextureData*>(that)) {
        return fGLInfo.info() == otherGL->fGLInfo.info();
    }
    return false;
}

bool GrGLBackendTextureData::isSameTexture(const GrBackendTextureData* that) const {
    if (auto otherGL = static_cast<const GrGLBackendTextureData*>(that)) {
        return fGLInfo.info().fID == otherGL->fGLInfo.info().fID;
    }
    return false;
}

GrBackendFormat GrGLBackendTextureData::getBackendFormat() const {
    return GrBackendFormats::MakeGL(fGLInfo.info().fFormat, fGLInfo.info().fTarget);
}

namespace GrBackendTextures {
GrBackendTexture MakeGL(int width,
                        int height,
                        skgpu::Mipmapped mipped,
                        const GrGLTextureInfo& glInfo,
                        std::string_view label) {
    auto newData = sk_make_sp<GrGLBackendTextureData>(glInfo,
                                                      sk_make_sp<GrGLTextureParameters>().get());
    auto tex = GrBackendTexture(width, height, label, mipped, GrBackendApi::kOpenGL,
            gl_target_to_gr_target(glInfo.fTarget), std::move(newData));
    // Make no assumptions about client's texture's parameters.
    GLTextureParametersModified(&tex);
    return tex;
}

GrBackendTexture MakeGL(int width,
                        int height,
                        skgpu::Mipmapped mipped,
                        const GrGLTextureInfo& glInfo,
                        sk_sp<GrGLTextureParameters> params,
                        std::string_view label) {
    auto newData = sk_make_sp<GrGLBackendTextureData>(glInfo, params.release());
    return GrBackendTexture(width, height, label, mipped, GrBackendApi::kOpenGL,
            gl_target_to_gr_target(glInfo.fTarget), std::move(newData));
}

bool GetGLTextureInfo(const GrBackendTexture& tex, GrGLTextureInfo* outInfo) {
    if (!tex.isValid() || tex.backend() != GrBackendApi::kOpenGL) {
        return false;
    }
    auto data = static_cast<const GrGLBackendTextureData*>(tex.backendData().get());
    SkASSERT(data);
    *outInfo = data->info().info();
    return true;
}

void GLTextureParametersModified(GrBackendTexture* tex) {
    if (tex && tex->isValid() && tex->backend() == GrBackendApi::kOpenGL) {
        auto data = static_cast<const GrGLBackendTextureData*>(tex->backendData().get());
        SkASSERT(data);
        data->info().parameters()->invalidate();
    }
}
}  // namespace GrBackendTextures


class GrGLRenderTargetData final : public GrBackendRenderTargetData {
public:
    GrGLRenderTargetData(GrGLFramebufferInfo info): fGLInfo(info) {}

    GrGLFramebufferInfo info() const { return fGLInfo; }
private:
    bool isValid() const override {
        // the glInfo must have a valid format
        return SkToBool(fGLInfo.fFormat);
    }

    GrBackendFormat getBackendFormat() const override {
        return GrBackendFormats::MakeGL(fGLInfo.fFormat, GR_GL_TEXTURE_NONE);
    }

    bool isProtected() const override {
        return fGLInfo.isProtected();
    }

    bool equal(const GrBackendRenderTargetData* that) const override {
        if (auto otherGL = static_cast<const GrGLRenderTargetData*>(that)) {
            return fGLInfo == otherGL->fGLInfo;
        }
        return false;
    }

    GrGLFramebufferInfo fGLInfo;
};

namespace GrBackendRenderTargets {
// The GrGLTextureInfo must have a valid fFormat. If wrapping in an SkSurface we require the
// stencil bits to be either 0, 8 or 16.
SK_API GrBackendRenderTarget MakeGL(int width,
                                    int height,
                                    int sampleCnt,
                                    int stencilBits,
                                    const GrGLFramebufferInfo& glInfo) {
    auto newData = sk_make_sp<GrGLRenderTargetData>(glInfo);
    return GrBackendRenderTarget(width, height, std::max(1, sampleCnt), stencilBits,
                                 GrBackendApi::kOpenGL, /*framebufferOnly=*/false,
                                 std::move(newData));
}

SK_API bool GetGLFramebufferInfo(const GrBackendRenderTarget& rt, GrGLFramebufferInfo* outInfo) {
    if (!rt.isValid() || rt.backend() != GrBackendApi::kOpenGL) {
        return false;
    }
    auto data = static_cast<const GrGLRenderTargetData*>(rt.backendData().get());
    SkASSERT(data);
    *outInfo = data->info();
    return true;
}

}  // namespace GrBackendRenderTargets
