/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/image/SkImage_Picture.h"

#include "include/core/SkColorSpace.h"
#include "include/core/SkImage.h"
#include "include/core/SkImageGenerator.h"
#include "include/core/SkPicture.h"
#include "include/core/SkSurfaceProps.h"
#include "src/base/SkTLazy.h"
#include "src/image/SkImageGeneratorPriv.h"
#include "src/image/SkImage_Lazy.h"
#include "src/image/SkPictureImageGenerator.h"

#include <memory>
#include <utility>

class SkMatrix;
class SkPaint;
struct SkISize;

sk_sp<SkImage> SkImage_Picture::Make(sk_sp<SkPicture> picture, const SkISize& dimensions,
                                     const SkMatrix* matrix, const SkPaint* paint,
                                     SkImages::BitDepth bitDepth, sk_sp<SkColorSpace> colorSpace,
                                     SkSurfaceProps props) {
    auto gen = SkImageGenerators::MakeFromPicture(dimensions, std::move(picture), matrix, paint,
                                                  bitDepth, std::move(colorSpace), props);

    SkImage_Lazy::Validator validator(
            SharedGenerator::Make(std::move(gen)), nullptr, nullptr);

    return validator ? sk_make_sp<SkImage_Picture>(&validator) : nullptr;
}

SkPictureImageGenerator* SkImage_Picture::gen() const {
    return static_cast<SkPictureImageGenerator*>(this->generator()->fGenerator.get());
}

SkPicture* SkImage_Picture::picture() const {
    return this->gen()->fPicture.get();
}

SkMatrix* SkImage_Picture::matrix() const {
    return &this->gen()->fMatrix;
}

SkPaint* SkImage_Picture::paint() const {
    return this->gen()->fPaint.getMaybeNull();
}

SkSurfaceProps* SkImage_Picture::props() const {
    return &this->gen()->fProps;
}

bool SkImage_Picture::getImageKeyValues(
        uint32_t keyValues[SkTiledImageUtils::kNumImageKeyValues]) const {

    if (this->paint()) {
        // A full paint complicates the potential key too much.
        return false;
    }

    const SkImageInfo& ii = this->gen()->getInfo();
    if (!ii.colorSpace()->isSRGB()) {
        // We only return key values if the colorSpace is boring. Alternatively, we could
        // use two uint32_ts to store the colorSpace's hash value.
        return false;
    }

    const SkMatrix& m = this->gen()->fMatrix;
    if (!m.isIdentity() && !m.isTranslate()) {
        // To keep the key small we only cache simple (<= translation) matrices
        return false;
    }



    bool isU8 = ii.colorType() != kRGBA_F16_SkColorType;
    int width = ii.width();
    int height = ii.height();
    float fX = m.getTranslateX();
    float fY = m.getTranslateX();

    SkASSERT(SkTFitsIn<uint32_t>(width));
    SkASSERT(SkTFitsIn<uint32_t>(height));


    keyValues[0] = isU8;
    keyValues[1] = this->picture()->uniqueID();
    keyValues[2] = width;
    keyValues[3] = height;
    keyValues[4] = 0;
    keyValues[5] = 0;
    return true;
}
