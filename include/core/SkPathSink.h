/*
 * Copyright 2025 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkPathSink_DEFINED
#define SkPathSink_DEFINED

#include "include/core/SkPathTypes.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSpan.h"

class SkMatrix;

class SK_API SkPathSink {
public:
    virtual ~SkPathSink() {}

    virtual void moveTo(SkPoint) = 0;
    virtual void lineTo(SkPoint) = 0;
    virtual void quadTo(SkPoint, SkPoint) = 0;
    virtual void conicTo(SkPoint, SkPoint, SkScalar w) = 0;
    virtual void cubicTo(SkPoint, SkPoint, SkPoint) = 0;
    virtual void close() = 0;

    virtual void incReserve(size_t ptsDelta, size_t vbsDelta);
    virtual void addPointsVerbs(SkSpan<const SkPoint>, SkSpan<const uint8_t>,
                                const SkMatrix* = nullptr);

    // helpers

    void addLine(SkPoint a, SkPoint b) { this->moveTo(a); this->lineTo(b); }
    void addRect(const SkRect&, SkPathDirection = SkPathDirection::kCW);
    void addOval(const SkRect&, SkPathDirection = SkPathDirection::kCW);
    void addCapsule(const SkRect&, SkPathDirection = SkPathDirection::kCW);
    void addCircle(SkPoint, float, SkPathDirection = SkPathDirection::kCW);
    void addRRect(const SkRect&, float rad, SkPathDirection = SkPathDirection::kCW);
    void addPolygon(SkSpan<const SkPoint>, bool closed, SkPathFillType, const SkMatrix* = nullptr);
};

#endif

