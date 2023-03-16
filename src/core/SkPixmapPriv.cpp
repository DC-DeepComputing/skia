/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkPixmapPriv.h"

#include "src/base/SkHalf.h"
#include "src/base/SkVx.h"
#include "src/core/SkColorSpacePriv.h"
#include "src/core/SkColorSpaceXformSteps.h"

namespace {

template <SkAlphaType AlphaType>
bool set_color(SkPixmap* pm, int x, int y, SkRGBA4f<AlphaType> c, const SkColorSpace* colorSpace) {
    if (kUnknown_SkColorType == pm->colorType()) {
        return false;
    }

    SkColorSpaceXformSteps steps{colorSpace,       AlphaType,
                                 pm->colorSpace(), pm->alphaType()};
    steps.apply(c.vec());

    switch (pm->colorType()) {
        case kRGBA_F16Norm_SkColorType:
        case kRGBA_F16_SkColorType: {
            uint64_t* addr = pm->writable_addr64(x, y);
            SkFloatToHalf_finite_ftz(skvx::float4::Load(c.vec())).store(addr);
            return true;
        }
        default:
            return false;
    }

    return false;
}

} // anonymous namespace

bool SkPixmapPriv::setColor4f(SkPixmap* pm, int x, int y, SkColor4f c,
                              const SkColorSpace* colorSpace) {
    return set_color(pm, x, y, c, colorSpace);
}

bool SkPixmapPriv::setPMColor4f(SkPixmap* pm, int x, int y, SkPMColor4f pmC,
                                const SkColorSpace* colorSpace) {
    return set_color(pm, x, y, pmC, colorSpace);
}
