/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkImage_CrossContext_DEFINED
#define SkImage_CrossContext_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/core/SkTypes.h"
#include "include/core/SkYUVAPixmaps.h"
#include "src/image/SkImage_Base.h"
#include "src/image/SkImage_Lazy.h"

#include <memory>

class GrDirectContext;
class SkImage;
class SkImageGenerator;
struct SkIRect;
class GrCaps;
class GrDirectContext;
class GrFragmentProcessor;
class GrRecordingContext;
class GrSurfaceProxyView;

class SkImage_LazyTexture final : public SkImage_Lazy {
public:
    SkImage_LazyTexture(SkImage_Lazy::Validator* validator): SkImage_Lazy(validator) {}

    SkImage_Base::Type type() const override { return SkImage_Base::Type::kLazyTexture; }

    sk_sp<SkImage> onMakeSubset(const SkIRect&, GrDirectContext*) const override;

private:
    std::tuple<GrSurfaceProxyView, GrColorType> onAsView(GrRecordingContext*,
                                                         skgpu::Mipmapped,
                                                         GrImageTexGenPolicy) const override;
    std::unique_ptr<GrFragmentProcessor> onAsFragmentProcessor(GrRecordingContext*,
                                                               SkSamplingOptions,
                                                               const SkTileMode[2],
                                                               const SkMatrix&,
                                                               const SkRect*,
                                                               const SkRect*) const override;

    GrSurfaceProxyView textureProxyViewFromPlanes(GrRecordingContext*, skgpu::Budgeted) const;
    sk_sp<SkCachedData> getPlanes(const SkYUVAPixmapInfo::SupportedDataTypes& supportedDataTypes,
                                  SkYUVAPixmaps* pixmaps) const;
    // Returns the texture proxy. CachingHint refers to whether the generator's output should be
    // cached in CPU memory. We will always cache the generated texture on success.
    GrSurfaceProxyView lockTextureProxyView(GrRecordingContext*,
                                            GrImageTexGenPolicy,
                                            skgpu::Mipmapped) const;

    // Returns the GrColorType to use with the GrTextureProxy returned from lockTextureProxy. This
    // may be different from the color type on the image in the case where we need up upload CPU
    // data to a texture but the GPU doesn't support the format of CPU data. In this case we convert
    // the data to RGBA_8888 unorm on the CPU then upload that.
    GrColorType colorTypeOfLockTextureProxy(const GrCaps* caps) const;

    bool readPixelsProxy(GrDirectContext*, const SkPixmap&) const override;

    void addUniqueIDListener(sk_sp<SkIDChangeListener>) const;

    // When the SkImage_Lazy goes away, we will iterate over all the listeners to inform them
    // of the unique ID's demise. This is used to remove cached textures from GrContext.
    mutable SkIDChangeListener::List fUniqueIDListeners;
};

sk_sp<SkImage> MakeCrossContextImage(std::unique_ptr<SkImageGenerator> generator);
#endif
