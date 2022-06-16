/*
 * Copyright 2010 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_RectanizerSkylineTestingPeer_DEFINED
#define skgpu_RectanizerSkylineTestingPeer_DEFINED

#include "src/gpu/Rectanizer.h"
#include "src/gpu/AtlasTypes.h"

struct SkIPoint16;

namespace skgpu {

class RectanizerSkylineTestingPeer {
public:
    int allocatedBytes(Rectanizer* rectanizer) const {
        return  rectanizer->fAreaSoFar;
    }

    int allBytes(Rectanizer* rectanizer) const {
        return rectanizer->fWidth * rectanizer->fHeight;
    }

    int allocatedBytes(Plot* plot) const {
        return allocatedBytes(plot->fRectanizer.get());
    }

    int allBytes(Plot* plot) const {
        return allBytes(plot->fRectanizer.get());
    }

    SkIRect getRect(Plot* plot) {
        return SkIRect::MakeXYWH(plot->fOffset.fX, plot->fOffset.fY, plot->fWidth, plot->fHeight);
    }

    unsigned char* data(Plot* plot) {
        return plot->fData;
    }
};

} // End of namespace skgpu

#endif
