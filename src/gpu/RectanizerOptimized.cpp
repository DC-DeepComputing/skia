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
    int bestLostBytes = this->width() * this->height();
    int lostBytes = 0;
    bool found = false;
    for (auto x = 0; x <= this->width(); ++x) {

        if (currLoc.fX + width == x) {
            // We found the area: [x0:x) * [y0: y)
            // Let's see if it's better (we do not take in account optimized flag)
            /*
            if (bestLoc.fY > currLoc.fY) {
                bestLoc = currLoc;
                bestOptimized = locOptimized;
                bestLostBytes = lostBytes;
            } else if (bestLoc.fY == currLoc.fY) {
                if (bestLostBytes > lostBytes) {
                    bestLoc = currLoc;
                    bestOptimized = locOptimized;
                    bestLostBytes = lostBytes;
                } else if (bestLoc.fX > currLoc.fX) {
                    bestLoc = currLoc;
                    bestOptimized = locOptimized;
                    bestLostBytes = lostBytes;
                }
            } else {
                // It's not the best
            }
            */
            if (bestLoc.fY > currLoc.fY) {
                bestLoc = currLoc;
                bestOptimized = locOptimized;
                bestLostBytes = lostBytes;
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
            lostBytes = 0;
            locOptimized = zeroPadding > 0 && !fTopPadding[x]; // Keep track if needed
            continue;
        }

        // Check if we fit horizontally or if we reached the end
        if (currLoc.fX + width > this->width() || x == this->width()) {
            break;
        }

        // We keep looking
        locOptimized &= !fTopPadding[x];

        //currLoc.fY = std::max(currLoc.fY, fProfile[x]);
        if (currLoc.fY < fProfile[x]) {
            lostBytes += (fProfile[x] - currLoc.fY) * (x - currLoc.fX);
            currLoc.fY = fProfile[x];
        } else if (currLoc.fY > fProfile[x]) {
            lostBytes += currLoc.fY - fProfile[x];
        }

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
        fAreaSoFar += bestOptimized ? width*(height-1) : width*height;
        if (bestOptimized) {
            fOptimized += width;
        }
        this->markAreaOccupied(zeroPadding, bestLoc, width, height);
        *loc = bestLoc;
        static uint64_t gAreaSoFar = 0;
        gAreaSoFar += bestOptimized ? width*(height-1) : width*height;
        SkDebugf("Add #%lu %d * %d : [%d:%d * %d:%d) %d %d %lu\n",
                 fGenId,
                 width,
                 height,
                 bestLoc.fX,
                 bestLoc.fY,
                 bestLoc.fX + width,
                 bestLoc.fY + height,
                 bestLostBytes,
                 fAreaSoFar, gAreaSoFar);

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
    auto prevX = -1;
    auto prevZero = false;
    for (auto x = 0; x < this->width(); ++x) {
        auto y = fProfile[x];
        auto zero = !fTopPadding[x];
        if (x == 0) {
            prevX = x;
            prevY = y;
            prevZero = zero;
        } else if (y == prevY && prevZero == zero) {
            // Keep looking
        } else {
            SkDebugf("%d%s%d ", prevY, prevZero ? "+" : "-", prevX);
            prevX = x;
            prevY = y;
            prevZero = zero;
        }
    }
    SkDebugf("%d%s%d ", prevY, prevZero ? "+" : "-", prevX);
    SkDebugf("\n");
}

///////////////////////////////////////////////////////////////////////////////

Rectanizer* Rectanizer::Factory(int width, int height) {
    return new RectanizerOptimized(width, height, 0);
}

} // End of namespace skgpu
