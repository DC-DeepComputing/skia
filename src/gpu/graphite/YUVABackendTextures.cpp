/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/gpu/graphite/YUVABackendTextures.h"

#include "include/gpu/graphite/Recorder.h"
#include "src/core/SkYUVAInfoLocation.h"
#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/RecorderPriv.h"

namespace skgpu::graphite {

namespace {
int num_channels(uint32_t ChannelMasks) {
    switch (ChannelMasks) {
        case kRed_SkColorChannelFlag        : return 1;
        case kAlpha_SkColorChannelFlag      : return 1;
        case kGray_SkColorChannelFlag       : return 1;
        case kGrayAlpha_SkColorChannelFlags : return 2;
        case kRG_SkColorChannelFlags        : return 2;
        case kRGB_SkColorChannelFlags       : return 3;
        case kRGBA_SkColorChannelFlags      : return 4;
        default                             : return 0;
    }
    SkUNREACHABLE;
}
}

YUVABackendTextureInfo::YUVABackendTextureInfo(const Recorder* recorder,
                                               const SkYUVAInfo& yuvaInfo,
                                               const TextureInfo textureInfo[kMaxPlanes],
                                               Mipmapped mipmapped)
        : fYUVAInfo(yuvaInfo)
        , fMipmapped(mipmapped) {
    if (!yuvaInfo.isValid()) {
        *this = {};
        SkASSERT(!this->isValid());
        return;
    }
    int n = yuvaInfo.numPlanes();
    for (int i = 0; i < n; ++i) {
        if (!textureInfo[i].isValid() || textureInfo[i].backend() != textureInfo[0].backend()) {
            *this = {};
            SkASSERT(!this->isValid());
            return;
        }
        int numRequiredChannels = yuvaInfo.numChannelsInPlane(i);
        SkASSERT(numRequiredChannels > 0);
        fPlaneChannelMasks[i] = recorder->priv().caps()->channelMask(textureInfo[i]);
        int numActualChannels = num_channels(fPlaneChannelMasks[i]);
        if (numActualChannels < numRequiredChannels) {
            *this = {};
            SkASSERT(!this->isValid());
            return;
        }
        fPlaneTextureInfos[i] = textureInfo[i];
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
    auto result = fYUVAInfo.toYUVALocations(fPlaneChannelMasks.data());
    SkDEBUGCODE(int numPlanes;)
    SkASSERT(SkYUVAInfo::YUVALocation::AreValidLocations(result, &numPlanes));
    SkASSERT(numPlanes == this->numPlanes());
    return result;
}

//////////////////////////////////////////////////////////////////////////////

YUVABackendTextures::YUVABackendTextures(const Recorder* recorder,
                                         const SkYUVAInfo& yuvaInfo,
                                         const BackendTexture textures[SkYUVAInfo::kMaxPlanes])
        : fYUVAInfo(yuvaInfo) {
    if (!fYUVAInfo.isValid()) {
        return;
    }
    SkISize planeDimensions[SkYUVAInfo::kMaxPlanes];
    int numPlanes = yuvaInfo.planeDimensions(planeDimensions);
    for (int i = 0; i < numPlanes; ++i) {
        int numRequiredChannels = fYUVAInfo.numChannelsInPlane(i);
        fPlaneChannelMasks[i] = recorder->priv().caps()->channelMask(textures[i].info());
        if (!textures[i].isValid() ||
            textures[i].dimensions() != planeDimensions[i] ||
            textures[i].backend() != textures[0].backend() ||
            num_channels(fPlaneChannelMasks[i]) < numRequiredChannels) {
            *this = {};
            return;
        }
        fPlaneTextures[i] = textures[i];
    }
}

SkYUVAInfo::YUVALocations YUVABackendTextures::toYUVALocations() const {
    auto result = fYUVAInfo.toYUVALocations(fPlaneChannelMasks.data());
    SkDEBUGCODE(int numPlanes;)
    SkASSERT(SkYUVAInfo::YUVALocation::AreValidLocations(result, &numPlanes));
    SkASSERT(numPlanes == this->numPlanes());
    return result;
}

}  // End of namespace skgpu::graphite
