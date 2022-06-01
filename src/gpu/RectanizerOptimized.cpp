/*
 * Copyright 2022 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkIPoint16.h"
#include "src/gpu/RectanizerOptimized.h"

#include <algorithm>
#include <include/core/SkString.h>

namespace skgpu {

bool RectanizerOptimized::isBottomEdgeZeroed(int index, int x1, int x2) const {
    auto& skyline = fSkyline[index];
    if (x1 >= skyline.fX + skyline.fWidth) {
        return false;
    } else if (x2 < skyline.fX) {
        return false;
    }

    return !fBottom[index].empty() &&
            std::find(fBottom[index].begin(), fBottom[index].end(), true) == fBottom[index].cend();
}

bool RectanizerOptimized::addRectOptimized(int width, int height, int zeroPadding, SkIPoint16* loc){

    if (false) {
        return RectanizerSkyline::addRect(width, height, loc);
    }

    if ((unsigned)width > (unsigned)this->width() ||
        (unsigned)height > (unsigned)this->height()) {
        return false;
    }

    std::vector<OptFlags> options = { OptFlags::None };
    bool optimized = zeroPadding == 1;
    if (optimized) {
        options.emplace_back(OptFlags::T);
    }

    int bestScore = 0;
    int bestWidth = this->width() + 1;
    int bestX = 0;
    int bestY = this->height() + 1;
    int bestIndex = -1;
    OptFlags bestFlags = OptFlags::None;
    for (int i = 0; i < fSkyline.count(); ++i) {
        for (auto& opt : options) {
            int y;
            auto score = this->rectangleFits(opt, i, width, height, &y);
            if (score >= bestScore){
                // optimize first, minimize y position second, then width of skyline
                if (y < bestY || (y == bestY && fSkyline[i].fWidth < bestWidth)) {
                    bestIndex = i;
                    bestWidth = fSkyline[i].fWidth;
                    bestX = fSkyline[i].fX;
                    bestY = y;
                    bestScore = score;
                    bestFlags = opt;
                }
            }
        }
    }

    // add rectangle to skyline
    if (-1 != bestIndex) {
        this->addSkylineLevel(optimized ? OptFlags::T : OptFlags::None,
                              bestIndex,
                              bestX,
                              bestY,
                              width,
                              height);
        loc->fX = bestX;
        loc->fY = bestY;

        fAreaSoFar += (bestFlags & OptFlags::T) == OptFlags::T ? width*(height-1) : width*height;
        return true;
    }

    loc->fX = 0;
    loc->fY = 0;
    return false;
}

int RectanizerOptimized::rectangleFits(
        OptFlags flags, int skylineIndex, int width, int height, int* ypos) const {
    int h = height - ((flags & OptFlags::T) == OptFlags::T ? 1 : 0);
    int x = fSkyline[skylineIndex].fX;
    if (x + width > this->width()) {
        // We don't have enough width even if we reuse the padding when allowed
        return -1;
    }

    int score = 0;
    bool topZeroed = true;
    int widthLeft = width;
    int i = skylineIndex;
    int y = fSkyline[skylineIndex].fY;
    while (widthLeft > 0) {

        if ((flags & OptFlags::T) == OptFlags::T) {
            // Let's check that the skyline above not touching our position or
            // its bottom edge is zeroed
            if (fSkyline[i].fY == y) {
                topZeroed = topZeroed && this->isBottomEdgeZeroed(i, x, x + widthLeft);
                if (topZeroed) score += widthLeft;
            } else {
                topZeroed = true;
            }
        }

        y = std::max(y, fSkyline[i].fY);
        if (y + h > this->height()) {
            return -1;
        }

        widthLeft -= fSkyline[i].fWidth;
        ++i;
        SkASSERT(i < fSkyline.count() || widthLeft <= 0);
    }

    if ((flags & OptFlags::T) == OptFlags::T) {
        if (topZeroed) {
            // We were able to fit the rectangle and save the top pixel padding
            --y;
        } else {
            // We were able to fit the rectangle but didn't save the top pixel padding
            // (which was required)
            return -1;
        }
    }

    *ypos = y;
    return 1;//score;
}

void RectanizerOptimized::addSkylineLevel(
        OptFlags flags, int skylineIndex, int x, int y, int width, int height) {
    SkylineSegment newSegment;
    newSegment.fX = x;
    newSegment.fY = y + height;
    newSegment.fWidth = width;
    fSkyline.insert(skylineIndex, 1, &newSegment);
    if ((flags & OptFlags::T) == OptFlags::T) {
        // We adding a rectangle that has zero padding
        fBottom.insert(fBottom.begin() + skylineIndex, std::vector<bool>(width, true));
    } else {
        fBottom.insert(fBottom.begin() + skylineIndex,std::vector<bool>());
    }

    SkASSERT(newSegment.fX + newSegment.fWidth <= this->width());
    SkASSERT(newSegment.fY <= this->height());

    // delete width of the new skyline segment from following ones
    int extended = x;
    for (int i = skylineIndex+1; i < fSkyline.count(); ++i) {
        // The new segment subsumes all or part of fSkyline[i]
        SkASSERT(fSkyline[i-1].fX <= fSkyline[i].fX);
        SkASSERT(skylineIndex+1 == i); // This is not obvious

        if (fSkyline[i].fX < fSkyline[i-1].fX + fSkyline[i-1].fWidth) {
            // If we managed to get here we do have space for out rectangle:
            SkASSERT(fSkyline[i].fY <= fSkyline[i-1].fY);

            int shrink = fSkyline[i-1].fX + fSkyline[i-1].fWidth - fSkyline[i].fX;
            int oldWidth = fSkyline[i].fWidth;
            int shift = fSkyline[i].fX - fSkyline[i-1].fX;

            auto lostY = y - fSkyline[i].fY;
            static int gLost = 0;
            if (lostY > 0) {
                gLost += shrink * lostY;fBottom[skylineIndex];
                SkDebugf("lost: %dx%d=%d -> %d/%d\n",
                         shrink, lostY, shrink * lostY, gLost, fAreaSoFar);
            }

            fSkyline[i].fX += shrink;
            fSkyline[i].fWidth -= shrink;

            // Transfer bottom info from i to i-1
            // TODO: We can be more accurate and detect the case when all pixels in shrank
            //  are not zero...
            // Make sure that we have a bitset to copy pieces
            auto start = fSkyline[i].fX;
            // Copy %shrink pixels from fBottom[i] to fBottom[skylineIndex] (@start)
            if (!fBottom[i].empty()) {
                if (fBottom[skylineIndex].empty()) {
                    // We have to create an array so we do not lose anythingfBottom[skylineIndex]
                    fBottom[skylineIndex] = std::vector<bool>(width, true);
                } else {
                    // We already created fBottom[skylineIndex] with correct size
                    // We just need to copy the values from fBottom[i]
                }
                // Copy the shink part
                std::copy(fBottom[i].begin(), fBottom[i].begin() + shrink,
                          fBottom[skylineIndex].begin() + shift);
            } else {
                if (!fBottom[skylineIndex].empty()) {
                    // It's already been resized
                    // Mark all pixels from oldWidth as 1
                    std::fill(fBottom[skylineIndex].begin() + shift,
                              fBottom[skylineIndex].begin() + shift + shrink,
                              true);
                }
            }

            if (fSkyline[i].fWidth <= 0) {
                // fully consumedfBottom[i]
                fSkyline.remove(i);
                fBottom.erase(fBottom.begin() + i);
                --i;
                extended += oldWidth;
            } else {
                // only partially consumed
                if (!fBottom[i].empty()) {
                    // We need to update fBottom[i]
                    std::copy(fBottom[i].begin() + shrink, fBottom[i].end(), fBottom[i].begin());
                    // Remove shrink part from fBottom[i]
                    fBottom[i].resize(fBottom[i].size() - shrink);
                }
                break;
            }
        } else {
            break;
        }
    }

    // merge fSkylines
    for (int i = 0; i < fSkyline.count()-1; ++i) {
        if (fSkyline[i].fY == fSkyline[i+1].fY) {
            // We can only get here starting from skylineIndex - 1
            SkASSERT(i >= skylineIndex - 1);

            auto oldWidth = fSkyline[i].fWidth;
            auto oldTail = fSkyline[i+1].fWidth;
            fSkyline[i].fWidth += fSkyline[i+1].fWidth;

            if (fBottom[i].empty() && fBottom[i+1].empty()) {
                // Merge nothing with nothing
            } else {
                fBottom[i].resize(oldWidth + oldTail);
                if (fBottom[i].empty()) {
                    std::fill(fBottom[i].begin(), fBottom[i].begin() + oldWidth, 1);
                }
                if (fBottom[i+1].empty()) {
                    std::fill(fBottom[i].begin() + oldWidth,
                              fBottom[i].begin() + oldWidth + oldTail,
                              1);
                } else {
                    std::copy(fBottom[i+1].begin(),
                              fBottom[i+1].begin() + oldTail,
                              fBottom[i].begin() + oldWidth);
                }
                SkASSERT(fBottom[i].size() == fSkyline[i].fWidth);
            }
            fSkyline.remove(i+1);
            fBottom.erase(fBottom.begin() + i+1);
            --i;
        }
    }
    SkDebugf("addSkylineLevel(%d, %d, %d, %d, %d): %d ",
             skylineIndex, x, y, width, height, fAreaSoFar);
    this->printProfile();
}

void RectanizerOptimized::printProfile() const {
    for (auto i = 0; i < fSkyline.size(); ++i) {
        auto& skyline = fSkyline[i];
        auto opt = this->isBottomEdgeZeroed(i, skyline.fX, skyline.fX+skyline.fWidth)
                    ? " 0" : " ";
        SkDebugf("[0:%d)x[%d:%d)%s ", skyline.fY, skyline.fX, skyline.fX+skyline.fWidth, opt);
        if (!fBottom[i].empty() && skyline.fWidth != fBottom[i].size()) {
            SkDebugf("Wrong: %d != %d\n", skyline.fWidth, fBottom[i].size());
        }
    }
    SkDebugf("\n");
}

///////////////////////////////////////////////////////////////////////////////

Rectanizer* Rectanizer::Factory(int width, int height) {
    return new RectanizerOptimized(width, height);
}

} // End of namespace skgpu
