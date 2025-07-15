/*
 * Copyright 2025 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tools/viewer/VGXSlide.h"

#if defined(SK_ENABLE_VGX)

#include "experimental/vgx/SkVGX.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkPictureRecorder.h"
#include "include/core/SkStream.h"

VGXSlide::VGXSlide(const SkString& name, const SkString& path)
    : fPath(path) {
    fName = name;
}

VGXSlide::~VGXSlide() = default;

void VGXSlide::load(float w, float h) {
    auto stream = SkStream::MakeFromFile(fPath.c_str());
    if (!stream) {
        return;
    }

    SkPictureRecorder rec;
    if (!skvgx::Parse(stream.get(), rec.beginRecording(w, h))) {
        return;
    }

    fPicture = rec.finishRecordingAsPicture();
}

void VGXSlide::unload() {
    fPicture = nullptr;
}

void VGXSlide::draw(SkCanvas* canvas) {
    if (fPicture) {
        canvas->drawPicture(fPicture);
    }
}

#endif  // defined(SK_ENABLE_VGX)
