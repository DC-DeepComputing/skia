
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
    /**
     *  Returns true if the two paths can be interpolated.
     *
     *  Returns true iff the paths have the same verbs and conicWeights,
     *  and the same number of points.
     */
    static bool CanInterpolate(const SkPath&, const SkPath&);

    /**
     *  Return a path whose points are lineraly interpolated between from and to,
     *  using t (typically between 0 and 1).
     *
     *  (logically) newPoint = (1 - t) * fromPoint + t * toPoint
     *
     *  The result has the same fillType as the 'from' path.
     *
     *  If the two paths are not interpolatable, this returns {}.
     */
    static std::optional<SkPath> Interpolate(const SkPath& from,
                                             const SkPath& to,
                                             float t);
};

#endif
