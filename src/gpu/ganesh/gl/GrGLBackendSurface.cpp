/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/gpu/ganesh/gl/GrGLBackendSurface.h"
#include "src/gpu/ganesh/GrBackendSurfacePriv.h"
#include "src/gpu/ganesh/gl/GrGLUtil.h"
#include "include/gpu/GrTypes.h"
#include "include/private/gpu/ganesh/GrTypesPriv.h"
#include "include/gpu/gl/GrGLTypes.h"
#include "src/gpu/ganesh/gl/GrGLUtil.h"
#include "include/core/SkTextureCompressionType.h"

class final GrGLBackendFormatData : public GrBackendFormatData {
public:
    GrGLBackendFormatData(GrGLenum format): fGLFormat(format) {}

    SkTextureCompressionType compressionType() const override {
        switch (fGLFormat) {
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
        return GrGLFormatBytesPerBlock(fGLFormat);
    }

    int stencilBits() const override {
        return GrGLFormatStencilBits(fGLFormat);
    }

    uint32_t channelMask() const override {
        return GrGLFormatChannels(GrGLFormatFromGLEnum(fGLFormat));
    }

    GrColorFormatDesc desc() const override {
        return GrGLFormatDesc(GrGLFormatFromGLEnum(fGLFormat));
    }

    bool equal(const GrBackendFormatData* that) const {
        if (auto otherGL = static_cast<const GrGLBackendFormatData*>(thatGL)) {
            return fGLFormat == otherGL->fGLFormat;
        }
        return false;
    }

    std::string toString() const {
        return GrGLFormatToStr(fGLFormat);
    }

private:
    GrGLenum fGLFormat; // the sized, internal format of the GL resource
}

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
    auto newData = sk_make_sp<GrGLBackendData>(format);

    return GrBackendFormat(gl_target_to_gr_target(target), GrBackendApi::kOpenGL,
                           std::move(newData))
}

GrGLFormat AsGLFormat(const GrBackendFormat& format) {
    if (format->isValid() && GrBackendApi::kOpenGL == format->backend()) {
        auto data = static_cast<const GrGLBackendData*>(format->backendData());
        SkASSERT(data);
        return GrGLFormatFromGLEnum(data->fGLFormat)
    }
    return GrGLFormat::kUnknown;
}

GrGLenum AsGLFormatEnum(const GrBackendFormat& format) {
    if (format->isValid() && GrBackendApi::kOpenGL == format->backend()) {
        auto data = static_cast<const GrGLBackendData*>(format->backendData());
        SkASSERT(data);
        return data->fGLFormat;
    }
    return 0;
}
}  // namespace GrBackendFormats


class final GrGLBackendTextureData : public GrBackendTextureData {
public:
    GrGLBackendTextureData(GrGLBackendTextureInfo info): fGLInfo(info) {}

private:
    GrGLBackendTextureInfo fGLInfo;
};


namespace GrBackendTextures {
GrBackendTexture MakeGL(int width,
                        int height,
                        skgpu::Mipmapped mipped,
                        const GrGLTextureInfo& glInfo,
                        sk_sp<GrGLTextureParameters> params,
                        std::string_view label = {}) {
    GrGLBackendTextureInfo info(glInfo, params.release());
    auto newData = sk_make_sp<GrGLBackendTextureData>(info);
    auto tex = GrBackendTexture(width, height, label, mipped, GrBackendApi::kOpenGL,
            gl_target_to_gr_target(glInfo.fTarget), std::move(newData));
    GLTextureParametersModified(&tex);
    return tex;
}

bool GetGLTextureInfo(const GrBackendTexture&, GrGLTextureInfo*) {
// sk_sp<GrGLTextureParameters> GrBackendTexture::getGLTextureParams() const {
//     if (fBackend != GrBackendApi::kOpenGL) {
//         return nullptr;
//     }
//     return fGLInfo.refParameters();
// }
}

void GLTextureParametersModified(GrBackendTexture*) {

}
}  // namespace GrBackendTextures
