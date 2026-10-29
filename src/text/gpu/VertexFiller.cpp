/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/text/gpu/VertexFiller.h"

#include "include/core/SkMatrix.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkScalar.h"
#include "include/core/SkTypes.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkWriteBuffer.h"
#include "src/gpu/MaskFormat.h"
#include "src/text/gpu/SubRunAllocator.h"
#include "src/text/gpu/SubRunContainer.h"

#include <cmath>
#include <optional>

using MaskFormat = skgpu::MaskFormat;

namespace sktext::gpu {

bool VertexFiller::ValidDirectPosition(SkPoint pt) {
    // Single-precision floats have a 24-bit significand. At or below 2^21, floats can represent
    // all 1/4 subpixel increments (ULP <= 0.25f) and exact integers. Bounding direct coordinates
    // and offsets to 2^21 ensures that glyph bounds and sums remain well within the 2^24 exact
    // integer range, prevents subpixel rounding absorption (e.g. pt + 0.5f == pt), and avoids
    // 32-bit integer overflow.
    return ValidDirectCoordRange(pt) &&
           SkScalarIsInt(pt.x()) && SkScalarIsInt(pt.y());
}

VertexFiller::VertexFiller(MaskFormat maskFormat,
                           const SkMatrix& creationMatrix,
                           SkRect creationBounds,
                           SkSpan<const SkPoint> leftTop,
                           bool canDrawDirect)
        : fMaskFormat{maskFormat}
        , fCanDrawDirect{canDrawDirect}
        , fCreationMatrix{creationMatrix}
        , fCreationBounds{creationBounds}
        , fLeftTop{leftTop} {}

VertexFiller VertexFiller::Make(MaskFormat maskType,
                                const SkMatrix &creationMatrix,
                                SkRect creationBounds,
                                SkSpan<const SkPoint> positions,
                                SubRunAllocator *alloc,
                                FillerType fillerType) {
    SkSpan<SkPoint> leftTop = alloc->makePODSpan<SkPoint>(positions);
#if defined(SK_DEBUG)
    if (fillerType == kIsDirect) {
        for (const SkPoint& pt : leftTop) {
            SkASSERT(ValidDirectPosition(pt));
        }
    }
#endif
    return VertexFiller{
            maskType, creationMatrix, creationBounds, leftTop, fillerType == kIsDirect};
}

SkMatrix VertexFiller::viewDifference(const SkMatrix &positionMatrix) const {
    if (SkMatrix inverse; fCreationMatrix.invert(&inverse)) {
        return SkMatrix::Concat(positionMatrix, inverse);
    }
    return SkMatrix::I();
}

// Check for integer translate with the same 2x2 matrix.
// Returns the translation, and true if the change from creation matrix to the position matrix
// supports using direct glyph masks.
std::tuple<bool, SkVector> VertexFiller::CanUseDirect(
        const SkMatrix& creationMatrix, const SkMatrix& drawMatrix, SkPoint drawOrigin) {
    // Bound drawMatrix and drawOrigin translations to 1e14 to guarantee double precision
    // (53 significand bits) maintains exact integer and subpixel precision without absorption.
    constexpr double kMaxDoubleCoord = 1e14;
    if (!creationMatrix.isFinite() || !drawMatrix.isFinite() || !drawOrigin.isFinite() ||
        creationMatrix.hasPerspective() || drawMatrix.hasPerspective() ||
        !ValidDirectCoordRange({creationMatrix.getTranslateX(),
                                creationMatrix.getTranslateY()}) ||
        std::abs(drawMatrix.getTranslateX()) > kMaxDoubleCoord ||
        std::abs(drawMatrix.getTranslateY()) > kMaxDoubleCoord ||
        std::abs(drawOrigin.x()) > kMaxDoubleCoord ||
        std::abs(drawOrigin.y()) > kMaxDoubleCoord) {
        return {false, {0, 0}};
    }

    if (creationMatrix.rc(0, 0) != drawMatrix.rc(0, 0) ||
        creationMatrix.rc(0, 1) != drawMatrix.rc(0, 1) ||
        creationMatrix.rc(1, 0) != drawMatrix.rc(1, 0) ||
        creationMatrix.rc(1, 1) != drawMatrix.rc(1, 1)) {
        return {false, {0, 0}};
    }

    // Compute the mapped origin and offset in double precision (53-bit mantissa). In single
    // precision float, coordinates above 2^23 have an ULP >= 1.0f, which causes subpixel fractions
    // (such as 0.5f) to be absorbed by rounding, and floating-point cancellation between large
    // opposing translations (e.g. drawOrigin = 3e6 with drawMatrix = -3e6) can corrupt the
    // offset. Double precision represents integers and subpixel fractions exactly up to 2^52,
    // ensuring the net offset is calculated with full precision before checking if it is an
    // exact integer.
    double drawOriginX = drawOrigin.x();
    double drawOriginY = drawOrigin.y();
    double mappedOriginX = (double)drawMatrix.rc(0, 0) * drawOriginX +
                           (double)drawMatrix.rc(0, 1) * drawOriginY +
                           (double)drawMatrix.getTranslateX();
    double mappedOriginY = (double)drawMatrix.rc(1, 0) * drawOriginX +
                           (double)drawMatrix.rc(1, 1) * drawOriginY +
                           (double)drawMatrix.getTranslateY();

    double offsetX = mappedOriginX - (double)creationMatrix.getTranslateX();
    double offsetY = mappedOriginY - (double)creationMatrix.getTranslateY();
    SkVector offset = {(float)offsetX, (float)offsetY};

    // Check offsetX == (double)offset.x() to catch cases where a fractional double offset
    // like 10.0 + 1e-9 rounded to 10.0f upon float conversion, ensuring exact integer values.
    if (!ValidDirectPosition(SkPoint{offset.x(), offset.y()}) ||
        offsetX != (double)offset.x() || offsetY != (double)offset.y()) {
        return {false, {0, 0}};
    }

    return {true, offset};
}

bool VertexFiller::isLCD() const { return fMaskFormat == MaskFormat::kA565; }

// Return true if the positionMatrix represents an integer translation. Return the device
// bounding box of all the glyphs. If the bounding box is empty, then something went singular
// and this operation should be dropped.
std::tuple<bool, SkRect> VertexFiller::deviceRectAndCheckTransform(
            const SkMatrix &positionMatrix) const {
    if (fCanDrawDirect) {
        const auto [directDrawCompatible, offset] = CanUseDirect(fCreationMatrix, positionMatrix);

        if (directDrawCompatible) {
            return {true, fCreationBounds.makeOffset(offset)};
        }
    }

    if (SkMatrix inverse; fCreationMatrix.invert(&inverse)) {
        SkMatrix viewDifference = SkMatrix::Concat(positionMatrix, inverse);
        return {false, viewDifference.mapRect(fCreationBounds)};
    }

    // initialPositionMatrix is singular. Do nothing.
    return {false, SkRect::MakeEmpty()};
}

std::tuple<SkRect, SkMatrix> VertexFiller::boundsAndDeviceMatrix(const SkMatrix& localToDevice,
                                                                 SkPoint drawOrigin,
                                                                 bool supportBilerpAtlas) const {
    if (auto [directDrawCompatible, offset] =
                CanUseDirect(fCreationMatrix, localToDevice, drawOrigin);
        directDrawCompatible) {
        // The offset is an integer (but make sure), which means the generated mask can be
        // accessed without changing how texels would be sampled.
        return {fCreationBounds, SkMatrix::Translate(offset)};
    }
    if (fCanDrawDirect && !supportBilerpAtlas) {
        // Direct-mask glyphs have no padding in the atlas when atlas bilerp is unsupported.
        // Drawing with a fractional translation or non-identical matrix causes sampling
        // outside the glyph's atlas bounds, leaking neighboring atlas texels.
        return {SkRect::MakeEmpty(), SkMatrix::I()};
    }

    // Otherwise compute the relative transformation from fCreationMatrix to
    // localToDevice, with the drawOrigin applied. If fCreationMatrix or the
    // concatenation is not invertible the returned Transform is marked invalid and the draw
    // will be automatically dropped.
    SkMatrix positionMatrix = localToDevice;
    positionMatrix.preTranslate(drawOrigin.x(), drawOrigin.y());
    const SkMatrix viewDifference = this->viewDifference(positionMatrix);
    return {fCreationBounds, viewDifference};
}

}  // namespace sktext::gpu
