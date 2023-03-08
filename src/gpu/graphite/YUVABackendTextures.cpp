/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/gpu/graphite/YUVABackendTextures.h"

#include "src/core/SkImageInfoPriv.h"
#include "src/core/SkYUVAInfoLocation.h"

namespace skgpu::graphite {

namespace {
int num_channels(SkColorType colorType) {
    switch (SkColorTypeChannelFlags(colorType)) {
        case kRed_SkColorChannelFlag        : return 1;
        case kAlpha_SkColorChannelFlag      : return 1;
        case kGray_SkColorChannelFlag       : return 1;
        case kGrayAlpha_SkColorChannelFlags : return 2;
        case kRG_SkColorChannelFlags        : return 2;
        case kRGB_SkColorChannelFlags       : return 3;
        case kRGBA_SkColorChannelFlags      : return 4;
        default                             : return 0;
    }
}
}

YUVABackendTextureInfo::YUVABackendTextureInfo(const SkYUVAInfo& yuvaInfo,
                                               const TextureInfo textureInfos[kMaxPlanes],
                                               const SkColorType colorTypes[kMaxPlanes],
                                               Mipmapped mipmapped)
        : fYUVAInfo(yuvaInfo)
        , fMipmapped(mipmapped) {
    if (!yuvaInfo.isValid()) {
        *this = {};
        SkASSERT(!this->isValid());
        return;
    }
    int n = yuvaInfo.numPlanes();
    for (size_t i = 0; i < static_cast<size_t>(n); ++i) {
        if (!textureInfos[i].isValid() || textureInfos[i].backend() != textureInfos[0].backend()) {
            *this = {};
            SkASSERT(!this->isValid());
            return;
        }
        int numRequiredChannels = yuvaInfo.numChannelsInPlane(i);
        SkASSERT(numRequiredChannels > 0);
        int numActualChannels = num_channels(colorTypes[i]);
        if (numActualChannels < numRequiredChannels) {
            *this = {};
            SkASSERT(!this->isValid());
            return;
        }
        fPlaneTextureInfos[i] = textureInfos[i];
    }
    SkASSERT(this->isValid());
}

bool YUVABackendTextureInfo::operator==(const YUVABackendTextureInfo& that) const {
    if (fYUVAInfo != that.fYUVAInfo || fMipmapped != that.fMipmapped) {
        return false;
    }
    return fPlaneTextureInfos == that.fPlaneTextureInfos;
}

SkYUVAInfo::YUVALocations YUVABackendTextureInfo::toYUVALocations() const {
    uint32_t channelFlags[] = {
        SkColorTypeChannelFlags(fPlaneColorTypes[0]),
        SkColorTypeChannelFlags(fPlaneColorTypes[1]),
        SkColorTypeChannelFlags(fPlaneColorTypes[2]),
        SkColorTypeChannelFlags(fPlaneColorTypes[3])
    };
    auto result = fYUVAInfo.toYUVALocations(channelFlags);
    SkDEBUGCODE(int numPlanes;)
    SkASSERT(SkYUVAInfo::YUVALocation::AreValidLocations(result, &numPlanes));
    SkASSERT(numPlanes == this->numPlanes());
    return result;
}

//////////////////////////////////////////////////////////////////////////////

YUVABackendTextures::YUVABackendTextures(const SkYUVAInfo& yuvaInfo,
                                         const BackendTexture textures[kMaxPlanes],
                                         const SkColorType colorTypes[kMaxPlanes])
        : fYUVAInfo(yuvaInfo) {
    if (!fYUVAInfo.isValid()) {
        return;
    }
    SkISize planeDimensions[SkYUVAInfo::kMaxPlanes];
    int numPlanes = yuvaInfo.planeDimensions(planeDimensions);
    for (int i = 0; i < numPlanes; ++i) {
        int numRequiredChannels = fYUVAInfo.numChannelsInPlane(i);
        if (!textures[i].isValid() ||
            textures[i].dimensions() != planeDimensions[i] ||
            textures[i].backend() != textures[0].backend() ||
            num_channels(colorTypes[i]) < numRequiredChannels) {
            *this = {};
            return;
        }
        fTextures[i] = textures[i];
    }
}

SkYUVAInfo::YUVALocations YUVABackendTextures::toYUVALocations() const {
    uint32_t channelFlags[] = {
        SkColorTypeChannelFlags(fColorTypes[0]),
        SkColorTypeChannelFlags(fColorTypes[1]),
        SkColorTypeChannelFlags(fColorTypes[2]),
        SkColorTypeChannelFlags(fColorTypes[3])
    };
    auto result = fYUVAInfo.toYUVALocations(channelFlags);
    SkDEBUGCODE(int numPlanes;)
    SkASSERT(SkYUVAInfo::YUVALocation::AreValidLocations(result, &numPlanes));
    SkASSERT(numPlanes == this->numPlanes());
    return result;
}

}  // End of namespace skgpu::graphite
