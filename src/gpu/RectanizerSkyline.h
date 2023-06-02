/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_RectanizerSkyline_DEFINED
#define skgpu_RectanizerSkyline_DEFINED

#include "include/core/SkRect.h"
#include "include/private/base/SkTDArray.h"
#include "src/core/SkIPoint16.h"
#include "src/gpu/Rectanizer.h"

#include <optional>

namespace skgpu {

// Pack rectangles and track the current silhouette
// Based, in part, on Jukka Jylanki's work at http://clb.demon.fi
//
// Mark this class final in an effort to avoid the vtable when this subclass is used explicitly.
class RectanizerSkyline final : public Rectanizer {
public:
    RectanizerSkyline(int w, int h) : Rectanizer(w, h) {
        this->reset();
    }

    ~RectanizerSkyline() final { }

    void reset() final {
        fAreaSoFar = 0;
        fSkyline.clear();
        fSkyline.push_back(SkylineSegment{0, 0, this->width()});
        fLastFound.reset();
    }

    bool addRect(int w, int h, SkIPoint16* loc) final;

    float percentFull() const final {
        return fAreaSoFar / ((float)this->width() * this->height());
    }

    // `findRect()` and `insertFoundRect()` define a two-step process for finding an available slot
    // and making an insertion that mutates the Rectanizer without repeating the slot search.
    //
    // `findRect()` returns true if there is enough space for a rectangle with the given dimensions
    // and records its metadata internally. If a call is successful and once the result is recorded,
    // it *must* be committed with a subsequent call to `insertFoundRect` before further calls to
    // `findRect()` and `addRect()` can be made. Returns false if there is no available space for
    // the requested dimensions OR if a previously found but uninserted record exists.
    //
    // `insertFoundRect` performs an insertion at a previously found slot location using the
    // internally stored record. It returns false if the most recent call to `findRect()` did not
    // find a valid location. Otherwise, makes the insertion, populates `outRegion` with the
    // Rectanizer-relative bounds of the inserted region.
    //
    // After a successful call to `findRect()`, the internally stored record is only valid until the
    // next call to `insertFoundRect` or `reset` as they clear the internal record.
    bool findRect(int width, int height);
    bool insertFoundRect(SkIRect* outRegion);

private:
    struct SkylineSegment {
        int  fX;
        int  fY;
        int  fWidth;
    };

    SkTDArray<SkylineSegment> fSkyline;

    int32_t fAreaSoFar;

    struct LocationRecord {
        SkIPoint16 fPos;
        unsigned fWidth = 0;
        unsigned fHeight = 0;
        int fIndex = -1;

        LocationRecord(SkIPoint16 pos, unsigned w, unsigned h, int index)
                : fPos(pos), fWidth(w), fHeight(h), fIndex(index) {}
    };
    std::optional<LocationRecord> fLastFound;

    // Can a width x height rectangle fit in the free space represented by
    // the skyline segments >= 'skylineIndex'? If so, return true and fill in
    // 'y' with the y-location at which it fits (the x location is pulled from
    // 'skylineIndex's segment.
    bool rectangleFits(int skylineIndex, int width, int height, int* y) const;
    // Update the skyline structure to include a width x height rect located
    // at x,y.
    void addSkylineLevel(int skylineIndex, int x, int y, int width, int height);
};

} // End of namespace skgpu

#endif
