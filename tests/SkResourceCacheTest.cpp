/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkImage.h"
#include "include/core/SkImageGenerator.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPicture.h"  // IWYU pragma: keep
#include "include/core/SkPictureRecorder.h"
#include "include/core/SkPixelRef.h"
#include "include/core/SkPixmap.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSamplingOptions.h"
#include "include/core/SkSize.h"
#include "include/core/SkSurface.h"
#include "include/core/SkTypes.h"
#include "include/private/chromium/SkDiscardableMemory.h"
#include "src/core/SkBitmapCache.h"
#include "src/core/SkCachedData.h"
#include "src/core/SkMipmap.h"
#include "src/core/SkResourceCache.h"
#include "src/image/SkImage_Base.h"
#include "src/image/SkImage_Raster.h"
#include "src/lazy/SkDiscardableMemoryPool.h"
#include "tests/Test.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <initializer_list>
#include <memory>

////////////////////////////////////////////////////////////////////////////////////////

enum LockedState {
    kNotLocked,
    kLocked,
};

enum CachedState {
    kNotInCache,
    kInCache,
};

static void check_data(skiatest::Reporter* reporter, const SkCachedData* data,
                       int refcnt, CachedState cacheState, LockedState lockedState) {
    REPORTER_ASSERT(reporter, data->testing_only_getRefCnt() == refcnt);
    REPORTER_ASSERT(reporter, data->testing_only_isInCache() == (kInCache == cacheState));
    bool isLocked = (data->data() != nullptr);
    REPORTER_ASSERT(reporter, isLocked == (lockedState == kLocked));
}

static void test_mipmapcache(skiatest::Reporter* reporter, SkResourceCache* cache) {
    cache->purgeAll();

    SkBitmap src;
    src.allocN32Pixels(5, 5);
    src.setImmutable();
    sk_sp<SkImage> img = src.asImage();
    const auto desc = SkBitmapCacheDesc::Make(img.get());

    const SkMipmap* mipmap = SkMipmapCache::FindAndRef(desc, cache);
    REPORTER_ASSERT(reporter, nullptr == mipmap);

    mipmap = SkMipmapCache::AddAndRef(as_IB(img.get()), cache);
    REPORTER_ASSERT(reporter, mipmap);

    {
        const SkMipmap* mm = SkMipmapCache::FindAndRef(desc, cache);
        REPORTER_ASSERT(reporter, mm);
        REPORTER_ASSERT(reporter, mm == mipmap);
        mm->unref();
    }

    check_data(reporter, mipmap, 2, kInCache, kLocked);

    mipmap->unref();
    // tricky, since technically after this I'm no longer an owner, but since the cache is
    // local, I know it won't get purged behind my back
    check_data(reporter, mipmap, 1, kInCache, kNotLocked);

    // find us again
    mipmap = SkMipmapCache::FindAndRef(desc, cache);
    check_data(reporter, mipmap, 2, kInCache, kLocked);

    cache->purgeAll();
    check_data(reporter, mipmap, 1, kNotInCache, kLocked);

    mipmap->unref();
}

static void test_mipmap_notify(skiatest::Reporter* reporter, SkResourceCache* cache) {
    const int N = 3;

    std::array<SkBitmap, N> src;
    std::array<sk_sp<SkImage>, N> img;
    std::array<SkBitmapCacheDesc, N> desc;
    for (int i = 0; i < N; ++i) {
        src[i].allocN32Pixels(5, 5);
        src[i].setImmutable();
        img[i] = src[i].asImage();
        SkMipmapCache::AddAndRef(as_IB(img[i].get()), cache)->unref();
        desc[i] = SkBitmapCacheDesc::Make(img[i].get());
    }

    for (int i = 0; i < N; ++i) {
        const SkMipmap* mipmap = SkMipmapCache::FindAndRef(desc[i], cache);
        // We're always using a local cache, so we know we won't be purged by other threads
        REPORTER_ASSERT(reporter, mipmap);
        SkSafeUnref(mipmap);

        img[i].reset(); // delete the image, which *should not* remove us from the cache
        mipmap = SkMipmapCache::FindAndRef(desc[i], cache);
        REPORTER_ASSERT(reporter, mipmap);
        SkSafeUnref(mipmap);

        src[i].reset(); // delete the underlying pixelref, which *should* remove us from the cache
        mipmap = SkMipmapCache::FindAndRef(desc[i], cache);
        REPORTER_ASSERT(reporter, !mipmap);
    }
}

static SkDiscardableMemoryPool* gPool = nullptr;
static int gFactoryCalls = 0;

static SkDiscardableMemory* pool_factory(size_t bytes) {
    SkASSERT(gPool);
    gFactoryCalls++;
    return gPool->create(bytes);
}

static void testBitmapCache_discarded_bitmap(skiatest::Reporter* reporter, SkResourceCache* cache,
                                             SkResourceCache::DiscardableFactory factory) {
    test_mipmapcache(reporter, cache);
    test_mipmap_notify(reporter, cache);
}

DEF_TEST(BitmapCache_discarded_bitmap, reporter) {
    const size_t byteLimit = 100 * 1024;
    {
        SkResourceCache cache(byteLimit);
        testBitmapCache_discarded_bitmap(reporter, &cache, nullptr);
    }
    {
        sk_sp<SkDiscardableMemoryPool> pool(SkDiscardableMemoryPool::Make(byteLimit));
        gPool = pool.get();
        SkResourceCache::DiscardableFactory factory = pool_factory;
        SkResourceCache cache(factory);
        testBitmapCache_discarded_bitmap(reporter, &cache, factory);
    }
    REPORTER_ASSERT(reporter, gFactoryCalls > 0);
}

static void test_discarded_image(skiatest::Reporter* reporter, const SkMatrix& transform,
                                 sk_sp<SkImage> (*buildImage)()) {
    auto surface(SkSurfaces::Raster(SkImageInfo::MakeN32Premul(10, 10)));
    SkCanvas* canvas = surface->getCanvas();

    // SkBitmapCache is global, so other threads could be evicting our bitmaps.  Loop a few times
    // to mitigate this risk.
    const unsigned kRepeatCount = 42;
    for (unsigned i = 0; i < kRepeatCount; ++i) {
        SkAutoCanvasRestore acr(canvas, true);

        sk_sp<SkImage> image(buildImage());

        // draw the image (with a transform, to tickle different code paths) to ensure
        // any associated resources get cached
        canvas->concat(transform);
        // always use high quality to ensure caching when scaled
        canvas->drawImage(image, 0, 0, SkSamplingOptions({1.0f/3, 1.0f/3}));

        const auto desc = SkBitmapCacheDesc::Make(image.get());

        // delete the image
        image.reset(nullptr);

        // all resources should have been purged
        SkBitmap result;
        REPORTER_ASSERT(reporter, !SkBitmapCache::Find(desc, &result));
    }
}


// Verify that associated bitmap cache entries are purged on SkImage destruction.
DEF_TEST(BitmapCache_discarded_image, reporter) {
    // Cache entries associated with SkImages fall into two categories:
    //
    // 1) generated image bitmaps (managed by the image cacherator)
    // 2) scaled/resampled bitmaps (cached when HQ filters are used)
    //
    // To exercise the first cache type, we use generated/picture-backed SkImages.
    // To exercise the latter, we draw scaled bitmap images using HQ filters.

    const auto xforms = std::to_array<SkMatrix>({
            SkMatrix::Scale(1, 1),
            SkMatrix::Scale(1.7f, 0.5f),
    });

    for (size_t i = 0; i < std::size(xforms); ++i) {
        test_discarded_image(reporter, xforms[i], []() {
            auto surface(SkSurfaces::Raster(SkImageInfo::MakeN32Premul(10, 10)));
            surface->getCanvas()->clear(SK_ColorCYAN);
            return surface->makeImageSnapshot();
        });

        test_discarded_image(reporter, xforms[i], []() {
            SkPictureRecorder recorder;
            SkCanvas* canvas = recorder.beginRecording(10, 10);
            canvas->clear(SK_ColorCYAN);
            return SkImages::DeferredFromPicture(recorder.finishRecordingAsPicture(),
                                                 SkISize::Make(10, 10),
                                                 nullptr,
                                                 nullptr,
                                                 SkImages::BitDepth::kU8,
                                                 SkColorSpace::MakeSRGB());
        });
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////

static void* gTestNamespace;

struct TestKey : SkResourceCache::Key {
    int32_t fData;

    TestKey(int sharedID, int32_t data) : fData(data) {
        this->init(&gTestNamespace, sharedID, sizeof(fData));
    }
};

struct TestRec : SkResourceCache::Rec {
    enum {
        kDidInstall = 1 << 0,
    };

    TestKey fKey;
    int*    fFlags;
    bool    fCanBePurged;

    TestRec(int sharedID, int32_t data, int* flagPtr) : fKey(sharedID, data), fFlags(flagPtr) {
        fCanBePurged = false;
    }

    const Key& getKey() const override { return fKey; }
    size_t bytesUsed() const override { return 1024; /* just need a value */ }
    bool canBePurged() override { return fCanBePurged; }
    void postAddInstall(void*) override {
        *fFlags |= kDidInstall;
    }
    const char* getCategory() const override { return "test-category"; }
};

static void test_duplicate_add(SkResourceCache* cache, skiatest::Reporter* reporter,
                               bool purgable) {
    int sharedID = 1;
    int data = 0;

    int flags0 = 0, flags1 = 0;

    auto rec0 = std::make_unique<TestRec>(sharedID, data, &flags0);
    auto rec1 = std::make_unique<TestRec>(sharedID, data, &flags1);
    SkASSERT(rec0->getKey() == rec1->getKey());

    TestRec* r0 = rec0.get();   // save the bare-pointer since we will release rec0
    r0->fCanBePurged = purgable;

    REPORTER_ASSERT(reporter, !(flags0 & TestRec::kDidInstall));
    REPORTER_ASSERT(reporter, !(flags1 & TestRec::kDidInstall));

    cache->add(rec0.release(), nullptr);
    REPORTER_ASSERT(reporter, flags0 & TestRec::kDidInstall);
    REPORTER_ASSERT(reporter, !(flags1 & TestRec::kDidInstall));
    flags0 = 0; // reset the flag

    cache->add(rec1.release(), nullptr);
    if (purgable) {
        // we purged rec0, and did install rec1
        REPORTER_ASSERT(reporter, !(flags0 & TestRec::kDidInstall));
        REPORTER_ASSERT(reporter, flags1 & TestRec::kDidInstall);
    } else {
        // we re-used rec0 and did not install rec1
        REPORTER_ASSERT(reporter, flags0 & TestRec::kDidInstall);
        REPORTER_ASSERT(reporter, !(flags1 & TestRec::kDidInstall));
        r0->fCanBePurged = true;  // so we can cleanup the cache
    }
}

/*
 *  Test behavior when the same key is added more than once.
 */
DEF_TEST(ResourceCache_purge, reporter) {
    for (bool purgable : { false, true }) {
        {
            SkResourceCache cache(1024 * 1024);
            test_duplicate_add(&cache, reporter, purgable);
        }
        {
            SkResourceCache cache(SkDiscardableMemory::Create);
            test_duplicate_add(&cache, reporter, purgable);
        }
    }
}

namespace {

class SolidColorGenerator final : public SkImageGenerator {
public:
    SolidColorGenerator(const SkImageInfo& info, SkColor color, uint64_t forcedUniqueID)
            : SkImageGenerator(info, forcedUniqueID), fColor(color) {}

protected:
    bool onGetPixels(const SkImageInfo& info,
                     void* pixels,
                     size_t rowBytes,
                     const Options&) override {
        SkPixmap pm(info, pixels, rowBytes);
        pm.erase(fColor);
        return true;
    }

private:
    SkColor fColor;
};

}  // namespace

DEF_TEST(MipmapCache_ForcedIDCollision, reporter) {
    constexpr int kSrcSize = 64;
    constexpr int kDstSize = 16;
    const SkImageInfo srcInfo = SkImageInfo::MakeN32Premul(kSrcSize, kSrcSize);
    const SkImageInfo dstInfo = SkImageInfo::MakeN32Premul(kDstSize, kDstSize);

    // 1. Two live same-sized raster images forced to share the exact same generation/unique ID.
    SkBitmap bm1;
    bm1.allocPixels(srcInfo);
    bm1.eraseColor(SK_ColorRED);
    bm1.setImmutable();
    const uint64_t collidingID = bm1.getGenerationID();

    SkBitmap bm2;
    bm2.allocPixels(srcInfo);
    bm2.eraseColor(SK_ColorBLUE);
    // Force bm2's PixelRef to collide with bm1's generation ID while both remain live.
    SkBitmapCache_setImmutableWithID(bm2.pixelRef(), collidingID);

    sk_sp<SkImage> img1 = bm1.asImage();
    sk_sp<SkImage> img2 = bm2.asImage();
    REPORTER_ASSERT(reporter, img1 && img2);
    REPORTER_ASSERT(reporter, bm1.getGenerationID() == bm2.getGenerationID());
    REPORTER_ASSERT(reporter, img1->uniqueID() == img2->uniqueID());
    REPORTER_ASSERT(reporter, img1->dimensions() == img2->dimensions());

    // 1a. Verify direct SkMipmapCache lookup rejects key hits whose provenance does not match.
    {
        SkResourceCache localCache(1024 * 1024);
        const auto desc1 = SkBitmapCacheDesc::Make(img1.get());
        const auto desc2 = SkBitmapCacheDesc::Make(img2.get());
        REPORTER_ASSERT(reporter, desc1.fImageID == desc2.fImageID);
        REPORTER_ASSERT(reporter, desc1.fSubset == desc2.fSubset);

        const SkMipmap* mm1Added = SkMipmapCache::AddAndRef(as_IB(img1.get()), &localCache);
        REPORTER_ASSERT(reporter, mm1Added != nullptr);

        // Lookup for img1 must succeed and return img1's mip levels.
        const SkMipmap* mm1Found = SkMipmapCache::FindAndRef(desc1, &localCache);
        REPORTER_ASSERT(reporter, mm1Found == mm1Added);
        SkSafeUnref(mm1Found);

        // Lookup for img2 (same key, different live provenance) must be rejected!
        const SkMipmap* mm2Found = SkMipmapCache::FindAndRef(desc2, &localCache);
        REPORTER_ASSERT(reporter, mm2Found == nullptr);

        // Rejecting img2 must not evict img1's live mipmap from the cache.
        const SkMipmap* mm1StillThere = SkMipmapCache::FindAndRef(desc1, &localCache);
        REPORTER_ASSERT(reporter, mm1StillThere == mm1Added);
        SkSafeUnref(mm1StillThere);

        SkSafeUnref(mm1Added);
    }

    // 1b. Verify end-to-end downscaled mipmapped rendering never cross-contaminates mip levels.
    for (SkMipmapMode mmMode : {SkMipmapMode::kLinear, SkMipmapMode::kNearest}) {
        const SkSamplingOptions sampling(SkFilterMode::kLinear, mmMode);
        auto surf = SkSurfaces::Raster(dstInfo);
        SkCanvas* canvas = surf->getCanvas();

        // Draw img1 downscaled to populate the global SkMipmapCache with red mip levels.
        canvas->clear(SK_ColorTRANSPARENT);
        canvas->drawImageRect(img1, SkRect::MakeIWH(kDstSize, kDstSize), sampling);

        SkBitmap readback1;
        readback1.allocPixels(dstInfo);
        REPORTER_ASSERT(reporter, surf->readPixels(readback1, 0, 0));
        for (int y = 0; y < kDstSize; ++y) {
            for (int x = 0; x < kDstSize; ++x) {
                REPORTER_ASSERT(reporter, readback1.getColor(x, y) == SK_ColorRED);
            }
        }

        // Now draw img2 downscaled while img1 and its cached mip levels are still live.
        // img2 must never receive img1's red mip levels.
        canvas->clear(SK_ColorTRANSPARENT);
        canvas->drawImageRect(img2, SkRect::MakeIWH(kDstSize, kDstSize), sampling);

        SkBitmap readback2;
        readback2.allocPixels(dstInfo);
        REPORTER_ASSERT(reporter, surf->readPixels(readback2, 0, 0));
        for (int y = 0; y < kDstSize; ++y) {
            for (int x = 0; x < kDstSize; ++x) {
                REPORTER_ASSERT(reporter, readback2.getColor(x, y) == SK_ColorBLUE);
            }
        }
    }

    // 2. Two live same-sized lazy images forced to share the exact same generator uniqueID.
    sk_sp<SkImage> lazy1 = SkImages::DeferredFromGenerator(
            std::make_unique<SolidColorGenerator>(srcInfo, SK_ColorGREEN, collidingID));
    sk_sp<SkImage> lazy2 = SkImages::DeferredFromGenerator(
            std::make_unique<SolidColorGenerator>(srcInfo, SK_ColorYELLOW, collidingID));
    REPORTER_ASSERT(reporter, lazy1 && lazy2);
    REPORTER_ASSERT(reporter, lazy1->uniqueID() == lazy2->uniqueID());
    REPORTER_ASSERT(reporter, lazy1->dimensions() == lazy2->dimensions());

    {
        SkResourceCache localCache(1024 * 1024);
        const auto desc1 = SkBitmapCacheDesc::Make(lazy1.get());
        const auto desc2 = SkBitmapCacheDesc::Make(lazy2.get());
        REPORTER_ASSERT(reporter, desc1.fImageID == desc2.fImageID);
        REPORTER_ASSERT(reporter, desc1.fSubset == desc2.fSubset);

        const SkMipmap* mm1Added = SkMipmapCache::AddAndRef(as_IB(lazy1.get()), &localCache);
        REPORTER_ASSERT(reporter, mm1Added != nullptr);

        const SkMipmap* mm2Found = SkMipmapCache::FindAndRef(desc2, &localCache);
        REPORTER_ASSERT(reporter, mm2Found == nullptr);

        SkSafeUnref(mm1Added);
    }

    {
        const SkSamplingOptions sampling(SkFilterMode::kLinear, SkMipmapMode::kLinear);
        auto surf = SkSurfaces::Raster(dstInfo);
        SkCanvas* canvas = surf->getCanvas();

        canvas->clear(SK_ColorTRANSPARENT);
        canvas->drawImageRect(lazy1, SkRect::MakeIWH(kDstSize, kDstSize), sampling);

        SkBitmap readback1;
        readback1.allocPixels(dstInfo);
        REPORTER_ASSERT(reporter, surf->readPixels(readback1, 0, 0));
        for (int y = 0; y < kDstSize; ++y) {
            for (int x = 0; x < kDstSize; ++x) {
                REPORTER_ASSERT(reporter, readback1.getColor(x, y) == SK_ColorGREEN);
            }
        }

        canvas->clear(SK_ColorTRANSPARENT);
        canvas->drawImageRect(lazy2, SkRect::MakeIWH(kDstSize, kDstSize), sampling);

        SkBitmap readback2;
        readback2.allocPixels(dstInfo);
        REPORTER_ASSERT(reporter, surf->readPixels(readback2, 0, 0));
        for (int y = 0; y < kDstSize; ++y) {
            for (int x = 0; x < kDstSize; ++x) {
                REPORTER_ASSERT(reporter, readback2.getColor(x, y) == SK_ColorYELLOW);
            }
        }
    }

    // 3. Same SkPixelRef mutated in place and forced back to the same generation ID:
    // contentId provenance must still reject the stale mip levels.
    {
        SkResourceCache localCache(1024 * 1024);
        SkBitmap mutableBm;
        mutableBm.allocPixels(srcInfo);
        mutableBm.eraseColor(SK_ColorRED);
        const uint64_t origGenID = mutableBm.getGenerationID();

        sk_sp<SkImage> tempImg = SkImage_Raster::MakeFromBitmap(mutableBm, SkCopyPixelsMode::kNever);
        const auto descBefore = SkBitmapCacheDesc::Make(tempImg.get());
        const SkMipmap* mmBefore = SkMipmapCache::AddAndRef(as_IB(tempImg.get()), &localCache);
        REPORTER_ASSERT(reporter, mmBefore != nullptr);

        // Mutate pixels (increments SkPixelStorage::contentId) and force the same genID back.
        mutableBm.eraseColor(SK_ColorBLUE);
        SkBitmapCache_setImmutableWithID(mutableBm.pixelRef(), origGenID);
        REPORTER_ASSERT(reporter, mutableBm.getGenerationID() == origGenID);

        sk_sp<SkImage> mutatedImg = mutableBm.asImage();
        const auto descAfter = SkBitmapCacheDesc::Make(mutatedImg.get());
        REPORTER_ASSERT(reporter, descAfter.fImageID == descBefore.fImageID);
        const SkMipmap* mmAfter = SkMipmapCache::FindAndRef(descAfter, &localCache);
        REPORTER_ASSERT(reporter, mmAfter == nullptr);

        SkSafeUnref(mmBefore);
    }
}
