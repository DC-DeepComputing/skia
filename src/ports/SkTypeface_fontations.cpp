/*
 * Copyright 2023 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkData.h"
#include "include/core/SkFontMetrics.h"
#include "src/core/SkFontPriv.h"
#include "src/ports/SkTypeface_fontations.h"

#include "src/ports/fontations/src/skpath_bridge.h"

namespace {

sk_sp<SkData> streamToData(const std::unique_ptr<SkStreamAsset>& font_data) {
  // TODO(drott): From a stream this causes a full read/copy. Make sure
  // we can instantiate this directly from the decompressed buffer that
  // Blink has after OTS and woff2 decompression.
    font_data->rewind();
    return SkData::MakeFromStream(font_data.get(), font_data->getLength());
}

::rust::Box<::fontations_ffi::BridgeFontRef> make_bridge_font_ref(sk_sp<SkData> fontData,
                                                                  uint32_t index = 0) {
    // TODO: What if font_data is empty?
    rust::Slice<const uint8_t> slice{fontData->bytes(), fontData->size()};
    return fontations_ffi::make_font_ref(slice, index);
}
}  // namespace

SkTypeface_Fontations::SkTypeface_Fontations(std::unique_ptr<SkStreamAsset> font_data)
        : SkTypeface(SkFontStyle(), true)
        , fFontData(streamToData(font_data))
        , fBridgeFontRef(make_bridge_font_ref(fFontData)) {}

int SkTypeface_Fontations::onGetUPEM() const { return 2048; }

void SkTypeface_Fontations::onCharsToGlyphs(const SkUnichar* chars,
                                            int count,
                                            SkGlyphID glyphs[]) const {
    sk_bzero(glyphs, count * sizeof(glyphs[0]));

    for (int i = 0; i < count; ++i) {
        glyphs[i] = fontations_ffi::lookup_glyph_or_zero(*fBridgeFontRef, chars[i]);
    }
}

void SkTypeface_Fontations::onFilterRec(SkScalerContextRec* rec) const {
    rec->setHinting(SkFontHinting::kNone);
}

class SkFontationsScalerContext : public SkScalerContext {
public:
    SkFontationsScalerContext(sk_sp<SkTypeface_Fontations> face,
                              const SkScalerContextEffects& effects,
                              const SkDescriptor* desc)
            : SkScalerContext(face, effects, desc)
            , fBridgeFontRef(static_cast<SkTypeface_Fontations*>(this->getTypeface())
                                     ->getBridgeFontRef()) {
        fRec.getSingleMatrix(&fMatrix);
        fFontData = face->fFontData;
        this->forceGenerateImageFromPath();
    }

protected:
    bool generateAdvance(SkGlyph* glyph) override {
        SkVector scale;
        SkMatrix remainingMatrix;
        if (!glyph ||
            !fRec.computeMatrices(
                    SkScalerContextRec::PreMatrixScale::kVertical, &scale, &remainingMatrix)) {
            return false;
        }
        float x_advance = 0.0f;
        x_advance = fontations_ffi::advance_width_or_zero(*fBridgeFontRef, scale.y(), glyph->getGlyphID());
        // TODO(drott): y-advance?
        const SkVector advance = remainingMatrix.mapXY(x_advance, SkFloatToScalar(0.f));
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
        SkVector scale;
        SkMatrix remainingMatrix;
        if (!fRec.computeMatrices(
                    SkScalerContextRec::PreMatrixScale::kVertical, &scale, &remainingMatrix)) {
            return false;
        }
        fontations_ffi::SkPathWrapper pathWrapper;

        if (!fontations_ffi::get_path(*fBridgeFontRef, glyph.getGlyphID(), scale.y(), pathWrapper)) {
            return false;
        }

        *path = pathWrapper.getPath();
        *path = path->makeTransform(remainingMatrix);
        return true;
    }

    void generateFontMetrics(SkFontMetrics* metrics) override {
        /* Hard-coded Roboto Regular metrics, to be replaced with Fontations calls. */
        metrics->fTop = -2163.f / getTypeface()->getUnitsPerEm();
        metrics->fAscent = -2146.f / getTypeface()->getUnitsPerEm();
        metrics->fDescent = 555.f / getTypeface()->getUnitsPerEm();
        metrics->fBottom = 555.f / getTypeface()->getUnitsPerEm();
        metrics->fLeading = 0;
        metrics->fAvgCharWidth = 0;
        metrics->fMaxCharWidth = 0;
        metrics->fXMin = -1825.f / getTypeface()->getUnitsPerEm();
        metrics->fXMax = 4188.f / getTypeface()->getUnitsPerEm();
        metrics->fXHeight = -1082.f / getTypeface()->getUnitsPerEm();
        metrics->fCapHeight = -1456.f / getTypeface()->getUnitsPerEm();
        metrics->fFlags = 0;

        SkFontPriv::ScaleFontMetrics(metrics, fMatrix.getScaleY());
    }

private:
    SkMatrix fMatrix;
    sk_sp<SkData> fFontData = nullptr;
    const rust::Box<::fontations_ffi::BridgeFontRef>& fBridgeFontRef;
};

std::unique_ptr<SkScalerContext> SkTypeface_Fontations::onCreateScalerContext(
        const SkScalerContextEffects& effects, const SkDescriptor* desc) const {
    return std::make_unique<SkFontationsScalerContext>(
            sk_ref_sp(const_cast<SkTypeface_Fontations*>(this)), effects, desc);
}
