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
    auto from = std::max(x1 - skyline.fX, 0);
    auto to = std::min(x2 - skyline.fX, skyline.fWidth);
    bool hasNotZero = false;
    if (fBottom[index].has_value()) {
        fBottom[index]->forEachSetIndex([&](int slot) {
            if (slot >= from - skyline.fX && slot < to - skyline.fX) {
                hasNotZero = true;
            }
        });
    } else {
        hasNotZero = true;
    }
    return !hasNotZero;
}

bool RectanizerOptimized::addRect(int width, int height, int zeroPadding, SkIPoint16* loc) {

    if ((unsigned)width > (unsigned)this->width() ||
        (unsigned)height > (unsigned)this->height()) {
        return false;
    }

    std::vector<OptFlags> options = { OptFlags::None };
    bool optimized = zeroPadding == 1 && false;
    if (optimized) {
        options.emplace_back(OptFlags::T);
    }

    int bestScore = -1;
    int bestWidth = this->width() + 1;
    int bestX = 0;
    int bestY = this->height() + 1;
    int bestIndex = -1;
    OptFlags bestFlags = OptFlags::None;
    for (int i = 0; i < fSkyline.count(); ++i) {
        for (auto& opt : options) {
            int y;
            auto score = this->rectangleFits(opt, i, width, height, &y);
            if (score > bestScore){
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
    return score;
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
        fBottom.insert(skylineIndex, 1, new std::optional<SkBitSet>(SkBitSet(width)));
        fBottom[skylineIndex]->reset(); // all zeroes
    } else {
        fBottom.insert(skylineIndex);
    }

    SkASSERT(newSegment.fX + newSegment.fWidth <= this->width());
    SkASSERT(newSegment.fY <= this->height());

    // delete width of the new skyline segment from following ones
    for (int i = skylineIndex+1; i < fSkyline.count(); ++i) {
        // The new segment subsumes all or part of fSkyline[i]
        SkASSERT(fSkyline[i-1].fX <= fSkyline[i].fX);

        if (fSkyline[i].fX < fSkyline[i-1].fX + fSkyline[i-1].fWidth) {
            int shrink = fSkyline[i-1].fX + fSkyline[i-1].fWidth - fSkyline[i].fX;

            auto lostY = y - fSkyline[i].fY;
            static int gLost = 0;
            if (lostY > 0) {
                gLost += shrink * lostY;
                SkDebugf("lost: %dx%d=%d -> %d/%d\n",
                         shrink, lostY, shrink * lostY, gLost, fAreaSoFar);
            }
            fSkyline[i].fX += shrink;
            fSkyline[i].fWidth -= shrink;

            if (fSkyline[i].fWidth <= 0) {
                // fully consumedfBottom[i
                //SkDebugf("Remove %d\n", i);
                fSkyline.remove(i);
                fBottom.remove(i);
                --i;
            } else {
                // only partially consumed
                //SkDebugf("Shrink %d - %d\n", i, shrink);
                if (fBottom[i].has_value()) {
                    SkBitSet shrank(fSkyline[i].fWidth);
                    shrank.reset();
                    fBottom[i]->forEachSetIndex([&](int slot){
                        if (slot >= shrink) {
                            shrank.set(slot - shrink);
                        }
                    });
                    fBottom[i] = std::optional<SkBitSet>(std::move(shrank));
                    SkASSERT(fBottom[i]->size() == fSkyline[i].fWidth);
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
            auto oldWidth = fSkyline[i].fWidth;
            auto oldTail = fSkyline[i+1].fWidth;
            fSkyline[i].fWidth += fSkyline[i+1].fWidth;
            //SkDebugf("merge %d and next\n", i);
            if (fBottom[i].has_value() && fBottom[i+1].has_value()) {
                SkBitSet merge(fSkyline[i].fWidth);
                merge.reset();
                fBottom[i]->forEachSetIndex([&](int slot){
                    merge.set(slot);
                });
                fBottom[i+1]->forEachSetIndex([&](int slot){
                    merge.set(oldWidth + slot);
                });
                fBottom[i] = std::optional<SkBitSet>(std::move(merge));
                SkASSERT(fBottom[i]->size() == fSkyline[i].fWidth);
            } else {
                fBottom[i].reset();
            }
            fSkyline.remove(i+1);
            fBottom.remove(i+1);
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
        auto& bottom = fBottom[i];
        auto opt = this->isBottomEdgeZeroed(i, skyline.fX, skyline.fX+skyline.fWidth)
                    ? " 0" : " ";
        SkDebugf("[0:%d)x[%d:%d)%s ", skyline.fY, skyline.fX, skyline.fX+skyline.fWidth, opt);
    }
    SkDebugf("\n");
}

///////////////////////////////////////////////////////////////////////////////

Rectanizer* Rectanizer::Factory(int width, int height) {
    return new RectanizerOptimized(width, height);
}

} // End of namespace skgpu
