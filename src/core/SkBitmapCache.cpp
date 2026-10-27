/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkBitmapCache.h"

#include "include/core/SkBitmap.h"
#include "include/core/SkFourByteTag.h"
#include "include/core/SkImage.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkPixelRef.h"
#include "include/core/SkPixmap.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkTypes.h"
#include "include/private/SkMalloc.h"
#include "include/private/SkMutex.h"
#include "include/private/chromium/SkDiscardableMemory.h"
#include "src/core/SkMipmap.h"
#include "src/core/SkNextID.h"
#include "src/core/SkResourceCache.h"
#include "src/image/SkImage_Base.h"

#include <cstddef>
#include <utility>

/**
 *  Use this for bitmapcache and mipmapcache entries.
 *  Bitmap generation IDs and image unique IDs never set bit 0 (see SkNextID::ImageID()).
 *  We set bit 0 here so bitmap shared IDs are always odd and never collide with
 *  SkPicturePriv::MakeSharedID (which is always even) or 0.
 */
uint64_t SkMakeResourceCacheSharedIDForBitmap(uint64_t bitmapGenID) {
    SkASSERT(bitmapGenID != 0);
    SkASSERT((bitmapGenID & 1ULL) == 0);
    return bitmapGenID | 1ULL;
}

void SkNotifyBitmapGenIDIsStale(uint64_t bitmapGenID) {
    SkResourceCache::PostPurgeSharedID(SkMakeResourceCacheSharedIDForBitmap(bitmapGenID));
}

///////////////////////////////////////////////////////////////////////////////////////////////////

SkMipmapProvenance SkMipmapProvenance::Make(const SkImage* image) {
    if (!image) {
        return {};
    }
    const SkImage_Base* ib = as_IB(image);
    if (const SkBitmap* bm = ib->onPeekBitmap(); bm && bm->pixelRef()) {
        SkPixelRef* pr = bm->pixelRef();
        return {pr->weakToken(),
                pr->getPixelStorageId(),
                pr->getContentId(),
                pr->getGenerationID(),
                bm->pixelRefOrigin()};
    }
    if (!ib->getPixelStorages().empty() && ib->getPixelStorages()[0]) {
        SkPixelStorage* storage = ib->getPixelStorages()[0].get();
        return {storage->weakToken(),
                storage->getPixelStorageId(),
                storage->getContentId(),
                ib->uniqueID(),
                {0, 0}};
    }
    return {ib->weakToken(),
            ib->imageInstanceID(),
            /*fContentID=*/0,
            ib->uniqueID(),
            {0, 0}};
}

SkBitmapCacheDesc SkBitmapCacheDesc::Make(uint64_t imageID, const SkIRect& subset) {
    SkASSERT(imageID);
    SkASSERT(subset.width() > 0 && subset.height() > 0);
    return { imageID, subset, {} };
}

SkBitmapCacheDesc SkBitmapCacheDesc::Make(const SkImage* image) {
    SkIRect bounds = SkIRect::MakeWH(image->width(), image->height());
    return { image->uniqueID(), bounds, SkMipmapProvenance::Make(image) };
}

namespace {
class RetainedProvenance {
public:
    explicit RetainedProvenance(const SkMipmapProvenance& provenance)
            : fProvenance(provenance) {
        if (fProvenance.fWeakToken) {
            fProvenance.fWeakToken->weak_ref();
        }
    }
    RetainedProvenance(const RetainedProvenance&) = delete;
    RetainedProvenance& operator=(const RetainedProvenance&) = delete;
    ~RetainedProvenance() {
        if (fProvenance.fWeakToken) {
            fProvenance.fWeakToken->weak_unref();
        }
    }

    bool isValid() const { return fProvenance.isValid(); }
    bool isExpired() const {
        return !fProvenance.fWeakToken || fProvenance.fWeakToken->weak_expired();
    }

    bool matches(const SkMipmapProvenance& expected) const {
        if (!this->isValid() || !expected.isValid() || this->isExpired()) {
            return false;
        }
        return fProvenance.fWeakToken == expected.fWeakToken &&
               fProvenance.fStorageID == expected.fStorageID &&
               fProvenance.fContentID == expected.fContentID &&
               fProvenance.fGenID     == expected.fGenID &&
               fProvenance.fOrigin    == expected.fOrigin;
    }

private:
    SkMipmapProvenance fProvenance;
};

static unsigned gBitmapKeyNamespaceLabel;

struct CacheKeyData {
    uint32_t fImageID_lo;
    uint32_t fImageID_hi;
    SkIRect  fSubset;

    explicit CacheKeyData(const SkBitmapCacheDesc& desc)
            : fImageID_lo(static_cast<uint32_t>(desc.fImageID & 0xFFFFFFFFULL))
            , fImageID_hi(static_cast<uint32_t>(desc.fImageID >> 32))
            , fSubset(desc.fSubset) {}
};
static_assert(sizeof(CacheKeyData) == 2 * sizeof(uint32_t) + sizeof(SkIRect));
static_assert(alignof(CacheKeyData) == 4);

struct BitmapKey : public SkResourceCache::Key {
public:
    BitmapKey(const SkBitmapCacheDesc& desc) : fData(desc) {
        this->init(&gBitmapKeyNamespaceLabel, SkMakeResourceCacheSharedIDForBitmap(desc.fImageID),
                   sizeof(fData));
    }

    const CacheKeyData fData;
};
static_assert(sizeof(BitmapKey) == sizeof(SkResourceCache::Key) + sizeof(CacheKeyData));
}  // namespace

//////////////////////

void SkBitmapCache_setImmutableWithID(SkPixelRef* pr, uint64_t id) {
    pr->setImmutableWithID(id);
}

struct BitmapFinderContext {
    const SkMipmapProvenance* fExpectedProvenance = nullptr;
    SkBitmap*                 fResult             = nullptr;
    bool                      fFound              = false;
};

class SkBitmapCache::Rec : public SkResourceCache::Rec {
public:
    Rec(const SkBitmapCacheDesc& desc, const SkImageInfo& info, size_t rowBytes,
        std::unique_ptr<SkDiscardableMemory> dm, void* block)
        : fKey(desc)
        , fProvenance(desc.fProvenance)
        , fDM(std::move(dm))
        , fMalloc(block)
        , fInfo(info)
        , fRowBytes(rowBytes)
    {
        SkASSERT(!(fDM && fMalloc));    // can't have both

        // We need an ID to return with the bitmap/pixelref. We can't necessarily use the key/desc
        // ID - lazy images cache the same ID with multiple keys (in different color types).
        fPrUniqueID = SkNextID::ImageID();
    }

    ~Rec() override {
        SkASSERT(0 == fExternalCounter);
        if (fDM && fDiscardableIsLocked) {
            SkASSERT(fDM->data());
            fDM->unlock();
        }
        sk_free(fMalloc);   // may be null
    }

    const Key& getKey() const override { return fKey; }
    size_t bytesUsed() const override {
        return sizeof(fKey) + fInfo.computeByteSize(fRowBytes);
    }
    bool canBePurged() override {
        SkAutoMutexExclusive ama(fMutex);
        return fExternalCounter == 0;
    }
    void postAddInstall(void* payload) override {
        SkAssertResult(this->install(static_cast<SkBitmap*>(payload)));
    }

    const char* getCategory() const override { return "bitmap"; }
    SkDiscardableMemory* diagnostic_only_getDiscardable() const override {
        return fDM.get();
    }

    static void ReleaseProc(void* addr, void* ctx) {
        Rec* rec = static_cast<Rec*>(ctx);
        SkAutoMutexExclusive ama(rec->fMutex);

        SkASSERT(rec->fExternalCounter > 0);
        rec->fExternalCounter -= 1;
        if (rec->fDM) {
            SkASSERT(rec->fMalloc == nullptr);
            if (rec->fExternalCounter == 0) {
                rec->fDM->unlock();
                rec->fDiscardableIsLocked = false;
            }
        } else {
            SkASSERT(rec->fMalloc != nullptr);
        }
    }

    bool install(SkBitmap* bitmap) {
        SkAutoMutexExclusive ama(fMutex);

        if (!fDM && !fMalloc) {
            return false;
        }

        if (fDM) {
            if (!fDiscardableIsLocked) {
                SkASSERT(fExternalCounter == 0);
                if (!fDM->lock()) {
                    fDM.reset(nullptr);
                    return false;
                }
                fDiscardableIsLocked = true;
            }
            SkASSERT(fDM->data());
        }

        bitmap->installPixels(fInfo, fDM ? fDM->data() : fMalloc, fRowBytes, ReleaseProc, this);
        SkBitmapCache_setImmutableWithID(bitmap->pixelRef(), fPrUniqueID);
        fExternalCounter++;

        return true;
    }

    static bool Finder(const SkResourceCache::Rec& baseRec, void* contextPtr) {
        Rec* rec = const_cast<Rec*>(static_cast<const Rec*>(&baseRec));
        BitmapFinderContext* ctx = static_cast<BitmapFinderContext*>(contextPtr);

        if (rec->fProvenance.isValid()) {
            if (rec->fProvenance.isExpired()) {
                if (rec->canBePurged()) {
                    return false;
                }
                ctx->fFound = false;
                return true;
            }
            if (!ctx->fExpectedProvenance || !rec->fProvenance.matches(*ctx->fExpectedProvenance)) {
                ctx->fFound = false;
                return true;
            }
        }
        if (!rec->install(ctx->fResult)) {
            return false;
        }
        ctx->fFound = true;
        return true;
    }

private:
    BitmapKey          fKey;
    RetainedProvenance fProvenance;

    SkMutex     fMutex;

    // either fDM or fMalloc can be non-null, but not both
    std::unique_ptr<SkDiscardableMemory> fDM;
    void*       fMalloc;

    SkImageInfo fInfo;
    size_t      fRowBytes;
    uint64_t    fPrUniqueID;

    // This field counts the number of external pixelrefs we have created.
    // They notify us when they are destroyed so we can decrement this.
    int  fExternalCounter     = 0;
    bool fDiscardableIsLocked = true;
};

void SkBitmapCache::PrivateDeleteRec(Rec* rec) { delete rec; }

SkBitmapCache::RecPtr SkBitmapCache::Alloc(const SkBitmapCacheDesc& desc, const SkImageInfo& info,
                                           SkPixmap* pmap) {
    // Ensure that the info matches the subset (i.e. the subset is the entire image)
    SkASSERT(info.width() == desc.fSubset.width());
    SkASSERT(info.height() == desc.fSubset.height());

    const size_t rb = info.minRowBytes();
    size_t size = info.computeByteSize(rb);
    if (SkImageInfo::ByteSizeOverflowed(size)) {
        return nullptr;
    }

    std::unique_ptr<SkDiscardableMemory> dm;
    void* block = nullptr;

    auto factory = SkResourceCache::GetDiscardableFactory();
    if (factory) {
        dm.reset(factory(size));
    } else {
        block = sk_malloc_canfail(size);
    }
    if (!dm && !block) {
        return nullptr;
    }
    *pmap = SkPixmap(info, dm ? dm->data() : block, rb);
    return RecPtr(new Rec(desc, info, rb, std::move(dm), block));
}

void SkBitmapCache::Add(RecPtr rec, SkBitmap* bitmap) {
    SkResourceCache::Add(rec.release(), bitmap);
}

bool SkBitmapCache::Find(const SkBitmapCacheDesc& desc, SkBitmap* result) {
    desc.validate();
    BitmapFinderContext ctx{&desc.fProvenance, result, false};
    if (!SkResourceCache::Find(BitmapKey(desc), SkBitmapCache::Rec::Finder, &ctx)) {
        return false;
    }
    return ctx.fFound;
}

//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////

#define CHECK_LOCAL(localCache, localName, globalName, ...) \
    ((localCache) ? localCache->localName(__VA_ARGS__) : SkResourceCache::globalName(__VA_ARGS__))

namespace {
static unsigned gMipMapKeyNamespaceLabel;

struct MipMapKey : public SkResourceCache::Key {
public:
    MipMapKey(const SkBitmapCacheDesc& desc) : fData(desc) {
        this->init(&gMipMapKeyNamespaceLabel, SkMakeResourceCacheSharedIDForBitmap(desc.fImageID),
                   sizeof(fData));
    }

    const CacheKeyData fData;
};
static_assert(sizeof(MipMapKey) == sizeof(SkResourceCache::Key) + sizeof(CacheKeyData));

struct MipMapFinderContext {
    const SkMipmapProvenance* fExpectedProvenance = nullptr;
    const SkMipmap*           fResult             = nullptr;
};

struct MipMapRec : public SkResourceCache::Rec {
    MipMapRec(const SkBitmapCacheDesc& desc, const SkMipmap* result)
        : fKey(desc)
        , fProvenance(desc.fProvenance)
        , fMipMap(result)
    {
        SkASSERT(fProvenance.isValid());
        fMipMap->attachToCacheAndRef();
    }

    ~MipMapRec() override {
        fMipMap->detachFromCacheAndUnref();
    }

    const Key& getKey() const override { return fKey; }
    size_t bytesUsed() const override { return sizeof(fKey) + fMipMap->size(); }
    const char* getCategory() const override { return "mipmap"; }
    SkDiscardableMemory* diagnostic_only_getDiscardable() const override {
        return fMipMap->diagnostic_only_getDiscardable();
    }

    static bool Finder(const SkResourceCache::Rec& baseRec, void* contextPtr) {
        const MipMapRec& rec = static_cast<const MipMapRec&>(baseRec);
        MipMapFinderContext* ctx = static_cast<MipMapFinderContext*>(contextPtr);

        if (rec.fProvenance.isExpired()) {
            // The source object that produced this mipmap is dead; tell SkResourceCache to purge.
            return false;
        }

        if (!ctx->fExpectedProvenance || !rec.fProvenance.matches(*ctx->fExpectedProvenance)) {
            // Key collision with a different live source or mutated generation.
            // Reject the hit for this caller without marking the live entry stale.
            ctx->fResult = nullptr;
            return true;
        }

        const SkMipmap* mm = SkRef(rec.fMipMap);
        // the call to ref() above triggers a "lock" in the case of discardable memory,
        // which means we can now check for null (in case the lock failed).
        if (nullptr == mm->data()) {
            mm->unref();    // balance our call to ref()
            return false;
        }
        // the caller must call unref() when they are done.
        ctx->fResult = mm;
        return true;
    }

private:
    MipMapKey          fKey;
    RetainedProvenance fProvenance;
    const SkMipmap*    fMipMap;
};
}  // namespace

const SkMipmap* SkMipmapCache::FindAndRef(const SkBitmapCacheDesc& desc,
                                          SkResourceCache* localCache) {
    desc.validate();
    MipMapKey key(desc);
    MipMapFinderContext ctx{&desc.fProvenance, nullptr};

    if (!CHECK_LOCAL(localCache, find, Find, key, MipMapRec::Finder, &ctx)) {
        return nullptr;
    }
    return ctx.fResult;
}

static SkResourceCache::DiscardableFactory get_fact(SkResourceCache* localCache) {
    return localCache ? localCache->discardableFactory()
                      : SkResourceCache::GetDiscardableFactory();
}

const SkMipmap* SkMipmapCache::AddAndRef(const SkImage_Base* image, SkResourceCache* localCache) {
    SkBitmap src;
    if (!image->getROPixels(nullptr, &src)) {
        return nullptr;
    }

    SkMipmap* mipmap = SkMipmap::Build(src, get_fact(localCache));
    if (mipmap) {
        MipMapRec* rec = new MipMapRec(SkBitmapCacheDesc::Make(image), mipmap);
        CHECK_LOCAL(localCache, add, Add, rec);
        image->notifyAddedToRasterCache();
    }
    return mipmap;
}
