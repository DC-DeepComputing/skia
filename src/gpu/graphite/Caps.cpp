/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/Caps.h"

#include "include/core/SkCapabilities.h"
#include "include/core/SkPaint.h"
#include "include/core/SkTextureCompressionType.h"
#include "include/gpu/ShaderErrorHandler.h"
#include "include/gpu/graphite/ContextOptions.h"
#include "include/gpu/graphite/TextureInfo.h"
#include "src/base/SkMathPriv.h"
#include "src/core/SkBlenderBase.h"
#include "src/gpu/graphite/GraphiteResourceKey.h"
#include "src/gpu/graphite/ResourceTypes.h"
#include "src/sksl/SkSLUtil.h"

namespace skgpu::graphite {

Caps::Caps()
        : fShaderCaps(std::make_unique<SkSL::ShaderCaps>())
        , fCapabilities(new SkCapabilities()) {}

Caps::~Caps() {}

void Caps::finishInitialization(const ContextOptions& options) {
    fCapabilities->initSkCaps(fShaderCaps.get());

    fDefaultMSAASamples = options.fInternalMultisampleCount;

    if (options.fShaderErrorHandler) {
        fShaderErrorHandler = options.fShaderErrorHandler;
    } else {
        fShaderErrorHandler = DefaultShaderErrorHandler();
    }

#if defined(GRAPHITE_TEST_UTILS)
    if (options.fOptionsPriv) {
        fMaxTextureSize = std::min(fMaxTextureSize, options.fOptionsPriv->fMaxTextureSizeOverride);
        fMaxTextureAtlasSize = options.fOptionsPriv->fMaxTextureAtlasSize;
        fRequestedPathRendererStrategy = options.fOptionsPriv->fPathRendererStrategy;
    }
#endif
    fGlyphCacheTextureMaximumBytes = options.fGlyphCacheTextureMaximumBytes;
    fMinDistanceFieldFontSize = options.fMinDistanceFieldFontSize;
    fGlyphsAsPathsFontSize = options.fGlyphsAsPathsFontSize;
    fMaxPathAtlasTextureSize = options.fMaxPathAtlasTextureSize;
    fAllowMultipleAtlasTextures = options.fAllowMultipleAtlasTextures;
    fSupportBilerpFromGlyphAtlas = options.fSupportBilerpFromGlyphAtlas;
    if (options.fDisableCachedGlyphUploads) {
        fRequireOrderedRecordings = true;
    }
}

sk_sp<SkCapabilities> Caps::capabilities() const { return fCapabilities; }

SkISize Caps::getDepthAttachmentDimensions(const TextureInfo& textureInfo,
                                           const SkISize colorAttachmentDimensions) const {
    return colorAttachmentDimensions;
}

bool Caps::isTexturable(const TextureInfo& info) const {
    if (info.numSamples() > 1) {
        return false;
    }
    return this->onIsTexturable(info);
}

GraphiteResourceKey Caps::makeSamplerKey(const SamplerDesc& samplerDesc) const {
    static const ResourceType kType = GraphiteResourceKey::GenerateResourceType();

    constexpr int kNumTileModeBits   = SkNextLog2_portable(int(SkTileMode::kLastTileMode)+1);
    constexpr int kNumFilterModeBits = SkNextLog2_portable(int(SkFilterMode::kLast)+1);
    constexpr int kNumMipmapModeBits = SkNextLog2_portable(int(SkMipmapMode::kLast)+1);

    constexpr int kTileModeXShift  = 0;
    constexpr int kTileModeYShift  = kTileModeXShift  + kNumTileModeBits;
    constexpr int kFilterModeShift = kTileModeYShift  + kNumTileModeBits;
    constexpr int kMipmapModeShift = kFilterModeShift + kNumFilterModeBits;

    static_assert(kMipmapModeShift + kNumMipmapModeBits <= 32);

    // Backend-agnostic sampler information can fit within one uint32_t.
    // TODO: Add aniso value when used.
    static_assert(sizeof(uint32_t) == 4);

    uint32_t baseKeyInfo =
            (static_cast<uint32_t>(samplerDesc.tileModeX())              << kTileModeXShift)  |
            (static_cast<uint32_t>(samplerDesc.tileModeY())              << kTileModeYShift)  |
            (static_cast<uint32_t>(samplerDesc.samplingOptions().filter) << kFilterModeShift) |
            (static_cast<uint32_t>(samplerDesc.samplingOptions().mipmap) << kMipmapModeShift) ;

    return this->finishSamplerKey(samplerDesc, kType, baseKeyInfo);
}

GraphiteResourceKey Caps::finishSamplerKey(const SamplerDesc& samplerDesc,
                                           ResourceType type,
                                           uint32_t baseKeyInfo) const {
    GraphiteResourceKey samplerKey;
    GraphiteResourceKey::Builder builder(&samplerKey, type, /*data32Count=*/1, Shareable::kYes);

    // The default impl. of this method adds no additional backend information to the key.
    builder[0] = baseKeyInfo;

    builder.finish();
    return samplerKey;
}

bool Caps::areColorTypeAndTextureInfoCompatible(SkColorType ct, const TextureInfo& info) const {
    // TODO: add SkTextureCompressionType handling
    // (can be handled by setting up the colorTypeInfo instead?)

    return SkToBool(this->getColorTypeInfo(ct, info));
}

static inline SkColorType color_type_fallback(SkColorType ct) {
    switch (ct) {
        // kRGBA_8888 is our default fallback for many color types that may not have renderable
        // backend formats.
        case kAlpha_8_SkColorType:
        case kRGB_565_SkColorType:
        case kARGB_4444_SkColorType:
        case kBGRA_8888_SkColorType:
        case kRGBA_1010102_SkColorType:
        case kBGRA_1010102_SkColorType:
        case kRGBA_F16_SkColorType:
        case kRGBA_F16Norm_SkColorType:
            return kRGBA_8888_SkColorType;
        case kA16_float_SkColorType:
            return kRGBA_F16_SkColorType;
        case kGray_8_SkColorType:
            return kRGB_888x_SkColorType;
        default:
            return kUnknown_SkColorType;
    }
}

SkColorType Caps::getRenderableColorType(SkColorType ct) const {
    do {
        auto texInfo = this->getDefaultSampledTextureInfo(ct,
                                                          Mipmapped::kNo,
                                                          Protected::kNo,
                                                          Renderable::kYes);
        // We continue to the fallback color type if there is no default renderable format
        if (texInfo.isValid() && this->isRenderable(texInfo)) {
            return ct;
        }
        ct = color_type_fallback(ct);
    } while (ct != kUnknown_SkColorType);
    return kUnknown_SkColorType;
}

skgpu::Swizzle Caps::getReadSwizzle(SkColorType ct, const TextureInfo& info) const {
    // TODO: add SkTextureCompressionType handling
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

DstReadRequirement Caps::getDstReadRequirement() const {
    // TODO(b/238757201): Currently this only supports dst reads by FB fetch and texture copy.
    if (this->shaderCaps()->fFBFetchSupport) {
        return DstReadRequirement::kFramebufferFetch;
    } else {
        return DstReadRequirement::kTextureCopy;
    }
}

sktext::gpu::SDFTControl Caps::getSDFTControl(bool useSDFTForSmallText) const {
#if !defined(SK_DISABLE_SDF_TEXT)
    return sktext::gpu::SDFTControl{
            this->shaderCaps()->supportsDistanceFieldText(),
            useSDFTForSmallText,
            true, /*ableToUsePerspectiveSDFT*/
            this->minDistanceFieldFontSize(),
            this->glyphsAsPathsFontSize()};
#else
    return sktext::gpu::SDFTControl{};
#endif
}

} // namespace skgpu::graphite
