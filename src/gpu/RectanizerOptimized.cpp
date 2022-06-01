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

bool RectanizerOptimized::addRectOptimized(int width, int height, int zeroPadding, SkIPoint16* loc){

    //SkDebugf("addRectOptimized(%d, %d, %d): %d %d\n", width, height, zeroPadding, fBusyArea, saved);

    if ((unsigned)width > (unsigned)this->width() ||
        (unsigned)height > (unsigned)this->height()) {
        return false;
    }

    SkIPoint16 bestLoc = SkIPoint16::Make(this->width(), this->height());
    SkIPoint16 currLoc = SkIPoint16::Make(-1, -1);
    bool bestOptimized = false;
    bool locOptimized = true;
    bool found = false;
    for (auto x = 0; x <= this->width(); ++x) {

        if (currLoc.fX + width == x) {
            // We found the area: [x0:x) * [y0: y)
            // Let's see if it's better (we do not take in account optimized flag)
            if (bestLoc.fY > currLoc.fY) {
                bestLoc = currLoc;
                bestOptimized = locOptimized;
            } else if (bestLoc.fY == currLoc.fY && bestLoc.fX > currLoc.fX) {
                bestLoc = currLoc;
                bestOptimized = locOptimized;
            } else {
                // It's not the best
            }
            // We found one; let's see if we can find better y (starting over from the next x)
            found = true;
            currLoc.fY = -1;
            x = currLoc.fX;
            continue;
        }

        if (currLoc.fY == -1) {
            // We start looking from the current position
            currLoc.fY = fProfile[x];
            currLoc.fX = x;
            locOptimized = zeroPadding > 0 && !fTopPadding[x]; // Keep track if needed
            continue;
        }

        // Check if we fit horizontally or if we reached the end
        if (currLoc.fX + width > this->width() || x == this->width()) {
            break;
        }

        // We keep looking
        locOptimized &= !fTopPadding[x];
        currLoc.fY = std::max(currLoc.fY, fProfile[x]);

        // Check if we still fit vertically
        if (locOptimized && currLoc.fY + height - 1 > this->height()) {
            // We start over from the next position (could not vertically even with optimization)
            currLoc.fY = -1;
            x = currLoc.fX;
        } else if (currLoc.fY + height > this->height()) {
            // We start over from the next position
            currLoc.fY = -1;
            x = currLoc.fX;
        }
    }

    if (found) {
        // Mark the area as allocated
        fBusyArea += bestOptimized ? width*(height-1) : width*height;
        if (bestOptimized) {
            fOptimized += width;
        }
        /*
        if (bestOptimized) {
            SkDebugf("%d x %d: %d -> %d + %d = %d\n",
                     width,
                     height - 1,
                     width,
                     fOptimized,
                     fBusyArea,
                     fOptimized + fBusyArea);
        } else {
            SkDebugf("%d x %d: %d -> %d + %d = %d\n",
                     width,
                     height,
                     0,
                     fOptimized,
                     fBusyArea,
                     fOptimized + fBusyArea);
        }
        */
        this->markAreaOccupied(zeroPadding, bestLoc, width, height);
        *loc = bestLoc;

        return true;
    }

    loc->fX = 0;
    loc->fY = 0;
    return false;

}

void RectanizerOptimized::markAreaOccupied(bool zeroPadding,
                                           SkIPoint16 loc,
                                           int width,
                                           int height) {
    auto y = loc.fY + height;
    for (auto x = loc.fX; x < loc.fX + width; ++x) {
        fTopPadding[x] = !zeroPadding;
        fProfile[x] = y;
    }
}

void RectanizerOptimized::printProfile() const {
    auto prevY = -1;
    auto prevZero = false;
    for (auto x = 0; x < this->width(); ++x) {
        auto y = fProfile[x];
        auto zero = !fTopPadding[x];
        if (x == 0) {
            prevY = y;
            prevZero = zero;
        } else if (y == prevY && prevZero == zero) {
            // Keep looking
        } else {
            //SkDebugf("%d%s%d ", prevY, prevZero ? "+" : "-", prevX);
            prevY = y;
            prevZero = zero;
        }
    }
    //SkDebugf("%d%s%d ", prevY, prevZero ? "+" : "-", prevX);
    //SkDebugf("\n");
}

///////////////////////////////////////////////////////////////////////////////

Rectanizer* Rectanizer::Factory(int width, int height) {
    return new RectanizerOptimized(width, height);
}

} // End of namespace skgpu
