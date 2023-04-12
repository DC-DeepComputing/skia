/*
 * Copyright 2023 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkFontMetrics.h"
#include "src/core/SkFontPriv.h"
#include "src/ports/SkTypeface_fontations.h"

constexpr float kDoubleDaggerAdvance = 1008.f;

namespace {
  void drawDoubleDaggerPath(SkPath* path) {
      path->setFillType(SkPathFillType::kWinding);
      path->moveTo(77, 38);
      path->lineTo(414, 38);
      path->lineTo(411, -452);
      path->lineTo(592, -452);
      path->lineTo(589, 38);
      path->lineTo(926, 38);
      path->lineTo(926, 204);
      path->lineTo(575, 204);
      path->lineTo(595, 534);
      path->lineTo(580, 855);
      path->lineTo(926, 855);
      path->lineTo(926, 1022);
      path->lineTo(594, 1022);
      path->lineTo(597, 1508);
      path->lineTo(406, 1508);
      path->lineTo(409, 1022);
      path->lineTo(77, 1022);
      path->lineTo(77, 855);
      path->lineTo(423, 855);
      path->lineTo(408, 534);
      path->lineTo(428, 204);
      path->lineTo(77, 204);
      path->close();
  }
}

int SkTypeface_Fontations::onGetUPEM() const { return 2048; }

void SkTypeface_Fontations::onCharsToGlyphs(const SkUnichar* chars,
                                            int count,
                                            SkGlyphID glyphs[]) const {
    sk_bzero(glyphs, count * sizeof(glyphs[0]));
}

void SkTypeface_Fontations::onFilterRec(SkScalerContextRec* rec) const {
    rec->setHinting(SkFontHinting::kNone);
}

class SkFontationsScalerContext : public SkScalerContext {
public:
    SkFontationsScalerContext(sk_sp<SkTypeface_Fontations> face,
                              const SkScalerContextEffects& effects,
                              const SkDescriptor* desc)
            : SkScalerContext(std::move(face), effects, desc) {
        fRec.getSingleMatrix(&fMatrix);
        this->forceGenerateImageFromPath();
    }

protected:

    bool generateAdvance(SkGlyph* glyph) override {
        const SkVector advance = fMatrix.mapXY(
                SkFloatToScalar(kDoubleDaggerAdvance / getTypeface()->getUnitsPerEm()),
                SkFloatToScalar(0.f));
        glyph->fAdvanceX = SkScalarToFloat(advance.fX);
        glyph->fAdvanceY = SkScalarToFloat(advance.fY);
        return true;
    }

    void generateMetrics(SkGlyph* glyph, SkArenaAlloc*) override {
        glyph->fMaskFormat = fRec.fMaskFormat;
        glyph->zeroMetrics();
        this->generateAdvance(glyph);
        // Always generates from paths, so SkScalerContext::makeGlyph will figure the bounds.
    }

    void generateImage(const SkGlyph&) override { SK_ABORT("Should have generated from path."); }

    bool generatePath(const SkGlyph& glyph, SkPath* path) override {
        drawDoubleDaggerPath(path);
        *path = path->makeScale(1 / getTypeface()->getUnitsPerEm(),
                                1 / getTypeface()->getUnitsPerEm());
        *path = path->makeTransform(fMatrix);
        return true;
    }

    void generateFontMetrics(SkFontMetrics* metrics) override {
        metrics->fTop = -2048 / getTypeface()->getUnitsPerEm();
        metrics->fAscent = -1900 / getTypeface()->getUnitsPerEm();
        metrics->fDescent = 300 / getTypeface()->getUnitsPerEm();
        metrics->fBottom = 300 / getTypeface()->getUnitsPerEm();
        metrics->fLeading = 0;
        metrics->fAvgCharWidth = 0;
        metrics->fMaxCharWidth = 0;
        metrics->fXMin = 0;
        metrics->fXMax = 1;
        metrics->fXHeight = 0;
        metrics->fCapHeight = 0;

        SkFontPriv::ScaleFontMetrics(metrics, fMatrix.getScaleY());
    }

private:
    SkMatrix fMatrix;
};

std::unique_ptr<SkScalerContext> SkTypeface_Fontations::onCreateScalerContext(
    const SkScalerContextEffects& effects, const SkDescriptor* desc) const
{
    return std::make_unique<SkFontationsScalerContext>(
            sk_ref_sp(const_cast<SkTypeface_Fontations*>(this)), effects, desc);
}
