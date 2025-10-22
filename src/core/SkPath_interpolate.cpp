/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/android/SkPathAndroid.h"
#include "include/core/SkPath.h"

#ifdef SK_LEGACY_PATH_INTERPOLATE

bool SkPath::isInterpolatable(const SkPath& compare) const {
    return SkPathAndroid::CanInterpolate(*this, compare);
}

SkPath SkPath::makeInterpolate(const SkPath& ending, SkScalar weight) const {
    return SkPathAndroid::Interpolate(ending, *this, weight).value_or(SkPath());
}

bool SkPath::interpolate(const SkPath& ending, SkScalar weight, SkPath* out) const {
    if (auto result = SkPathAndroid::Interpolate(ending, *this, weight)) {
        *out = *result;
        return true;
    }
    return false;
}

#endif
