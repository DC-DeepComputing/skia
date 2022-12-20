/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 * This file contains private enums related to paths. See also skbug.com/10670
 */

#ifndef SkPathEnums_DEFINED
#define SkPathEnums_DEFINED

enum class SkPathConvexity {
    kConvex,
    kConcave,
    kUnknown,
};

enum class SkPathFirstDirection {
    kCW,         // == SkPathDirection::kCW
    kCCW,        // == SkPathDirection::kCCW
    kUnknown,
};

namespace skpathenums {
    /**
     *  Return the opposite of the specified direction. kUnknown is its own
     *  opposite.
     */
    inline SkPathFirstDirection OppositeFirstDirection(SkPathFirstDirection dir) {
        static const SkPathFirstDirection gOppositeDir[] = {
                SkPathFirstDirection::kCCW, SkPathFirstDirection::kCW,
                SkPathFirstDirection::kUnknown,
        };
        return gOppositeDir[(unsigned) dir];
    }
}

#endif
