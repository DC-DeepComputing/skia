/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/graphite/sparse_strips/CoverageTestUtils.h"

#include "include/core/SkCanvas.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "include/core/SkPathBuilder.h"
#include "include/core/SkPathTypes.h"
#include "include/core/SkPathUtils.h"
#include "include/core/SkPicture.h"
#include "include/core/SkPoint.h"
#include "include/core/SkStream.h"
#include "include/core/SkString.h"
#include "src/core/SkCubics.h"
#include "src/core/SkQuads.h"
#include "src/gpu/graphite/sparse_strips/Polyline.h"
#include "src/gpu/graphite/sparse_strips/SparseStripsTypes.h"
#include "src/gpu/graphite/sparse_strips/Tiler.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <vector>

namespace skgpu::graphite {

void ScanlineOracle8x::intersectLine(const SkPoint pts[2], skvx::float8 targetY) {
    float dy = pts[1].fY - pts[0].fY;
    if (dy != 0.0f) {
        float invDy = 1.0f / dy;
        float dx = pts[1].fX - pts[0].fX;
        int32_t dir = dy > 0.0f ? 1 : -1;

        skvx::float8 t = (targetY - pts[0].fY) * invDy;
        auto valid = (t >= 0.0f) & (t < 1.0f);
        if (any(valid)) {
            CurveIntersection hit;
            hit.x = pts[0].fX + t * dx;
            hit.dir = if_then_else(valid, skvx::int8(dir), skvx::int8(0));
            fHits.push_back(hit);
        }
    }
}

void ScanlineOracle8x::intersectQuad(const SkPoint pts[3], skvx::float8 targetY) {
    double Ay = pts[2].fY - 2.0 * pts[1].fY + pts[0].fY;
    double By = 2.0 * (pts[1].fY - pts[0].fY);
    double Cy = pts[0].fY;

    double Ax = pts[2].fX - 2.0 * pts[1].fX + pts[0].fX;
    double Bx = 2.0 * (pts[1].fX - pts[0].fX);
    double Cx = pts[0].fX;

    CurveIntersection hits[2];
    for (int r = 0; r < 2; ++r) {
        hits[r].x = 0.0f;
        hits[r].dir = 0;
    }

    for (int k = 0; k < 8; ++k) {
        double roots[2];
        int numRoots = SkQuads::RootsReal(Ay, By, Cy - targetY[k], roots);
        int validCount = 0;
        for (int i = 0; i < numRoots; ++i) {
            double t = roots[i];
            if (t >= 0.0 && t < 1.0) {
                float derivativeY = static_cast<float>(2.0 * Ay * t + By);
                if (std::abs(derivativeY) > 1e-5f) {
                    float x = static_cast<float>(Ax * t * t + Bx * t + Cx);
                    int32_t dir = derivativeY > 0.0f ? 1 : -1;
                    if (validCount < 2) {
                        hits[validCount].x[k] = x;
                        hits[validCount].dir[k] = dir;
                        validCount++;
                    }
                }
            }
        }
    }

    for (int r = 0; r < 2; ++r) {
        if (any(hits[r].dir != 0)) {
            fHits.push_back(hits[r]);
        }
    }
}

void ScanlineOracle8x::intersectConic(
        const SkPoint pts[3], float weight, skvx::float8 targetY) {
    CurveIntersection hits[2];
    for (int r = 0; r < 2; ++r) {
        hits[r].x = 0.0f;
        hits[r].dir = 0;
    }

    for (int k = 0; k < 8; ++k) {
        double d0 = pts[0].fY - targetY[k];
        double d1 = pts[1].fY - targetY[k];
        double d2 = pts[2].fY - targetY[k];

        double Ay = d2 - 2.0 * weight * d1 + d0;
        double By = 2.0 * (weight * d1 - d0);
        double Cy = d0;

        double roots[2];
        int numRoots = SkQuads::RootsReal(Ay, By, Cy, roots);
        int validCount = 0;
        for (int i = 0; i < numRoots; ++i) {
            double t = roots[i];
            if (t >= 0.0 && t < 1.0) {
                double denom = (1.0 - t) * (1.0 - t) + 2.0 * weight * t * (1.0 - t) + t * t;
                if (denom != 0.0) {
                    double numX = (1.0 - t) * (1.0 - t) * pts[0].fX +
                                   2.0 * weight * t * (1.0 - t) * pts[1].fX +
                                   t * t * pts[2].fX;
                    float x = static_cast<float>(numX / denom);

                    // denom is always positive, so we only need the numerator to find the sign
                    float derivativeY = static_cast<float>(2.0 * Ay * t + By);
                    if (std::abs(derivativeY) > 1e-5f) {
                        int32_t dir = derivativeY > 0.0f ? 1 : -1;
                        if (validCount < 2) {
                            hits[validCount].x[k] = x;
                            hits[validCount].dir[k] = dir;
                            validCount++;
                        }
                    }
                }
            }
        }
    }

    for (int r = 0; r < 2; ++r) {
        if (any(hits[r].dir != 0)) {
            fHits.push_back(hits[r]);
        }
    }
}

void ScanlineOracle8x::intersectCubic(
        const SkPoint pts[4], skvx::float8 targetY) {
    double Ay = pts[3].fY - 3.0 * pts[2].fY + 3.0 * pts[1].fY - pts[0].fY;
    double By = 3.0 * (pts[2].fY - 2.0 * pts[1].fY + pts[0].fY);
    double Cy = 3.0 * (pts[1].fY - pts[0].fY);
    double Dy = pts[0].fY;

    double Ax = pts[3].fX - 3.0 * pts[2].fX + 3.0 * pts[1].fX - pts[0].fX;
    double Bx = 3.0 * (pts[2].fX - 2.0 * pts[1].fX + pts[0].fX);
    double Cx = 3.0 * (pts[1].fX - pts[0].fX);
    double Dx = pts[0].fX;

    CurveIntersection hits[3];
    for (int r = 0; r < 3; ++r) {
        hits[r].x = 0.0f;
        hits[r].dir = 0;
    }

    for (int k = 0; k < 8; ++k) {
        double roots[3];
        int numRoots = SkCubics::RootsReal(Ay, By, Cy, Dy - targetY[k], roots);
        int validCount = 0;
        for (int i = 0; i < numRoots; ++i) {
            double t = roots[i];
            if (t >= 0.0 && t < 1.0) {
                float x = static_cast<float>(SkCubics::EvalAt(Ax, Bx, Cx, Dx, t));
                float derivativeY = static_cast<float>(3.0 * Ay * t * t + 2.0 * By * t + Cy);
                if (std::abs(derivativeY) > 1e-5f) {
                    int32_t dir = derivativeY > 0.0f ? 1 : -1;
                    if (validCount < 3) {
                        hits[validCount].x[k] = x;
                        hits[validCount].dir[k] = dir;
                        validCount++;
                    }
                }
            }
        }
    }

    for (int r = 0; r < 3; ++r) {
        if (any(hits[r].dir != 0)) {
            fHits.push_back(hits[r]);
        }
    }
}

void ScanlineOracle8x::collectPathIntersections(skvx::float8 targetY) {
    SkASSERT(fPath);
    this->collectPathIntersections(*fPath, targetY);
}

void ScanlineOracle8x::collectPathIntersections(
        const SkPath& path, skvx::float8 targetY) {
    fHits.clear();
    SkPath::Iter iter(path, true);
    SkPoint pts[4];
    SkPath::Verb verb;

    while ((verb = iter.next(pts)) != SkPath::kDone_Verb) {
        switch (verb) {
            case SkPath::kLine_Verb:
                this->intersectLine(pts, targetY);
                break;
            case SkPath::kQuad_Verb:
                this->intersectQuad(pts, targetY);
                break;
            case SkPath::kConic_Verb:
                this->intersectConic(pts, iter.conicWeight(), targetY);
                break;
            case SkPath::kCubic_Verb:
                this->intersectCubic(pts, targetY);
                break;
            default:
                break;
        }
    }
}

skvx::int8 ScanlineOracle8x::evaluatePixelWinding(float px) const {
    static const skvx::float8 kSubX =
            (skvx::cast<float>(skvx::byte8::Load(kMsaaPattern<uint8_t>.data())) + 0.5f) / 8.0f;
    skvx::float8 vPx = px + kSubX;
    skvx::int8 pixelWinding(0);

    for (const auto& hit : fHits) {
        pixelWinding += if_then_else(vPx >= hit.x, hit.dir, skvx::int8(0));
    }
    return pixelWinding;
}

std::vector<ScanlineOracle8x::RowWindingInterval> ScanlineOracle8x::buildRowIntervals(int py) {
    SkASSERT(fPath);
    return this->buildRowIntervals(py, *fPath);
}

std::vector<ScanlineOracle8x::RowWindingInterval> ScanlineOracle8x::buildRowIntervals(
        int py, const SkPath& path) {
    static const skvx::float8 kSubY =
            (skvx::float8{0.0f, 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f} + 0.5f) / 8.0f;
    skvx::float8 vPy = static_cast<float>(py) + kSubY;

    this->collectPathIntersections(path, vPy);
    if (fHits.empty()) {
        return {};
    }

    SkRect bounds = path.getBounds();
    int minX = static_cast<int>(std::floor(bounds.fLeft));
    int maxX = static_cast<int>(std::ceil(bounds.fRight));

    std::vector<RowWindingInterval> intervals;
    skvx::int8 lastWinding(0);
    bool hasLast = false;

    for (int px = minX; px <= maxX; ++px) {
        skvx::int8 w = this->evaluatePixelWinding(static_cast<float>(px));
        if (!hasLast || any(w != lastWinding)) {
            intervals.push_back({px, w});
            lastWinding = w;
            hasLast = true;
        }
    }

    // Append terminator at maxX + 1 to define the upper bound of the final span [x_k, maxX + 1)
    intervals.push_back({maxX + 1, skvx::int8(0)});

    return intervals;
}

skvx::int8 ScanlineOracle8x::evaluate(SkPoint pt) {
    SkASSERT(fPath);
    return this->evaluate(pt, *fPath);
}

skvx::int8 ScanlineOracle8x::evaluate(SkPoint pt, const SkPath& path) {
    static const skvx::float8 kSubY =
            (skvx::float8{0.0f, 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f} + 0.5f) / 8.0f;
    skvx::float8 vPy = pt.fY + kSubY;

    this->collectPathIntersections(path, vPy);

    return this->evaluatePixelWinding(pt.fX);
}

// -------------------------------------------------------------------------------------------------
// 3. SkpPathExtractor Implementation
// -------------------------------------------------------------------------------------------------
namespace {

class PathSnifferCanvas : public SkCanvas {
public:
    PathSnifferCanvas(int width, int height) : SkCanvas(width, height, nullptr) {}

    const std::vector<SkpExtractedPath>& extractedPaths() const { return fPaths; }

protected:
    void onDrawPath(const SkPath& path, const SkPaint& paint) override {
        this->recordPath(path, paint);
    }

    void onDrawRect(const SkRect& rect, const SkPaint& paint) override {
        this->recordPath(SkPath::Rect(rect), paint);
    }

    void onDrawRRect(const SkRRect& rrect, const SkPaint& paint) override {
        this->recordPath(SkPath::RRect(rrect), paint);
    }

    void onDrawOval(const SkRect& oval, const SkPaint& paint) override {
        this->recordPath(SkPath::Oval(oval), paint);
    }

    void onDrawArc(const SkRect& oval, SkScalar startAngle, SkScalar sweepAngle,
                   bool useCenter, const SkPaint& paint) override {
        SkPathBuilder b;
        b.addArc(oval, startAngle, sweepAngle);
        if (useCenter) {
            b.lineTo(oval.centerX(), oval.centerY());
            b.close();
        }
        this->recordPath(b.detach(), paint);
    }

    void onDrawDRRect(const SkRRect& outer, const SkRRect& inner, const SkPaint& paint) override {
        SkPathBuilder b;
        b.addRRect(outer, SkPathDirection::kCW);
        b.addRRect(inner, SkPathDirection::kCCW);
        this->recordPath(b.detach(), paint);
    }

    void onDrawPoints(PointMode mode, size_t count, const SkPoint pts[],
                      const SkPaint& paint) override {
        if (count == 0) return;
        SkPathBuilder b;
        if (mode == kPoints_PointMode) {
            for (size_t i = 0; i < count; ++i) {
                b.moveTo(pts[i]);
                b.lineTo(pts[i]);
            }
        } else if (mode == kLines_PointMode) {
            for (size_t i = 0; i + 1 < count; i += 2) {
                b.moveTo(pts[i]);
                b.lineTo(pts[i + 1]);
            }
        } else if (mode == kPolygon_PointMode) {
            b.moveTo(pts[0]);
            for (size_t i = 1; i < count; ++i) {
                b.lineTo(pts[i]);
            }
        }
        this->recordPath(b.detach(), paint);
    }

private:
    void recordPath(const SkPath& path, const SkPaint& paint) {
        SkMatrix totalMat = this->getTotalMatrix();
        SkPath devPath;
        if (paint.getStyle() == SkPaint::kStroke_Style) {
            if (paint.getStrokeWidth() == 0.0f) {
                // Hairline: transform to device space first, then stroke with 1.0px width
                SkPath devSrc = path.makeTransform(totalMat);
                SkPaint strokePaint = paint;
                strokePaint.setStyle(SkPaint::kStroke_Style);
                strokePaint.setStrokeWidth(1.0f);
                devPath = skpathutils::FillPathWithPaint(devSrc, strokePaint);
            } else {
                SkPathBuilder builder;
                skpathutils::FillPathWithPaint(path, paint, &builder, nullptr, totalMat);
                devPath = builder.detach().makeTransform(totalMat);
            }
        } else if (paint.getStyle() == SkPaint::kStrokeAndFill_Style) {
            if (paint.getStrokeWidth() == 0.0f) {
                devPath = path.makeTransform(totalMat);
            } else {
                SkPathBuilder builder;
                skpathutils::FillPathWithPaint(path, paint, &builder, nullptr, totalMat);
                devPath = builder.detach().makeTransform(totalMat);
            }
        } else {
            devPath = path.makeTransform(totalMat);
        }
        fPaths.push_back({path, paint, totalMat, devPath});
    }

    std::vector<SkpExtractedPath> fPaths;
};

}  // namespace

std::vector<SkpExtractedPath> SkpPathExtractor::ExtractPaths(const char* filepath) {
    SkFILEStream stream(filepath);
    if (!stream.isValid()) {
        return {};
    }
    return ExtractPaths(&stream);
}

std::vector<SkpExtractedPath> SkpPathExtractor::ExtractPaths(SkStream* stream) {
    if (!stream) {
        return {};
    }
    sk_sp<SkPicture> picture = SkPicture::MakeFromStream(stream, nullptr);
    if (!picture) {
        return {};
    }
    return ExtractPaths(picture.get());
}

std::vector<SkpExtractedPath> SkpPathExtractor::ExtractPaths(const SkPicture* picture) {
    if (!picture) {
        return {};
    }
    SkRect cull = picture->cullRect();
    int width = std::max(1, static_cast<int>(std::ceil(cull.width())));
    int height = std::max(1, static_cast<int>(std::ceil(cull.height())));

    PathSnifferCanvas canvas(width, height);
    picture->playback(&canvas);
    return canvas.extractedPaths();
}

}  // namespace skgpu::graphite
