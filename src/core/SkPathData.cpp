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

// This just sets-up the spans to point inside our allocation
//
SkPathData::SkPathData(size_t npts, size_t nvbs, size_t ncns) {
    SkASSERT(npts == 0 && nvbs == 0 && ncns == 0 ||
             npts != 0 && nvbs != 0);

    if (nvbs == 0) {
        SkASSERT(fPoints.empty());
        SkASSERT(fVerbs.empty());
        SkASSERT(fConics.empty());
    } else {
        std::byte* data = reinterpret_cast<std::byte*>(this + 1);

        fPoints = {reinterpret_cast<SkPoint*>(data), npts};
        data += fPoints.size_bytes();

        fConics = {reinterpret_cast<SkPoint*>(data), ncns};
        data += fConics.size_bytes();

        fVerbs = {reinterpret_cast<SkPathVerb*>(data), nvbs};
    }
    // fBounds is initialized in finishInit()
}

// NOTE: This only allocates and initializes the span pointers (points, verbs),
//       it does NOT set the other fields
sk_sp<SkPathData> SkPathData::Alloc(size_t npts, size_t nvbs, size_t ncns) {
    auto total_size = sizeof(Path)
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

    mx.map(path->writable_points(), pts);
    memcpy(path->writable_verbs().data(), vbs.data(), vbs.size_bytes());
    memcpy(path->writable_conics().data(), conics.data(), conics.size_bytes());

    return path->finishInit(bounds) ? path : nullptr;
}

bool SkPathData::finishInit(const SkRect* bounds) {
    const auto pts = this->points();

#ifdef SK_DEBUG
    size_t n = count_points(this->verbs());
    SkASSERT(pts.size() == n);
#endif

    if (bounds) {
        fBounds = *bounds;
#ifdef SK_DEBUG
        if (pts.size()) {
            auto r = SkRect::Bounds(this->points()).value();
            SkASSERT(r == fBounds);
        } else {
            SkASSERT(fBounds.isEmpty());
        }
#endif
    } else {
        if (pts.size()) {
            fBounds = SkRect::Bounds(pts);
        } else {
            fBounds = SkRect::MakeEmpty();
        }
    }
    return fBounds.isFinite();
}

sk_sp<SkPathData> SkPathData::transform(const SkMatrix& mx) const {
    if (mx.isIdentity()) {
        return sk_ref_sp(this);
    }

    auto path = Alloc(this->points().size(), this->verbs().size(), this->conics().size());
    mx.map(path->writable_points(), this->points());
    memcpy(path->writable_verbs().data(), this->verbs().data(), this->verbs().size_bytes());
    memcpy(path->writable_conics().data(), this->conics().data(), this->conics().size_bytes());

    return path->finishInit(nullptr) ? path : nullptr;
}

sk_sp<SkPathData> SkPathData::offset(SkVector v) const {
    return this->transform(SkMatrix::Translate(v));
}
