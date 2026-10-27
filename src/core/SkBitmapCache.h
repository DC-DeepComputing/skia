/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkBitmapCache_DEFINED
#define SkBitmapCache_DEFINED

#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/private/SkAssert.h"
#include "include/private/SkWeakRefCnt.h"

#include <cstdint>
#include <memory>
#include <utility>

class SkBitmap;
class SkImage;
class SkImage_Base;
class SkMipmap;
class SkPixmap;
class SkResourceCache;
struct SkImageInfo;

uint64_t SkMakeResourceCacheSharedIDForBitmap(uint64_t bitmapGenID);

void SkNotifyBitmapGenIDIsStale(uint64_t bitmapGenID);

struct SkMipmapProvenance {
    const SkWeakRefCnt* fWeakToken = nullptr;
    uint64_t            fStorageID = 0;
    uint32_t            fContentID = 0;
    uint64_t            fGenID     = 0;
    SkIPoint            fOrigin    = {0, 0};

    bool isValid() const { return fWeakToken != nullptr && fStorageID != 0 && fGenID != 0; }

    static SkMipmapProvenance Make(const SkImage* image);
};

struct SkBitmapCacheDesc {
    uint64_t           fImageID = 0;          // != 0
    SkIRect            fSubset  = {0, 0, 0, 0}; // always set to a valid rect (entire or subset)
    SkMipmapProvenance fProvenance;

    void validate() const {
        SkASSERT(fImageID);
        SkASSERT(fSubset.fLeft >= 0 && fSubset.fTop >= 0);
        SkASSERT(fSubset.width() > 0 && fSubset.height() > 0);
    }

    static SkBitmapCacheDesc Make(const SkImage*);
    static SkBitmapCacheDesc Make(uint64_t genID, const SkIRect& subset);
};

class SkBitmapCache {
public:
    /**
     *  Search based on the desc. If found, returns true and
     *  result will be set to the matching bitmap with its pixels already locked.
     */
    static bool Find(const SkBitmapCacheDesc&, SkBitmap* result);

    class Rec;
    struct RecDeleter { void operator()(Rec* r) { PrivateDeleteRec(r); } };
    typedef std::unique_ptr<Rec, RecDeleter> RecPtr;

    static RecPtr Alloc(const SkBitmapCacheDesc&, const SkImageInfo&, SkPixmap*);
    static void Add(RecPtr, SkBitmap*);

private:
    static void PrivateDeleteRec(Rec*);
};

class SkMipmapCache {
public:
    static const SkMipmap* FindAndRef(const SkBitmapCacheDesc&,
                                      SkResourceCache* localCache = nullptr);
    static const SkMipmap* AddAndRef(const SkImage_Base*,
                                     SkResourceCache* localCache = nullptr);
};

#endif
