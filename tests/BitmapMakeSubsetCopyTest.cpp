/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkBitmap.h"
#include "src/core/SkMipmap.h"
#include "src/core/SkMipmapBuilder.h"
#include "tests/Test.h"

class BitmapMipmapTestAccess {
public:
    static SkMipmap* Get(const SkBitmap& bm) {
        return bm.fMips.get();
    }

    static void Set(SkBitmap* bm, sk_sp<SkMipmap> mips) {
        bm->fMips = mips;
    }
};

namespace {

static const SkColor4f kBaseRingColor   = SkColors::kRed;
static const SkColor4f kBaseCenterColor = SkColors::kGreen;
static const SkColor4f kBlendedColor    = { 0.75f, 0.25f, 0.0f, 1.0f };

static const int kExpectedNumMipLevels = 3;
static const SkColor4f kMipColors[kExpectedNumMipLevels] = {
        { SkColors::kBlue    },  // 4x4
        { SkColors::kCyan    },  // 2x2
        { SkColors::kMagenta },  // 1x1
};

bool close_to(const SkColor4f& c1, const SkColor4f& c2) {
    float kTol = 0.01f;

    return std::abs(c1.fR - c2.fR) < kTol &&
           std::abs(c1.fG - c2.fG) < kTol &&
           std::abs(c1.fB - c2.fB) < kTol &&
           std::abs(c1.fA - c2.fA) < kTol;
}

void is_ring(skiatest::Reporter* reporter, const SkPixmap& pm,
             SkColor4f ringColor, SkColor4f centerColor) {
    SkIRect center = SkIRect::MakeSize(pm.dimensions()).makeInset(pm.width() / 4, pm.height() / 4);

    for (int y = 0; y < pm.width(); ++y) {
        for (int x = 0; x < pm.height(); ++x) {
            if (y < center.fTop || y >= center.fBottom || x < center.fLeft || x >= center.fRight) {
                REPORTER_ASSERT(reporter, close_to(pm.getColor4f(x, y), ringColor));
            } else {
                REPORTER_ASSERT(reporter, close_to(pm.getColor4f(x, y), centerColor));
            }
        }
    }
}

void is_solid(skiatest::Reporter* reporter, const SkPixmap& pm, SkColor4f color) {
    for (int y = 0; y < pm.height(); ++y) {
        for (int x = 0; x < pm.width(); ++x) {
            REPORTER_ASSERT(reporter, close_to(pm.getColor4f(x, y), color));
        }
    }
}

void check_full(skiatest::Reporter* reporter, const SkBitmap& bm, bool originalMipmaps) {
    SkASSERT(bm.width() == 8 && bm.height() == 8);

    is_ring(reporter, bm.pixmap(), kBaseRingColor, kBaseCenterColor);

    if (bm.hasMipmaps()) {
        SkMipmap* mips = BitmapMipmapTestAccess::Get(bm);

        SkASSERT(mips->countLevels() == kExpectedNumMipLevels);

        for (int i = 0; i < mips->countLevels(); ++i) {
            SkMipmap::Level level;
            mips->getLevel(i, &level);

            if (originalMipmaps) {
                is_solid(reporter, level.fPixmap, kMipColors[i]);
            } else {
                if (i == 0) {
                    is_ring(reporter, level.fPixmap, kBaseRingColor, kBaseCenterColor);
                } else {
                    is_solid(reporter, level.fPixmap, kBlendedColor);
                }
            }
        }
    }
}

// The subset bitmap should be 'kBaseCenterColor' colored everywhere including the mipmaps,
// if present.
void check_subset(skiatest::Reporter* reporter, const SkBitmap& bm) {
        SkASSERT(bm.width() == 4 && bm.height() == 4);

    is_solid(reporter, bm.pixmap(), kBaseCenterColor);

    if (bm.hasMipmaps()) {
        SkMipmap* mips = BitmapMipmapTestAccess::Get(bm);

        SkASSERT(mips->countLevels() == kExpectedNumMipLevels-1);

        for (int i = 0; i < mips->countLevels(); ++i) {
            SkMipmap::Level level;
            mips->getLevel(i, &level);

            is_solid(reporter, level.fPixmap, kBaseCenterColor);
        }
    }
}

// Create a bitmap with a base level consisting of a red ring around a green center.
// If mipmaps are requested they will each have a different color (see kMipColors).
SkBitmap makeBM(bool withMipmaps) {

    SkImageInfo ii = SkImageInfo::Make(8, 8, kRGBA_8888_SkColorType, kPremul_SkAlphaType);
    SkBitmap bitmap;
    bitmap.allocPixels(ii);

    SkIRect center = SkIRect::MakeSize(bitmap.dimensions()).makeInset(bitmap.width() / 4,
                                                                      bitmap.height() / 4);

    bitmap.eraseColor(kBaseRingColor);
    bitmap.erase(kBaseCenterColor, center);

    if (withMipmaps) {
        SkMipmapBuilder builder(ii);

        SkASSERT(builder.countLevels() == kExpectedNumMipLevels);
        for (int i = 0; i < builder.countLevels(); ++i) {
            builder.level(i).erase(kMipColors[i]);
        }

        BitmapMipmapTestAccess::Set(&bitmap, builder.refMipmap());
    }

    bitmap.setImmutable();
    return bitmap;
}

} // anonymous namespace

DEF_TEST(BitmapMakeSubsetCopyPixels, reporter) {

    for (bool createdWithMips : { true, false }) {
        SkBitmap bm = makeBM(createdWithMips);

        for (bool copyWithMips : { true, false }) {
            SkIRect subset = SkIRect::MakeSize(bm.dimensions());

            SkBitmap fullCopy = bm.makeSubsetCopy(subset, copyWithMips);
            REPORTER_ASSERT(reporter, fullCopy.hasMipmaps() == copyWithMips);
            check_full(reporter, fullCopy, createdWithMips);

            subset.inset(subset.width() / 4, subset.height() / 4);
            SkBitmap subsetCopy = bm.makeSubsetCopy(subset, copyWithMips);
            REPORTER_ASSERT(reporter, subsetCopy.hasMipmaps() == copyWithMips);
            check_subset(reporter, subsetCopy);
        }
    }
}
