/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/base/SkArenaAlloc.h"
#include "src/core/SkAutoBlitterChoose.h"
#include "src/core/SkBlendModePriv.h"
#include "src/core/SkBlenderBase.h"
#include "src/core/SkBlitter.h"
#include "src/core/SkColorSpacePriv.h"
#include "src/core/SkColorSpaceXformSteps.h"
#include "src/core/SkCoreBlitters.h"
#include "src/core/SkDevice.h"
#include "src/core/SkDraw.h"
#include "src/core/SkMatrixProvider.h"
#include "src/core/SkPathEffectBase.h"
#include "src/core/SkRasterClip.h"
#include "src/core/SkRasterPipeline.h"
#include "src/core/SkRectPriv.h"
#include "src/core/SkScan.h"
#include "src/shaders/SkTransformShader.h"

using namespace skia_private;

SkDraw::SkDraw() {
    fBlitterChooser = SkBlitter::Choose;
}

SkDraw::SkDraw(const SkDrawBase& b) : SkDrawBase(b) {
    fBlitterChooser = SkBlitter::Choose;
}

struct PtProcRec {
    SkCanvas::PointMode fMode;
    const SkPaint*  fPaint;
    const SkRegion* fClip;
    const SkRasterClip* fRC;

    // computed values
    SkRect   fClipBounds;
    SkScalar fRadius;

    typedef void (*Proc)(const PtProcRec&, const SkPoint devPts[], int count,
                         SkBlitter*);

    bool init(SkCanvas::PointMode, const SkPaint&, const SkMatrix* matrix,
              const SkRasterClip*);
    Proc chooseProc(SkBlitter** blitter);

private:
    SkAAClipBlitterWrapper fWrapper;
};

static void bw_pt_rect_hair_proc(const PtProcRec& rec, const SkPoint devPts[],
                                 int count, SkBlitter* blitter) {
    SkASSERT(rec.fClip->isRect());
    const SkIRect& r = rec.fClip->getBounds();

    for (int i = 0; i < count; i++) {
        int x = SkScalarFloorToInt(devPts[i].fX);
        int y = SkScalarFloorToInt(devPts[i].fY);
        if (r.contains(x, y)) {
            blitter->blitH(x, y, 1);
        }
    }
}

static void bw_pt_rect_16_hair_proc(const PtProcRec& rec,
                                    const SkPoint devPts[], int count,
                                    SkBlitter* blitter) {
    SkASSERT(rec.fRC->isRect());
    const SkIRect& r = rec.fRC->getBounds();
    uint32_t value;
    const SkPixmap* dst = blitter->justAnOpaqueColor(&value);
    SkASSERT(dst);

    uint16_t* addr = dst->writable_addr16(0, 0);
    size_t    rb = dst->rowBytes();

    for (int i = 0; i < count; i++) {
        int x = SkScalarFloorToInt(devPts[i].fX);
        int y = SkScalarFloorToInt(devPts[i].fY);
        if (r.contains(x, y)) {
            ((uint16_t*)((char*)addr + y * rb))[x] = SkToU16(value);
        }
    }
}

static void bw_pt_rect_32_hair_proc(const PtProcRec& rec,
                                    const SkPoint devPts[], int count,
                                    SkBlitter* blitter) {
    SkASSERT(rec.fRC->isRect());
    const SkIRect& r = rec.fRC->getBounds();
    uint32_t value;
    const SkPixmap* dst = blitter->justAnOpaqueColor(&value);
    SkASSERT(dst);

    SkPMColor* addr = dst->writable_addr32(0, 0);
    size_t     rb = dst->rowBytes();

    for (int i = 0; i < count; i++) {
        int x = SkScalarFloorToInt(devPts[i].fX);
        int y = SkScalarFloorToInt(devPts[i].fY);
        if (r.contains(x, y)) {
            ((SkPMColor*)((char*)addr + y * rb))[x] = value;
        }
    }
}

static void bw_pt_hair_proc(const PtProcRec& rec, const SkPoint devPts[],
                            int count, SkBlitter* blitter) {
    for (int i = 0; i < count; i++) {
        int x = SkScalarFloorToInt(devPts[i].fX);
        int y = SkScalarFloorToInt(devPts[i].fY);
        if (rec.fClip->contains(x, y)) {
            blitter->blitH(x, y, 1);
        }
    }
}

static void bw_line_hair_proc(const PtProcRec& rec, const SkPoint devPts[],
                              int count, SkBlitter* blitter) {
    for (int i = 0; i < count; i += 2) {
        SkScan::HairLine(&devPts[i], 2, *rec.fRC, blitter);
    }
}

static void bw_poly_hair_proc(const PtProcRec& rec, const SkPoint devPts[],
                              int count, SkBlitter* blitter) {
    SkScan::HairLine(devPts, count, *rec.fRC, blitter);
}

// aa versions

static void aa_line_hair_proc(const PtProcRec& rec, const SkPoint devPts[],
                              int count, SkBlitter* blitter) {
    for (int i = 0; i < count; i += 2) {
        SkScan::AntiHairLine(&devPts[i], 2, *rec.fRC, blitter);
    }
}

static void aa_poly_hair_proc(const PtProcRec& rec, const SkPoint devPts[],
                              int count, SkBlitter* blitter) {
    SkScan::AntiHairLine(devPts, count, *rec.fRC, blitter);
}

// square procs (strokeWidth > 0 but matrix is square-scale (sx == sy)

static SkRect make_square_rad(SkPoint center, SkScalar radius) {
    return {
        center.fX - radius, center.fY - radius,
        center.fX + radius, center.fY + radius
    };
}

static SkXRect make_xrect(const SkRect& r) {
    SkASSERT(SkRectPriv::FitsInFixed(r));
    return {
        SkScalarToFixed(r.fLeft), SkScalarToFixed(r.fTop),
        SkScalarToFixed(r.fRight), SkScalarToFixed(r.fBottom)
    };
}

static void bw_square_proc(const PtProcRec& rec, const SkPoint devPts[],
                           int count, SkBlitter* blitter) {
    for (int i = 0; i < count; i++) {
        SkRect r = make_square_rad(devPts[i], rec.fRadius);
        if (r.intersect(rec.fClipBounds)) {
            SkScan::FillXRect(make_xrect(r), *rec.fRC, blitter);
        }
    }
}

static void aa_square_proc(const PtProcRec& rec, const SkPoint devPts[],
                           int count, SkBlitter* blitter) {
    for (int i = 0; i < count; i++) {
        SkRect r = make_square_rad(devPts[i], rec.fRadius);
        if (r.intersect(rec.fClipBounds)) {
            SkScan::AntiFillXRect(make_xrect(r), *rec.fRC, blitter);
        }
    }
}

// If this returns true, then chooseProc() must return a valid proc
bool PtProcRec::init(SkCanvas::PointMode mode, const SkPaint& paint,
                     const SkMatrix* matrix, const SkRasterClip* rc) {
    if ((unsigned)mode > (unsigned)SkCanvas::kPolygon_PointMode) {
        return false;
    }
    if (paint.getPathEffect() || paint.getMaskFilter()) {
        return false;
    }
    SkScalar width = paint.getStrokeWidth();
    SkScalar radius = -1;   // sentinel value, a "valid" value must be > 0

    if (0 == width) {
        radius = 0.5f;
    } else if (paint.getStrokeCap() != SkPaint::kRound_Cap &&
               matrix->isScaleTranslate() && SkCanvas::kPoints_PointMode == mode) {
        SkScalar sx = matrix->get(SkMatrix::kMScaleX);
        SkScalar sy = matrix->get(SkMatrix::kMScaleY);
        if (SkScalarNearlyZero(sx - sy)) {
            radius = SkScalarHalf(width * SkScalarAbs(sx));
        }
    }
    if (radius > 0) {
        SkRect clipBounds = SkRect::Make(rc->getBounds());
        // if we return true, the caller may assume that the constructed shapes can be represented
        // using SkFixed (after clipping), so we preflight that here.
        if (!SkRectPriv::FitsInFixed(clipBounds)) {
            return false;
        }
        fMode = mode;
        fPaint = &paint;
        fClip = nullptr;
        fRC = rc;
        fClipBounds = clipBounds;
        fRadius = radius;
        return true;
    }
    return false;
}

PtProcRec::Proc PtProcRec::chooseProc(SkBlitter** blitterPtr) {
    Proc proc = nullptr;

    SkBlitter* blitter = *blitterPtr;
    if (fRC->isBW()) {
        fClip = &fRC->bwRgn();
    } else {
        fWrapper.init(*fRC, blitter);
        fClip = &fWrapper.getRgn();
        blitter = fWrapper.getBlitter();
        *blitterPtr = blitter;
    }

    // for our arrays
    SkASSERT(0 == SkCanvas::kPoints_PointMode);
    SkASSERT(1 == SkCanvas::kLines_PointMode);
    SkASSERT(2 == SkCanvas::kPolygon_PointMode);
    SkASSERT((unsigned)fMode <= (unsigned)SkCanvas::kPolygon_PointMode);

    if (fPaint->isAntiAlias()) {
        if (0 == fPaint->getStrokeWidth()) {
            static const Proc gAAProcs[] = {
                aa_square_proc, aa_line_hair_proc, aa_poly_hair_proc
            };
            proc = gAAProcs[fMode];
        } else if (fPaint->getStrokeCap() != SkPaint::kRound_Cap) {
            SkASSERT(SkCanvas::kPoints_PointMode == fMode);
            proc = aa_square_proc;
        }
    } else {    // BW
        if (fRadius <= 0.5f) {    // small radii and hairline
            if (SkCanvas::kPoints_PointMode == fMode && fClip->isRect()) {
                uint32_t value;
                const SkPixmap* bm = blitter->justAnOpaqueColor(&value);
                if (bm && kRGB_565_SkColorType == bm->colorType()) {
                    proc = bw_pt_rect_16_hair_proc;
                } else if (bm && kN32_SkColorType == bm->colorType()) {
                    proc = bw_pt_rect_32_hair_proc;
                } else {
                    proc = bw_pt_rect_hair_proc;
                }
            } else {
                static Proc gBWProcs[] = {
                    bw_pt_hair_proc, bw_line_hair_proc, bw_poly_hair_proc
                };
                proc = gBWProcs[fMode];
            }
        } else {
            proc = bw_square_proc;
        }
    }
    return proc;
}

// each of these costs 8-bytes of stack space, so don't make it too large
// must be even for lines/polygon to work
#define MAX_DEV_PTS     32

void SkDraw::drawPoints(SkCanvas::PointMode mode, size_t count,
                        const SkPoint pts[], const SkPaint& paint,
                        SkBaseDevice* device) const {
    // if we're in lines mode, force count to be even
    if (SkCanvas::kLines_PointMode == mode) {
        count &= ~(size_t)1;
    }

    if ((long)count <= 0) {
        return;
    }

    SkASSERT(pts != nullptr);
    SkDEBUGCODE(this->validate();)

     // nothing to draw
    if (fRC->isEmpty()) {
        return;
    }

    if (!SkScalarsAreFinite(&pts[0].fX, count * 2)) {
        return;
    }

    SkMatrix ctm = fMatrixProvider->localToDevice();
    PtProcRec rec;
    if (!device && rec.init(mode, paint, &ctm, fRC)) {
        SkAutoBlitterChoose blitter(*this, nullptr, paint);

        SkPoint             devPts[MAX_DEV_PTS];
        SkBlitter*          bltr = blitter.get();
        PtProcRec::Proc     proc = rec.chooseProc(&bltr);
        // we have to back up subsequent passes if we're in polygon mode
        const size_t backup = (SkCanvas::kPolygon_PointMode == mode);

        do {
            int n = SkToInt(count);
            if (n > MAX_DEV_PTS) {
                n = MAX_DEV_PTS;
            }
            ctm.mapPoints(devPts, pts, n);
            if (!SkScalarsAreFinite(&devPts[0].fX, n * 2)) {
                return;
            }
            proc(rec, devPts, n, bltr);
            pts += n - backup;
            SkASSERT(SkToInt(count) >= n);
            count -= n;
            if (count > 0) {
                count += backup;
            }
        } while (count != 0);
    } else {
        switch (mode) {
            case SkCanvas::kPoints_PointMode: {
                // temporarily mark the paint as filling.
                SkPaint newPaint(paint);
                newPaint.setStyle(SkPaint::kFill_Style);

                SkScalar width = newPaint.getStrokeWidth();
                SkScalar radius = SkScalarHalf(width);

                if (newPaint.getStrokeCap() == SkPaint::kRound_Cap) {
                    if (device) {
                        for (size_t i = 0; i < count; ++i) {
                            SkRect r = SkRect::MakeLTRB(pts[i].fX - radius, pts[i].fY - radius,
                                                        pts[i].fX + radius, pts[i].fY + radius);
                            device->drawOval(r, newPaint);
                        }
                    } else {
                        SkPath     path;
                        SkMatrix   preMatrix;

                        path.addCircle(0, 0, radius);
                        for (size_t i = 0; i < count; i++) {
                            preMatrix.setTranslate(pts[i].fX, pts[i].fY);
                            // pass true for the last point, since we can modify
                            // then path then
                            path.setIsVolatile((count-1) == i);
                            this->drawPath(path, newPaint, &preMatrix, (count-1) == i);
                        }
                    }
                } else {
                    SkRect  r;

                    for (size_t i = 0; i < count; i++) {
                        r.fLeft = pts[i].fX - radius;
                        r.fTop = pts[i].fY - radius;
                        r.fRight = r.fLeft + width;
                        r.fBottom = r.fTop + width;
                        if (device) {
                            device->drawRect(r, newPaint);
                        } else {
                            this->drawRect(r, newPaint);
                        }
                    }
                }
                break;
            }
            case SkCanvas::kLines_PointMode:
                if (2 == count && paint.getPathEffect()) {
                    // most likely a dashed line - see if it is one of the ones
                    // we can accelerate
                    SkStrokeRec stroke(paint);
                    SkPathEffectBase::PointData pointData;

                    SkPath path = SkPath::Line(pts[0], pts[1]);

                    SkRect cullRect = SkRect::Make(fRC->getBounds());

                    if (as_PEB(paint.getPathEffect())->asPoints(&pointData, path, stroke, ctm,
                                                                &cullRect)) {
                        // 'asPoints' managed to find some fast path

                        SkPaint newP(paint);
                        newP.setPathEffect(nullptr);
                        newP.setStyle(SkPaint::kFill_Style);

                        if (!pointData.fFirst.isEmpty()) {
                            if (device) {
                                device->drawPath(pointData.fFirst, newP);
                            } else {
                                this->drawPath(pointData.fFirst, newP);
                            }
                        }

                        if (!pointData.fLast.isEmpty()) {
                            if (device) {
                                device->drawPath(pointData.fLast, newP);
                            } else {
                                this->drawPath(pointData.fLast, newP);
                            }
                        }

                        if (pointData.fSize.fX == pointData.fSize.fY) {
                            // The rest of the dashed line can just be drawn as points
                            SkASSERT(pointData.fSize.fX == SkScalarHalf(newP.getStrokeWidth()));

                            if (SkPathEffectBase::PointData::kCircles_PointFlag & pointData.fFlags) {
                                newP.setStrokeCap(SkPaint::kRound_Cap);
                            } else {
                                newP.setStrokeCap(SkPaint::kButt_Cap);
                            }

                            if (device) {
                                device->drawPoints(SkCanvas::kPoints_PointMode,
                                                   pointData.fNumPoints,
                                                   pointData.fPoints,
                                                   newP);
                            } else {
                                this->drawPoints(SkCanvas::kPoints_PointMode,
                                                 pointData.fNumPoints,
                                                 pointData.fPoints,
                                                 newP,
                                                 device);
                            }
                            break;
                        } else {
                            // The rest of the dashed line must be drawn as rects
                            SkASSERT(!(SkPathEffectBase::PointData::kCircles_PointFlag &
                                      pointData.fFlags));

                            SkRect r;

                            for (int i = 0; i < pointData.fNumPoints; ++i) {
                                r.setLTRB(pointData.fPoints[i].fX - pointData.fSize.fX,
                                          pointData.fPoints[i].fY - pointData.fSize.fY,
                                          pointData.fPoints[i].fX + pointData.fSize.fX,
                                          pointData.fPoints[i].fY + pointData.fSize.fY);
                                if (device) {
                                    device->drawRect(r, newP);
                                } else {
                                    this->drawRect(r, newP);
                                }
                            }
                        }

                        break;
                    }
                }
                [[fallthrough]]; // couldn't take fast path
            case SkCanvas::kPolygon_PointMode: {
                count -= 1;
                SkPath path;
                SkPaint p(paint);
                p.setStyle(SkPaint::kStroke_Style);
                size_t inc = (SkCanvas::kLines_PointMode == mode) ? 2 : 1;
                path.setIsVolatile(true);
                for (size_t i = 0; i < count; i += inc) {
                    path.moveTo(pts[i]);
                    path.lineTo(pts[i+1]);
                    if (device) {
                        device->drawPath(path, p, true);
                    } else {
                        this->drawPath(path, p, nullptr, true);
                    }
                    path.rewind();
                }
                break;
            }
        }
    }
}

