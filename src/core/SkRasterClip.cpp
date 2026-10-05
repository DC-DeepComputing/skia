/*
 * Copyright 2010 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkRasterClip.h"

#include "include/core/SkBlendMode.h"
#include "include/core/SkClipOp.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPath.h"
#include "include/core/SkScalar.h"
#include "include/private/SkDebug.h"
#include "src/core/SkRegionPriv.h"

class SkBlitter;

SkRasterClip::SkRasterClip(const SkRasterClip& that)
        : fIsBW(that.fIsBW)
        , fIsEmpty(that.fIsEmpty)
        , fIsRect(that.fIsRect)
        , fShader(that.fShader)
{
    AUTO_RASTERCLIP_VALIDATE(that);

    if (fIsBW) {
        fBW = that.fBW;
    } else {
        fAA = that.fAA;
    }

    SkDEBUGCODE(this->validate();)
}

SkRasterClip& SkRasterClip::operator=(const SkRasterClip& that) {
    AUTO_RASTERCLIP_VALIDATE(that);

    fIsBW = that.fIsBW;
    if (fIsBW) {
        fBW = that.fBW;
    } else {
        fAA = that.fAA;
    }

    fIsEmpty = that.isEmpty();
    fIsRect = that.isRect();
    fShader = that.fShader;
    SkDEBUGCODE(this->validate();)
    return *this;
}

SkRasterClip::SkRasterClip(const SkRegion& rgn) : fBW(rgn) {
    fIsBW = true;
    fIsEmpty = this->computeIsEmpty();  // bounds might be empty, so compute
    fIsRect = !fIsEmpty;
    SkDEBUGCODE(this->validate();)
}

SkRasterClip::SkRasterClip(const SkIRect& bounds) : fBW(bounds) {
    fIsBW = true;
    fIsEmpty = this->computeIsEmpty();  // bounds might be empty, so compute
    fIsRect = !fIsEmpty;
    SkDEBUGCODE(this->validate();)
}

SkRasterClip::SkRasterClip() {
    fIsBW = true;
    fIsEmpty = true;
    fIsRect = false;
    SkDEBUGCODE(this->validate();)
}

SkRasterClip::SkRasterClip(const SkPath& path, const SkIRect& bounds, bool doAA) {
    if (doAA) {
        fIsBW = false;
        fAA.setPath(path, bounds, true);
    } else {
        fIsBW = true;
        fBW.setPath(path, SkRegion(bounds));
    }
    fIsEmpty = this->computeIsEmpty();  // bounds might be empty, so compute
    fIsRect = this->computeIsRect();
    SkDEBUGCODE(this->validate();)
}

SkRasterClip::~SkRasterClip() {
    SkDEBUGCODE(this->validate();)
}

bool SkRasterClip::setEmpty() {
    AUTO_RASTERCLIP_VALIDATE(*this);

    fIsBW = true;
    fBW.setEmpty();
    fAA.setEmpty();
    fIsEmpty = true;
    fIsRect = false;
    return false;
}

bool SkRasterClip::setRect(const SkIRect& rect) {
    AUTO_RASTERCLIP_VALIDATE(*this);

    fIsBW = true;
    fAA.setEmpty();
    fIsRect = fBW.setRect(rect);
    fIsEmpty = !fIsRect;
    return fIsRect;
}

/////////////////////////////////////////////////////////////////////////////////////

bool SkRasterClip::op(const SkIRect& rect, SkClipOp op) {
    AUTO_RASTERCLIP_VALIDATE(*this);

    if (fIsBW) {
        fBW.op(rect, (SkRegion::Op) op);
    } else {
        fAA.op(rect, op);
    }
    return this->updateCacheAndReturnNonEmpty();
}

bool SkRasterClip::op(const SkRegion& rgn, SkClipOp op) {
    AUTO_RASTERCLIP_VALIDATE(*this);

    if (fIsBW) {
        (void)fBW.op(rgn, (SkRegion::Op) op);
    } else {
        SkAAClip tmp;
        tmp.setRegion(rgn);
        (void)fAA.op(tmp, op);
    }
    return this->updateCacheAndReturnNonEmpty();
}

/**
 *  Our antialiasing currently has a granularity of 1/4 of a pixel along each
 *  axis. Thus we can treat an axis coordinate as an integer if it differs
 *  from its nearest int by < half of that value (1/8 in this case).
 */
static bool nearly_integral(SkScalar x) {
    static const SkScalar domain = SK_Scalar1 / 4;
    static const SkScalar halfDomain = domain / 2;

    x += halfDomain;
    return x - SkScalarFloorToScalar(x) < domain;
}

bool SkRasterClip::op(const SkRect& localRect, const SkMatrix& matrix, SkClipOp op, bool doAA) {
    AUTO_RASTERCLIP_VALIDATE(*this);

    const bool isScaleTrans = matrix.isScaleTranslate();
    if (!isScaleTrans) {
        return this->op(SkPath::Rect(localRect), matrix, op, doAA);
    }

    SkRect devRect = matrix.mapRect(localRect);
    if (fIsBW && doAA) {
        // check that the rect really needs aa, or is it close enought to
        // integer boundaries that we can just treat it as a BW rect?
        if (nearly_integral(devRect.fLeft) && nearly_integral(devRect.fTop) &&
            nearly_integral(devRect.fRight) && nearly_integral(devRect.fBottom)) {
            doAA = false;
        }
    }

    if (fIsBW && !doAA) {
        (void)fBW.op(devRect.round(), (SkRegion::Op) op);
    } else {
        if (fIsBW) {
            this->convertToAA();
        }
        (void)fAA.op(devRect, op, doAA);
    }
    return this->updateCacheAndReturnNonEmpty();
}

bool SkRasterClip::op(const SkRRect& rrect, const SkMatrix& matrix, SkClipOp op, bool doAA) {
    return this->op(SkPath::RRect(rrect), matrix, op, doAA);
}

#include "include/private/SkMutex.h"
#include "src/core/SkChecksum.h"
#include <cstring>

namespace {

struct AAClipPathKey {
    uint32_t fPathGenID;
    uint32_t fPathFillType;
    uint32_t fPrevIsRect;
    uint32_t fPrevAAGenID;
    SkIRect  fPrevBounds;
    uint32_t fClipOp;
    uint32_t fDoAA;
    float    fMatrix[6];

    bool operator==(const AAClipPathKey& o) const {
        return memcmp(this, &o, sizeof(*this)) == 0;
    }
};
static_assert(sizeof(AAClipPathKey) == 64, "AAClipPathKey must have no padding");

class AAClipPathCache {
public:
    static AAClipPathCache& Get() {
        static AAClipPathCache* gCache = new AAClipPathCache();
        return *gCache;
    }

    bool find(const AAClipPathKey& key, SkAAClip* outClip) {
        const uint32_t hash = SkChecksum::Hash32(&key, sizeof(key));
        const size_t setIdx = hash & (kNumSets - 1);
        SkAutoMutexExclusive lock(fMutex);
        Set& set = fSets[setIdx];
        for (size_t i = 0; i < kWays; ++i) {
            Entry& e = set.fWays[i];
            if (e.fValid && e.fHash == hash && e.fKey == key) {
                e.fLastUsed = ++fCounter;
                *outClip = e.fClip;
                return true;
            }
        }
        return false;
    }

    void insert(const AAClipPathKey& key, const SkAAClip& clip) {
        const uint32_t hash = SkChecksum::Hash32(&key, sizeof(key));
        const size_t setIdx = hash & (kNumSets - 1);
        SkAutoMutexExclusive lock(fMutex);
        Set& set = fSets[setIdx];
        size_t victim = 0;
        uint64_t oldest = UINT64_MAX;
        for (size_t i = 0; i < kWays; ++i) {
            Entry& e = set.fWays[i];
            if (!e.fValid || (e.fHash == hash && e.fKey == key)) {
                victim = i;
                break;
            }
            if (e.fLastUsed < oldest) {
                oldest = e.fLastUsed;
                victim = i;
            }
        }
        Entry& dst = set.fWays[victim];
        dst.fKey = key;
        dst.fHash = hash;
        dst.fClip = clip;
        dst.fLastUsed = ++fCounter;
        dst.fValid = true;
    }

private:
    static constexpr size_t kNumSets = 256;
    static constexpr size_t kWays = 8;

    struct Entry {
        AAClipPathKey fKey;
        uint32_t fHash = 0;
        bool fValid = false;
        uint64_t fLastUsed = 0;
        SkAAClip fClip;
    };
    struct Set {
        Entry fWays[kWays];
    };

    SkMutex fMutex;
    uint64_t fCounter = 0;
    Set fSets[kNumSets];
};

}  // namespace

bool SkRasterClip::op(const SkPath& path, const SkMatrix& matrix, SkClipOp op, bool doAA) {
    AUTO_RASTERCLIP_VALIDATE(*this);

    const bool canCache = doAA && !path.isVolatile() && !matrix.hasPerspective() &&
                          (this->isRect() || !this->isBW()) && !this->isEmpty();
    AAClipPathKey cacheKey;
    if (canCache) {
        auto canon = [](float v) { return v == 0.0f ? 0.0f : v; };
        const bool prevIsRect = this->isRect();
        cacheKey.fPathGenID = path.getGenerationID();
        cacheKey.fPathFillType = static_cast<uint32_t>(path.getFillType());
        cacheKey.fPrevIsRect = prevIsRect ? 1u : 0u;
        cacheKey.fPrevAAGenID = prevIsRect ? 0u : fAA.getGenID();
        cacheKey.fPrevBounds = this->getBounds();
        cacheKey.fClipOp = static_cast<uint32_t>(op);
        cacheKey.fDoAA = 1u;
        cacheKey.fMatrix[0] = canon(matrix.getScaleX());
        cacheKey.fMatrix[1] = canon(matrix.getSkewX());
        cacheKey.fMatrix[2] = canon(matrix.getTranslateX());
        cacheKey.fMatrix[3] = canon(matrix.getSkewY());
        cacheKey.fMatrix[4] = canon(matrix.getScaleY());
        cacheKey.fMatrix[5] = canon(matrix.getTranslateY());

        SkAAClip cachedAA;
        if (AAClipPathCache::Get().find(cacheKey, &cachedAA)) {
            fBW.setEmpty();
            fAA = cachedAA;
            fIsBW = false;
            return this->updateCacheAndReturnNonEmpty();
        }
    }

    SkPath devPath = path.makeTransform(matrix);

    // Since op is either intersect or difference, the clip is always shrinking; that means we can
    // always use our current bounds as the limiting factor for region/aaclip operations.
    if (this->isRect() && op == SkClipOp::kIntersect) {
        // However, in the relatively common case of intersecting a new path with a rectangular
        // clip, it's faster to convert the path into a region/aa-mask in place than evaluate the
        // actual intersection. See skbug.com/40043482
        if (doAA && fIsBW) {
            this->convertToAA();
        }
        if (fIsBW) {
            fBW.setPath(devPath, SkRegion(this->getBounds()));
        } else {
            fAA.setPath(devPath, this->getBounds(), doAA);
            if (canCache) {
                AAClipPathCache::Get().insert(cacheKey, fAA);
            }
        }
        return this->updateCacheAndReturnNonEmpty();
    } else {
        if (canCache) {
            if (this->isBW()) {
                this->convertToAA();
            }
            SkAAClip otherAA;
            otherAA.setPath(devPath, this->getBounds(), doAA);
            (void)fAA.op(otherAA, op);
            AAClipPathCache::Get().insert(cacheKey, fAA);
            return this->updateCacheAndReturnNonEmpty();
        }
        return this->op(SkRasterClip(devPath, this->getBounds(), doAA), op);
    }
}

bool SkRasterClip::op(sk_sp<SkShader> sh) {
    AUTO_RASTERCLIP_VALIDATE(*this);

    if (!fShader) {
        fShader = sh;
    } else {
        fShader = SkShaders::Blend(SkBlendMode::kSrcIn, sh, fShader);
    }
    return !this->isEmpty();
}

bool SkRasterClip::op(const SkRasterClip& clip, SkClipOp op) {
    AUTO_RASTERCLIP_VALIDATE(*this);
    clip.validate();

    if (this->isBW() && clip.isBW()) {
        (void)fBW.op(clip.fBW, (SkRegion::Op) op);
    } else {
        SkAAClip tmp;
        const SkAAClip* other;

        if (this->isBW()) {
            this->convertToAA();
        }
        if (clip.isBW()) {
            tmp.setRegion(clip.bwRgn());
            other = &tmp;
        } else {
            other = &clip.aaRgn();
        }
        (void)fAA.op(*other, op);
    }
    return this->updateCacheAndReturnNonEmpty();
}

void SkRasterClip::translate(int dx, int dy, SkRasterClip* dst) const {
    if (nullptr == dst) {
        return;
    }

    AUTO_RASTERCLIP_VALIDATE(*this);

    if (this->isEmpty()) {
        dst->setEmpty();
        return;
    }
    if (0 == (dx | dy)) {
        *dst = *this;
        return;
    }

    dst->fIsBW = fIsBW;
    if (fIsBW) {
        fBW.translate(dx, dy, &dst->fBW);
        dst->fAA.setEmpty();
    } else {
        fAA.translate(dx, dy, &dst->fAA);
        dst->fBW.setEmpty();
    }
    dst->updateCacheAndReturnNonEmpty();
}

void SkRasterClip::convertToAA() {
    AUTO_RASTERCLIP_VALIDATE(*this);

    SkASSERT(fIsBW);
    fAA.setRegion(fBW);
    fIsBW = false;

    // since we are being explicitly asked to convert-to-aa, we pass false so we don't "optimize"
    // ourselves back to BW.
    (void)this->updateCacheAndReturnNonEmpty(false);
}

#ifdef SK_DEBUG
void SkRasterClip::validate() const {
    // can't ever assert that fBW is empty, since we may have called forceGetBW
    if (fIsBW) {
        SkASSERT(fAA.isEmpty());
    }

    SkRegionPriv::Validate(fBW);
    fAA.validate();

    SkASSERT(this->computeIsEmpty() == fIsEmpty);
    SkASSERT(this->computeIsRect() == fIsRect);
}
#endif

///////////////////////////////////////////////////////////////////////////////

SkAAClipBlitterWrapper::SkAAClipBlitterWrapper() {
    SkDEBUGCODE(fClipRgn = nullptr;)
    SkDEBUGCODE(fBlitter = nullptr;)
}

SkAAClipBlitterWrapper::SkAAClipBlitterWrapper(const SkRasterClip& clip,
                                               SkBlitter* blitter) {
    this->init(clip, blitter);
}

SkAAClipBlitterWrapper::SkAAClipBlitterWrapper(const SkAAClip* aaclip,
                                               SkBlitter* blitter) {
    SkASSERT(blitter);
    SkASSERT(aaclip);
    fBWRgn.setRect(aaclip->getBounds());
    fAABlitter.init(blitter, aaclip);
    // now our return values
    fClipRgn = &fBWRgn;
    fBlitter = &fAABlitter;
}

void SkAAClipBlitterWrapper::init(const SkRasterClip& clip, SkBlitter* blitter) {
    SkASSERT(blitter);
    if (clip.isBW()) {
        fClipRgn = &clip.bwRgn();
        fBlitter = blitter;
    } else {
        const SkAAClip& aaclip = clip.aaRgn();
        fBWRgn.setRect(aaclip.getBounds());
        fAABlitter.init(blitter, &aaclip);
        // now our return values
        fClipRgn = &fBWRgn;
        fBlitter = &fAABlitter;
    }
}
