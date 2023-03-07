/*
* Copyright 2016 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "tools/viewer/ImageSlide.h"

#include "include/core/SkCanvas.h"
#include "include/core/SkData.h"
#include "include/core/SkImage.h"

ImageSlide::ImageSlide(const SkString& name, const SkString& path) : fPath(path) {
    fName = name;
}

ImageSlide::ImageSlide(const SkString& name, sk_sp<SkData> data) : fData(std::move(data)) {
    fName = name;
}

SkISize ImageSlide::getDimensions() const {
    return fImage ? fImage->dimensions() : SkISize::Make(0, 0);
}

void ImageSlide::draw(SkCanvas* canvas) {
    SkASSERT(fImage);
    canvas->drawImage(fImage, 0, 0);
}

void ImageSlide::load(SkScalar, SkScalar) {
    sk_sp<SkData> encoded = fData ? fData : SkData::MakeFromFileName(fPath.c_str());
    fImage = SkImage::MakeFromEncoded(encoded);
}

void ImageSlide::unload() {
    fImage.reset(nullptr);
}
