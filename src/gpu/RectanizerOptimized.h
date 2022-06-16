/*
 * Copyright 2022 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_RectanizerOptimized_DEFINED
#define skgpu_RectanizerOptimized_DEFINED

#include "include/private/SkBitmaskEnum.h"
#include "include/private/SkTDArray.h"
#include "src/gpu/RectanizerSkyline.h"
#include "src/utils/SkBitSet.h"

#include <vector>

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
        fAreaSoFar = 0;
        fProfile.resize(this->width());
        std::fill(fProfile.begin(), fProfile.end(), 0);
    }

    bool addRect(int w, int h, SkIPoint16* loc) override;

    void printProfile_testingOnly() const;

private:
    // Update the skyline structure to include a width x height rect located
    // at x,y and the information about the pixels
    void markAreaOccupied(SkIPoint16 loc, int width, int height);

    // Keep informaiton about pixels at the bottom edges of the skyline profile
    std::vector<int16_t> fProfile;
};

} // End of namespace skgpu

#endif
