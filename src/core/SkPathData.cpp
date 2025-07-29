/*
 * Copyright 2025 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkArc.h"
#include "include/core/SkPathTypes.h"
#include "include/core/SkRRect.h"
#include "include/core/SkSpan.h"

#include "src/core/SkPathData.h"
#include "src/core/SkPathEnums.h"
#include "src/core/SkPathPriv.h"
#include "src/core/SkPathRawShapes.h"

#include <type_traits>

namespace {
template <typename T> bool spaneq(SkSpan<T> a, SkSpan<T> b) {
    if (a.size() != b.size()) {
        return false;
    }
    for (size_t i = 0; i < a.size(); ++i) {
        if (a[i] != b[i]) {
            return false;
        }
    }
    return true;
}

template <typename T> void spancpy(SkSpan<T> dst, SkSpan<const T> src) {
    SkASSERT(dst.size() == src.size());
    memcpy(dst.data(), src.data(), src.size_bytes());
}
}

#ifdef SK_DEBUG
const uint8_t gPtsPerVerb[] = {
    1, 1, 2, 2, 3, 0,  // move, line, quad, conic, cubic, close
};

static size_t count_points(SkSpan<const SkPathVerb> vbs) {
    size_t n = 0;
    for (auto v : vbs) {
        SkASSERT((unsigned)v < sizeof(gPtsPerVerb));
        n += gPtsPerVerb[(unsigned)v];
    }
    return n;
}
#endif

const uint8_t gVerbToSegmentMask[] = {
    0,  // move
    kLine_SkPathSegmentMask,
    kQuad_SkPathSegmentMask,
    kConic_SkPathSegmentMask,
    kCubic_SkPathSegmentMask,
    0,  // close
};

// This just sets-up the spans to point inside our allocation
//
SkPathData::SkPathData(size_t npts, size_t nvbs, size_t ncns, bool isArc) {
    SkASSERT((npts == 0 && nvbs == 0 && ncns == 0) ||
             (npts != 0 && nvbs != 0));

    if (nvbs == 0) {
        SkASSERT(fPoints.empty());
        SkASSERT(fVerbs.empty());
        SkASSERT(fConics.empty());
        SkASSERT(!isArc);
    } else {
        auto data = reinterpret_cast<std::byte*>(this + 1);

        if (isArc) {
            data += sizeof(ArcData);
        }

        fPoints = {reinterpret_cast<SkPoint*>(data), npts};
        data += fPoints.size_bytes();

        fConics = {reinterpret_cast<float*>(data), ncns};
        data += fConics.size_bytes();

        fVerbs = {reinterpret_cast<SkPathVerb*>(data), nvbs};
    }

    // override these after this constructor (as needed)
    fIsA = IsA::kGeneral;
    fConvexity.store((uint8_t)SkPathConvexity::kUnknown, std::memory_order_relaxed);

    // fBounds is initialized in finishInit()
}

// NOTE: This only allocates and initializes the span pointers (points, verbs),
//       it does NOT set the other fields
sk_sp<SkPathData> SkPathData::Alloc(size_t npts, size_t nvbs, size_t ncns, bool isArc) {
    auto total_size = sizeof(SkPathData)
                    + npts * sizeof(SkPoint)
                    + ncns * sizeof(SkPoint)
                    + nvbs * sizeof(SkPathVerb)
                    + (isArc ? sizeof(ArcData) : 0);

    // This trick allows us to just make one allocation, for us and our buffer
    // rather than allocating us and also allocating the buffer (via malloc or new[])
    void* storage = ::operator new (total_size);
    sk_sp<SkPathData> path(new (storage) SkPathData(npts, nvbs, ncns, isArc));

    return path;
}

sk_sp<SkPathData> SkPathData::finishInit(const SkRect* bounds) {
    // This is being moved to SkPathPriv
    {
        unsigned mask = 0;
        for (auto v : fVerbs) {
            unsigned i = static_cast<unsigned>(v);
            SkASSERT(i < std::size(gVerbToSegmentMask));
            mask |= gVerbToSegmentMask[i];
        }
        fSegmentMask = mask;
    }

    auto const pts = this->points();

#ifdef SK_DEBUG
    size_t n = count_points(this->verbs());
    SkASSERT(this->points().size() == n);
#endif

    if (bounds) {
        fBounds = *bounds;
#ifdef SK_DEBUG
        if (pts.empty()) {
            SkASSERT(fBounds.isEmpty());
        } else {
            auto r = SkRect::Bounds(pts).value();
            SkASSERT(r == fBounds);
        }
#endif
    } else {
        if (pts.empty()) {
            fBounds = SkRect::MakeEmpty();
        } else {
            fBounds = SkRect::Bounds(pts).value();
        }
    }

    return fBounds.isFinite() ? sk_ref_sp(this) : nullptr;
}

sk_sp<SkPathData> SkPathData::transform(const SkMatrix& mx) const {
    if (mx.isIdentity() || this->empty()) {
        return sk_ref_sp(this);
    }

    sk_sp<SkPathData> path;

    // we always lose Arc-ness if we transform, so we don't have to alloc space for ArcData

    if (mx.hasPerspective()) {
        SkASSERT(false);    // todo: modify conics, subdivid cubics, ...
    } else {
        path = Alloc(this->points().size(), this->verbs().size(), this->conics().size(), false);
        mx.mapPoints(path->fPoints, this->points());
        spancpy(path->fConics, this->conics());
        spancpy(path->fVerbs,  this->verbs());
    }

    // Now deal with fIsA preservation (or not)

    if (mx.rectStaysRect() && (fIsA == IsA::kOval || fIsA == IsA::kRRect)) {
#if 0
        unsigned start = src.fRRectOrOvalStartIdx;
        bool isCCW = SkToBool(src.fRRectOrOvalIsCCW);
        transform_dir_and_start(mx, fIsA == IsA::kRRect, &isCCW, &start);

        path->fIsA            = fIsA;
        path->fIsA_isCCW      = isCCW;
        path->fISA_startIndex = start;
#endif
    } else {
        path->fIsA = IsA::kGeneral;
    }

    return path->finishInit(nullptr);
}

sk_sp<SkPathData> SkPathData::offset(SkVector v) const {
    return this->transform(SkMatrix::Translate(v));
}

size_t SkPathData::approximateBytesUsed() const {
    return sizeof(*this)
         + fPoints.size_bytes()
         + fConics.size_bytes()
         + fVerbs.size_bytes()
         + (this->getArcDataPtr() ? sizeof(ArcData) : 0);
}

bool operator==(const SkPathData& a, const SkPathData& b) {
    if (&a == &b) {
        return true;
    }

    if (a.fPoints.size() != b.fPoints.size() ||
        a.fConics.size() != b.fConics.size() ||
        a.fVerbs.size()  != b.fVerbs.size())
    {
        return false;
    }

    return  spaneq(a.fPoints, b.fPoints) &&
            spaneq(a.fConics, b.fConics) &&
            spaneq(a.fVerbs,  b.fVerbs);
}

/////////////////////////////////////

sk_sp<SkPathData> SkPathData::Make(SkSpan<const SkPoint> pts,
                                   SkSpan<const SkPathVerb> vbs,
                                   SkSpan<const float> conics,
                                   const SkRect* boundsOrNull) {
    auto path = Alloc(pts.size(), vbs.size(), conics.size(), false);

    spancpy(path->fPoints, pts);
    spancpy(path->fConics, conics);
    spancpy(path->fVerbs,  vbs);

    return path->finishInit(nullptr);
}

sk_sp<SkPathData> SkPathData::Empty() {
    static sk_sp<SkPathData> gEmpty;
    if (!gEmpty) {
        gEmpty = Make({}, {}, {}, nullptr);
    }
    return gEmpty;
}

void SkPathData::initIsAClosed(IsA isa, SkPathDirection dir, unsigned index) {
    fConvexity.store((uint8_t)SkPathConvexity::kConvex, std::memory_order_relaxed);

    SkASSERT(isa == IsA::kOval || isa == IsA::kRRect);
    fIsA = isa;

    fIsA_isCCW = (dir == SkPathDirection::kCCW);
    SkASSERT((index & 0xF) == 0);
    fIsA_startIndex = index;
}

sk_sp<SkPathData> SkPathData::Rect(const SkRect& r, SkPathDirection dir, unsigned index) {
    SkPathRawShapes::Rect raw(r, dir, index);
    return Make(raw.points(), raw.verbs(), raw.conics(), &raw.fBounds);
}

sk_sp<SkPathData> SkPathData::Oval(const SkRect& r, SkPathDirection dir, unsigned index) {
    SkPathRawShapes::Oval raw(r, dir, index);
    auto path = Make(raw.points(), raw.verbs(), raw.conics(), &raw.fBounds);

    path->initIsAClosed(IsA::kOval, dir, index);
    return path;
}

sk_sp<SkPathData> SkPathData::RRect(const SkRRect& r, SkPathDirection dir, unsigned index) {
    SkPathRawShapes::RRect raw(r, dir, index);
    auto path = Make(raw.points(), raw.verbs(), raw.conics(), &raw.fBounds);

    path->initIsAClosed(IsA::kRRect, dir, index);
    return path;
}

sk_sp<SkPathData> SkPathData::Polygon(SkSpan<const SkPoint> pts, bool isClosed) {
    if (pts.size() == 0) {
        return Empty();
    }

    const size_t nverbs = pts.size() + isClosed;    // +1 for the kClose verb
    const size_t nconics = 0;
    auto path = Alloc(pts.size(), nverbs, nconics, false);

    spancpy(path->fPoints, pts);

    path->fVerbs[0] = SkPathVerb::kMove;
    for (size_t i = 1; i < pts.size(); ++i) {
        path->fVerbs[i] = SkPathVerb::kLine;
    }
    if (isClosed) {
        path->fVerbs.back() = SkPathVerb::kClose;
    }

    return path->finishInit(nullptr);
}

sk_sp<SkPathData> SkPathData::Arc(const SkArc& arc) {
    // Need shared code (path, builder, etc.) for computing pts/verbs
    // Need Alloc to know to allocate extra space for ArcData{}
    // Handle sweepangle == 0 specially?
    return nullptr;
}

/////////////////////////////////////

std::optional<SkArc> SkPathData::asArc() const {
    if (auto data = this->getArcDataPtr()) {
        return SkArc::Make(data->fOvalBounds,
                           data->fStartAngle,
                           data->fSweepAngle,
                           fIsA == IsA::kArcWedge);
    }
    return {};
}

std::optional<std::array<SkPoint, 2>> SkPathData::asLine() const {
    if (fPoints.size() == 2 &&
        fVerbs.size() == 2 &&
        fVerbs[0] == SkPathVerb::kMove && fVerbs[1] == SkPathVerb::kLine &&
        fConics.size() == 0)
    {
        return {{ fPoints[0], fPoints[1] }};
    }
    return {};
}

std::optional<SkPathRectInfo> SkPathData::asRect() const {
    if (auto rc = SkPathPriv::IsRectContour(fPoints, fVerbs, false)) {
        SkASSERT(rc->fRect == fBounds);
        return {{
            fBounds,
            rc->fDirection,
            0, // start index???
        }};
    }
    return {};
}

std::optional<SkPathOvalInfo> SkPathData::asOval() const {
    if (fIsA == IsA::kOval) {
        return {{
            fBounds,
            fIsA_isCCW ? SkPathDirection::kCCW : SkPathDirection::kCW,
            fIsA_startIndex,
        }};
    }
    return {};
}

std::optional<SkPathRRectInfo> SkPathData::asRRect() const {
    if (fIsA == IsA::kRRect) {
        return {{
            SkPathPriv::DeduceRRectFromContour(fBounds, fPoints, fVerbs),
            fIsA_isCCW ? SkPathDirection::kCCW : SkPathDirection::kCW,
            fIsA_startIndex,
        }};
    }
    return {};
}

