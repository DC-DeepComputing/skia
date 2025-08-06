/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkPathRawBuilder_DEFINED
#define SkPathRawBuilder_DEFINED

#include "include/core/SkPathTypes.h"
#include "include/core/SkPoint.h"
#include "include/core/SkSpan.h"
#include "src/core/SkPathRaw.h"

class SkPathRawBuilder {
public:
    SkPathRawBuilder(SkSpan<SkPoint> ptStore, SkSpan<SkPathVerb> vbStore, SkSpan<float> cnStore)
        : fPtStorage(ptStore)
        , fVbStorage(vbStore)
        , fCnStorage(cnStore)
        , fPts(0), fCns(0), fVbs(0)
    {}

    // suggestive of a SkPathSink ...
    void moveTo(SkPoint);
    void lineTo(SkPoint);
    void quadTo(SkPoint, SkPoint);
    void conicTo(SkPoint, SkPoint, float w);
    void cubicTo(SkPoint, SkPoint, SkPoint);
    void close();

    SkPathRaw raw(SkPathFillType, bool isConvex = false) const;

    SkSpan<SkPoint> points() const { return {fPtStorage.data(), fPts}; }

private:
    SkSpan<SkPoint>    fPtStorage;
    SkSpan<SkPathVerb> fVbStorage;
    SkSpan<SkScalar>   fCnStorage;
    size_t             fPts, fCns, fVbs;

    void check_extend_pts(size_t n) const {
        SkASSERT(fPts + n <= fPtStorage.size());
    }
    void check_extend_vbs(size_t n) const {
        SkASSERT(fVbs + n <= fVbStorage.size());
    }
    void check_extend_cns(size_t n) const {
        SkASSERT(fCns + n <= fCnStorage.size());
    }
};

#endif
