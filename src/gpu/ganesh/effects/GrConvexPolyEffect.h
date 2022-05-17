/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrConvexPolyEffect_DEFINED
#define GrConvexPolyEffect_DEFINED

#include "include/private/gpu/ganesh/GrTypesPriv.h"
#include "src/gpu/ganesh/GrCaps.h"
#include "src/gpu/ganesh/GrFragmentProcessor.h"
#include "src/gpu/ganesh/GrProcessor.h"

class GrInvariantOutput;
class SkPath;

/**
 * An effect that renders a convex polygon. It is intended to be used as a coverage effect.
 * Bounding geometry is rendered and the effect computes coverage based on the fragment's
 * position relative to the polygon.
 */
namespace GrConvexPolyEffect  {
    constexpr int kMaxEdges = 8;

    /**
     * Creates an effect that clips against the path. If the path is not a convex polygon, is
     * inverse filled, or has too many edges, creation will fail.
     */
    GrFPResult Make(std::unique_ptr<GrFragmentProcessor>, GrClipEdgeType, const SkPath&);
};


#endif
