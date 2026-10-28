/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkYUVPlanesCache.h"

#include "include/core/SkYUVAPixmaps.h"
#include "src/core/SkBitmapCache.h"
#include "src/core/SkCachedData.h"
#include "src/core/SkResourceCache.h"

#include <cstddef>

class SkDiscardableMemory;

#define CHECK_LOCAL(localCache, localName, globalName, ...) \
    ((localCache) ? localCache->localName(__VA_ARGS__) : SkResourceCache::globalName(__VA_ARGS__))

namespace {
static unsigned gYUVPlanesKeyNamespaceLabel;

struct YUVValue {
    SkYUVAPixmaps fPixmaps;
    SkCachedData* fData;
};

struct YUVPlanesKey : public SkResourceCache::Key {
    YUVPlanesKey(uint64_t genID)
        : fGenID_lo(static_cast<uint32_t>(genID & 0xFFFFFFFFULL))
        , fGenID_hi(static_cast<uint32_t>(genID >> 32))
    {
        this->init(&gYUVPlanesKeyNamespaceLabel, SkMakeResourceCacheSharedIDForBitmap(genID),
                   2 * sizeof(uint32_t));
    }

    uint32_t fGenID_lo;
    uint32_t fGenID_hi;
};
static_assert(sizeof(YUVPlanesKey) == sizeof(SkResourceCache::Key) + 2 * sizeof(uint32_t),
              "YUVPlanesKey must have no padding");

struct YUVPlanesRec : public SkResourceCache::Rec {
    YUVPlanesRec(YUVPlanesKey key, SkCachedData* data, const SkYUVAPixmaps& pixmaps)
        : fKey(key)
    {
        fValue.fData = data;
        fValue.fPixmaps = pixmaps;
        fValue.fData->attachToCacheAndRef();
    }
    ~YUVPlanesRec() override {
        fValue.fData->detachFromCacheAndUnref();
    }

    YUVPlanesKey  fKey;
    YUVValue      fValue;

    const Key& getKey() const override { return fKey; }
    size_t bytesUsed() const override { return sizeof(*this) + fValue.fData->size(); }
    const char* getCategory() const override { return "yuv-planes"; }
    SkDiscardableMemory* diagnostic_only_getDiscardable() const override {
        return fValue.fData->diagnostic_only_getDiscardable();
    }

    static bool Visitor(const SkResourceCache::Rec& baseRec, void* contextData) {
        const YUVPlanesRec& rec = static_cast<const YUVPlanesRec&>(baseRec);
        YUVValue* result = static_cast<YUVValue*>(contextData);

        SkCachedData* tmpData = rec.fValue.fData;
        tmpData->ref();
        if (nullptr == tmpData->data()) {
            tmpData->unref();
            return false;
        }
        result->fData = tmpData;
        result->fPixmaps = rec.fValue.fPixmaps;
        return true;
    }
};
} // namespace

SkCachedData* SkYUVPlanesCache::FindAndRef(uint64_t genID,
                                           SkYUVAPixmaps* pixmaps,
                                           SkResourceCache* localCache) {
    YUVValue result;
    YUVPlanesKey key(genID);
    if (!CHECK_LOCAL(localCache, find, Find, key, YUVPlanesRec::Visitor, &result)) {
        return nullptr;
    }

    *pixmaps = result.fPixmaps;
    return result.fData;
}

void SkYUVPlanesCache::Add(uint64_t genID, SkCachedData* data, const SkYUVAPixmaps& pixmaps,
                           SkResourceCache* localCache) {
    YUVPlanesKey key(genID);
    return CHECK_LOCAL(localCache, add, Add, new YUVPlanesRec(key, data, pixmaps));
}
