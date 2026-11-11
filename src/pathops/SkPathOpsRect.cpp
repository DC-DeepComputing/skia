/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include <cstdint>
#include "include/core/SkSpan.h"
#include "include/private/SkTo.h"
#include "src/pathops/SkPathOpsRect.h"

#include "src/pathops/SkPathOpsConic.h"
#include "src/pathops/SkPathOpsCubic.h"
#include "src/pathops/SkPathOpsQuad.h"
#include "src/pathops/SkPathOpsTCurve.h"

#include <array>

void SkDRect::setBounds(const SkDQuad& curve, const SkDQuad& sub, double startT, double endT) {
    set(sub[0]);
    add(sub[2]);
    std::array<double, 2> tValues;
    int roots = 0;
    if (!sub.monotonicInX()) {
        roots = SkDQuad::FindExtrema(SkSpan(&sub[0].fX, 6), tValues.data());
    }
    if (!sub.monotonicInY()) {
        roots += SkDQuad::FindExtrema(SkSpan(&sub[0].fY, 5), &tValues[roots]);
    }
    for (int index = 0; index < roots; ++index) {
        double t = startT + (endT - startT) * tValues[index];
        add(curve.ptAtT(t));
    }
}

void SkDRect::setBounds(const SkDConic& curve, const SkDConic& sub, double startT, double endT) {
    set(sub[0]);
    add(sub[2]);
    std::array<double, 2> tValues;
    int roots = 0;
    if (!sub.monotonicInX()) {
        roots = SkDConic::FindExtrema(SkSpan(&sub[0].fX, 6), sub.fWeight, SkSpan(tValues).first<1>());
    }
    if (!sub.monotonicInY()) {
        roots += SkDConic::FindExtrema(SkSpan(&sub[0].fY, 5), sub.fWeight, SkSpan<double>(tValues).subspan(SkTo<size_t>(roots)).first<1>());
    }
    for (int index = 0; index < roots; ++index) {
        double t = startT + (endT - startT) * tValues[index];
        add(curve.ptAtT(t));
    }
}

void SkDRect::setBounds(const SkDCubic& curve, const SkDCubic& sub, double startT, double endT) {
    set(sub[0]);
    add(sub[3]);
    double tValues[4];
    int roots = 0;
    if (!sub.monotonicInX()) {
        roots = SkDCubic::FindExtrema(SkSpan(&sub[0].fX, 8), tValues);
    }
    if (!sub.monotonicInY()) {
        roots += SkDCubic::FindExtrema(SkSpan(&sub[0].fY, 7), &tValues[roots]);
    }
    for (int index = 0; index < roots; ++index) {
        double t = startT + (endT - startT) * tValues[index];
        add(curve.ptAtT(t));
    }
}

void SkDRect::setBounds(const SkTCurve& curve) {
    curve.setBounds(this);
}
