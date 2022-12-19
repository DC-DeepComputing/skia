/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "src/core/SkMatrixUtils.h"

#include "include/core/SkMatrix.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkScalar.h"
#include "include/core/SkTypes.h"
#include "src/core/SkSamplingPriv.h"

#include <cmath>

bool SkTreatAsSprite(const SkMatrix& mat, const SkISize& size, const SkSamplingOptions& sampling,
                     const SkPaint& paint) {
    if (!SkSamplingPriv::NoChangeWithIdentityMatrix(sampling)) {
        return false;
    }

    // Our path aa is 2-bits, and our rect aa is 8, so we could use 8,
    // but in practice 4 seems enough (still looks smooth) and allows
    // more slightly fractional cases to fall into the fast (sprite) case.
    static const unsigned kAntiAliasSubpixelBits = 4;

    const unsigned subpixelBits = paint.isAntiAlias() ? kAntiAliasSubpixelBits : 0;

    // quick reject on affine or perspective
    if (mat.getType() & ~(SkMatrix::kScale_Mask | SkMatrix::kTranslate_Mask)) {
        return false;
    }

    // quick success check
    if (!subpixelBits && !(mat.getType() & ~SkMatrix::kTranslate_Mask)) {
        return true;
    }

    // mapRect supports negative scales, so we eliminate those first
    if (mat.getScaleX() < 0 || mat.getScaleY() < 0) {
        return false;
    }

    SkRect dst;
    SkIRect isrc = SkIRect::MakeSize(size);

    {
        SkRect src;
        src.set(isrc);
        mat.mapRect(&dst, src);
    }

    // just apply the translate to isrc
    isrc.offset(SkScalarRoundToInt(mat.getTranslateX()),
                SkScalarRoundToInt(mat.getTranslateY()));

    if (subpixelBits) {
        isrc.fLeft = SkLeftShift(isrc.fLeft, subpixelBits);
        isrc.fTop = SkLeftShift(isrc.fTop, subpixelBits);
        isrc.fRight = SkLeftShift(isrc.fRight, subpixelBits);
        isrc.fBottom = SkLeftShift(isrc.fBottom, subpixelBits);

        const float scale = 1 << subpixelBits;
        dst.fLeft *= scale;
        dst.fTop *= scale;
        dst.fRight *= scale;
        dst.fBottom *= scale;
    }

    SkIRect idst;
    dst.round(&idst);
    return isrc == idst;
}

// helper function to determine if upper-left 2x2 of matrix is degenerate
static inline bool is_degenerate_2x2(SkScalar scaleX, SkScalar skewX,
                                     SkScalar skewY,  SkScalar scaleY) {
    SkScalar perp_dot = scaleX*scaleY - skewX*skewY;
    return SkScalarNearlyZero(perp_dot, SK_ScalarNearlyZero*SK_ScalarNearlyZero);
}

// A square matrix M can be decomposed (via polar decomposition) into two matrices --
// an orthogonal matrix Q and a symmetric matrix S. In turn we can decompose S into U*W*U^T,
// where U is another orthogonal matrix and W is a scale matrix. These can be recombined
// to give M = (Q*U)*W*U^T, i.e., the product of two orthogonal matrices and a scale matrix.
//
// The one wrinkle is that traditionally Q may contain a reflection -- the
// calculation has been rejiggered to put that reflection into W.
bool SkDecomposeUpper2x2(const SkMatrix& matrix,
                         SkPoint* rotation1,
                         SkPoint* scale,
                         SkPoint* rotation2) {

    SkScalar A = matrix[SkMatrix::kMScaleX];
    SkScalar B = matrix[SkMatrix::kMSkewX];
    SkScalar C = matrix[SkMatrix::kMSkewY];
    SkScalar D = matrix[SkMatrix::kMScaleY];

    if (is_degenerate_2x2(A, B, C, D)) {
        return false;
    }

    double w1, w2;
    SkScalar cos1, sin1;
    SkScalar cos2, sin2;

    // do polar decomposition (M = Q*S)
    SkScalar cosQ, sinQ;
    double Sa, Sb, Sd;
    // if M is already symmetric (i.e., M = I*S)
    if (SkScalarNearlyEqual(B, C)) {
        cosQ = 1;
        sinQ = 0;

        Sa = A;
        Sb = B;
        Sd = D;
    } else {
        cosQ = A + D;
        sinQ = C - B;
        SkScalar reciplen = SkScalarInvert(SkScalarSqrt(cosQ*cosQ + sinQ*sinQ));
        cosQ *= reciplen;
        sinQ *= reciplen;

        // S = Q^-1*M
        // we don't calc Sc since it's symmetric
        Sa = A*cosQ + C*sinQ;
        Sb = B*cosQ + D*sinQ;
        Sd = -B*sinQ + D*cosQ;
    }

    // Now we need to compute eigenvalues of S (our scale factors)
    // and eigenvectors (bases for our rotation)
    // From this, should be able to reconstruct S as U*W*U^T
    if (SkScalarNearlyZero(SkDoubleToScalar(Sb))) {
        // already diagonalized
        cos1 = 1;
        sin1 = 0;
        w1 = Sa;
        w2 = Sd;
        cos2 = cosQ;
        sin2 = sinQ;
    } else {
        double diff = Sa - Sd;
        double discriminant = sqrt(diff*diff + 4.0*Sb*Sb);
        double trace = Sa + Sd;
        if (diff > 0) {
            w1 = 0.5*(trace + discriminant);
            w2 = 0.5*(trace - discriminant);
        } else {
            w1 = 0.5*(trace - discriminant);
            w2 = 0.5*(trace + discriminant);
        }

        cos1 = SkDoubleToScalar(Sb); sin1 = SkDoubleToScalar(w1 - Sa);
        SkScalar reciplen = SkScalarInvert(SkScalarSqrt(cos1*cos1 + sin1*sin1));
        cos1 *= reciplen;
        sin1 *= reciplen;

        // rotation 2 is composition of Q and U
        cos2 = cos1*cosQ - sin1*sinQ;
        sin2 = sin1*cosQ + cos1*sinQ;

        // rotation 1 is U^T
        sin1 = -sin1;
    }

    if (scale) {
        scale->fX = SkDoubleToScalar(w1);
        scale->fY = SkDoubleToScalar(w2);
    }
    if (rotation1) {
        rotation1->fX = cos1;
        rotation1->fY = sin1;
    }
    if (rotation2) {
        rotation2->fX = cos2;
        rotation2->fY = sin2;
    }

    return true;
}
