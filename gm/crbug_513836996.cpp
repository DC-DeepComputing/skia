/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkImage.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSamplingOptions.h"
#include "include/core/SkString.h"
#include "include/core/SkSurface.h"

using namespace skia_private;

class CrBug513836996GM : public skiagm::GM {
public:
    CrBug513836996GM() {
        this->setBGColor(SK_ColorBLACK);
    }

protected:
    SkString getName() const override {
        return SkString("crbug_513836996");
    }

    SkISize getISize() override {
        return SkISize::Make(200, 100);
    }

    DrawResult onDraw(SkCanvas* canvas, SkString* errorMsg) override {
        constexpr int kW = 200, kH = 100;
        auto dst_ii = SkImageInfo::Make(kW, kH, kRGBA_8888_SkColorType, kPremul_SkAlphaType);

        auto dst = canvas->makeSurface(dst_ii);
        if (!dst) {
            dst = SkSurfaces::Raster(dst_ii);
        }
        SkCanvas* dstCanvas = dst->getCanvas();
        dstCanvas->clear(SK_ColorBLACK);

        // Solid green 32x32 source image
        auto src_ii = SkImageInfo::Make(32, 32, kRGBA_8888_SkColorType, kPremul_SkAlphaType);
        auto src_surf = dstCanvas->makeSurface(src_ii);
        if (!src_surf) {
            src_surf = SkSurfaces::Raster(src_ii);
        }
        src_surf->getCanvas()->clear(SK_ColorGREEN);
        sk_sp<SkImage> green = src_surf->makeImageSnapshot();

        const SkPoint dstClips[8] = {
            // entry 0 clip (LEFT)
            {10, 10}, {60, 10}, {60, 60}, {10, 60},
            // entry 1 clip (RIGHT)
            {110, 10}, {190, 10}, {190, 90}, {110, 90},
        };

        SkCanvas::ImageSetEntry entries[2];

        // Entry 0: src rect ENTIRELY outside [0,32)x[0,32)
        entries[0].fImage = green;
        entries[0].fSrcRect = SkRect::MakeLTRB(-100, -100, -50, -50);
        entries[0].fDstRect = SkRect::MakeWH(kW, kH);
        entries[0].fMatrixIndex = -1;
        entries[0].fAlpha = 1.f;
        entries[0].fAAFlags = SkCanvas::kNone_QuadAAFlags;
        entries[0].fHasClip = true;

        // Entry 1: normal src rect. Should draw at dstClips[4..7] (RIGHT).
        entries[1].fImage = green;
        entries[1].fSrcRect = SkRect::MakeWH(32, 32);
        entries[1].fDstRect = SkRect::MakeWH(kW, kH);
        entries[1].fMatrixIndex = -1;
        entries[1].fAlpha = 1.f;
        entries[1].fAAFlags = SkCanvas::kNone_QuadAAFlags;
        entries[1].fHasClip = true;

        dstCanvas->experimental_DrawEdgeAAImageSet(
            entries, 2, dstClips, /*preViewMatrices=*/nullptr, SkSamplingOptions(),
            /*paint=*/nullptr, SkCanvas::kFast_SrcRectConstraint);

        SkBitmap bm;
        bm.allocPixels(dst_ii);
        if (!dst->readPixels(bm, 0, 0)) {
            *errorMsg = "Failed to read pixels from offscreen surface.";
            return DrawResult::kFail;
        }

        const SkColor right_px = bm.getColor(150, 50);  // inside RIGHT clip
        const SkColor left_px  = bm.getColor(35, 35);   // inside LEFT clip

        if (right_px != SK_ColorGREEN || left_px != SK_ColorBLACK) {
            errorMsg->printf("BUG: left(35,35)=0x%08x right(150,50)=0x%08x "
                             "(expected left=BLACK right=GREEN). DstClipIndex desync occurred.",
                             left_px, right_px);
            return DrawResult::kFail;
        }

        *errorMsg = "Success";
        return DrawResult::kSkip;
    }
};

DEF_GM(return new CrBug513836996GM;)
