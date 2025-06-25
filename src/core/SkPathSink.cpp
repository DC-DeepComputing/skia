/*
 * Copyright 2025 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkPathSink.h"
#include "src/core/SkPathRaw.h"

#if 0
    virtual void moveTo(SkPoint) = 0;
    virtual void lineTo(SkPoint) = 0;
    virtual void quadTo(SkPoint, SkPoint) = 0;
    virtual void conicTo(SkPoint, SkPoint, SkScalar w) = 0;
    virtual void cubicTo(SkPoint, SkPoint, SkPoint) = 0;
    virtual void close() = 0;

    virtual void incReserve(size_t ptsDelta, size_t vbsDelta);
    virtual void addPointsVerbs(SkSpan<const SkPoint>, SkSpan<const uint8_t>,
                                const SkMatrix* = nullptr);
#endif

void SkPathSink::addRect(const SkRect& r, SkPathDirection dir) {
    this->incReserve(4, 5);    // M L L L C
    this->moveTo(r.TL());
    if (dir == SkPathDirection::kCW) {
        this->lineTo(r.TR());
        this->lineTo(r.BR());
        this->lineTo(r.BL());
    } else {
        this->lineTo(r.BL());
        this->lineTo(r.BR());
        this->lineTo(r.TR());
    }
    this->close();
}

void SkPathSink::addOval(const SkRect& r, SkPathDirection dir)
{}
void SkPathSink::addCapsule(const SkRect& r, SkPathDirection dir)
{}
void SkPathSink::addCircle(SkPoint center, float radius, SkPathDirection dir)
{}
void SkPathSink::addRRect(const SkRect& r, float rad, SkPathDirection dir)
{}
void SkPathSink::addPolygon(SkSpan<const SkPoint> pts, bool closed, SkPathFillType ft,
                            const SkMatrix* mx)
{}

/////////////////////////////

void SkPathSink::visit(const SkPathRaw& raw) {
    for (auto iter = raw.iter(); auto rec = iter.next(); ) {
        auto pts = rec->pts;
        switch (rec->vrb) {
            case SkPathVerb::kMove:  this->moveTo(pts[0]); break;
            case SkPathVerb::kLine:  this->lineTo(pts[1]); break;
            case SkPathVerb::kQuad:  this->quadTo(pts[1], pts[2]); break;
            case SkPathVerb::kConic: this->conicTo(pts[1], pts[2], rec->w); break;
            case SkPathVerb::kCubic: this->cubicTo(pts[1], pts[2], pts[3]); break;
            case SkPathVerb::kClose: this->close(); break;
        }
    }
}
