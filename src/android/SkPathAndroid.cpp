/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/android/SkPathAndroid.h"
#include "include/core/SkPath.h"
#include "src/core/SkSpanPriv.h"

#include <vector>

bool SkPathAndroid::CanInterpolate(const SkPath& from, const SkPath& to) {
    return from.points().size() == to.points().size() &&
           SkSpanPriv::EQ(from.verbs(), to.verbs()) &&
           SkSpanPriv::EQ(from.conicWeights(), to.conicWeights());
}

static SkPoint operator*(float s, SkPoint p) {
    return {
        p.fX * s,
        p.fY * s
    };
}

static SkPoint lerp(SkPoint from, SkPoint to, float t) {
    return from + t * (to - from);
}

std::optional<SkPath> SkPathAndroid::Interpolate(const SkPath& from, const SkPath& to, float t) {
    if (!CanInterpolate(from, to)) {
        return {};
    }

    // note, if from and to have different fill-types, that's up to the client to deal with.
    const SkPathFillType fillType = from.getFillType();

    const SkSpan<const SkPoint> fromPts = from.points(),
                                  toPts = to.points();

    std::vector<SkPoint> dst(fromPts.size());

    for (size_t i = 0; i < fromPts.size(); ++i) {
        dst[i] = lerp(fromPts[i], toPts[i], t);
    }
    return SkPath::Raw({dst.data(), dst.size()}, from.verbs(), from.conicWeights(), fillType);
}
