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

<<<<<<< PATCH SET (a9c989 Proof of concept Skia Buffet build)
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

=======
>>>>>>> BASE      (9423a8 Roll vulkan-deps from 61636505d609 to 7ed449387d3b (1 revisi)
#endif
