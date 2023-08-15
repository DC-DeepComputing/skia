/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "include/gpu/ganesh/vk/GrVkBackendSurface.h"

#include "include/core/SkRefCnt.h"
#include "include/core/SkTextureCompressionType.h"
#include "include/gpu/GrBackendSurface.h"
#include "include/gpu/GrTypes.h"
#include "include/private/base/SkAssert.h"
#include "include/private/base/SkTo.h"
#include "include/private/gpu/ganesh/GrTypesPriv.h"
#include "src/gpu/MutableTextureStateRef.h"
#include "src/gpu/ganesh/GrBackendSurfacePriv.h"
#include "src/gpu/ganesh/vk/GrVkUtil.h"
#include "src/gpu/vk/VulkanUtilsPriv.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <string>

class GrVkBackendFormatData final : public GrBackendFormatData {
public:
    GrVkBackendFormatData(VkFormat format, const GrVkYcbcrConversionInfo& ycbcrInfo)
            : fFormat(format), fYcbcrConversionInfo(ycbcrInfo) {}

    VkFormat asVkFormat() const { return fFormat; }
    const GrVkYcbcrConversionInfo* getYcbcrConversionInfo() const { return &fYcbcrConversionInfo; }

private:
    SkTextureCompressionType compressionType() const override {
        switch (fFormat) {
            case VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK:
                return SkTextureCompressionType::kETC2_RGB8_UNORM;
            case VK_FORMAT_BC1_RGB_UNORM_BLOCK:
                return SkTextureCompressionType::kBC1_RGB8_UNORM;
            case VK_FORMAT_BC1_RGBA_UNORM_BLOCK:
                return SkTextureCompressionType::kBC1_RGBA8_UNORM;
            default:
                return SkTextureCompressionType::kNone;
        }
    }

    size_t bytesPerBlock() const override {
        return skgpu::VkFormatBytesPerBlock(fFormat);
    }

    int stencilBits() const override {
        return skgpu::VkFormatStencilBits(fFormat);
    }

    uint32_t channelMask() const override {
        return skgpu::VkFormatChannels(fFormat);
    }

    GrColorFormatDesc desc() const override {
        return GrVkFormatDesc(fFormat);
    }

    bool equal(const GrBackendFormatData* that) const override {
        SkASSERT(!that || that->type() == GrBackendApi::kVulkan);
        if (auto otherVk = static_cast<const GrVkBackendFormatData*>(that)) {
            return fFormat == otherVk->fFormat &&
                   fYcbcrConversionInfo == otherVk->fYcbcrConversionInfo;
        }
        return false;
    }

    std::string toString() const override {
#if defined(SK_DEBUG) || GR_TEST_UTILS
        return skgpu::VkFormatToStr(fFormat);
#else
        return "";
#endif
    }

    void copyTo(AnyFormatData& formatData) const override {
        formatData.emplace<GrVkBackendFormatData>(fFormat, fYcbcrConversionInfo);
    }

#if defined(SK_DEBUG)
    GrBackendApi type() const override { return GrBackendApi::kVulkan; }
#endif

    VkFormat fFormat;
    GrVkYcbcrConversionInfo fYcbcrConversionInfo;
};

static const GrVkBackendFormatData* get_and_cast_data(const GrBackendFormat& format) {
    auto data = GrBackendSurfacePriv::GetBackendData(format);
    SkASSERT(!data || data->type() == GrBackendApi::kVulkan);
    return static_cast<const GrVkBackendFormatData*>(data);
}

namespace GrBackendFormats {

GrBackendFormat MakeVk(VkFormat format, bool willUseDRMFormatModifiers) {
    return GrBackendSurfacePriv::MakeGrBackendFormat(
            GrTextureType::k2D,
            GrBackendApi::kVulkan,
            GrVkBackendFormatData(format, GrVkYcbcrConversionInfo{}));
}

GrBackendFormat MakeVk(const GrVkYcbcrConversionInfo& ycbcrInfo, bool willUseDRMFormatModifiers) {
    SkASSERT(ycbcrInfo.isValid());
    GrTextureType textureType =
            ((ycbcrInfo.isValid() && ycbcrInfo.fExternalFormat) || willUseDRMFormatModifiers)
                    ? GrTextureType::kExternal
                    : GrTextureType::k2D;
    return GrBackendSurfacePriv::MakeGrBackendFormat(
            textureType,
            GrBackendApi::kVulkan,
            GrVkBackendFormatData(ycbcrInfo.fFormat, ycbcrInfo));
}

bool AsVkFormat(const GrBackendFormat& format, VkFormat* vkFormat) {
    SkASSERT(vkFormat);
    if (format.isValid() && format.backend() == GrBackendApi::kVulkan) {
        const GrVkBackendFormatData* data = get_and_cast_data(format);
        SkASSERT(data);
        *vkFormat = data->asVkFormat();
        return true;
    }
    return false;
}

const GrVkYcbcrConversionInfo* GetVkYcbcrConversionInfo(const GrBackendFormat& format) {
    if (format.isValid() && format.backend() == GrBackendApi::kVulkan) {
        const GrVkBackendFormatData* data = get_and_cast_data(format);
        SkASSERT(data);
        return data->getYcbcrConversionInfo();
    }
    return nullptr;
}

}  // namespace GrBackendFormats


class GrVkBackendTextureData final : public GrBackendTextureData {
public:
    GrVkBackendTextureData(const GrGLTextureInfo& info, sk_sp<GrGLTextureParameters> params);

    const GrVkBackendSurfaceInfo& info() const { return fVkInfo; }

private:
    void copyTo(AnyTextureData&) const override;
    bool isProtected() const override;
    bool equal(const GrBackendTextureData* that) const override;
    bool isSameTexture(const GrBackendTextureData* that) const override;
    GrBackendFormat getBackendFormat() const override;
#if defined(SK_DEBUG)
    GrBackendApi type() const override { return GrBackendApi::kOpenGL; }
#endif

    GrVkBackendSurfaceInfo fVkInfo;
};

GrGLBackendTextureData::GrGLBackendTextureData(const GrGLTextureInfo& info,
                                               sk_sp<GrGLTextureParameters> params)
        : fGLInfo(info, params) {}

void GrGLBackendTextureData::copyTo(AnyTextureData& textureData) const {
    textureData.emplace<GrGLBackendTextureData>(fGLInfo.info(), fGLInfo.refParameters());
}

bool GrGLBackendTextureData::isProtected() const { return fGLInfo.isProtected(); }

bool GrGLBackendTextureData::equal(const GrBackendTextureData* that) const {
    SkASSERT(!that || that->type() == GrBackendApi::kOpenGL);
    if (auto otherGL = static_cast<const GrGLBackendTextureData*>(that)) {
        return fGLInfo.info() == otherGL->fGLInfo.info();
    }
    return false;
}

bool GrGLBackendTextureData::isSameTexture(const GrBackendTextureData* that) const {
    SkASSERT(!that || that->type() == GrBackendApi::kOpenGL);
    if (auto otherGL = static_cast<const GrGLBackendTextureData*>(that)) {
        return fGLInfo.info().fID == otherGL->fGLInfo.info().fID;
    }
    return false;
}

GrBackendFormat GrGLBackendTextureData::getBackendFormat() const {
    return GrBackendFormats::MakeGL(fGLInfo.info().fFormat, fGLInfo.info().fTarget);
}

static const GrGLBackendTextureData* get_and_cast_data(const GrBackendTexture& texture) {
    auto data = GrBackendSurfacePriv::GetBackendData(texture);
    SkASSERT(!data || data->type() == GrBackendApi::kOpenGL);
    return static_cast<const GrGLBackendTextureData*>(data);
}

static GrGLBackendTextureData* get_and_cast_data(GrBackendTexture* texture) {
    auto data = GrBackendSurfacePriv::GetBackendData(texture);
    SkASSERT(!data || data->type() == GrBackendApi::kOpenGL);
    return static_cast<GrGLBackendTextureData*>(data);
}

namespace GrBackendTextures {
GrBackendTexture MakeGL(int width,
                        int height,
                        skgpu::Mipmapped mipped,
                        const GrGLTextureInfo& glInfo,
                        std::string_view label) {
    auto tex = GrBackendSurfacePriv::MakeGrBackendTexture(
            width,
            height,
            label,
            mipped,
            GrBackendApi::kOpenGL,
            gl_target_to_gr_target(glInfo.fTarget),
            GrGLBackendTextureData(glInfo, sk_make_sp<GrGLTextureParameters>()));
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
    return GrBackendSurfacePriv::MakeGrBackendTexture(width,
                                                      height,
                                                      label,
                                                      mipped,
                                                      GrBackendApi::kOpenGL,
                                                      gl_target_to_gr_target(glInfo.fTarget),
                                                      GrGLBackendTextureData(glInfo, params));
}

bool GetGLTextureInfo(const GrBackendTexture& tex, GrGLTextureInfo* outInfo) {
    if (!tex.isValid() || tex.backend() != GrBackendApi::kOpenGL) {
        return false;
    }
    const GrGLBackendTextureData* data = get_and_cast_data(tex);
    SkASSERT(data);
    *outInfo = data->info().info();
    return true;
}

void GLTextureParametersModified(GrBackendTexture* tex) {
    if (tex && tex->isValid() && tex->backend() == GrBackendApi::kOpenGL) {
        GrGLBackendTextureData* data = get_and_cast_data(tex);
        SkASSERT(data);
        data->info().parameters()->invalidate();
    }
}
}  // namespace GrBackendTextures

class GrGLBackendRenderTargetData final : public GrBackendRenderTargetData {
public:
    GrGLBackendRenderTargetData(GrGLFramebufferInfo info) : fGLInfo(info) {}

    GrGLFramebufferInfo info() const { return fGLInfo; }

private:
    bool isValid() const override {
        // the glInfo must have a valid format
        return SkToBool(fGLInfo.fFormat);
    }

    GrBackendFormat getBackendFormat() const override {
        return GrBackendFormats::MakeGL(fGLInfo.fFormat, GR_GL_TEXTURE_NONE);
    }

    bool isProtected() const override { return fGLInfo.isProtected(); }

    bool equal(const GrBackendRenderTargetData* that) const override {
        SkASSERT(!that || that->type() == GrBackendApi::kOpenGL);
        if (auto otherGL = static_cast<const GrGLBackendRenderTargetData*>(that)) {
            return fGLInfo == otherGL->fGLInfo;
        }
        return false;
    }

    void copyTo(AnyRenderTargetData& rtData) const override {
        rtData.emplace<GrGLBackendRenderTargetData>(fGLInfo);
    }

#if defined(SK_DEBUG)
    GrBackendApi type() const override { return GrBackendApi::kOpenGL; }
#endif

    GrGLFramebufferInfo fGLInfo;
};

static const GrGLBackendRenderTargetData* get_and_cast_data(const GrBackendRenderTarget& rt) {
    auto data = GrBackendSurfacePriv::GetBackendData(rt);
    SkASSERT(!data || data->type() == GrBackendApi::kOpenGL);
    return static_cast<const GrGLBackendRenderTargetData*>(data);
}

namespace GrBackendRenderTargets {
// The GrGLTextureInfo must have a valid fFormat. If wrapping in an SkSurface we require the
// stencil bits to be either 0, 8 or 16.
SK_API GrBackendRenderTarget
MakeGL(int width, int height, int sampleCnt, int stencilBits, const GrGLFramebufferInfo& glInfo) {
    return GrBackendSurfacePriv::MakeGrBackendRenderTarget(width,
                                                           height,
                                                           std::max(1, sampleCnt),
                                                           stencilBits,
                                                           GrBackendApi::kOpenGL,
                                                           /*framebufferOnly=*/false,
                                                           GrGLBackendRenderTargetData(glInfo));
}

SK_API bool GetGLFramebufferInfo(const GrBackendRenderTarget& rt, GrGLFramebufferInfo* outInfo) {
    if (!rt.isValid() || rt.backend() != GrBackendApi::kOpenGL) {
        return false;
    }
    const GrGLBackendRenderTargetData* data = get_and_cast_data(rt);
    SkASSERT(data);
    *outInfo = data->info();
    return true;
}

}  // namespace GrBackendRenderTargets

#if !defined(SK_DISABLE_LEGACY_GL_BACKEND_SURFACE) && defined(SK_GL)
GrBackendFormat GrBackendFormat::MakeGL(GrGLenum format, GrGLenum target) {
    return GrBackendFormats::MakeGL(format, target);
}

GrGLFormat GrBackendFormat::asGLFormat() const {
    return GrBackendFormats::AsGLFormat(*this);
}

GrGLenum GrBackendFormat::asGLFormatEnum() const {
    return GrBackendFormats::AsGLFormatEnum(*this);
}

GrBackendTexture::GrBackendTexture(int width,
                                   int height,
                                   skgpu::Mipmapped mipped,
                                   const GrGLTextureInfo& glInfo,
                                   std::string_view label)
        : GrBackendTexture(width,
                           height,
                           label,
                           mipped,
                           GrBackendApi::kOpenGL,
                           gl_target_to_gr_target(glInfo.fTarget),
                           GrGLBackendTextureData(glInfo, sk_make_sp<GrGLTextureParameters>())) {
    // Make no assumptions about client's texture's parameters.
    GrBackendTextures::GLTextureParametersModified(this);
}

bool GrBackendTexture::getGLTextureInfo(GrGLTextureInfo* outInfo) const {
    return GrBackendTextures::GetGLTextureInfo(*this, outInfo);
}

void GrBackendTexture::glTextureParametersModified() {
    GrBackendTextures::GLTextureParametersModified(this);
}

GrBackendRenderTarget::GrBackendRenderTarget(
        int width, int height, int sampleCnt, int stencilBits, const GrGLFramebufferInfo& glInfo)
        : GrBackendRenderTarget(width,
                                height,
                                std::max(1, sampleCnt),
                                stencilBits,
                                GrBackendApi::kOpenGL,
                                /*framebufferOnly=*/false,
                                GrGLBackendRenderTargetData(glInfo)) {}

bool GrBackendRenderTarget::getGLFramebufferInfo(GrGLFramebufferInfo* outInfo) const {
    return GrBackendRenderTargets::GetGLFramebufferInfo(*this, outInfo);
}
#endif
