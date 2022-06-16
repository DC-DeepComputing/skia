/*
 * Copyright 2022 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkIPoint16.h"
#include "src/gpu/RectanizerOptimized.h"

#include <algorithm>

namespace skgpu {

bool RectanizerOptimized::addRect(int width, int height, SkIPoint16* loc) {

    if ((unsigned)width > (unsigned)this->width() ||
        (unsigned)height > (unsigned)this->height()) {
        return false;
    }

    SkIPoint16 bestLoc = SkIPoint16::Make(this->width(), this->height());
    SkIPoint16 currLoc = SkIPoint16::Make(-1, -1);
    bool found = false;
    for (int16_t x = 0; x <= SkToS16(this->width()); ++x) {

        if (currLoc.fY == -1) {
            if (x == SkToS16(this->width())) {
                break;
            }
            // We start looking from the current position
            currLoc.fY = fProfile[x];
            currLoc.fX = x;
        } else if (currLoc.fX + width == x) {
            // We found the area: [x0:x) * [y0: y)
            // Let's see if it's better (we do not take in account optimized flag)
            if (bestLoc.fY > currLoc.fY) {
                bestLoc = currLoc;
            }
            // We found one; let's see if we can find better y (starting over from the currLoc.fX+1)
            found = true;
            currLoc.fY = -1;
            x = currLoc.fX;
        } else if (currLoc.fX + width > this->width() || x == this->width()) {
            // We reached the end
            break;
        } else {
            // Continue looking
            currLoc.fY = std::max(currLoc.fY, fProfile[x]);

            // Check if we still fit vertically
            if (currLoc.fY + height > this->height()) {
                // We start over from the next x (nothing before will be good enough)
                currLoc.fY = -1;
            }
        }
    }

    if (found) {
        // Mark the area as allocated
        fAreaSoFar += width*height;
        this->markAreaOccupied(bestLoc, width, height);
        *loc = bestLoc;
        return true;
    }

    loc->fX = 0;
    loc->fY = 0;
    return false;

}

void RectanizerOptimized::markAreaOccupied(SkIPoint16 loc,
                                           int width,
                                           int height) {
    auto y = loc.fY + SkToS16(height);
    for (int16_t x = loc.fX; x < loc.fX + SkToS16(width); ++x) {
        fProfile[x] = y;
    }
}

///////////////////////////////////////////////////////////////////////////////

Rectanizer* Rectanizer::Factory(int width, int height) {
    return new RectanizerOptimized(width, height);
}

} // End of namespace skgpu
