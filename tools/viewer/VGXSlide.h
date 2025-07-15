/*
 * Copyright 2025 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef VGXSlide_DEFINED
#define VGXSlide_DEFINED

#include "include/core/SkPicture.h"
#include "include/core/SkString.h"
#include "tools/viewer/Slide.h"

class VGXSlide final : public Slide {
public:
    VGXSlide(const SkString& name, const SkString& path);
    ~VGXSlide() override;

    void load(SkScalar winWidth, SkScalar winHeight) override;
    void unload() override;
    void draw(SkCanvas*) override;

private:
    const SkString fPath;

    sk_sp<SkPicture> fPicture;
};

#endif  // VGXSlide_DEFINED
