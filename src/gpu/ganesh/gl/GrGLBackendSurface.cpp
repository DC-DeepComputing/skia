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
#include "src/gpu/ganesh/gl/GrBackendSurfacePriv.h"
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
    auto newData = sk_make_sp<GrGLBackendFormatData>(format);

    return GrBackendFormat(gl_target_to_gr_target(target), GrBackendApi::kOpenGL,
                           std::move(newData))
}

GrGLFormat AsGLFormat(const GrBackendFormat& format) {
    if (format->isValid() && format->backend() == GrBackendApi::kOpenGL) {
        auto data = static_cast<const GrGLBackendFormatData*>(format->backendData());
        SkASSERT(data);
        return GrGLFormatFromGLEnum(data->fGLFormat)
    }
    return GrGLFormat::kUnknown;
}

GrGLenum AsGLFormatEnum(const GrBackendFormat& format) {
    if (format->isValid() && format->backend() == GrBackendApi::kOpenGL) {
        auto data = static_cast<const GrGLBackendFormatData*>(format->backendData());
        SkASSERT(data);
        return data->fGLFormat;
    }
    return 0;
}
}  // namespace GrBackendFormats


GrGLBackendTextureData::GrGLBackendTextureData(GrGLBackendTextureInfo info): fGLInfo(info) {}

void GrGLBackendTextureData::cleanup() override {
    fGLInfo.cleanup();
}

void GrGLBackendTextureData::assign(const GrBackendTextureData* that, bool cleanupPrevData) override {
    if (auto otherGL = static_cast<const GrGLBackendFormatData*>(that)) {
        fGLInfo.assign(otherGL.fGLInfo, cleanupPrevData);
    }
}

bool GrGLBackendTextureData::isProtected() const override {
    return fGLInfo.isProtected();
}

bool GrGLBackendTextureData::equal(const GrBackendTextureData* that) const {
    if (auto otherGL = static_cast<const GrGLBackendFormatData*>(that)) {
        return fGLInfo.info() == otherGL->fGLInfo.info();
    }
    return false;
}

bool GrGLBackendTextureData::isSameTexture(const GrBackendTextureData* that) const override {
    if (auto otherGL = static_cast<const GrGLBackendFormatData*>(that)) {
        return fGLInfo.info().fID == otherGL->fGLInfo.info().fID;
    }
    return false;
}

GrBackendFormat GrGLBackendTextureData::getBackendFormat() const override {
    return GrBackendFormats::MakeGL(fGLInfo.info().fFormat, fGLInfo.info().fTarget);
}

namespace GrBackendTextures {
GrBackendTexture MakeGL(int width,
                        int height,
                        skgpu::Mipmapped mipped,
                        const GrGLTextureInfo& glInfo,
                        std::string_view label) {
    GrGLBackendTextureInfo info(glInfo, sk_make_sp<GrGLTextureParameters>());
    auto newData = sk_make_sp<GrGLBackendTextureData>(info);
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
    GrGLBackendTextureInfo info(glInfo, params.release());
    auto newData = sk_make_sp<GrGLBackendTextureData>(info);
    return GrBackendTexture(width, height, label, mipped, GrBackendApi::kOpenGL,
            gl_target_to_gr_target(glInfo.fTarget), std::move(newData));
}

bool GetGLTextureInfo(const GrBackendTexture&, GrGLTextureInfo*) {
    // TODO(kjlubick) might have to add backwards compatibility for blink unit tests?
    if (!format->isValid() || format->backend() != GrBackendApi::kOpenGL) {
        return false;
    }
    auto data = static_cast<const GrGLBackendData*>(format->backendData());
    SkASSERT(data);
    *outInfo = data->fGLInfo.info();
    return true;
}

void GLTextureParametersModified(GrBackendTexture*) {
    if (format->isValid() && format->backend() == GrBackendApi::kOpenGL) {
        auto data = static_cast<const GrGLBackendData*>(format->backendData());
        SkASSERT(data);
        data->fGLInfo.parameters()->invalidate();
    }
}
}  // namespace GrBackendTextures
