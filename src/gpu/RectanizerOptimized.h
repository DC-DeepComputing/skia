/*
 * Copyright 2022 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_RectanizerOptimized_DEFINED
#define skgpu_RectanizerOptimized_DEFINED

#include "include/private/SkTDArray.h"
#include "src/gpu/RectanizerSkyline.h"
#include "src/utils/SkBitSet.h"
#include "include/private/SkBitmaskEnum.h"

namespace skgpu {
enum class OptFlags {
    None = 0x00,
    T = 0x01,
    L = 0x02,
    R = 0x04,
    TL = 0x03,
    TR = 0x05,
    LR = 0x06,
    TLR = 0x07,
};
}

namespace sknonstd {
template <> struct is_bitmask_enum<skgpu::OptFlags> : std::true_type {};
}

namespace skgpu {
// Pack rectangles and track the current silhouette
// Based, in part, on Jukka Jylanki's work at http://clb.demon.fi
//
// Mark this class final in an effort to avoid the vtable when this subclass is used explicitly.
class RectanizerOptimized final : public RectanizerSkyline {
public:
    RectanizerOptimized(int w, int h) : RectanizerSkyline(w, h) {
        this->reset();
    }

    ~RectanizerOptimized() final { }

    void reset() final {
        RectanizerSkyline::reset();
        fBottom.clear();
        fBottom.emplace_back(std::vector<bool>());
    }

    bool addRectOptimized(int w, int h, int zeroPadding, SkIPoint16* loc);

    void printProfile() const;

private:
    // Can a width x height rectangle fit in the free space represented by
    // the skyline segments >= 'skylineIndex'? If so, return true and fill in
    // 'y' with the y-location at which it fits (the x location is pulled from
    // 'skylineIndex's segment.
    // We need to tak in account a chance to optimize padding, so we have to check all
    // possible variants in case we find the one that has zeros already
    int rectangleFits(OptFlags flags, int skylineIndex, int width, int height, int* y) const;
    // Update the skyline structure to include a width x height rect located
    // at x,y and the information about the pixels
    void addSkylineLevel(OptFlags flags, int skylineIndex, int x, int y, int width, int height);

    bool isBottomEdgeZeroed(int index, int x1, int x2) const;

    // Keep informaiton about pixels at the bottom edges of the skyline profile
    std::vector<std::vector<bool>> fBottom; // [fX: fX + fWidth)
    // TODO: Optimize left and right edges, too
};

} // End of namespace skgpu

#endif
