/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/image/SkImage_Lazy.h"

#include "include/core/SkAlphaType.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkData.h"
#include "include/core/SkImageGenerator.h"
#include "include/core/SkPixmap.h"
#include "include/core/SkRect.h"
#include "include/core/SkSize.h"
#include "include/core/SkYUVAInfo.h"
#include "src/core/SkBitmapCache.h"
#include "src/core/SkCachedData.h"
#include "src/core/SkNextID.h"
#include "src/core/SkResourceCache.h"
#include "src/core/SkYUVPlanesCache.h"

#if defined(SK_GRAPHITE)
#include "src/gpu/graphite/TextureUtils.h"
#endif

#include <utility>

class SkMatrix;
enum SkColorType : int;
enum class SkTileMode;

sk_sp<SharedGenerator> SharedGenerator::Make(std::unique_ptr<SkImageGenerator> gen) {
    return gen ? sk_sp<SharedGenerator>(new SharedGenerator(std::move(gen))) : nullptr;
}

SharedGenerator::SharedGenerator(std::unique_ptr<SkImageGenerator> gen)
        : fGenerator(std::move(gen)) {
    SkASSERT(fGenerator);
}

// This is thread safe.  It is a const field set in the constructor.
const SkImageInfo& SharedGenerator::getInfo() { return fGenerator->getInfo(); }

///////////////////////////////////////////////////////////////////////////////

SkImage_Lazy::Validator::Validator(sk_sp<SharedGenerator> gen, const SkColorType* colorType,
                                   sk_sp<SkColorSpace> colorSpace)
        : fSharedGenerator(std::move(gen)) {
    if (!fSharedGenerator) {
        return;
    }

    // The following generator accessors are safe without acquiring the mutex (const getters).
    // TODO: refactor to use a ScopedGenerator instead, for clarity.
    fInfo = fSharedGenerator->fGenerator->getInfo();
    if (fInfo.isEmpty()) {
        fSharedGenerator.reset();
        return;
    }

    fUniqueID = fSharedGenerator->fGenerator->uniqueID();

    if (colorType && (*colorType == fInfo.colorType())) {
        colorType = nullptr;
    }

    if (colorType || colorSpace) {
        if (colorType) {
            fInfo = fInfo.makeColorType(*colorType);
        }
        if (colorSpace) {
            fInfo = fInfo.makeColorSpace(colorSpace);
        }
        fUniqueID = SkNextID::ImageID();
    }
}

SkImage_Lazy::SkImage_Lazy(Validator* validator)
    : SkImage_Base(validator->fInfo, validator->fUniqueID)
    , fSharedGenerator(std::move(validator->fSharedGenerator))
{
    SkASSERT(fSharedGenerator);
}

bool SkImage_Lazy::getROPixels(GrDirectContext* ctx, SkBitmap* bitmap,
                               SkImage::CachingHint chint) const {
    auto check_output_bitmap = [bitmap]() {
        SkASSERT(bitmap->isImmutable());
        SkASSERT(bitmap->getPixels());
        (void)bitmap;
    };

    auto desc = SkBitmapCacheDesc::Make(this);
    if (SkBitmapCache::Find(desc, bitmap)) {
        check_output_bitmap();
        return true;
    }

    if (SkImage::kAllow_CachingHint == chint) {
        SkPixmap pmap;
        SkBitmapCache::RecPtr cacheRec = SkBitmapCache::Alloc(desc, this->imageInfo(), &pmap);
        if (!cacheRec) {
            return false;
        }
        bool success = false;
        {   // make sure ScopedGenerator goes out of scope before we try readPixelsProxy
            success = ScopedGenerator(fSharedGenerator)->getPixels(pmap);
        }
        if (!success && !this->readPixelsProxy(ctx, pmap)) {
            return false;
        }
        SkBitmapCache::Add(std::move(cacheRec), bitmap);
        this->notifyAddedToRasterCache();
    } else {
        if (!bitmap->tryAllocPixels(this->imageInfo())) {
            return false;
        }
        bool success = false;
        {   // make sure ScopedGenerator goes out of scope before we try readPixelsProxy
            success = ScopedGenerator(fSharedGenerator)->getPixels(bitmap->pixmap());
        }
        if (!success && !this->readPixelsProxy(ctx, bitmap->pixmap())) {
            return false;
        }
        bitmap->setImmutable();
    }
    check_output_bitmap();
    return true;
}

bool SkImage_Lazy::readPixelsProxy(GrDirectContext*, const SkPixmap&) const {
    return false;
}

bool SkImage_Lazy::onReadPixels(GrDirectContext* dContext,
                                const SkImageInfo& dstInfo,
                                void* dstPixels,
                                size_t dstRB,
                                int srcX,
                                int srcY,
                                CachingHint chint) const {
    SkBitmap bm;
    if (this->getROPixels(dContext, &bm, chint)) {
        return bm.readPixels(dstInfo, dstPixels, dstRB, srcX, srcY);
    }
    return false;
}

sk_sp<SkData> SkImage_Lazy::onRefEncoded() const {
    // check that we aren't a subset or colortype/etc modification of the original
    if (fSharedGenerator->fGenerator->uniqueID() == this->uniqueID()) {
        ScopedGenerator generator(fSharedGenerator);
        return generator->refEncodedData();
    }
    return nullptr;
}

bool SkImage_Lazy::isValid(GrRecordingContext* context) const {
    ScopedGenerator generator(fSharedGenerator);
    return generator->isValid(context);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

sk_sp<SkImage> SkImage_Lazy::onMakeSubset(const SkIRect& subset, GrDirectContext*) const {
    auto pixels = this->makeRasterImage();
    return pixels ? pixels->makeSubset(subset, nullptr) : nullptr;
}

#if defined(SK_GRAPHITE)

sk_sp<SkImage> SkImage_Lazy::onMakeSubset(const SkIRect& subset,
                                          skgpu::graphite::Recorder* recorder,
                                          RequiredImageProperties requiredProperties) const {
    // TODO: can we do this more efficiently, by telling the generator we want to
    //       "realize" a subset?

    sk_sp<SkImage> nonLazyImg = recorder ? this->makeTextureImage(recorder, requiredProperties)
                                         : this->makeRasterImage();

    return nonLazyImg ? nonLazyImg->makeSubset(subset, recorder, requiredProperties) : nullptr;
}

#endif // SK_GRAPHITE

sk_sp<SkImage> SkImage_Lazy::onMakeColorTypeAndColorSpace(SkColorType targetCT,
                                                          sk_sp<SkColorSpace> targetCS,
                                                          GrDirectContext*) const {
    SkAutoMutexExclusive autoAquire(fOnMakeColorTypeAndSpaceMutex);
    if (fOnMakeColorTypeAndSpaceResult &&
        targetCT == fOnMakeColorTypeAndSpaceResult->colorType() &&
        SkColorSpace::Equals(targetCS.get(), fOnMakeColorTypeAndSpaceResult->colorSpace())) {
        return fOnMakeColorTypeAndSpaceResult;
    }
    Validator validator(fSharedGenerator, &targetCT, targetCS);
    sk_sp<SkImage> result = validator ? sk_sp<SkImage>(new SkImage_Lazy(&validator)) : nullptr;
    if (result) {
        fOnMakeColorTypeAndSpaceResult = result;
    }
    return result;
}

sk_sp<SkImage> SkImage_Lazy::onReinterpretColorSpace(sk_sp<SkColorSpace> newCS) const {
    // TODO: The correct thing is to clone the generator, and modify its color space. That's hard,
    // because we don't have a clone method, and generator is public (and derived-from by clients).
    // So do the simple/inefficient thing here, and fallback to raster when this is called.

    // We allocate the bitmap with the new color space, then generate the image using the original.
    SkBitmap bitmap;
    if (bitmap.tryAllocPixels(this->imageInfo().makeColorSpace(std::move(newCS)))) {
        SkPixmap pixmap = bitmap.pixmap();
        pixmap.setColorSpace(this->refColorSpace());
        if (ScopedGenerator(fSharedGenerator)->getPixels(pixmap)) {
            bitmap.setImmutable();
            return bitmap.asImage();
        }
    }
    return nullptr;
}


#if defined(SK_GRAPHITE)

/*
 *  We only have 2 ways to create a Graphite-backed image.
 *
 *  1. Ask the generator to natively create one
 *  2. Ask the generator to return RGB(A) data, which the GPU can convert
 */
sk_sp<SkImage> SkImage_Lazy::onMakeTextureImage(skgpu::graphite::Recorder* recorder,
                                                RequiredImageProperties requiredProps) const {
    using namespace skgpu::graphite;

    // 1. Ask the generator to natively create one.
    {
        // Disable mipmaps here bc Graphite doesn't currently support mipmap regeneration
        // In this case, we would allocate the mipmaps and fill in the base layer but the mipmap
        // levels would never be filled out - yielding incorrect draws. Please see: b/238754357.
        requiredProps.fMipmapped = skgpu::Mipmapped::kNo;

        ScopedGenerator generator(fSharedGenerator);
        sk_sp<SkImage> newImage = generator->makeTextureImage(recorder,
                                                              this->imageInfo(),
                                                              requiredProps.fMipmapped);
        if (newImage) {
            SkASSERT(as_IB(newImage)->isGraphiteBacked());
            return newImage;
        }
    }

    // 2. Ask the generator to return a bitmap, which the GPU can convert.
    if (SkBitmap bitmap; this->getROPixels(nullptr, &bitmap, CachingHint::kDisallow_CachingHint)) {
        return skgpu::graphite::MakeFromBitmap(recorder,
                                               this->imageInfo().colorInfo(),
                                               bitmap,
                                               nullptr,
                                               skgpu::Budgeted::kNo,
                                               requiredProps);
    }

    return nullptr;
}

sk_sp<SkImage> SkImage_Lazy::onMakeColorTypeAndColorSpace(
        SkColorType targetCT,
        sk_sp<SkColorSpace> targetCS,
        skgpu::graphite::Recorder* recorder,
        RequiredImageProperties requiredProps) const {
    SkAutoMutexExclusive autoAquire(fOnMakeColorTypeAndSpaceMutex);
    if (fOnMakeColorTypeAndSpaceResult &&
        targetCT == fOnMakeColorTypeAndSpaceResult->colorType() &&
        SkColorSpace::Equals(targetCS.get(), fOnMakeColorTypeAndSpaceResult->colorSpace())) {
        return fOnMakeColorTypeAndSpaceResult;
    }
    Validator validator(fSharedGenerator, &targetCT, targetCS);
    sk_sp<SkImage> result = validator ? sk_sp<SkImage>(new SkImage_Lazy(&validator)) : nullptr;
    if (result) {
        fOnMakeColorTypeAndSpaceResult = result;
    }

    if (recorder) {
        return result->makeTextureImage(recorder, requiredProps);
    } else {
        return result;
    }
}

#endif

// TODO(kjlubick) move SharedGenerate to SkImage_Lazy.h and this to SkImage_LazyFactories
namespace SkImages {

sk_sp<SkImage> DeferredFromGenerator(std::unique_ptr<SkImageGenerator> generator) {
    SkImage_Lazy::Validator validator(
            SharedGenerator::Make(std::move(generator)), nullptr, nullptr);

    return validator ? sk_make_sp<SkImage_Lazy>(&validator) : nullptr;
}

}  // namespace SkImages
