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

#include <type_traits>

template <typename T> void spancpy(SkSpan<T> dst, SkSpan<const T> src) {
    SkASSERT(dst.size() == src.size());
    memcpy(dst.data(), src.data(), src.size_bytes());
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

sk_sp<SkPathData> SkPathData::Make(SkSpan<const SkPoint> pts,
                                   SkSpan<const SkPathVerb> vbs,
                                   SkSpan<const float> conics,
                                   const SkRect* boundsOrNull) {
    auto path = Alloc(pts.size(), vbs.size(), conics.size());

    spancpy(path->fPoints, pts);
    spancpy(path->fConics, conics);
    spancpy(path->fVerbs,  vbs);

    return path->finishInit(boundsOrNull) ? path : nullptr;
}

bool SkPathData::finishInit(const SkRect* bounds) {
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
            fBounds = SkRect::Bounds(pts).value();
        } else {
            fBounds = SkRect::MakeEmpty();
        }
    }
    return fBounds.isFinite();
}

sk_sp<SkPathData> SkPathData::transform(const SkMatrix& mx) const {
    if (mx.isIdentity() || this->isEmpty()) {
        return sk_ref_sp(this);
    }

    sk_sp<SkPathData> path;

    if (mx.hasPerspective()) {
        SkASSERT(false);    // todo: modify conics, subdivid cubics, ...
    } else {
        path = Alloc(this->points().size(), this->verbs().size(), this->conics().size());
        mx.mapPoints(path->fPoints, this->points());
        spancpy(path->fConics, this->conics());
        spancpy(path->fVerbs,  this->verbs());
    }

    return path->finishInit(nullptr) ? path : nullptr;
}

sk_sp<SkPathData> SkPathData::offset(SkVector v) const {
    return this->transform(SkMatrix::Translate(v));
}

/////////////////////////////////////

sk_sp<SkPathData> SkPathData::Empty() {
    static sk_sp<SkPathData> gEmpty;
    if (gEmpty) {
        gEmpty = Make({}, {}, {}, nullptr);
    }
    return gEmpty;
}

sk_sp<SkPathData> SkPathData::Polygon(SkSpan<const SkPoint> pts, bool isClosed) {
    if (pts.size() == 0) {
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

    return path->finishInit(nullptr) ? path : nullptr;
}
