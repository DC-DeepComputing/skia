/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkPoint.h"
#include "include/core/SkScalar.h"
#include "include/private/SkFloatingPoint.h"

#include <cmath>

struct SkDoubleVector {
    double fX;
    double fY;

    SkDoubleVector& set(double x, double y) {
        fX = x;
        fY = y;
        return *this;
    }

    double cross(const SkDoubleVector& a) const {
        return fX * a.fY - fY * a.fX;
    }

    double dot(const SkDoubleVector& a) const {
        return fX * a.fX + fY * a.fY;
    }

    double length() const {
        return sqrt(fX * fX + fY * fY);
    }

    SkDoubleVector& normalize() {
        double inverseLength = sk_ieee_double_divide(1, this->length());
        fX *= inverseLength;
        fY *= inverseLength;
        return *this;
    }

    bool isFinite() const {
        return std::isfinite(fX) && std::isfinite(fY);
    }

    SkVector asSkVector() const {
        SkVector v = {SkDoubleToScalar(fX), SkDoubleToScalar(fY)};
        return v;
    }
};