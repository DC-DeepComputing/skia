/*
 * Copyright 2022 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"

#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkImage.h"
#include "include/core/SkTileMode.h"
#include "include/gpu/GrRecordingContext.h"
#include "src/shaders/SkImageShader.h"
#include "tools/Resources.h"
#include "tools/ToolUtils.h"

namespace {

constexpr SkTileMode kTileModes[] = {
        SkTileMode::kClamp,
        SkTileMode::kRepeat,
        SkTileMode::kMirror,
        SkTileMode::kDecal,
};

constexpr SkFilterMode kFilterModes[] = {
        SkFilterMode::kNearest,
        SkFilterMode::kLinear,
};

constexpr SkMipmapMode kMipmapModes[] = {
        SkMipmapMode::kNone,
        SkMipmapMode::kNearest,
        SkMipmapMode::kLinear,
};

constexpr int kTileSize = 100;
constexpr int kSubsetSize = kTileSize / 2;

constexpr SkRect srcSubRect = SkRect::MakeXYWH(1, 1, 2, 2);

constexpr SkRect dstSubRect = SkRect::MakeXYWH(0, 0, kSubsetSize, kSubsetSize);
constexpr SkRect dstRect = SkRect::MakeXYWH(0, 0, kTileSize, kTileSize);

sk_sp<SkImage> makeRingedImage(SkCanvas* canvas) {
    // These are kRGBA_8888_SkColorType values.
    static constexpr uint32_t kRingColor = 0xFF0000FF,
                              kCheckColor1 = 0xFF000000,
                              kCheckColor2 = 0xFFFFFFFF;

    SkImageInfo info = SkImageInfo::Make(4, 4, kRGBA_8888_SkColorType, kPremul_SkAlphaType);
    size_t rowBytes = SkAlign4(info.minRowBytes());
    SkBitmap bitmap;
    bitmap.allocPixels(info, rowBytes);

    uint32_t* scanline = bitmap.getAddr32(0, 0);
    scanline[0] = kRingColor;
    scanline[1] = kRingColor;
    scanline[2] = kRingColor;
    scanline[3] = kRingColor;

    scanline = bitmap.getAddr32(0, 1);
    scanline[0] = kRingColor;
    scanline[1] = kCheckColor1;
    scanline[2] = kCheckColor2;
    scanline[3] = kRingColor;

    scanline = bitmap.getAddr32(0, 2);
    scanline[0] = kRingColor;
    scanline[1] = kCheckColor2;
    scanline[2] = kCheckColor1;
    scanline[3] = kRingColor;

    scanline = bitmap.getAddr32(0, 3);
    scanline[0] = kRingColor;
    scanline[1] = kRingColor;
    scanline[2] = kRingColor;
    scanline[3] = kRingColor;

    bitmap.setImmutable();
    return bitmap.asImage();
}

void drawTile(int cx,
              int ry,
              SkCanvas* canvas,
              sk_sp<SkImage> image,
              SkSamplingOptions samplingOptions,
              SkTileMode tileMode) {
    canvas->save();
    canvas->translate(10 + cx * (kTileSize + 10), 10 + ry * (kTileSize + 10));

    SkPaint p;
    SkMatrix srcToDst = SkMatrix::RectToRect(srcSubRect, dstSubRect);
    p.setShader(SkImageShader::MakeSubset(std::move(image),
                                          srcSubRect,
                                          kTileModes[ry],
                                          kTileModes[ry],
                                          samplingOptions,
                                          &srcToDst));
    canvas->drawRect(dstRect, p);

    canvas->restore();
}

void drawColumn(int cx, SkCanvas* canvas, sk_sp<SkImage> image, SkSamplingOptions samplingOptions) {
    int ry = 0;
    for (SkTileMode tileMode : kTileModes) {
        drawTile(cx, ry++, canvas, image, samplingOptions, tileMode);
    }
}

}  // namespace

namespace skiagm {

DEF_SIMPLE_GM_BG_CAN_FAIL(tilemode_subset, canvas, errorMsg, 780, 450, SK_ColorCYAN) {
    if (!canvas->recordingContext()  // Not Ganesh.
        && !canvas->recorder()) {    // Not Graphite.
        *errorMsg = GM::kErrorMsg_DrawSkippedGpuOnly;
        return DrawResult::kSkip;
    }

    sk_sp<SkImage> image = makeRingedImage(canvas);
    image = image->makeRasterImage();
    image = ToolUtils::MakeTextureImage(canvas, std::move(image));

    int cx = 0;
    for (SkMipmapMode mipmapMode : kMipmapModes) {
        for (SkFilterMode filterMode : kFilterModes) {
            drawColumn(cx++, canvas, image, SkSamplingOptions(filterMode, mipmapMode));
        }
    }
    drawColumn(cx++, canvas, image, SkSamplingOptions(SkCubicResampler::Mitchell()));

    return DrawResult::kOk;
}

}  // namespace skiagm
