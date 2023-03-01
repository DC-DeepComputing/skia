/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_YUVABackendTextures_DEFINED
#define skgpu_graphite_YUVABackendTextures_DEFINED

#include "include/core/SkYUVAInfo.h"
#include "include/gpu/graphite/BackendTexture.h"

#include <tuple>

namespace skgpu::graphite {

/**
 * A description of a set of BackendTextures that hold the planar data described by a SkYUVAInfo.
 */
class SK_API YUVABackendTextureInfo {
public:
    static constexpr auto kMaxPlanes = SkYUVAInfo::kMaxPlanes;

    /** Default YUVABackendTextureInfo is invalid. */
    YUVABackendTextureInfo() = default;

    /**
     * Initializes a YUVABackendTextureInfo to describe a set of textures that can store the
     * planes indicated by the SkYUVAInfo. The texture dimensions are taken from the SkYUVAInfo's
     * plane dimensions. All the described textures share a common origin. The planar image this
     * describes will be mip mapped if all the textures are individually mip mapped as indicated
     * by GrMipmapped. This will produce an invalid result (return false from isValid()) if the
     * passed formats' channels don't agree with SkYUVAInfo.
     */
    YUVABackendTextureInfo(const SkYUVAInfo&,
                           const TextureInfo[kMaxPlanes],
                           Mipmapped);

    YUVABackendTextureInfo(const YUVABackendTextureInfo&) = default;

    YUVABackendTextureInfo& operator=(const YUVABackendTextureInfo&) = default;

    bool operator==(const YUVABackendTextureInfo&) const;
    bool operator!=(const YUVABackendTextureInfo& that) const { return !(*this == that); }

    const SkYUVAInfo& yuvaInfo() const { return fYUVAInfo; }

    SkYUVColorSpace yuvColorSpace() const { return fYUVAInfo.yuvColorSpace(); }

    Mipmapped mipmapped() const { return fMipmapped; }

    /** The number of SkPixmap planes, 0 if this YUVABackendTextureInfo is invalid. */
    int numPlanes() const { return fYUVAInfo.numPlanes(); }

    /** TextureInfo for the ith plane, or invalid if i >= numPlanes() */
    const TextureInfo& planeTextureInfo(int i) const { return fPlaneTextureInfo[i]; }

    /**
     * Returns true if this has been configured with a valid SkYUVAInfo with compatible texture
     * formats.
     */
    bool isValid() const { return fYUVAInfo.isValid(); }

    /**
     * Computes a YUVALocations representation of the planar layout. The result is guaranteed to be
     * valid if this->isValid().
     */
    SkYUVAInfo::YUVALocations toYUVALocations() const;

private:
    SkYUVAInfo fYUVAInfo;
    TextureInfo fPlaneTextureInfo[kMaxPlanes];
    Mipmapped fMipmapped = Mipmapped::kNo;
};

/**
 * A set of GrBackendTextures that hold the planar data for an image described a SkYUVAInfo.
 */
class SK_API YUVABackendTextures {
public:
    YUVABackendTextures() = default;
    YUVABackendTextures(const YUVABackendTextures&) = delete;
    YUVABackendTextures(YUVABackendTextures&&) = default;

    YUVABackendTextures& operator=(const YUVABackendTextures&) = delete;
    YUVABackendTextures& operator=(YUVABackendTextures&&) = default;

    YUVABackendTextures(const SkYUVAInfo&,
                        const BackendTexture[SkYUVAInfo::kMaxPlanes]);

    const std::array<BackendTexture, SkYUVAInfo::kMaxPlanes>& textures() const {
        return fTextures;
    }

    BackendTexture texture(int i) const {
        SkASSERT(i >= 0 && i < SkYUVAInfo::kMaxPlanes);
        return fTextures[static_cast<size_t>(i)];
    }

    const SkYUVAInfo& yuvaInfo() const { return fYUVAInfo; }

    int numPlanes() const { return fYUVAInfo.numPlanes(); }

    bool isValid() const { return fYUVAInfo.isValid(); }

    /**
     * Computes a YUVALocations representation of the planar layout. The result is guaranteed to be
     * valid if this->isValid().
     */
    SkYUVAInfo::YUVALocations toYUVALocations() const;

private:
    SkYUVAInfo fYUVAInfo;
    std::array<BackendTexture, SkYUVAInfo::kMaxPlanes> fTextures;
};

}  // End of namespace skgpu::graphite

#endif  // skgpu_graphite_YUVABackendTextures_DEFINED
