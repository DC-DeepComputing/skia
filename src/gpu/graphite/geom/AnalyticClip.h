/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_geom_AnalyticClip_DEFINED
#define skgpu_graphite_geom_AnalyticClip_DEFINED

#include "src/gpu/graphite/geom/Rect.h"

namespace skgpu::graphite {

/**
 * Represents a rect or rrect clip with any non-rect corners having the same circular radii.
 */
struct CircularRRectClip {
    enum EdgeFlags {
        kLeft_EdgeFlag = 0x1,
        kTop_EdgeFlag = 0x2,
        kRight_EdgeFlag = 0x4,
        kBottom_EdgeFlag = 0x8
    };
    static constexpr uint32_t kAllEdges = kLeft_EdgeFlag|
                                          kTop_EdgeFlag|
                                          kRight_EdgeFlag|
                                          kBottom_EdgeFlag;

    Rect     fBounds;           // Bounds of clip
    float    fRadius = 0;       // Circular radius, if any
    uint32_t fEdgeFlags = 0;    // Indicate which edges are adjacent to circular corners.
    bool     fInverted = false;

    bool isEmpty() const { return fBounds.isEmptyNegativeOrNaN(); }
    SkRect edgeSelectRect() const {
        return { fEdgeFlags & kLeft_EdgeFlag ? 1.f : 0.f,
                 fEdgeFlags & kTop_EdgeFlag ? 1.f : 0.f,
                 fEdgeFlags & kRight_EdgeFlag ? 1.f : 0.f,
                 fEdgeFlags & kBottom_EdgeFlag ? 1.f : 0.f };
    }
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_geom_AnalyticClip_DEFINED
