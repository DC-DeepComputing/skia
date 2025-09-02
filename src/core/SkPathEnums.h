/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 * This file contains private enums related to paths. See also skbug.com/40042016
 */

#ifndef SkPathEnums_DEFINED
#define SkPathEnums_DEFINED

#include "include/core/SkPathTypes.h"

enum class SkPathConvexity {
    kConvex_CW,
    kConvex_CCW,
    kConvex_Unknown,

    kConcave,

    kUnknown,
};

static inline bool SkPathConvexity_IsConvex(SkPathConvexity cv) {
    return cv == SkPathConvexity::kConvex_CW
        || cv == SkPathConvexity::kConvex_CCW
        || cv == SkPathConvexity::kConvex_Unknown;
}

static inline SkPathConvexity SkPathConvexity_OppositeConvexDirection(SkPathConvexity cv) {
    SkASSERT(SkPathConvexity_IsConvex(cv));
    switch (cv) {
        case SkPathConvexity::kConvex_CW:  cv = SkPathConvexity::kConvex_CCW; break;
        case SkPathConvexity::kConvex_CCW: cv = SkPathConvexity::kConvex_CW;  break;
        default: break;
    }
    return cv;
}

enum class SkPathFirstDirection {
    kCW,         // == SkPathDirection::kCW
    kCCW,        // == SkPathDirection::kCCW
    kUnknown,
};

static inline SkPathConvexity SkPathDirection_ToConvexity(SkPathDirection dir) {
    switch (dir) {
        case SkPathDirection::kCW:  return SkPathConvexity::kConvex_CW;
        case SkPathDirection::kCCW: return SkPathConvexity::kConvex_CCW;
    }
    SkASSERT(false);
    return SkPathConvexity::kConvex_CW;
}


static inline std::optional<SkPathDirection> SkPathConvexity_ToDirection(SkPathConvexity cv) {
    if (cv == SkPathConvexity::kConvex_CW) {
        return SkPathDirection::kCW;
    }
    if (cv == SkPathConvexity::kConvex_CCW) {
        return SkPathDirection::kCCW;
    }
    return {};
}

static inline SkPathFirstDirection SkPathDirectionToFirst(SkPathDirection dir) {
    return dir == SkPathDirection::kCW ? SkPathFirstDirection::kCW
                                       : SkPathFirstDirection::kCCW;
}

#endif
