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
#include "include/core/SkImageInfo.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPicture.h"
#include "include/core/SkPictureRecorder.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSamplingOptions.h"
#include "include/core/SkSurface.h"
#include "include/core/SkSurfaceProps.h"
#include "include/core/SkTileMode.h"
#include "include/core/SkTiledImageUtils.h"
#include "src/core/SkBitmapCache.h"
#include "src/core/SkPicturePriv.h"
#include "src/core/SkResourceCache.h"
#include "tests/Test.h"

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <initializer_list>

// Test that the SkPictureShader cache is purged on shader deletion.
DEF_TEST(PictureShader_caching, reporter) {
    auto makePicture = [] () {
        SkPictureRecorder recorder;
        recorder.beginRecording(100, 100)->drawColor(SK_ColorGREEN);
        return recorder.finishRecordingAsPicture();
    };

    sk_sp<SkPicture> picture = makePicture();
    REPORTER_ASSERT(reporter, picture->unique());

    sk_sp<SkSurface> surface = SkSurfaces::Raster(SkImageInfo::MakeN32Premul(100, 100));

    {
        SkPaint paint;
        paint.setShader(picture->makeShader(SkTileMode::kRepeat, SkTileMode::kRepeat,
                                            SkFilterMode::kNearest));
        surface->getCanvas()->drawPaint(paint);

        // We should have about 3 refs by now: local + shader + shader cache.
        REPORTER_ASSERT(reporter, !picture->unique());
    }

    // Draw another picture shader to have a chance to purge.
    {
        SkPaint paint;
        paint.setShader(makePicture()->makeShader(SkTileMode::kRepeat, SkTileMode::kRepeat,
                                                  SkFilterMode::kNearest));
        surface->getCanvas()->drawPaint(paint);

    }

    // All but the local ref should be gone now.
    REPORTER_ASSERT(reporter, picture->unique());
}

/*
 *  Check caching of picture-shaders
 *  - we do cache the underlying image (i.e. there is a cache entry)
 *  - there is only 1 entry, even with differing tile modes
 *  - after deleting the picture, the cache entry is purged
 */
DEF_TEST(PictureShader_caching2, reporter) {
    auto picture = []() {
        SkPictureRecorder recorder;
        recorder.beginRecording(100, 100)->drawColor(SK_ColorGREEN);
        return recorder.finishRecordingAsPicture();
    }();
    REPORTER_ASSERT(reporter, picture->unique());

    struct Data {
        uint64_t sharedID;
        int counter;
    } data = {
        picture->uniqueID(),
        0,
    };

    auto counter = [](const SkResourceCache::Rec& rec, void* dataPtr) {
        if (rec.getKey().getSharedID() == ((Data*)dataPtr)->sharedID &&
            rec.getKey().getSharedIDTag() == SkPicturePriv::kResourceCacheTag) {
            ((Data*)dataPtr)->counter += 1;
        }
    };

    SkResourceCache::VisitAll(counter, &data);
    REPORTER_ASSERT(reporter, data.counter == 0);

    // Draw with a view variants of picture-shaders that all use the same picture.
    // Only expect 1 cache entry for all (since same CTM for all).
    sk_sp<SkSurface> surface = SkSurfaces::Raster(SkImageInfo::MakeN32Premul(100, 100));
    for (SkTileMode m : {
        SkTileMode::kClamp, SkTileMode::kRepeat, SkTileMode::kRepeat, SkTileMode::kDecal
    }) {
        SkPaint paint;
        paint.setShader(picture->makeShader(m, m, SkFilterMode::kNearest));
        surface->getCanvas()->drawPaint(paint);
    }

    // Don't expect any additional refs on the picture
    REPORTER_ASSERT(reporter, picture->unique());

    // Check that we did cache something, but only 1 thing
    data.counter = 0;
    SkResourceCache::VisitAll(counter, &data);
    REPORTER_ASSERT(reporter, data.counter == 1);

    // Now delete the picture, and check the we purge the cache entry

    picture.reset();
    SkResourceCache::CheckMessages();

    data.counter = 0;
    SkResourceCache::VisitAll(counter, &data);
    REPORTER_ASSERT(reporter, data.counter == 0);
}

namespace {
static unsigned gTestPictureNamespace;
static unsigned gTestBitmapNamespace;

struct TestSharedIDKey : public SkResourceCache::Key {
    uint32_t fPayload;

    TestSharedIDKey(void* ns, uint64_t sharedID, uint32_t tag, uint32_t payload)
            : fPayload(payload) {
        this->init(ns, sharedID, sizeof(fPayload), tag);
    }
};

struct TestSharedIDRec : public SkResourceCache::Rec {
    TestSharedIDKey fKey;
    int fValue;

    TestSharedIDRec(const TestSharedIDKey& key, int value) : fKey(key), fValue(value) {}

    const Key& getKey() const override { return fKey; }
    size_t bytesUsed() const override { return sizeof(*this); }
    const char* getCategory() const override { return "test-picture-shared-id"; }

    static bool Visitor(const SkResourceCache::Rec& baseRec, void* ctx) {
        *static_cast<int*>(ctx) = static_cast<const TestSharedIDRec&>(baseRec).fValue;
        return true;
    }
};
}  // namespace

DEF_TEST(PictureShader_64BitSharedIDAndDomainIsolation, reporter) {
    SkResourceCache cache(1024 * 1024);

    // 1. Two 64-bit picture IDs differing only in the upper 32 bits must not collide or cross-purge.
    constexpr uint64_t kBaseID = 42;
    constexpr uint64_t kUpperBitDiffID = kBaseID + (1ULL << 32);

    TestSharedIDKey picKey1(&gTestPictureNamespace, kBaseID, SkPicturePriv::kResourceCacheTag, 1);
    TestSharedIDKey picKey2(
            &gTestPictureNamespace, kUpperBitDiffID, SkPicturePriv::kResourceCacheTag, 1);

    cache.add(new TestSharedIDRec(picKey1, 100));
    cache.add(new TestSharedIDRec(picKey2, 200));

    int found = 0;
    REPORTER_ASSERT(reporter, cache.find(picKey1, TestSharedIDRec::Visitor, &found) && found == 100);
    REPORTER_ASSERT(reporter, cache.find(picKey2, TestSharedIDRec::Visitor, &found) && found == 200);

    cache.purgeSharedID(kBaseID, SkPicturePriv::kResourceCacheTag);
    REPORTER_ASSERT(reporter, !cache.find(picKey1, TestSharedIDRec::Visitor, &found));
    REPORTER_ASSERT(reporter, cache.find(picKey2, TestSharedIDRec::Visitor, &found) && found == 200);

    // 2. A picture whose 64-bit ID numerically matches a bitmap's packed sharedID must remain
    //    isolated during purge because of kResourceCacheTag.
    const uint64_t collidingID = SkMakeResourceCacheSharedIDForBitmap(7);
    TestSharedIDKey bitmapKey(&gTestBitmapNamespace, collidingID, /*tag=*/0, 1);
    TestSharedIDKey pictureKey(
            &gTestPictureNamespace, collidingID, SkPicturePriv::kResourceCacheTag, 1);

    cache.add(new TestSharedIDRec(bitmapKey, 300));
    cache.add(new TestSharedIDRec(pictureKey, 400));

    cache.purgeSharedID(collidingID, SkPicturePriv::kResourceCacheTag);
    REPORTER_ASSERT(reporter, !cache.find(pictureKey, TestSharedIDRec::Visitor, &found));
    REPORTER_ASSERT(reporter,
                    cache.find(bitmapKey, TestSharedIDRec::Visitor, &found) && found == 300);

    // 3. Verify SkTiledImageUtils::GetImageKeyValues disambiguates picture-backed and bitmap-backed
    //    images across all kNumImageKeyValues slots.
    SkPictureRecorder recorder;
    recorder.beginRecording(16, 16)->drawColor(SK_ColorRED);
    sk_sp<SkPicture> pic = recorder.finishRecordingAsPicture();
    sk_sp<SkImage> picImg = SkImages::DeferredFromPicture(pic,
                                                          {16, 16},
                                                          nullptr,
                                                          nullptr,
                                                          SkImages::BitDepth::kU8,
                                                          SkColorSpace::MakeSRGB(),
                                                          SkSurfaceProps{});
    REPORTER_ASSERT(reporter, picImg);

    SkBitmap bm;
    bm.allocN32Pixels(16, 16);
    bm.setImmutable();
    sk_sp<SkImage> bmImg = bm.asImage();

    uint32_t picKeyVals[SkTiledImageUtils::kNumImageKeyValues];
    uint32_t bmKeyVals[SkTiledImageUtils::kNumImageKeyValues];
    SkTiledImageUtils::GetImageKeyValues(picImg.get(), picKeyVals);
    SkTiledImageUtils::GetImageKeyValues(bmImg.get(), bmKeyVals);

    REPORTER_ASSERT(reporter, picKeyVals[1] != 0 || picKeyVals[2] != 0);
    REPORTER_ASSERT(reporter, bmKeyVals[1] == 0 && bmKeyVals[2] == 0);
    REPORTER_ASSERT(reporter, 0 != memcmp(picKeyVals, bmKeyVals, sizeof(picKeyVals)));
}
