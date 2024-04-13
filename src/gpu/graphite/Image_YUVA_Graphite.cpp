/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/Image_YUVA_Graphite.h"

#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkImage.h"
#include "include/core/SkSurface.h"
#include "include/gpu/GpuTypes.h"
#include "include/gpu/graphite/Image.h"
#include "include/gpu/graphite/Recorder.h"
#include "include/gpu/graphite/Surface.h"
#include "src/core/SkYUVAInfoLocation.h"
#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/Image_Graphite.h"
#include "src/gpu/graphite/Log.h"
#include "src/gpu/graphite/RecorderPriv.h"
#include "src/gpu/graphite/ResourceProvider.h"
#include "src/gpu/graphite/Texture.h"
#include "src/gpu/graphite/TextureProxy.h"
#include "src/gpu/graphite/TextureProxyView.h"
#include "src/gpu/graphite/TextureUtils.h"
#include "src/shaders/SkImageShader.h"


namespace skgpu::graphite {

namespace {

constexpr auto kAssumedColorType = kRGBA_8888_SkColorType;

static constexpr int kY = static_cast<int>(SkYUVAInfo::kY);
static constexpr int kU = static_cast<int>(SkYUVAInfo::kU);
static constexpr int kV = static_cast<int>(SkYUVAInfo::kV);
static constexpr int kA = static_cast<int>(SkYUVAInfo::kA);

static SkAlphaType yuva_alpha_type(const SkYUVAInfo& yuvaInfo) {
    // If an alpha channel is present we always use kPremul. This is because, although the planar
    // data is always un-premul, the final interleaved RGBA sample produced in the shader is premul
    // (and similar if flattened).
    return yuvaInfo.hasAlpha() ? kPremul_SkAlphaType : kOpaque_SkAlphaType;
}

} // anonymous

Image_YUVA::Image_YUVA(const YUVAProxies& proxies,
                       const SkYUVAInfo& yuvaInfo,
                       sk_sp<SkColorSpace> imageColorSpace,
                       std::tuple<int, int> uvSubsampleFactors)
        : Image_Base(SkImageInfo::Make(yuvaInfo.dimensions(),
                                       kAssumedColorType,
                                       yuva_alpha_type(yuvaInfo),
                                       std::move(imageColorSpace)),
                     kNeedNewImageUniqueID)
        , fProxies(std::move(proxies))
        , fYUVAInfo(yuvaInfo)
        , fUVSubsampleFactors(uvSubsampleFactors) {
    // The caller should have checked this, just verifying.
    SkASSERT(fYUVAInfo.isValid());
    for (int i = 0; i < SkYUVAInfo::kYUVAChannelCount; ++i) {
        if (!fProxies[i]) {
            SkASSERT(i == kA);
            continue;
        }
        if (fProxies[i].proxy()->mipmapped() == Mipmapped::kNo) {
            fMipmapped = Mipmapped::kNo;
        }
        if (fProxies[i].proxy()->isProtected()) {
            fProtected = Protected::kYes;
        }
    }
}

Image_YUVA::~Image_YUVA() = default;

sk_sp<Image_YUVA> Image_YUVA::Wrap(const Caps* caps,
                                   const SkYUVAInfo& yuvaInfo,
                                   sk_sp<SkColorSpace> imageColorSpace,
                                   PlaneProxyFactoryFn planeFn) {
    if (!yuvaInfo.isValid()) {
        return nullptr;
    }
    SkImageInfo info = SkImageInfo::Make(
            yuvaInfo.dimensions(), kAssumedColorType, yuva_alpha_type(yuvaInfo), imageColorSpace);
    if (!SkImageInfoIsValid(info)) {
        return nullptr;
    }

    // Invoke the PlaneProxyFactoryFn for each plane
    const int numPlanes = yuvaInfo.numPlanes();
    TextureProxyView planeProxies[SkYUVAInfo::kMaxPlanes];
    uint32_t pixmapChannelmasks[SkYUVAInfo::kMaxPlanes];
    for (int i = 0; i < numPlanes; ++i) {
        TextureProxyView planeView = planeFn(i);
        if (!planeView) {
            return nullptr;
        }
        pixmapChannelmasks[i] = caps->channelMask(planeView.proxy()->textureInfo());
        planeProxies[i] = planeView;
    }

    // Re-arrange the proxies from planes to channels
    SkYUVAInfo::YUVALocations locations = yuvaInfo.toYUVALocations(pixmapChannelmasks);
    int expectedPlanes;
    if (!SkYUVAInfo::YUVALocation::AreValidLocations(locations, &expectedPlanes) ||
        expectedPlanes != numPlanes) {
        return nullptr;
    }
    // Y channel should match the YUVAInfo dimensions
    if (planeProxies[locations[kY].fPlane].dimensions() != yuvaInfo.dimensions()) {
        return nullptr;
    }
    // UV channels should have planes with the same dimensions and subsampling factor.
    if (planeProxies[locations[kU].fPlane].dimensions() !=
                planeProxies[locations[kV].fPlane].dimensions()) {
        return nullptr;
    }
    // If A channel is present, it should match the Y channel
    if (locations[kA].fPlane >= 0 &&
        planeProxies[locations[kA].fPlane].dimensions() != yuvaInfo.dimensions()) {
        return nullptr;
    }

    std::tuple<int, int> uSubsampleFactors = yuvaInfo.planeSubsamplingFactors(locations[kU].fPlane);
    if (yuvaInfo.planeSubsamplingFactors(locations[kV].fPlane) != uSubsampleFactors) {
        return nullptr;
    }

    // Re-arrange into YUVA channel order and apply the location to the swizzle
    YUVAProxies channelProxies;
    for (int i = 0; i < SkYUVAInfo::kYUVAChannelCount; ++i) {
        auto [plane, channel] = locations[i];
        if (plane >= 0) {
            // Adjust the plane proxy's view swizzle to select the specific channel for the current
            // yuva location.
            Swizzle channelSwizzle{planeProxies[plane].swizzle()[(int) channel]};
            channelProxies[i] = planeProxies[plane].replaceSwizzle(channelSwizzle);
        } else if (i == kA) {
            // The alpha channel is allowed to be not provided, set it to an empty view
            channelProxies[i] = {};
        } else {
            SKGPU_LOG_W("YUVA channel %d does not have a valid location", i);
            return nullptr;
        }
    }

    return sk_sp<Image_YUVA>(new Image_YUVA(std::move(channelProxies),
                                            yuvaInfo,
                                            std::move(imageColorSpace),
                                            uSubsampleFactors));
}

sk_sp<Image_YUVA> Image_YUVA::WrapImages(const Caps* caps,
                                         const SkYUVAInfo& yuvaInfo,
                                         SkSpan<const sk_sp<SkImage>> images,
                                         sk_sp<SkColorSpace> imageColorSpace) {
    if (SkTo<int>(images.size()) < yuvaInfo.numPlanes()) {
        return nullptr;
    }
    sk_sp<Image_YUVA> image = Wrap(caps, yuvaInfo, std::move(imageColorSpace),
            [images](int plane) -> TextureProxyView {
                TextureProxyView planeView = AsView(images[plane]);
                if (!planeView) {
                    // A null image, or not graphite-backed, or not backed by a single texture.
                    return {};
                }
                // The YUVA shader expects to sample from the red channel for single-channel
                // textures, so reset the swizzle for alpha-only textures to compensate for that
                if (images[plane]->isAlphaOnly()) {
                    planeView = planeView.makeSwizzle(Swizzle("aaaa"));
                }
                return planeView;
            });
    if (image) {
        // Unlike the other factories, this YUVA image shares the texture proxies with each plane
        // Image, so if those are linked to Devices, it must inherit those same links.
        for (int plane = 0; plane < yuvaInfo.numPlanes(); ++plane) {
            SkASSERT(as_IB(images[plane])->isGraphiteBacked());
            image->linkDevices(static_cast<Image_Base*>(images[plane].get()));
        }
    }
    return image;
}

size_t Image_YUVA::textureSize() const {
    size_t size = 0;
    for (int i = 0; i < SkYUVAInfo::kMaxPlanes; ++i) {
        if (fProxies[i] && fProxies[i].proxy()->texture()) {
            size += fProxies[i].proxy()->texture()->gpuMemorySize();
        }
    }
    return size;
}

sk_sp<SkImage> Image_YUVA::onReinterpretColorSpace(sk_sp<SkColorSpace> newCS) const {
    sk_sp<Image_YUVA> view{new Image_YUVA(fProxies,
                                          fYUVAInfo,
                                          std::move(newCS),
                                          fUVSubsampleFactors)};
    // The new Image object shares the same texture planes, so it should also share linked Devices
    view->linkDevices(this);
    return view;
}

}  // namespace skgpu::graphite
