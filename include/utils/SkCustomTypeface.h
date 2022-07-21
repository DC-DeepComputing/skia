/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkCustomTypeface_DEFINED
#define SkCustomTypeface_DEFINED

#include "include/core/SkDrawable.h"
#include "include/core/SkFontMetrics.h"
#include "include/core/SkFontStyle.h"
#include "include/core/SkPath.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkTypes.h"

#include <vector>

class SkImage;
class SkPaint;
class SkStream;
class SkTypeface;

SK_API class SkCustomTypefaceBuilder {
public:
    SkCustomTypefaceBuilder();

    void setGlyph(SkGlyphID, float advance, const SkPath&);

    struct DrawableRec {
        sk_sp<SkDrawable> fDraw;
        SkRect            fBounds = SkRect::MakeEmpty();
    };
    void setGlyph(SkGlyphID, float advance, DrawableRec);

    void setMetrics(const SkFontMetrics& fm, float scale = 1);
    void setFontStyle(SkFontStyle);

    sk_sp<SkTypeface> detach();

private:
    std::vector<SkPath>      fPaths;
    std::vector<DrawableRec> fDrawables;
    std::vector<float>       fAdvances;
    SkFontMetrics            fMetrics;
    SkFontStyle              fStyle;

    void ensureStorage(SkGlyphID);

    static sk_sp<SkTypeface> Deserialize(SkStream*);

    friend class SkTypeface;
};

#endif
