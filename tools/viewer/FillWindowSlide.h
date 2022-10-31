/*
 * Copyright 2022 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef FillWindowSlide_DEFINED
#define FillWindowSlide_DEFINED

#include "tools/viewer/Slide.h"

/**
 * Slides have a content "size" that is independent of the window size. Some slides just resize
 * their content to the window size. This base class makes that easy.
 */
class FillWindowSlide : virtual public Slide {
public:
    SkISize getDimensions() const final { return fSize; }

    void load(SkScalar winWidth, SkScalar winHeight) final {
        this->onLoad(winWidth, winHeight);
        fSize = {SkScalarCeilToInt(winWidth), SkScalarCeilToInt(winHeight)};
    }

    void resize(SkScalar winWidth, SkScalar winHeight) final {
        this->onResize(winWidth, winHeight);
        fSize = {SkScalarCeilToInt(winWidth), SkScalarCeilToInt(winHeight)};
    }

private:
    virtual void onLoad(SkScalar winWidth, SkScalar winHeight) {}
    virtual void onResize(SkScalar winWidth, SkScalar winHeight) {}

    SkISize fSize;
};

#endif
