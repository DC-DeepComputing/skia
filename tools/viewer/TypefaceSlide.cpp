/*
 * Copyright 2024 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "include/core/SkCanvas.h"
#include "include/core/SkFont.h"
#include "include/core/SkFontMgr.h"
#include "include/core/SkFontMetrics.h"
#include "include/core/SkPath.h"
#include "include/core/SkRRect.h"
#include "include/core/SkTypeface.h"
#include "src/base/SkRandom.h"
#include "tools/SkMetaData.h"
#include "tools/ToolUtils.h"
#include "tools/fonts/FontToolUtils.h"
#include "tools/timer/TimeUtils.h"
#include "tools/viewer/Slide.h"

class TypefaceSlide : public Slide {
public:
    TypefaceSlide() { fName = "Typeface Viewer"; }

    void load(SkScalar w, SkScalar h) override {
        fBaseTypeface = ToolUtils::CreateTypefaceFromResource("fonts/Variable.ttf");
        //fBaseTypeface = ToolUtils::TestFontMgr()->makeFromFile("/path/to/cairoplay2.ttf");
        fCurrentTypeface = fBaseTypeface;
        fWindowSize = {w, h};
        fVariationSliders = ToolUtils::VariationSliders(fCurrentTypeface.get(), fVariationPosition);
    }

    void resize(SkScalar w, SkScalar h) override { fWindowSize = {w, h}; }

    void unload() override {
        fBaseTypeface = nullptr;
        fCurrentTypeface = nullptr;
    }

    SkISize getDimensions() const override { return fDrawArea; }

    void updateCurrentTypeface() {
        if (!fBaseTypeface) {
            return;
        }
        SkSpan<const SkFontArguments::VariationPosition::Coordinate> coords =
                fVariationSliders.getCoordinates();
        SkFontArguments::VariationPosition varPos = {coords.data(),
                                                     static_cast<int>(coords.size())};
        SkFontArguments args;
        args.setVariationDesignPosition(varPos);
        fCurrentTypeface = fBaseTypeface->makeClone(args);
        fCurrentTypefaceDirty = false;
    }

    void draw(SkCanvas* canvas) override {
        SkPaint paint;
        if (fCurrentTypefaceDirty) {
            this->updateCurrentTypeface();
        }
        SkFont font(fCurrentTypeface, fFontSize);
        SkFontMetrics metrics;
        font.getMetrics(&metrics);

        SkPoint origin{0, 0};
        SkPoint position{0, -metrics.fAscent + metrics.fLeading};
        SkRect drawBounds = SkRect::MakeXYWH(0, 0, 1, 1);

        int numGlyphs = fCurrentTypeface->countGlyphs();
        bool glyphOnLine = false;
        for (SkGlyphID glyph = 0; glyph < numGlyphs; ++glyph) {
            SkScalar advance;
            SkRect glyphBounds;
            font.getWidths(&glyph, 1, &advance, &glyphBounds);
            if (glyphOnLine && fWindowSize.width() < position.x() + advance) {
                position.fX = 0;
                position.fY += -metrics.fAscent + metrics.fDescent + metrics.fLeading;
                glyphOnLine = false;
            }

            canvas->drawGlyphs(1, &glyph, &position, origin, font, paint);
            drawBounds.join(glyphBounds.makeOffset(position.x(), position.y()));
            glyphOnLine = true;
            position.fX += advance;
        }

        fDrawArea = drawBounds.roundOut().size();
    }

    bool onGetControls(SkMetaData* controls) override {
        SkScalar size[3] = {fFontSize, 0, 256};
        controls->setScalars("Size", 3, size);

        // TODO: toggle glyph numbers on top?
        // TODO: limit number of glyphs and set first glyph
        // TODO: choose typeface factory

        return fVariationSliders.writeControls(controls);
    }

    void onSetControls(const SkMetaData& controls) override {
        SkScalar size[3] = {0};
        int numReturnedScalars = 0;
        SkASSERT_RELEASE(controls.findScalars("Size", &numReturnedScalars, size));
        SkASSERT_RELEASE(numReturnedScalars == 3);
        fFontSize = size[0];

        fVariationSliders.readControls(controls, &fCurrentTypefaceDirty);
    }

private:
    sk_sp<SkTypeface> fBaseTypeface;
    sk_sp<SkTypeface> fCurrentTypeface;

    std::unique_ptr<SkFontArguments::VariationPosition::Coordinate[]> fCoordinates;
    SkFontArguments::VariationPosition fVariationPosition;
    ToolUtils::VariationSliders fVariationSliders;
    bool fCurrentTypefaceDirty = true;
    SkScalar fFontSize = 12;

    SkSize fWindowSize;
    SkISize fDrawArea;
};

//////////////////////////////////////////////////////////////////////////////

DEF_SLIDE( return new TypefaceSlide(); )
