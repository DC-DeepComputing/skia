/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/Caps.h"

#include "include/core/SkCapabilities.h"
#include "include/gpu/ShaderErrorHandler.h"
#include "include/gpu/graphite/ContextOptions.h"
#include "include/gpu/graphite/TextureInfo.h"
#include "include/private/SkImageInfoPriv.h"
#include "src/sksl/SkSLUtil.h"

namespace skgpu::graphite {

Caps::Caps()
        : fShaderCaps(std::make_unique<SkSL::ShaderCaps>())
        , fCapabilities(new SkCapabilities()) {}

Caps::~Caps() {}

void Caps::finishInitialization(const ContextOptions& options) {
    fCapabilities->initSkCaps(fShaderCaps.get());

    if (options.fShaderErrorHandler) {
        fShaderErrorHandler = options.fShaderErrorHandler;
    } else {
        fShaderErrorHandler = DefaultShaderErrorHandler();
    }

#if GRAPHITE_TEST_UTILS
    fMaxTextureAtlasSize = options.fMaxTextureAtlasSize;
#endif
    fGlyphCacheTextureMaximumBytes = options.fGlyphCacheTextureMaximumBytes;
    fMinDistanceFieldFontSize = options.fMinDistanceFieldFontSize;
    fGlyphsAsPathsFontSize = options.fGlyphsAsPathsFontSize;
    fAllowMultipleGlyphCacheTextures = options.fAllowMultipleGlyphCacheTextures;
    fSupportBilerpFromGlyphAtlas = options.fSupportBilerpFromGlyphAtlas;
}

sk_sp<SkCapabilities> Caps::capabilities() const { return fCapabilities; }

bool Caps::isTexturable(const TextureInfo& info) const {
    if (info.numSamples() > 1) {
        return false;
    }
    return this->onIsTexturable(info);
}

bool Caps::areColorTypeAndTextureInfoCompatible(SkColorType ct, const TextureInfo& info) const {
    // TODO: add SkImage::CompressionType handling
    // (can be handled by setting up the colorTypeInfo instead?)

    return SkToBool(this->getColorTypeInfo(ct, info));
}

bool Caps::textureSupportsWritePixels(const TextureProxy* proxy) const {
    // TODO: yeah, the proxy doesn't have this info
    // What happens if we instantiate and it's readonly then?
    return /*proxy->readOnly() ? false :*/ this->onTextureSupportsWritePixels(proxy);
}

Caps::SupportedRead Caps::supportedReadPixelsColorType(SkColorType srcColorType,
                                                       const TextureInfo& srcTextureInfo,
                                                       SkColorType dstColorType) const {
    SupportedRead read = this->onSupportedReadPixelsColorType(srcColorType, srcTextureInfo,
                                                              dstColorType);

    // There are known problems with 24 vs 32 bit BPP with this color type. Just fail for now if
    // using a transfer buffer.
    if (read.fColorType == kRGB_888x_SkColorType) {
        read.fOffsetAlignmentForTransferBuffer = 0;
    }
    // It's very convenient to access 1 byte-per-channel 32 bit color types as uint32_t on the CPU.
    // Make those aligned reads out of the buffer even if the underlying API doesn't require it.
    auto channelFlags = SkColorTypeChannelFlags(read.fColorType);
    if ((channelFlags == kRGBA_SkColorChannelFlags || channelFlags == kRGB_SkColorChannelFlags ||
         channelFlags == kAlpha_SkColorChannelFlag || channelFlags == kGray_SkColorChannelFlag) &&
        SkColorTypeBytesPerPixel(read.fColorType) == 4) {
        switch (read.fOffsetAlignmentForTransferBuffer & 0b11) {
            // offset alignment already a multiple of 4
            case 0:
                break;
            // offset alignment is a multiple of 2 but not 4.
            case 2:
                read.fOffsetAlignmentForTransferBuffer *= 2;
                break;
            // offset alignment is not a multiple of 2.
            default:
                read.fOffsetAlignmentForTransferBuffer *= 4;
                break;
        }
    }
    return read;
}

skgpu::Swizzle Caps::getReadSwizzle(SkColorType ct, const TextureInfo& info) const {
    // TODO: add SkImage::CompressionType handling
    // (can be handled by setting up the colorTypeInfo instead?)

    auto colorTypeInfo = this->getColorTypeInfo(ct, info);
    if (!colorTypeInfo) {
        SkDEBUGFAILF("Illegal color type (%d) and format combination.", static_cast<int>(ct));
        return {};
    }

    return colorTypeInfo->fReadSwizzle;
}

skgpu::Swizzle Caps::getWriteSwizzle(SkColorType ct, const TextureInfo& info) const {
    auto colorTypeInfo = this->getColorTypeInfo(ct, info);
    if (!colorTypeInfo) {
        SkDEBUGFAILF("Illegal color type (%d) and format combination.", static_cast<int>(ct));
        return {};
    }

    return colorTypeInfo->fWriteSwizzle;
}

sktext::gpu::SDFTControl Caps::getSDFTControl(bool useSDFTForSmallText) const {
    return sktext::gpu::SDFTControl{
            this->shaderCaps()->supportsDistanceFieldText(),
            useSDFTForSmallText,
            true, /*ableToUsePerspectiveSDFT*/
            this->minDistanceFieldFontSize(),
            this->glyphsAsPathsFontSize()};
}

} // namespace skgpu::graphite
