/*
 * Copyright 2022 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkIPoint16.h"
#include "src/gpu/RectanizerOptimized.h"

#include <algorithm>
#include "include/core/SkRect.h"
#include "include/core/SkString.h"

namespace skgpu {

bool RectanizerOptimized::addRect(int width, int height, SkIPoint16* loc) {

    if ((unsigned)width > (unsigned)this->width() ||
        (unsigned)height > (unsigned)this->height()) {
        return false;
    }

    SkIPoint16 bestLoc = SkIPoint16::Make(this->width(), this->height());
    SkIPoint16 currLoc = SkIPoint16::Make(-1, -1);
    bool found = false;
    for (auto x = 0; x <= this->width(); ++x) {

        if (currLoc.fY == -1) {
            // We start looking from the current position
            currLoc.fY = fProfile[x];
            currLoc.fX = x;
            continue;
        }

        if (currLoc.fX + width == x) {
            // We found the area: [x0:x) * [y0: y)
            // Let's see if it's better (we do not take in account optimized flag)
            if (bestLoc.fY > currLoc.fY) {
                bestLoc = currLoc;
            }
            // We found one; let's see if we can find better y (starting over from the next x)
            found = true;
            currLoc.fY = -1;
            x = currLoc.fX;
            continue;
        }

        // Check if we fit horizontally or if we reached the end
        if (currLoc.fX + width > this->width() || x == this->width()) {
            break;
        }

        currLoc.fY = std::max(currLoc.fY, fProfile[x]);

        // Check if we still fit vertically
        if (currLoc.fY + height > this->height()) {
            // We start over from the next position
            currLoc.fY = -1;
            x = currLoc.fX;
        }
    }

    if (found) {
        // Mark the area as allocated
        fAreaSoFar += width*height;
        this->markAreaOccupied(bestLoc, width, height);
        *loc = bestLoc;
        /*
        static uint64_t gAreaSoFar = 0;
        gAreaSoFar += width*height;
        SkDebugf("Add %d * %d : [%d:%d * %d:%d) %d %lu\n",
                 width,
                 height,
                 bestLoc.fX,
                 bestLoc.fY,
                 bestLoc.fX + width,
                 bestLoc.fY + height,
                 fAreaSoFar, gAreaSoFar);
        */
        return true;
    }

    loc->fX = 0;
    loc->fY = 0;
    return false;

}

void RectanizerOptimized::markAreaOccupied(SkIPoint16 loc,
                                           int width,
                                           int height) {
    auto y = loc.fY + height;
    for (auto x = loc.fX; x < loc.fX + width; ++x) {
        fProfile[x] = y;
    }
}

void RectanizerOptimized::printProfile_testingOnly() const {
    auto prevY = -1;
    auto prevX = -1;
    for (auto x = 0; x < this->width(); ++x) {
        auto y = fProfile[x];
        if (x == 0) {
            prevX = x;
            prevY = y;
        } else {
            SkDebugf("%d-%d ", prevY, prevX);
            prevX = x;
            prevY = y;
        }
    }
    SkDebugf("%d-%d\n", prevY, prevX);
}

///////////////////////////////////////////////////////////////////////////////

Rectanizer* Rectanizer::Factory(int width, int height) {
    return new RectanizerOptimized(width, height);
}

} // End of namespace skgpu
