/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkSpan.h"
#include "include/effects/SkColorMatrix.h"
#include "src/core/SkYUVMath.h"

enum SkYUVColorSpace : int;

SkColorMatrix SkColorMatrix::RGBtoYUV(SkYUVColorSpace cs) {
    SkColorMatrix m;
    SkColorMatrix_RGB2YUV(cs, m.fMat.data());
    return m;
}

SkColorMatrix SkColorMatrix::YUVtoRGB(SkYUVColorSpace cs) {
    SkColorMatrix m;
    SkColorMatrix_YUV2RGB(cs, m.fMat.data());
    return m;
}

enum {
    kR_Scale = 0,
    kG_Scale = 6,
    kB_Scale = 12,
    kA_Scale = 18,

    kR_Trans = 4,
    kG_Trans = 9,
    kB_Trans = 14,
    kA_Trans = 19,
};

static void set_concat(SkSpan<float, 20> result, SkSpan<const float, 20> outer, SkSpan<const float, 20> inner) {
    float    tmp[20];
    SkSpan<float>target;

    if (outer.data() == result.data() || inner.data() == result.data()) {
        target = tmp;   // will memcpy answer when we're done into result
    } else {
        target = result;
    }

    int index = 0;
    for (int j = 0; j < 20; j += 5) {
        for (int i = 0; i < 4; i++) {
            target[index++] =   outer[j + 0] * inner[i + 0] +
                                outer[j + 1] * inner[i + 5] +
                                outer[j + 2] * inner[i + 10] +
                                outer[j + 3] * inner[i + 15];
        }
        target[index++] =   outer[j + 0] * inner[4] +
                            outer[j + 1] * inner[9] +
                            outer[j + 2] * inner[14] +
                            outer[j + 3] * inner[19] +
                            outer[j + 4];
    }

    if (target.data() != result.data()) {
        std::copy_n(target.data(), 20, result.data());
    }
}

void SkColorMatrix::setIdentity() {
    fMat.fill(0.0f);
    fMat[kR_Scale] = fMat[kG_Scale] = fMat[kB_Scale] = fMat[kA_Scale] = 1;
}

void SkColorMatrix::setScale(float rScale, float gScale, float bScale, float aScale) {
    fMat.fill(0.0f);
    fMat[kR_Scale] = rScale;
    fMat[kG_Scale] = gScale;
    fMat[kB_Scale] = bScale;
    fMat[kA_Scale] = aScale;
}

void SkColorMatrix::postTranslate(float dr, float dg, float db, float da) {
    fMat[kR_Trans] += dr;
    fMat[kG_Trans] += dg;
    fMat[kB_Trans] += db;
    fMat[kA_Trans] += da;
}

///////////////////////////////////////////////////////////////////////////////

void SkColorMatrix::setConcat(const SkColorMatrix& matA, const SkColorMatrix& matB) {
    set_concat(fMat, matA.fMat, matB.fMat);
}

///////////////////////////////////////////////////////////////////////////////

static void setrow(SkSpan<float> row, float r, float g, float b) {
    row[0] = r;
    row[1] = g;
    row[2] = b;
}

static const float kHueR = 0.213f;
static const float kHueG = 0.715f;
static const float kHueB = 0.072f;

void SkColorMatrix::setSaturation(float sat) {
    fMat.fill(0.0f);

    const float R = kHueR * (1 - sat);
    const float G = kHueG * (1 - sat);
    const float B = kHueB * (1 - sat);

    setrow(SkSpan(fMat).subspan(0), R + sat, G, B);
    setrow(SkSpan(fMat).subspan(5), R, G + sat, B);
    setrow(SkSpan(fMat).subspan(10), R, G, B + sat);
    fMat[kA_Scale] = 1;
}
