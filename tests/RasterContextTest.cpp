/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "include/core/SkBlurTypes.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkMaskFilter.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRRect.h"
#include "include/core/SkRasterContext.h"
#include "include/core/SkRasterRecorder.h"
#include "include/core/SkSurface.h"
#include "src/core/SkRasterContextPriv.h"
#include "src/core/SkResourceCache.h"
#include "tests/Test.h"

#include <memory>

DEF_TEST(RasterSurface_UsesContextAndRecorderToDraw_UsesConfiguredCache, reporter) {
    // empirically determined to be a bit larger than the mask that will be cached.
    constexpr size_t kCacheSize = 10000;

    SkRasterContext::Options opts;
    opts.fResourceCacheSize = kCacheSize;

    auto ctx = SkRasterContext::Make(opts);

    std::unique_ptr<SkRasterRecorder> recorder = ctx->makeRecorder();
    SkImageInfo imageInfo =
            SkImageInfo::Make(100, 100, kRGBA_8888_SkColorType, kPremul_SkAlphaType);
    auto surface = SkSurfaces::Raster(recorder.get(), imageInfo, imageInfo.minRowBytes());

    SkPaint paint;
    paint.setColor(SK_ColorRED);
    paint.setMaskFilter(SkMaskFilter::MakeBlur(SkBlurStyle::kNormal_SkBlurStyle, 3.1f));
    surface->getCanvas()->drawRRect(SkRRect::MakeRectXY(SkRect::MakeWH(50, 50), 10, 15), paint);

    //std::unique_ptr<SkRasterRecording> recording = recorder->snap();

    const SkRasterContextPriv* ctxPriv = static_cast<const SkRasterContextPriv*>(ctx.get());
    sk_sp<SkResourceCache> cache = ctxPriv->resourceCache();
    REPORTER_ASSERT(reporter, cache->getTotalBytesUsed() > 0);
    REPORTER_ASSERT(reporter, cache->getTotalByteLimit() == kCacheSize);
    cache->dump();
}
