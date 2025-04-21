/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "modules/skcapture/SkCaptureCanvas.h"

#include <memory>
#include "include/core/SkPicture.h"
#include "include/core/SkRasterHandleAllocator.h"
#include "include/core/SkRefCnt.h"
#include "include/private/base/SkAssert.h"
#include "src/core/SkCanvasPriv.h"

SkCaptureCanvas::SkCaptureCanvas(SkCanvas* canvas)
        : INHERITED(canvas->imageInfo().width(), canvas->imageInfo().height()) {
    fBaseCanvas = canvas;
    this->addCanvas(canvas);
}

void SkCaptureCanvas::pollCapturingStatus() {
    bool shouldPoll = false;  // TODO: = fBaseCanvas->rootDevice()->recorder()->isCapturing();
    if (fCapturing != shouldPoll) {
        if (shouldPoll) {
            attachRecordingCanvas();
        } else {
            detachRecordingCanvas();
        }
        fCapturing = shouldPoll;
    }
}

sk_sp<SkPicture> SkCaptureCanvas::snapSKP() {
    if (!fCapturing) {
        return nullptr;
    }
    auto skp = fRecorder->finishRecordingAsPicture();
    detachRecordingCanvas();  // remove the stale recording canvas
    attachRecordingCanvas();
    return skp;
}

void SkCaptureCanvas::attachRecordingCanvas() {
    SkASSERT(this->fList.size() == 1);
    this->addCanvas(fRecorder->beginRecording(fBaseCanvas->imageInfo().width(),
                                              fBaseCanvas->imageInfo().height()));
}

void SkCaptureCanvas::detachRecordingCanvas() {
    SkASSERT(this->fList.size() == 2);
    this->removeCanvas(fRecorder->getRecordingCanvas());
}

//////////////////// Function forwarding ///////////////////////

void SkCaptureCanvas::willSave() {
    this->pollCapturingStatus();
    this->INHERITED::willSave();
}

SkCanvas::SaveLayerStrategy SkCaptureCanvas::getSaveLayerStrategy(const SaveLayerRec& rec) {
    this->pollCapturingStatus();
    return this->INHERITED::getSaveLayerStrategy(rec);
}

bool SkCaptureCanvas::onDoSaveBehind(const SkRect* bounds) {
    this->pollCapturingStatus();
    return this->INHERITED::onDoSaveBehind(bounds);
}

void SkCaptureCanvas::willRestore() {
    this->pollCapturingStatus();
    this->INHERITED::willRestore();
}

void SkCaptureCanvas::didConcat44(const SkM44& m) {
    this->pollCapturingStatus();
    this->INHERITED::didConcat44(m);
}

void SkCaptureCanvas::didSetM44(const SkM44& matrix) {
    this->pollCapturingStatus();
    this->INHERITED::didSetM44(matrix);
}

void SkCaptureCanvas::didTranslate(SkScalar x, SkScalar y) {
    this->pollCapturingStatus();
    this->INHERITED::didTranslate(x, y);
}

void SkCaptureCanvas::didScale(SkScalar x, SkScalar y) {
    this->pollCapturingStatus();
    this->INHERITED::didScale(x, y);
}

void SkCaptureCanvas::onClipRect(const SkRect& rect, SkClipOp op, ClipEdgeStyle edgeStyle) {
    this->pollCapturingStatus();
    this->INHERITED::onClipRect(rect, op, edgeStyle);
}

void SkCaptureCanvas::onClipRRect(const SkRRect& rrect, SkClipOp op, ClipEdgeStyle edgeStyle) {
    this->pollCapturingStatus();
    this->INHERITED::onClipRRect(rrect, op, edgeStyle);
}

void SkCaptureCanvas::onClipPath(const SkPath& path, SkClipOp op, ClipEdgeStyle edgeStyle) {
    this->pollCapturingStatus();
    this->INHERITED::onClipPath(path, op, edgeStyle);
}

void SkCaptureCanvas::onClipShader(sk_sp<SkShader> sh, SkClipOp op) {
    this->pollCapturingStatus();
    this->INHERITED::onClipShader(sh, op);
}

void SkCaptureCanvas::onClipRegion(const SkRegion& deviceRgn, SkClipOp op) {
    this->pollCapturingStatus();
    this->INHERITED::onClipRegion(deviceRgn, op);
}

void SkCaptureCanvas::onResetClip() {
    this->pollCapturingStatus();
    this->INHERITED::onResetClip();
}

void SkCaptureCanvas::onDrawPaint(const SkPaint& paint) {
    this->pollCapturingStatus();
    this->INHERITED::onDrawPaint(paint);
}

void SkCaptureCanvas::onDrawBehind(const SkPaint& paint) {
    this->pollCapturingStatus();
    this->INHERITED::onDrawBehind(paint);
}

void SkCaptureCanvas::onDrawPoints(PointMode mode,
                                   size_t count,
                                   const SkPoint pts[],
                                   const SkPaint& paint) {
    this->pollCapturingStatus();
    this->INHERITED::onDrawPoints(mode, count, pts, paint);
}

void SkCaptureCanvas::onDrawRect(const SkRect& rect, const SkPaint& paint) {
    this->pollCapturingStatus();
    this->INHERITED::onDrawRect(rect, paint);
}

void SkCaptureCanvas::onDrawRegion(const SkRegion& region, const SkPaint& paint) {
    this->pollCapturingStatus();
    this->INHERITED::onDrawRegion(region, paint);
}

void SkCaptureCanvas::onDrawOval(const SkRect& rect, const SkPaint& paint) {
    this->pollCapturingStatus();
    this->INHERITED::onDrawOval(rect, paint);
}

void SkCaptureCanvas::onDrawArc(const SkRect& rect,
                                SkScalar startAngle,
                                SkScalar sweepAngle,
                                bool useCenter,
                                const SkPaint& paint) {
    this->pollCapturingStatus();
    this->INHERITED::onDrawArc(rect, startAngle, sweepAngle, useCenter, paint);
}

void SkCaptureCanvas::onDrawRRect(const SkRRect& rrect, const SkPaint& paint) {
    this->pollCapturingStatus();
    this->INHERITED::onDrawRRect(rrect, paint);
}

void SkCaptureCanvas::onDrawDRRect(const SkRRect& outer,
                                   const SkRRect& inner,
                                   const SkPaint& paint) {
    this->pollCapturingStatus();
    this->INHERITED::onDrawDRRect(outer, inner, paint);
}

void SkCaptureCanvas::onDrawPath(const SkPath& path, const SkPaint& paint) {
    this->pollCapturingStatus();
    this->INHERITED::onDrawPath(path, paint);
}

void SkCaptureCanvas::onDrawImage2(const SkImage* image,
                                   SkScalar left,
                                   SkScalar top,
                                   const SkSamplingOptions& sampling,
                                   const SkPaint* paint) {
    this->pollCapturingStatus();
    this->INHERITED::onDrawImage2(image, left, top, sampling, paint);
}

void SkCaptureCanvas::onDrawImageRect2(const SkImage* image,
                                       const SkRect& src,
                                       const SkRect& dst,
                                       const SkSamplingOptions& sampling,
                                       const SkPaint* paint,
                                       SrcRectConstraint constraint) {
    this->pollCapturingStatus();
    this->INHERITED::onDrawImageRect2(image, src, dst, sampling, paint, constraint);
}

void SkCaptureCanvas::onDrawImageLattice2(const SkImage* image,
                                          const Lattice& lattice,
                                          const SkRect& dst,
                                          SkFilterMode filter,
                                          const SkPaint* paint) {
    this->pollCapturingStatus();
    this->INHERITED::onDrawImageLattice2(image, lattice, dst, filter, paint);
}

void SkCaptureCanvas::onDrawAtlas2(const SkImage* image,
                                   const SkRSXform xform[],
                                   const SkRect tex[],
                                   const SkColor colors[],
                                   int count,
                                   SkBlendMode bmode,
                                   const SkSamplingOptions& sampling,
                                   const SkRect* cull,
                                   const SkPaint* paint) {
    this->pollCapturingStatus();
    this->INHERITED::onDrawAtlas2(image, xform, tex, colors, count, bmode, sampling, cull, paint);
}

void SkCaptureCanvas::onDrawGlyphRunList(const sktext::GlyphRunList& list, const SkPaint& paint) {
    this->pollCapturingStatus();
    this->INHERITED::onDrawGlyphRunList(list, paint);
}

void SkCaptureCanvas::onDrawTextBlob(const SkTextBlob* blob,
                                     SkScalar x,
                                     SkScalar y,
                                     const SkPaint& paint) {
    this->pollCapturingStatus();
    this->INHERITED::onDrawTextBlob(blob, x, y, paint);
}

void SkCaptureCanvas::onDrawSlug(const sktext::gpu::Slug* slug, const SkPaint& paint) {
    this->pollCapturingStatus();
    this->INHERITED::onDrawSlug(slug, paint);
}

void SkCaptureCanvas::onDrawPicture(const SkPicture* picture,
                                    const SkMatrix* matrix,
                                    const SkPaint* paint) {
    this->pollCapturingStatus();
    this->INHERITED::onDrawPicture(picture, matrix, paint);
}

void SkCaptureCanvas::onDrawDrawable(SkDrawable* drawable, const SkMatrix* matrix) {
    this->pollCapturingStatus();
    this->INHERITED::onDrawDrawable(drawable, matrix);
}

void SkCaptureCanvas::onDrawVerticesObject(const SkVertices* vertices,
                                           SkBlendMode bmode,
                                           const SkPaint& paint) {
    this->pollCapturingStatus();
    this->INHERITED::onDrawVerticesObject(vertices, bmode, paint);
}

void SkCaptureCanvas::onDrawPatch(const SkPoint cubics[12],
                                  const SkColor colors[4],
                                  const SkPoint texCoords[4],
                                  SkBlendMode bmode,
                                  const SkPaint& paint) {
    this->pollCapturingStatus();
    this->INHERITED::onDrawPatch(cubics, colors, texCoords, bmode, paint);
}

void SkCaptureCanvas::onDrawShadowRec(const SkPath& path, const SkDrawShadowRec& rec) {
    this->pollCapturingStatus();
    this->INHERITED::onDrawShadowRec(path, rec);
}

void SkCaptureCanvas::onDrawAnnotation(const SkRect& rect, const char key[], SkData* data) {
    this->pollCapturingStatus();
    this->INHERITED::onDrawAnnotation(rect, key, data);
}

void SkCaptureCanvas::onDrawEdgeAAQuad(const SkRect& rect,
                                       const SkPoint clip[4],
                                       QuadAAFlags aa,
                                       const SkColor4f& color,
                                       SkBlendMode mode) {
    this->pollCapturingStatus();
    this->INHERITED::onDrawEdgeAAQuad(rect, clip, aa, color, mode);
}

void SkCaptureCanvas::onDrawEdgeAAImageSet2(const ImageSetEntry set[],
                                            int count,
                                            const SkPoint dstClips[],
                                            const SkMatrix preViewMatrices[],
                                            const SkSamplingOptions& sampling,
                                            const SkPaint* paint,
                                            SrcRectConstraint constraint) {
    this->pollCapturingStatus();
    this->INHERITED::onDrawEdgeAAImageSet2(
            set, count, dstClips, preViewMatrices, sampling, paint, constraint);
}
