
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkPathAndroid_DEFINED
#define SkPathAndroid_DEFINED

#include "include/core/SkPath.h"

#include <optional>

class SkPathAndroid {
public:
    static bool CanInterpolate(const SkPath&, const SkPath&);

    static std::optional<SkPath> Interpolate(const SkPath& from,
                                             const SkPath& to,
                                             float t);
};

#endif
