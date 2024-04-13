/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_Image_YUVA_Graphite_DEFINED
#define skgpu_graphite_Image_YUVA_Graphite_DEFINED

#include "include/core/SkYUVAInfo.h"
#include "src/gpu/graphite/Image_Base_Graphite.h"
#include "src/gpu/graphite/TextureProxyView.h"

#include <functional>

namespace skgpu::graphite {

class Recorder;

class Image_YUVA final : public Image_Base {
public:
    ~Image_YUVA() override;

    using PlaneProxyFactoryFn = std::function<TextureProxyView(int)>;
    // The caller should expect `planeFn` to be called for 0 to `yuvaInfo.numPlanes()-1`, so it
    // must externally validate that the proxy source has enough data, or returns a null proxy
    // for planes it can't fulfill.
    static sk_sp<Image_YUVA> Wrap(const Caps* caps,
                                  const SkYUVAInfo& yuvaInfo,
                                  sk_sp<SkColorSpace> imageColorSpace,
                                  PlaneProxyFactoryFn planeFn);

    // Wraps the Graphite-backed Image planes into a YUV[A] image. The returned image shares
    // textures as well as any links to Devices that might modify those textures.
    static sk_sp<Image_YUVA> WrapImages(const Caps* caps,
                                        const SkYUVAInfo& yuvaInfo,
                                        SkSpan<const sk_sp<SkImage>> images,
                                        sk_sp<SkColorSpace> imageColorSpace);

    SkImage_Base::Type type() const override { return SkImage_Base::Type::kGraphiteYUVA; }

    size_t textureSize() const override;

    bool onHasMipmaps() const override { return fMipmapped == Mipmapped::kYes; }

    bool onIsProtected() const override { return fProtected == Protected::kYes; }

    sk_sp<SkImage> onReinterpretColorSpace(sk_sp<SkColorSpace>) const override;

    // Returns the proxy view that provides value for the YUVA channel specified by 'channelIndex'.
    // The view of the returned proxy applies a swizzle to map the relevant data channel into all
    // slots of the sample value. The alpha proxy may be null.
    const TextureProxyView& proxyView(int channelIndex) const {
        SkASSERT(channelIndex >= 0 && channelIndex < SkYUVAInfo::kYUVAChannelCount);
        return fProxies[channelIndex];
    }

    std::tuple<int, int> uvSubsampleFactors() const { return fUVSubsampleFactors; }

    const SkYUVAInfo& yuvaInfo() const { return fYUVAInfo; }

private:
    // The proxy views are ordered Y,U,V,A and if the channels are held in the same plane, the
    // respective proxy views will share the underlying TextureProxy but have the appropriate
    // swizzle to access the appropriate channel and return it in the R slot.
    using YUVAProxies = std::array<TextureProxyView, SkYUVAInfo::kYUVAChannelCount>;

    Image_YUVA(const YUVAProxies&,
               const SkYUVAInfo&,
               sk_sp<SkColorSpace>,
               std::tuple<int, int> uvSubsampleFactors);

    YUVAProxies fProxies;
    SkYUVAInfo fYUVAInfo;
    std::tuple<int, int> fUVSubsampleFactors;

    // Aggregate mipmap/protected status from the proxies
    Mipmapped fMipmapped = Mipmapped::kYes;
    Protected fProtected = Protected::kNo;
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_Image_YUVA_Graphite_DEFINED
