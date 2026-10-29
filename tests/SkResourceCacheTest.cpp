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
#include "include/core/SkPixmap.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSamplingOptions.h"
#include "include/core/SkSize.h"
#include "include/core/SkSpan.h"
#include "include/core/SkSurface.h"
#include "include/core/SkTiledImageUtils.h"
#include "include/core/SkTypes.h"
#include "include/core/SkYUVAInfo.h"
#include "include/core/SkYUVAPixmaps.h"
#include "include/private/chromium/SkDiscardableMemory.h"
#include "src/core/SkBitmapCache.h"
#include "src/core/SkCachedData.h"
#include "src/core/SkMipmap.h"
#include "src/core/SkPicturePriv.h"
#include "src/core/SkResourceCache.h"
#include "src/core/SkYUVPlanesCache.h"
#include "src/image/SkImage_Base.h"
#include "src/lazy/SkDiscardableMemoryPool.h"
#include "tests/Test.h"

#if defined(SK_GANESH)
#include "src/gpu/ResourceKey.h"
#include "src/gpu/ganesh/SkGr.h"
#endif

#include <array>
#include <cstddef>
#include <cstdint>
#include <initializer_list>
#include <memory>
#include <utility>

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

    TestKey(uint64_t sharedID, int32_t data) : fData(data) {
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

    TestRec(uint64_t sharedID, int32_t data, int* flagPtr) : fKey(sharedID, data), fFlags(flagPtr) {
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
class SolidColorImageGenerator final : public SkImageGenerator {
public:
    SolidColorImageGenerator(const SkImageInfo& info, SkColor color, uint64_t uniqueID)
            : SkImageGenerator(info, uniqueID), fColor(color) {}

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

// Verify that caches and key builders distinguish 64-bit IDs that share the same lower 32 bits
// and differ only in the upper 32 bits.
DEF_TEST(ResourceCache_64BitImageIDDisambiguation, reporter) {
    constexpr uint64_t kID1 = 0x0000000100000100ULL;
    constexpr uint64_t kID2 = 0x0000000200000100ULL;
    static_assert((kID1 & 0xFFFFFFFFULL) == (kID2 & 0xFFFFFFFFULL));

    const SkImageInfo info = SkImageInfo::MakeN32Premul(8, 8);
    sk_sp<SkImage> img1 = SkImages::DeferredFromGenerator(
            std::make_unique<SolidColorImageGenerator>(info, SK_ColorRED, kID1));
    sk_sp<SkImage> img2 = SkImages::DeferredFromGenerator(
            std::make_unique<SolidColorImageGenerator>(info, SK_ColorBLUE, kID2));
    REPORTER_ASSERT(reporter, img1 && img2);
    REPORTER_ASSERT(reporter, img1->uniqueID() == kID1);
    REPORTER_ASSERT(reporter, img2->uniqueID() == kID2);

    // 1. SkBitmapCache disambiguation
    const SkBitmapCacheDesc desc1 = SkBitmapCacheDesc::Make(img1.get());
    const SkBitmapCacheDesc desc2 = SkBitmapCacheDesc::Make(img2.get());
    {
        SkBitmap bm1, bm2;
        REPORTER_ASSERT(reporter, as_IB(img1.get())->getROPixels(nullptr, &bm1));
        REPORTER_ASSERT(reporter, bm1.getColor(0, 0) == SK_ColorRED);

        // Looking up desc2 before img2 is decoded must not hit img1's cached bitmap.
        SkBitmap found;
        REPORTER_ASSERT(reporter, !SkBitmapCache::Find(desc2, &found));

        REPORTER_ASSERT(reporter, as_IB(img2.get())->getROPixels(nullptr, &bm2));
        REPORTER_ASSERT(reporter, bm2.getColor(0, 0) == SK_ColorBLUE);
        REPORTER_ASSERT(reporter, SkBitmapCache::Find(desc1, &found));
        REPORTER_ASSERT(reporter, found.getColor(0, 0) == SK_ColorRED);
        REPORTER_ASSERT(reporter, SkBitmapCache::Find(desc2, &found));
        REPORTER_ASSERT(reporter, found.getColor(0, 0) == SK_ColorBLUE);
    }

    // 2. SkMipmapCache disambiguation
    {
        SkResourceCache localCache(1024 * 1024);
        const SkMipmap* mm1 = SkMipmapCache::AddAndRef(as_IB(img1.get()), &localCache);
        REPORTER_ASSERT(reporter, mm1);
        REPORTER_ASSERT(reporter, SkMipmapCache::FindAndRef(desc2, &localCache) == nullptr);

        const SkMipmap* mm2 = SkMipmapCache::AddAndRef(as_IB(img2.get()), &localCache);
        REPORTER_ASSERT(reporter, mm2);
        REPORTER_ASSERT(reporter, mm1 != mm2);

        const SkMipmap* found1 = SkMipmapCache::FindAndRef(desc1, &localCache);
        const SkMipmap* found2 = SkMipmapCache::FindAndRef(desc2, &localCache);
        REPORTER_ASSERT(reporter, found1 == mm1);
        REPORTER_ASSERT(reporter, found2 == mm2);
        SkSafeUnref(found1);
        SkSafeUnref(found2);
        mm1->unref();
        mm2->unref();
    }

    // 3. SkYUVPlanesCache disambiguation
    {
        SkResourceCache localCache(1024 * 1024);
        SkYUVAInfo yuvaInfo({8, 8},
                            SkYUVAInfo::PlaneConfig::kY_U_V,
                            SkYUVAInfo::Subsampling::k444,
                            kJPEG_Full_SkYUVColorSpace);
        SkYUVAPixmapInfo pmInfo(yuvaInfo,
                                SkYUVAPixmapInfo::DataType::kUnorm8,
                                /*rowBytes=*/nullptr);
        sk_sp<SkCachedData> data1(SkResourceCache::NewCachedData(pmInfo.computeTotalBytes()));
        SkYUVAPixmaps pixmaps1 = SkYUVAPixmaps::FromExternalMemory(pmInfo, data1->writable_data());
        SkYUVPlanesCache::Add(kID1, data1.get(), pixmaps1, &localCache);

        SkYUVAPixmaps foundPixmaps;
        REPORTER_ASSERT(reporter,
                        SkYUVPlanesCache::FindAndRef(kID2, &foundPixmaps, &localCache) == nullptr);
        SkCachedData* found1 = SkYUVPlanesCache::FindAndRef(kID1, &foundPixmaps, &localCache);
        REPORTER_ASSERT(reporter, found1 == data1.get());
        SkSafeUnref(found1);
    }

    // 4. SkTiledImageUtils::GetImageKeyValues disambiguation
    {
        std::array<uint32_t, SkTiledImageUtils::kNumImageKeyValues> kv1{};
        std::array<uint32_t, SkTiledImageUtils::kNumImageKeyValues> kv2{};
        SkTiledImageUtils::GetImageKeyValues(img1.get(), SkSpan(kv1));
        SkTiledImageUtils::GetImageKeyValues(img2.get(), SkSpan(kv2));
        REPORTER_ASSERT(reporter, kv1 != kv2);
    }

#if defined(SK_GANESH)
    // 5. GrMakeKeyFromImageID disambiguation
    {
        skgpu::UniqueKey key1, key2;
        const SkIRect bounds = SkIRect::MakeWH(8, 8);
        GrMakeKeyFromImageID(&key1, kID1, bounds);
        GrMakeKeyFromImageID(&key2, kID2, bounds);
        REPORTER_ASSERT(reporter, !(key1 == key2));
    }
#endif
}

// Verify that 64-bit bitmap shared IDs (> UINT32_MAX) and SkPicturePriv::MakeSharedID
// never collide (even when pictureID has bit 31 set) and purge independently.
DEF_TEST(ResourceCache_64BitSharedIDPurgeIsolation, reporter) {
    // Verify MakeSharedID does not overflow into the 'pict' tag when bit 31 of pictureID is set.
    constexpr uint32_t kLowPicID = 0x00000001u;
    constexpr uint32_t kHighPicID = 0x80000001u;
    const uint64_t picSharedLow = SkPicturePriv::MakeSharedID(kLowPicID);
    const uint64_t picSharedHigh = SkPicturePriv::MakeSharedID(kHighPicID);
    REPORTER_ASSERT(reporter, picSharedLow != picSharedHigh);
    REPORTER_ASSERT(reporter, (picSharedLow & 1ULL) == 0);
    REPORTER_ASSERT(reporter, (picSharedHigh & 1ULL) == 0);
    // Upper 31 bits (the 'pict' tag) must be identical regardless of bit 31 in pictureID.
    REPORTER_ASSERT(reporter, (picSharedLow >> 33) == (picSharedHigh >> 33));

    // Construct a 64-bit bitmap generation ID whose upper bits match picSharedHigh's tag/ID bits.
    const uint64_t bitmapGenID = picSharedHigh;
    const uint64_t bitmapSharedID = SkMakeResourceCacheSharedIDForBitmap(bitmapGenID);
    REPORTER_ASSERT(reporter, (bitmapSharedID & 1ULL) == 1ULL);
    REPORTER_ASSERT(reporter, bitmapSharedID != picSharedHigh);

    int flagsBitmap = 0, flagsPicLow = 0, flagsPicHigh = 0;
    auto recBitmap = std::make_unique<TestRec>(bitmapSharedID, 10, &flagsBitmap);
    auto recPicLow = std::make_unique<TestRec>(picSharedLow, 20, &flagsPicLow);
    auto recPicHigh = std::make_unique<TestRec>(picSharedHigh, 30, &flagsPicHigh);
    recBitmap->fCanBePurged = true;
    recPicLow->fCanBePurged = true;
    recPicHigh->fCanBePurged = true;

    const TestKey keyBitmap = recBitmap->fKey;
    const TestKey keyPicLow = recPicLow->fKey;
    const TestKey keyPicHigh = recPicHigh->fKey;

    SkResourceCache::Add(recBitmap.release(), nullptr);
    SkResourceCache::Add(recPicLow.release(), nullptr);
    SkResourceCache::Add(recPicHigh.release(), nullptr);

    auto isPresent = [](const TestKey& key) {
        return SkResourceCache::Find(
                key, [](const SkResourceCache::Rec&, void*) { return true; }, nullptr);
    };

    REPORTER_ASSERT(reporter, isPresent(keyBitmap));
    REPORTER_ASSERT(reporter, isPresent(keyPicLow));
    REPORTER_ASSERT(reporter, isPresent(keyPicHigh));

    // Purging the 64-bit bitmap GenID must purge only the bitmap entry and leave picture entries.
    SkNotifyBitmapGenIDIsStale(bitmapGenID);
    SkResourceCache::CheckMessages();

    REPORTER_ASSERT(reporter, !isPresent(keyBitmap));
    REPORTER_ASSERT(reporter, isPresent(keyPicLow));
    REPORTER_ASSERT(reporter, isPresent(keyPicHigh));

    // Purging the high picture ID must not purge the low picture ID.
    SkResourceCache::PostPurgeSharedID(picSharedHigh);
    SkResourceCache::CheckMessages();

    REPORTER_ASSERT(reporter, !isPresent(keyPicHigh));
    REPORTER_ASSERT(reporter, isPresent(keyPicLow));

    SkResourceCache::PostPurgeSharedID(picSharedLow);
    SkResourceCache::CheckMessages();
    REPORTER_ASSERT(reporter, !isPresent(keyPicLow));
}
