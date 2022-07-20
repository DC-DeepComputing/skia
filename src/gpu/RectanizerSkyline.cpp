/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkIPoint16.h"
#include "src/gpu/RectanizerSkyline.h"

#include <algorithm>

namespace skgpu {

bool RectanizerSkyline::addRect(int width, int height, SkIPoint16* loc) {
    if ((unsigned)width > (unsigned)this->width() ||
        (unsigned)height > (unsigned)this->height()) {
        return false;
    }

    // find position for new rectangle
    int bestWidth = this->width() + 1;
    int bestX = 0;
    int bestY = this->height() + 1;
    int bestIndex = -1;
    for (int i = 0; i < fSkyline.count(); ++i) {
        int y;
        if (this->rectangleFits(i, width, height, &y)) {
            // minimize y position first, then width of skyline
            if (y < bestY || (y == bestY && fSkyline[i].fWidth < bestWidth)) {
                bestIndex = i;
                bestWidth = fSkyline[i].fWidth;
                bestX = fSkyline[i].fX;
                bestY = y;
            }
        }
    }

    // add rectangle to skyline
    if (-1 != bestIndex) {
        this->addSkylineLevel(bestIndex, bestX, bestY, width, height);
        loc->fX = bestX;
        loc->fY = bestY;

        fAreaSoFar += width*height;
        return true;
    }

    loc->fX = 0;
    loc->fY = 0;
    return false;
}

bool RectanizerSkyline::addRect(int width, int height, SkIPoint16* loc, PadAllGlyphs padAllGlyphs) {
    if ((unsigned)width > (unsigned)this->width() ||
        (unsigned)height > (unsigned)this->height()) {
        return false;
    }

    // find position for new rectangle
    int bestWidth = this->width() + 1;
    int bestX = 0;
    int bestY = this->height() + 1;
    int bestIndex = -1;
    int bestSavedPixels = -1;
    for (int i = 0; i < fSkyline.count(); ++i) {
        int y;
        int savedPixels;
        if (this->rectangleFits(i, width, height, padAllGlyphs, &y, &savedPixels)) {
            // minimize y position first, then width of skyline
            if (y < bestY || (y == bestY && fSkyline[i].fWidth < bestWidth)) {
                // TODO: for now when we look for the best fit we do not take in account
                //  bestSavedPixels; we need to decide what is better - take longer skyline (and
                //  possibly lose space) and save on the padding or the opposite.
                bestIndex = i;
                bestWidth = fSkyline[i].fWidth;
                bestX = fSkyline[i].fX;
                bestY = y;
                bestSavedPixels = savedPixels;
            }
        }
    }

    // add rectangle to skyline
    if (-1 != bestIndex) {
        this->addSkylineLevel(bestIndex, bestX, bestY, width, height, padAllGlyphs);
        loc->fX = bestX;
        loc->fY = bestY;

        fAreaSoFar += width*height;
        return true;
    }

    loc->fX = 0;
    loc->fY = 0;
    return false;
}

bool RectanizerSkyline::rectangleFits(int skylineIndex,
                                      int width,
                                      int height,
                                      PadAllGlyphs padAllGlyphs,
                                      int* ypos,
                                      int* savedPixels) const {
    int x = fSkyline[skylineIndex].fX;
    if (x + width > this->width()) {
        return false;
    }

    int widthLeft = width;
    int i = skylineIndex;
    int y = fSkyline[skylineIndex].fY;
    if (PadAllGlyphs::kYes == fSkyline[skylineIndex].fPadAllGlyphs &&
        PadAllGlyphs::kYes == padAllGlyphs) {
        // We can reuse the bottom padding
        --y;
    }
    int reusedPixels = 0;
    while (widthLeft > 0) {
        if (PadAllGlyphs::kYes == fSkyline[i].fPadAllGlyphs && PadAllGlyphs::kYes == padAllGlyphs) {
            if (y <= fSkyline[i].fY - 1) {
                // We can reuse the bottom padding
                reusedPixels += std::min(fSkyline[i].fWidth, widthLeft);
                y = fSkyline[i].fY - 1;
            }
        } else {
            y = std::max(y, fSkyline[i].fY);
        }
        if (y + height > this->height()) {
            return false;
        }
        widthLeft -= fSkyline[i].fWidth;
        ++i;
        SkASSERT(i < fSkyline.count() || widthLeft <= 0);
    }

    *ypos = y;
    *savedPixels = reusedPixels;
    return true;
}

void RectanizerSkyline::addSkylineLevel(
        int skylineIndex, int x, int y, int width, int height, PadAllGlyphs padAllGlyphs) {
    SkylineSegment newSegment;
    newSegment.fX = x;
    newSegment.fY = y + height;
    newSegment.fWidth = width;
    newSegment.fPadAllGlyphs = padAllGlyphs;
    fSkyline.insert(skylineIndex, 1, &newSegment);

    SkASSERT(newSegment.fX + newSegment.fWidth <= this->width());
    SkASSERT(newSegment.fY <= this->height());

    // delete width of the new skyline segment from following ones
    for (int i = skylineIndex+1; i < fSkyline.count(); ++i) {
        // The new segment subsumes all or part of fSkyline[i]
        SkASSERT(fSkyline[i-1].fX <= fSkyline[i].fX);

        if (fSkyline[i].fX < fSkyline[i-1].fX + fSkyline[i-1].fWidth) {
            int shrink = fSkyline[i-1].fX + fSkyline[i-1].fWidth - fSkyline[i].fX;

            fSkyline[i].fX += shrink;
            fSkyline[i].fWidth -= shrink;

            if (fSkyline[i].fWidth <= 0) {
                // fully consumed
                fSkyline.remove(i);
                --i;
            } else {
                // only partially consumed
                break;
            }
        } else {
            break;
        }
    }

    // merge fSkylines
    for (int i = 0; i < fSkyline.count()-1; ++i) {
        if (fSkyline[i].fY == fSkyline[i+1].fY &&
            fSkyline[i].fPadAllGlyphs == fSkyline[i+1].fPadAllGlyphs) {
            fSkyline[i].fWidth += fSkyline[i+1].fWidth;
            fSkyline.remove(i+1);
            --i;
        }
    }
}

///////////////////////////////////////////////////////////////////////////////

Rectanizer* Rectanizer::Factory(int width, int height) {
    return new RectanizerSkyline(width, height);
}

} // End of namespace skgpu
