/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkImage_Lazy_DEFINED
#define SkImage_Lazy_DEFINED

#include "include/core/SkColorSpace.h"
#include "include/core/SkImage.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSamplingOptions.h"
#include "include/core/SkTypes.h"
#include "include/private/SkIDChangeListener.h"
#include "include/private/base/SkMutex.h"
#include "src/image/SkImage_Base.h"

#include <cstddef>
#include <cstdint>
#include <memory>
#include <tuple>

class SharedGenerator;
class SkBitmap;
class SkCachedData;
class SkData;
class SkMatrix;
class SkPixmap;
enum SkColorType : int;
enum class GrColorType;
enum class GrImageTexGenPolicy : int;
enum class SkTileMode;
struct SkIRect;
struct SkRect;

namespace skgpu {
enum class Budgeted : bool;
enum class Mipmapped : bool;
}

class SkImage_Lazy : public SkImage_Base {
public:
    struct Validator {
        Validator(sk_sp<SharedGenerator>, const SkColorType*, sk_sp<SkColorSpace>);

        explicit operator bool() const { return fSharedGenerator.get(); }

        sk_sp<SharedGenerator> fSharedGenerator;
        SkImageInfo            fInfo;
        sk_sp<SkColorSpace>    fColorSpace;
        uint32_t               fUniqueID;
    };

    SkImage_Lazy(Validator* validator);

    // From SkImage.h
    bool isValid(GrRecordingContext*) const override;

    // From SkImage_Base.h
    bool onHasMipmaps() const override {
        // TODO: Should we defer to the generator? The generator interface currently doesn't have
        // a way to provide content for levels other than via SkImageGenerator::generateTexture().
        return false;
    }
    bool onReadPixels(GrDirectContext*, const SkImageInfo&, void*, size_t, int srcX, int srcY,
                      CachingHint) const override;
    sk_sp<SkData> onRefEncoded() const override;
    sk_sp<SkImage> onMakeSubset(const SkIRect&, GrDirectContext*) const override;
#if defined(SK_GRAPHITE)
    sk_sp<SkImage> onMakeSubset(const SkIRect&,
                                skgpu::graphite::Recorder*,
                                RequiredImageProperties) const override;
    sk_sp<SkImage> onMakeColorTypeAndColorSpace(SkColorType targetCT,
                                                sk_sp<SkColorSpace> targetCS,
                                                skgpu::graphite::Recorder*,
                                                RequiredImageProperties) const override;
#endif
    bool getROPixels(GrDirectContext*, SkBitmap*, CachingHint) const override;
    SkImage_Base::Type type() const override { return SkImage_Base::Type::kLazy; }
    sk_sp<SkImage> onMakeColorTypeAndColorSpace(SkColorType, sk_sp<SkColorSpace>,
                                                GrDirectContext*) const override;
    sk_sp<SkImage> onReinterpretColorSpace(sk_sp<SkColorSpace>) const final;

protected:
    class ScopedGenerator;
    // Note that this->imageInfo() is not necessarily the info from the generator. It may be
    // cropped by onMakeSubset and its color type/space may be changed by
    // onMakeColorTypeAndColorSpace.
    sk_sp<SharedGenerator> fSharedGenerator;

private:
    virtual bool readPixelsProxy(GrDirectContext*, const SkPixmap&) const;

#if defined(SK_GRAPHITE)
    sk_sp<SkImage> onMakeTextureImage(skgpu::graphite::Recorder*,
                                      RequiredImageProperties) const override;
#endif

    // Repeated calls to onMakeColorTypeAndColorSpace will result in a proliferation of unique IDs
    // and SkImage_Lazy instances. Cache the result of the last successful call.
    mutable SkMutex        fOnMakeColorTypeAndSpaceMutex;
    mutable sk_sp<SkImage> fOnMakeColorTypeAndSpaceResult;

};

// Ref-counted tuple(SkImageGenerator, SkMutex) which allows sharing one generator among N images
class SharedGenerator final : public SkNVRefCnt<SharedGenerator> {
public:
    static sk_sp<SharedGenerator> Make(std::unique_ptr<SkImageGenerator> gen);

    // This is thread safe.  It is a const field set in the constructor.
    const SkImageInfo& getInfo();

private:
    explicit SharedGenerator(std::unique_ptr<SkImageGenerator> gen);

    friend class ScopedGenerator;
    friend class SkImage_Lazy;

    std::unique_ptr<SkImageGenerator> fGenerator;
    SkMutex                           fMutex;
};


// Helper for exclusive access to a shared generator.
class SkImage_Lazy::ScopedGenerator {
public:
    ScopedGenerator(const sk_sp<SharedGenerator>& gen)
      : fSharedGenerator(gen)
      , fAutoAcquire(gen->fMutex) {}

    SkImageGenerator* operator->() const {
        fSharedGenerator->fMutex.assertHeld();
        return fSharedGenerator->fGenerator.get();
    }

    operator SkImageGenerator*() const {
        fSharedGenerator->fMutex.assertHeld();
        return fSharedGenerator->fGenerator.get();
    }

private:
    const sk_sp<SharedGenerator>& fSharedGenerator;
    SkAutoMutexExclusive          fAutoAcquire;
};

#endif
