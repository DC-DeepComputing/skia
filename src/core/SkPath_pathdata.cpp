/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 * This file contains private enums related to paths. See also skbug.com/40042016
 */

#include "include/core/SkPath.h"
#include "include/core/SkPathTypes.h"
#include "include/core/SkSpan.h"
#include "src/core/SkPathEnums.h"
#include "src/core/SkPathPriv.h"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <limits.h>
#include <utility>

/*  Contains path methods that require SkPathData -- not SkPathRef:
 *
 *  The remaining fields:
 *  - fFillType
 *  - fIsVolatile
 *
 *  ... are shared in both implemtations.
 */

#ifdef SK_PATH_USES_PATHDATA

static const SkPathData* peek_error_pathdata_singleton() {
    static const SkPathData* gErrorSingleton = SkPathData::Empty(true).release();

    SkASSERT(gErrorSingleton != SkPathData::Empty().get());
    return gErrorSingleton;
}

static sk_sp<SkPathData> make_error_pathdata() {
    return sk_ref_sp(peek_error_pathdata_singleton());
}

SkPath SkPath::MakeNullCheck(sk_sp<SkPathData> pdata, SkPathFillType ft, bool isVolatile) {
    if (!pdata) {
        pdata = make_error_pathdata();
    }
    return SkPath(std::move(pdata), ft, isVolatile);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

SkPath::SkPath(sk_sp<SkPathData> pd, SkPathFillType ft, bool isVolatile)
    : fPathData(std::move(pd))
    , fFillType(ft)
    , fIsVolatile(isVolatile)
{
    SkASSERT(fPathData);
}

SkPath::SkPath(SkPathFillType ft)
    : fPathData(SkPathData::Empty())
    , fFillType(ft)
    , fIsVolatile(false)
{}

SkPath::SkPath(const SkPath& that)
    : fPathData(that.fPathData)
    , fFillType(that.fFillType)
    , fIsVolatile(that.fIsVolatile)
{}

SkPath& SkPath::operator=(const SkPath& o) {
    if (this != &o) {
        fPathData   = o.fPathData;
        fFillType   = o.fFillType;
        fIsVolatile = o.fIsVolatile;
    }
    return *this;
}

// This is the public-facing non-const setConvexity().
void SkPath::setConvexity(SkPathConvexity c) {
    fPathData->setConvexity(c);
}

// Const hooks for working with fConvexity and fFirstDirection from const methods.
void SkPath::setConvexity(SkPathConvexity c) const {
    fPathData->setConvexity(c);
}

SkPathConvexity SkPath::getConvexityOrUnknown() const {
    return fPathData->getConvexityOrUnknown();
}

bool operator==(const SkPath& a, const SkPath& b) {
    return &a == &b ||
          (a.fFillType == b.fFillType && *a.fPathData == *b.fPathData);
}

void SkPath::swap(SkPath& that) {
    if (this != &that) {
        fPathData.swap(that.fPathData);
        std::swap(fFillType, that.fFillType);
        std::swap(fIsVolatile, that.fIsVolatile);
    }
}

SkPath& SkPath::reset() {
    *this = SkPath();
    return *this;
}

const SkRect& SkPath::getBounds() const {
    return fPathData->bounds();
}

uint32_t SkPath::getSegmentMasks() const {
    return fPathData->segmentMask();
}

bool SkPath::isFinite() const {
    return fPathData.get() != peek_error_pathdata_singleton();
}

bool SkPath::isValid() const { return this->isFinite(); }

bool SkPath::hasComputedBounds() const { return true; }

uint32_t SkPath::getGenerationID() const { return fPathData->uniqueID(); }

#ifdef SK_DEBUG
void SkPath::validate() const {}

void SkPath::validateRef() const {}
#endif

std::optional<SkPathOvalInfo> SkPath::getOvalInfo() const { return fPathData->asOval(); }
std::optional<SkPathRRectInfo> SkPath::getRRectInfo() const { return fPathData->asRRect(); }

SkSpan<const SkPoint> SkPath::points() const { return fPathData->points(); }
SkSpan<const SkPathVerb> SkPath::verbs() const { return fPathData->verbs(); }
SkSpan<const float> SkPath::conicWeights() const { return fPathData->conics(); }

//////////////////////////////////////////////////////////////////////////////////////////////////

SkPath SkPath::Raw(SkSpan<const SkPoint> pts, SkSpan<const SkPathVerb> vbs,
                   SkSpan<const float> ws, SkPathFillType ft, bool isVolatile) {
    return MakeNullCheck(SkPathData::Make(pts, vbs, ws), ft, isVolatile);
}

SkPath SkPath::Rect(const SkRect& r, SkPathFillType ft, SkPathDirection dir, unsigned startIndex) {
    return MakeNullCheck(SkPathData::Rect(r, dir, startIndex), ft, false);
}

SkPath SkPath::Oval(const SkRect& r, SkPathDirection dir, unsigned startIndex) {
    return MakeNullCheck(SkPathData::Oval(r, dir, startIndex), SkPathFillType::kDefault, false);
}

SkPath SkPath::RRect(const SkRRect& rr, SkPathDirection dir, unsigned startIndex) {
    return MakeNullCheck(SkPathData::RRect(rr, dir, startIndex), SkPathFillType::kDefault, false);
}

SkPath SkPath::Polygon(SkSpan<const SkPoint> pts, bool isClosed,
                       SkPathFillType ft, bool isVolatile) {
    return MakeNullCheck(SkPathData::Polygon(pts, isClosed), ft, isVolatile);
}

std::optional<SkPath> SkPath::tryMakeTransform(const SkMatrix& matrix) const {
    if (auto pdata = fPathData->makeTransform(matrix)) {
        return SkPath(std::move(pdata), fFillType, fIsVolatile);
    }
    return {};
}

SkPath SkPath::makeTransform(const SkMatrix& matrix) const {
    if (auto newpath = this->tryMakeTransform(matrix)) {
        return *newpath;
    }
    return SkPath(make_error_pathdata(), fFillType, false);
}

std::optional<SkPathRaw> SkPath::raw(SkResolveConvexity rc) const {
    return fPathData->raw(fFillType, rc);
}

//////////////////////////////////////////////////////////////////////////////////////////////

int SkPathPriv::GenIDChangeListenersCount(const SkPath& path) {
#ifdef SK_PATH_USES_PATHDATA
    //return path.fPathData->genIDChangeListenerCount();
    return 0;
#else
    return path.fPathRef->genIDChangeListenerCount();
#endif
}

/** Returns true if the underlying SkPathRef has one single owner. */
bool SkPathPriv::TestingOnly_unique(const SkPath& path) {
#ifdef SK_PATH_USES_PATHDATA
    return path.fPathData->unique();
#else
    return path.fPathRef->unique();
#endif
}

void SkPathPriv::AddGenIDChangeListener(const SkPath& path, sk_sp<SkIDChangeListener> listener) {
#ifdef SK_PATH_USES_PATHDATA
    // todo
#else
    path.fPathRef->addGenIDChangeListener(std::move(listener));
#endif
}

//////////////////////////////////////////////////////////////////////////////////////////////

SkPathBuilder& SkPathBuilder::operator=(const SkPath& src) {
    this->reset().setFillType(src.getFillType());
    this->setIsVolatile(src.isVolatile());

    if (src.isEmpty()) {
        return *this;
    }

    const SkPathData& pdata = *src.fPathData;

    this->addRaw(pdata.raw(src.getFillType(), SkResolveConvexity::kYes));

    // These are not part of SkPathRaw, so we set them separately

    fLastMoveIndex = SkPathPriv::FindLastMoveToIndex(pdata.verbs(), pdata.points().size());
    fType = pdata.fType;
    fIsA  = pdata.fIsA;

    return *this;
}

SkPath SkPathBuilder::snapshot(const SkMatrix* mx) const {
    if (!mx) {
        mx = &SkMatrix::I();
    }

    sk_sp<SkPathData> pdata;
    if (auto raw = SkPathPriv::Raw(*this, SkResolveConvexity::kYes)) {
        pdata = SkPathData::MakeTransform(*raw, *mx);
    }
    if (pdata && fType != SkPathIsAType::kGeneral) {
        pdata->setupIsA(fType, fIsA.fDirection, fIsA.fStartIndex);
    }
    return SkPath::MakeNullCheck(std::move(pdata), fFillType, false);
}

#endif
