/*
 * Copyright 2025 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkPathTypes.h"
#include "include/core/SkRRect.h"
#include "include/core/SkSpan.h"

#include "src/core/SkPathData.h"
#include "src/core/SkPathEnums.h"
#include "src/core/SkPathPriv.h"
#include "src/core/SkPathRawShapes.h"

#include "include/private/base/SkFloatingPoint.h"

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

const uint8_t gPtsPerVerb[] = {
    1, 1, 2, 2, 3, 0,  // move, line, quad, conic, cubic, close
};

static std::pair<size_t, size_t> count_pts_cns(SkSpan<const SkPathVerb> vbs) {
    size_t pts = 0,
           cns = 0;
    for (auto v : vbs) {
        SkASSERT((unsigned)v < sizeof(gPtsPerVerb));
        pts += gPtsPerVerb[(unsigned)v];
        cns += (v == SkPathVerb::kConic);
    }
    return {pts, cns};
}

// If it detects a single trailing Move verb, remove it and it's corresponding point.
// Otherwise return leave the spans unchanged.
//
static void trim_trailing_move(SkSpan<const SkPoint>* pts, SkSpan<const SkPathVerb>* vbs) {
    if (!vbs->empty() && vbs->back() == SkPathVerb::kMove) {
        *vbs = vbs->first(vbs->size() - 1);
        *pts = pts->first(pts->size() - 1);
    }
}

static bool valid_verbs(SkSpan<const SkPathVerb> vbs) {
    if (vbs.size() == 0) {
        return true;
    }

    // We must begin with a Move (unless we're empty)
    if (vbs.front() != SkPathVerb::kMove) {
        return false;
    }
    SkPathVerb prev = SkPathVerb::kMove;

    /*
     *  Check that we have a valid sequence.
     */
    for (size_t i = 1; i < vbs.size(); ++i) {
        SkPathVerb curr = vbs[i];

        if (static_cast<unsigned>(curr) > static_cast<unsigned>(SkPathVerb::kLast_Verb)) {
            return false;
        }

        // Previous verb             Valid next verb
        // -----------------------------------------
        // Move                  --> not Move
        // Line/Quad/Conic/Cubic --> *
        // Close                 --> Move
        //
        if (prev == SkPathVerb::kMove && curr == SkPathVerb::kMove) {
            return false;
        }
        if (prev == SkPathVerb::kClose && curr != SkPathVerb::kMove) {
            return false;
        }
        prev = curr;
    }

    // A trailing Move is also illegal, since it creates an empty contour,
    // which will confuse some of our callers
    //
    // See trim_trailing_move() helper, which may be called before calling us.
    //
    return vbs.back() != SkPathVerb::kMove;
}

// Handing in debugging, to set a break-point here if you want to know why we
// failed to create a pathdta
//
static void report_pathdata_make_failure(const char reason[]) {
//  `    SkDEBUGF("SkPathData::Make failed: %s\n", reason);
}

// This just sets-up the spans to point inside our allocation
//
SkPathData::SkPathData(size_t npts, size_t nvbs, size_t ncns) {
    SkASSERT((npts == 0 && nvbs == 0 && ncns == 0) ||
             (npts != 0 && nvbs != 0));

    if (nvbs == 0) {
        SkASSERT(fPoints.empty());
        SkASSERT(fVerbs.empty());
        SkASSERT(fConics.empty());
    } else {
        auto data = reinterpret_cast<std::byte*>(this + 1);

        fPoints = {reinterpret_cast<SkPoint*>(data), npts};
        data += fPoints.size_bytes();

        fConics = {reinterpret_cast<float*>(data), ncns};
        data += fConics.size_bytes();

        fVerbs = {reinterpret_cast<SkPathVerb*>(data), nvbs};
    }

    // override these after this constructor (as needed)
    fType = SkPathIsAType::kGeneral;
    fConvexity.store((uint8_t)SkPathConvexity::kUnknown, std::memory_order_relaxed);

    // fBounds is initialized in finishInit()
}

// NOTE: This only allocates and initializes the span pointers (points, verbs),
//       it does NOT set the other fields
sk_sp<SkPathData> SkPathData::Alloc(size_t npts, size_t nvbs, size_t ncns) {
    auto total_size = sizeof(SkPathData)
                    + npts * sizeof(SkPoint)
                    + ncns * sizeof(SkPoint)
                    + nvbs * sizeof(SkPathVerb);

    // This trick allows us to just make one allocation, for us and our buffer
    // rather than allocating us and also allocating the buffer (via malloc or new[])
    void* storage = ::operator new (total_size);
    sk_sp<SkPathData> path(new (storage) SkPathData(npts, nvbs, ncns));

    return path;
}

sk_sp<SkPathData> SkPathData::finishInit(std::optional<SkRect> bounds,
                                         std::optional<uint8_t> segmentMask) {
    if (fPoints.empty()) {
        fBounds = SkRect::MakeEmpty();
        fSegmentMask = 0;
        return sk_ref_sp(this);
    }

    if (segmentMask.has_value()) {
        fSegmentMask = segmentMask.value();
    } else {
        fSegmentMask = SkPathPriv::ComputeSegmentMask(fVerbs);
    }

    if (bounds.has_value()) {
        fBounds = bounds.value();
        SkASSERT(SkIsFinite(&fPoints.data()->fX, fPoints.size() * 2));
    } else {
        if (auto r = SkRect::Bounds(fPoints)) {
            fBounds = r.value();
        } else {
            report_pathdata_make_failure("non-finite bounds");
            return nullptr;
        }
    }

    return sk_ref_sp(this);
}

sk_sp<SkPathData> SkPathData::transform(const SkMatrix& mx) const {
    if (mx.isIdentity() || this->empty()) {
        return sk_ref_sp(this);
    }

    if (mx.hasPerspective()) {
        SkPathBuilder bu;
        bu.addRaw(this->raw(SkPathFillType::kDefault));
        bu.transform(mx);
        return bu.detachData();
    }

    // Allocate our result, so we can map the new points directly into it
    auto result = Alloc(this->points().size(), this->verbs().size(), this->conics().size());
    mx.mapPoints(result->fPoints, this->points());
    spancpy(result->fConics, this->conics());
    spancpy(result->fVerbs,  this->verbs());

    std::optional<SkRect> transformedBounds;
    if (mx.rectStaysRect()) {
        // safe us from having to compute our transformed bounds in finishInit()
        transformedBounds = mx.mapRect(fBounds);
        if (!transformedBounds.value().isFinite()) {
            report_pathdata_make_failure("transform created non-finite bounds");
            return nullptr;
        }
    }

    if (!result->finishInit(transformedBounds, fSegmentMask)) {
        return nullptr;
    }

    // result is ready to go -- but now we see if we can maintian our IsA status ...

    if ((fType == SkPathIsAType::kOval || fType == SkPathIsAType::kRRect) &&
        mx.rectStaysRect() && SkPathPriv::IsAxisAligned(fPoints))
    {
        auto [dir, start] =
        SkPathPriv::TransformDirAndStart(mx, fType == SkPathIsAType::kRRect,
                                         fIsA.fDirection, fIsA.fStartIndex);
        result->setupIsA(fType, dir, start);
    }
    return result;
}

sk_sp<SkPathData> SkPathData::offset(SkVector v) const {
    return this->transform(SkMatrix::Translate(v));
}

size_t SkPathData::approximateBytesUsed() const {
    return sizeof(*this)
         + fPoints.size_bytes()
         + fConics.size_bytes()
         + fVerbs.size_bytes();
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

sk_sp<SkPathData> SkPathData::MakeNoCheck(SkSpan<const SkPoint> pts,
                                          SkSpan<const SkPathVerb> vbs,
                                          SkSpan<const float> conics,
                                          std::optional<SkRect> bounds,
                                          std::optional<unsigned> segmentMask) {
    trim_trailing_move(&pts, &vbs);
    SkASSERT(valid_verbs(vbs));

    auto path = Alloc(pts.size(), vbs.size(), conics.size());

    spancpy(path->fPoints, pts);
    spancpy(path->fConics, conics);
    spancpy(path->fVerbs,  vbs);

    return path->finishInit(bounds, segmentMask);
}

sk_sp<SkPathData> SkPathData::MakeNoCheck(const SkPathRaw& raw) {
    return MakeNoCheck(raw.points(), raw.verbs(), raw.conics(), raw.fBounds, raw.fSegmentMask);
}

sk_sp<SkPathData> SkPathData::Empty() {
    static sk_sp<SkPathData> gEmpty;
    if (!gEmpty) {
        gEmpty = MakeNoCheck({}, {}, {}, {}, {});
    }
    return gEmpty;
}

void SkPathData::setupIsA(SkPathIsAType type, SkPathDirection dir, unsigned index) {
    fConvexity.store((uint8_t)SkPathConvexity::kConvex, std::memory_order_relaxed);

    SkASSERT(type == SkPathIsAType::kOval || type == SkPathIsAType::kRRect);
    fType = type;

    fIsA.fDirection  = dir;
    fIsA.fStartIndex = SkTo<uint8_t>(index);
}

sk_sp<SkPathData> SkPathData::Rect(const SkRect& r, SkPathDirection dir, unsigned index) {
    if (!r.isFinite()) {
        return nullptr;
    }
    SkPathRawShapes::Rect raw(r, dir, index);
    return MakeNoCheck(raw.points(), raw.verbs(), raw.conics(), raw.fBounds, raw.fSegmentMask);
}

sk_sp<SkPathData> SkPathData::Oval(const SkRect& r, SkPathDirection dir, unsigned index) {
    if (!r.isFinite()) {
        return nullptr;
    }
    SkPathRawShapes::Oval raw(r, dir, index);
    auto path = MakeNoCheck(raw.points(), raw.verbs(), raw.conics(), raw.fBounds, raw.fSegmentMask);

    path->setupIsA(SkPathIsAType::kOval, dir, index);
    return path;
}

sk_sp<SkPathData> SkPathData::RRect(const SkRRect& r, SkPathDirection dir, unsigned index) {
    if (!r.isValid()) {
        return nullptr;
    }
    SkPathRawShapes::RRect raw(r, dir, index);
    auto path = MakeNoCheck(raw.points(), raw.verbs(), raw.conics(), raw.fBounds, raw.fSegmentMask);

    path->setupIsA(SkPathIsAType::kRRect, dir, index);
    return path;
}

sk_sp<SkPathData> SkPathData::Polygon(SkSpan<const SkPoint> pts, bool isClosed) {
    if (pts.size() == 0 || (pts.size() == 1 && !isClosed)) {
        return Empty();
    }

    const size_t nverbs = pts.size() + isClosed;    // +1 for the kClose verb
    const size_t nconics = 0;
    auto path = Alloc(pts.size(), nverbs, nconics);

    spancpy(path->fPoints, pts);

    path->fVerbs[0] = SkPathVerb::kMove;
    for (size_t i = 1; i < pts.size(); ++i) {
        path->fVerbs[i] = SkPathVerb::kLine;
    }
    if (isClosed) {
        path->fVerbs.back() = SkPathVerb::kClose;
    }

    return path->finishInit({}, kLine_SkPathSegmentMask);
}

/////////////////////////////////////

bool SkPathData::isConvex() const {
    return fConvexity.load(std::memory_order_relaxed) == (uint8_t)SkPathConvexity::kConvex;
}

SkPathRaw SkPathData::raw(SkPathFillType ft) const {
    return {
        fPoints,
        fVerbs,
        fConics,
        fBounds,
        ft,
        this->isConvex(),
        fSegmentMask,
    };
}

std::optional<std::array<SkPoint, 2>> SkPathData::asLine() const {
    if (fPoints.size() == 2 && fVerbs.size() == 2 && fConics.size() == 0 &&
        fVerbs[1] == SkPathVerb::kLine)
    {
        SkASSERT(fVerbs[0] == SkPathVerb::kMove);
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
    if (fType == SkPathIsAType::kOval) {
        return {{
            fBounds,
            fIsA.fDirection,
            fIsA.fStartIndex,
        }};
    }
    return {};
}

std::optional<SkPathRRectInfo> SkPathData::asRRect() const {
    if (fType == SkPathIsAType::kRRect) {
        return {{
            SkPathPriv::DeduceRRectFromContour(fBounds, fPoints, fVerbs),
            fIsA.fDirection,
            fIsA.fStartIndex,
        }};
    }
    return {};
}

/////////////////

sk_sp<SkPathData> SkPathData::Make(SkSpan<const SkPoint> pts,
                                   SkSpan<const SkPathVerb> vbs,
                                   SkSpan<const float> conics) {
    trim_trailing_move(&pts, &vbs);
    if (!valid_verbs(vbs)) {
        report_pathdata_make_failure("invalid verb sequence");
        return nullptr;
    }

    auto [npts, ncns] = count_pts_cns(vbs);
    if (pts.size() != npts || conics.size() != ncns) {
        report_pathdata_make_failure("unexpected # points or conics");
        return nullptr;
    }

    // Now we can check for a dangling kMove verb, and just ignore it
    if (!vbs.empty() && vbs.back() == SkPathVerb::kMove) {
        SkASSERT(!pts.empty());
        vbs = vbs.first(vbs.size() - 1);
        pts = pts.first(pts.size() - 1);
    }
    if (vbs.empty()) {
        return Empty();
    }

    if (!conics.empty() && !SkIsFinite(conics.data(), conics.size())) {
        report_pathdata_make_failure("non-finite conics");
        return nullptr;
    }

    // MakeNoCheck *does* compute/check bounds if we don't pass them in
    return MakeNoCheck(pts, vbs, conics, {}, {});
}
