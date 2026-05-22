/*
 * Copyright 2026 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkCanvas.h"
#include "include/core/SkData.h"
#include "include/core/SkDrawable.h"
#include "include/core/SkFont.h"
#include "include/core/SkFontMetrics.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "include/core/SkPicture.h"
#include "include/core/SkPictureRecorder.h"
#include "include/core/SkRSXform.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSerialProcs.h"
#include "include/core/SkSurface.h"
#include "include/core/SkTextBlob.h"
#include "include/core/SkTypeface.h"
#include "include/utils/SkCustomTypeface.h"
#include "src/core/SkGlyph.h"  // Required internal header for SkPictureBackedGlyphDrawable
#include "src/core/SkReadBuffer.h"
#include "src/core/SkWriteBuffer.h"
#include "tests/Test.h"

namespace {

DEF_TEST(SkiaGlyphRunUAF, reporter) {
    sk_sp<SkData> data = nullptr;
    {
        data = SkData::MakeFromFileName("/tmp/outer_picture.skp");
        if (!data) {
            SkDebugf("Failed to load /tmp/outer_picture.skp\n");
            REPORTER_ASSERT(reporter, data);
        }
    }

    sk_sp<SkPicture> newPicture = SkPicture::MakeFromData(data.get());

    sk_sp<SkSurface> surface = SkSurfaces::Raster(SkImageInfo::MakeN32Premul(1024, 1024));
    REPORTER_ASSERT(reporter, surface);
    SkCanvas* canvas = surface->getCanvas();

    canvas->drawPicture(newPicture);
}

}  // namespace
