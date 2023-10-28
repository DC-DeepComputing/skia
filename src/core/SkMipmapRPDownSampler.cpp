/*
 * Copyright 2023 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkBitmap.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPixmap.h"
#include "src/core/SkDraw.h"
#include "src/core/SkMipmap.h"
#include "src/core/SkRasterClip.h"

namespace {

struct RPDownSampler : SkMipmapDownSampler {

    void buildLevel(const SkPixmap& dst, const SkPixmap& src) override;
};

void RPDownSampler::buildLevel(const SkPixmap& dst, const SkPixmap& src) {
    const SkRasterClip rclip(dst.bounds());
    const SkRect dstBounds = SkRect::Make(dst.bounds());
    const SkMatrix ctm = SkMatrix::I();
    const auto cubic = SkCubicResampler::CatmullRom();
    const auto sampling = SkSamplingOptions(cubic);

    SkDraw draw;
    draw.fDst = dst;
    draw.fCTM = &ctm;
    draw.fRC = &rclip;

    SkBitmap bitmap;
    bitmap.installPixels(src.info(), const_cast<void*>(src.addr()), src.rowBytes());

    draw.drawBitmap(bitmap, ctm, &dstBounds, sampling, SkPaint());
}

} // namespace

std::unique_ptr<SkMipmapDownSampler> SkMakeRPDownSampler(const SkPixmap& root) {
    // check if root's colortype/alphatype are 'renderable'
    auto sampler = std::make_unique<RPDownSampler>();
    return sampler;
}

