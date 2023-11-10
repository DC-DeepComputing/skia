/*
 * Copyright 2023 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkCanvas.h"
#include "include/core/SkData.h"
#include "include/core/SkFontMetrics.h"
#include "include/core/SkPictureRecorder.h"
#include "include/core/SkStream.h"
#include "include/pathops/SkPathOps.h"
#include "include/ports/SkTypeface_fontations.h"
#include "src/core/SkFontDescriptor.h"
#include "src/core/SkFontPriv.h"
#include "src/ports/SkTypeface_fontations_priv.h"

#include "src/ports/fontations/src/skpath_bridge.h"

namespace {

sk_sp<SkData> streamToData(const std::unique_ptr<SkStreamAsset>& font_data) {
    // TODO(drott): From a stream this causes a full read/copy. Make sure
    // we can instantiate this directly from the decompressed buffer that
    // Blink has after OTS and woff2 decompression.
    font_data->rewind();
    return SkData::MakeFromStream(font_data.get(), font_data->getLength());
}

rust::Box<::fontations_ffi::BridgeFontRef> make_bridge_font_ref(sk_sp<SkData> fontData,
                                                                uint32_t index) {
    rust::Slice<const uint8_t> slice{fontData->bytes(), fontData->size()};
    return fontations_ffi::make_font_ref(slice, index);
}

static_assert(sizeof(fontations_ffi::SkiaDesignCoordinate) ==
                      sizeof(SkFontArguments::VariationPosition::Coordinate) &&
              sizeof(fontations_ffi::SkiaDesignCoordinate::axis) ==
                      sizeof(SkFontArguments::VariationPosition::Coordinate::axis) &&
              sizeof(fontations_ffi::SkiaDesignCoordinate::value) ==
                      sizeof(SkFontArguments::VariationPosition::Coordinate::value) &&
              offsetof(fontations_ffi::SkiaDesignCoordinate, axis) ==
                      offsetof(SkFontArguments::VariationPosition::Coordinate, axis) &&
              offsetof(fontations_ffi::SkiaDesignCoordinate, value) ==
                      offsetof(SkFontArguments::VariationPosition::Coordinate, value) &&
              "Struct fontations_ffi::SkiaDesignCoordinate must match "
              "SkFontArguments::VariationPosition::Coordinate.");

rust::Box<fontations_ffi::BridgeNormalizedCoords> make_normalized_coords(
        fontations_ffi::BridgeFontRef const& bridgeFontRef, const SkFontArguments& args) {
    SkFontArguments::VariationPosition variationPosition = args.getVariationDesignPosition();
    // Cast is safe because of static_assert matching the structs above.
    rust::Slice<const fontations_ffi::SkiaDesignCoordinate> coordinates(
            reinterpret_cast<const fontations_ffi::SkiaDesignCoordinate*>(
                    variationPosition.coordinates),
            variationPosition.coordinateCount);
    return resolve_into_normalized_coords(bridgeFontRef, coordinates);
}
}  // namespace

SK_API sk_sp<SkTypeface> SkTypeface_Make_Fontations(std::unique_ptr<SkStreamAsset> fontData,
                                                    const SkFontArguments& args) {
    return SkTypeface_Fontations::MakeFromStream(std::move(fontData), args);
}

SkTypeface_Fontations::SkTypeface_Fontations(sk_sp<SkData> fontData, const SkFontArguments& args)
        : SkTypeface(SkFontStyle(), true)
        , fFontData(fontData)
        , fTtcIndex(args.getCollectionIndex())
        , fBridgeFontRef(make_bridge_font_ref(fFontData, fTtcIndex))
        , fBridgeNormalizedCoords(make_normalized_coords(*fBridgeFontRef, args)) {}

sk_sp<SkTypeface> SkTypeface_Fontations::MakeFromStream(std::unique_ptr<SkStreamAsset> stream,
                                                        const SkFontArguments& args) {
    return MakeFromData(streamToData(stream), args);
}

sk_sp<SkTypeface> SkTypeface_Fontations::MakeFromData(sk_sp<SkData> data,
                                                      const SkFontArguments& args) {
    sk_sp<SkTypeface_Fontations> probeTypeface(new SkTypeface_Fontations(data, args));
    return probeTypeface->hasValidBridgeFontRef() ? probeTypeface : nullptr;
}

namespace sk_fontations {

// Path sanitization ported from SkFTGeometrySink.
void PathGeometrySink::going_to(SkPoint point) {
    if (!fStarted) {
        fStarted = true;
        fPath.moveTo(fCurrent);
    }
    fCurrent = point;
}

bool PathGeometrySink::current_is_not(SkPoint point) { return fCurrent != point; }

void PathGeometrySink::move_to(float x, float y) {
    if (fStarted) {
        fPath.close();
        fStarted = false;
    }
    fCurrent = SkPoint::Make(SkFloatToScalar(x), SkFloatToScalar(y));
}

void PathGeometrySink::line_to(float x, float y) {
    SkPoint pt0 = SkPoint::Make(SkFloatToScalar(x), SkFloatToScalar(y));
    if (current_is_not(pt0)) {
        going_to(pt0);
        fPath.lineTo(pt0);
    }
}

void PathGeometrySink::quad_to(float cx0, float cy0, float x, float y) {
    SkPoint pt0 = SkPoint::Make(SkFloatToScalar(cx0), SkFloatToScalar(cy0));
    SkPoint pt1 = SkPoint::Make(SkFloatToScalar(x), SkFloatToScalar(y));
    if (current_is_not(pt0) || current_is_not(pt1)) {
        going_to(pt1);
        fPath.quadTo(pt0, pt1);
    }
}
void PathGeometrySink::curve_to(float cx0, float cy0, float cx1, float cy1, float x, float y) {
    SkPoint pt0 = SkPoint::Make(SkFloatToScalar(cx0), SkFloatToScalar(cy0));
    SkPoint pt1 = SkPoint::Make(SkFloatToScalar(cx1), SkFloatToScalar(cy1));
    SkPoint pt2 = SkPoint::Make(SkFloatToScalar(x), SkFloatToScalar(y));
    if (current_is_not(pt0) || current_is_not(pt1) || current_is_not(pt2)) {
        going_to(pt2);
        fPath.cubicTo(pt0, pt1, pt2);
    }
}

void PathGeometrySink::close() { fPath.close(); }

SkPath PathGeometrySink::into_inner() && { return std::move(fPath); }

AxisWrapper::AxisWrapper(SkFontParameters::Variation::Axis axisArray[], size_t axisCount)
        : fAxisArray(axisArray), fAxisCount(axisCount) {}

bool AxisWrapper::populate_axis(
        size_t i, uint32_t axisTag, float min, float def, float max, bool hidden) {
    if (i >= fAxisCount) {
        return false;
    }
    SkFontParameters::Variation::Axis& axis = fAxisArray[i];
    axis.tag = axisTag;
    axis.min = min;
    axis.def = def;
    axis.max = max;
    axis.setHidden(hidden);
    return true;
}

size_t AxisWrapper::size() const { return fAxisCount; }

}  // namespace sk_fontations

int SkTypeface_Fontations::onGetUPEM() const {
    return fontations_ffi::units_per_em_or_zero(*fBridgeFontRef);
}

void SkTypeface_Fontations::onGetFamilyName(SkString* familyName) const {
    rust::String readFamilyName = fontations_ffi::family_name(*fBridgeFontRef);
    *familyName = SkString(readFamilyName.data(), readFamilyName.size());
}

bool SkTypeface_Fontations::onGetPostScriptName(SkString* postscriptName) const {
    rust::String readPsName;
    if (fontations_ffi::postscript_name(*fBridgeFontRef, readPsName)) {
        *postscriptName = SkString(readPsName.data(), readPsName.size());
        return true;
    }

    return false;
}

bool SkTypeface_Fontations::onGlyphMaskNeedsCurrentColor() const {
    fGlyphMasksMayNeedCurrentColorOnce([this] {
        static constexpr SkFourByteTag COLRTag = SkSetFourByteTag('C', 'O', 'L', 'R');
        fGlyphMasksMayNeedCurrentColor = this->getTableSize(COLRTag) > 0;
    });
    return fGlyphMasksMayNeedCurrentColor;
}

void SkTypeface_Fontations::onCharsToGlyphs(const SkUnichar* chars,
                                            int count,
                                            SkGlyphID glyphs[]) const {
    sk_bzero(glyphs, count * sizeof(glyphs[0]));

    for (int i = 0; i < count; ++i) {
        glyphs[i] = fontations_ffi::lookup_glyph_or_zero(*fBridgeFontRef, chars[i]);
    }
}
int SkTypeface_Fontations::onCountGlyphs() const {
    return fontations_ffi::num_glyphs(*fBridgeFontRef);
}

bool SkTypeface_Fontations::hasValidBridgeFontRef() const {
    return fontations_ffi::font_ref_is_valid(*fBridgeFontRef);
}

void SkTypeface_Fontations::onFilterRec(SkScalerContextRec* rec) const {
    rec->setHinting(SkFontHinting::kNone);
}

class SkrifaLocalizedStrings : public SkTypeface::LocalizedStrings {
public:
    SkrifaLocalizedStrings(
            rust::Box<::fontations_ffi::BridgeLocalizedStrings> bridge_localized_strings)
            : fBridgeLocalizedStrings(std::move(bridge_localized_strings)) {}
    bool next(SkTypeface::LocalizedString* localized_string) override {
        fontations_ffi::BridgeLocalizedName localizedName;
        if (!fontations_ffi::localized_name_next(*fBridgeLocalizedStrings, localizedName)) {
            return false;
        }
        localized_string->fString =
                SkString(localizedName.string.data(), localizedName.string.size());
        localized_string->fLanguage =
                SkString(localizedName.language.data(), localizedName.language.size());
        return true;
    }

private:
    rust::Box<::fontations_ffi::BridgeLocalizedStrings> fBridgeLocalizedStrings;
};

SkTypeface::LocalizedStrings* SkTypeface_Fontations::onCreateFamilyNameIterator() const {
    return new SkrifaLocalizedStrings(fontations_ffi::get_localized_strings(*fBridgeFontRef));
}

class SkFontationsScalerContext : public SkScalerContext {
public:
    SkFontationsScalerContext(sk_sp<SkTypeface_Fontations> face,
                              const SkScalerContextEffects& effects,
                              const SkDescriptor* desc)
            : SkScalerContext(face, effects, desc)
            , fBridgeFontRef(
                      static_cast<SkTypeface_Fontations*>(this->getTypeface())->getBridgeFontRef())
            , fBridgeNormalizedCoords(static_cast<SkTypeface_Fontations*>(this->getTypeface())
                                              ->getBridgeNormalizedCoords()) {
        fRec.getSingleMatrix(&fMatrix);
        // this->forceGenerateImageFromPath();
    }

protected:
    GlyphMetrics generateMetrics(const SkGlyph& glyph, SkArenaAlloc*) override {
        GlyphMetrics mx(fRec.fMaskFormat);

        SkVector scale;
        SkMatrix remainingMatrix;
        if (!fRec.computeMatrices(
                    SkScalerContextRec::PreMatrixScale::kVertical, &scale, &remainingMatrix)) {
            return mx;
        }
        float x_advance = 0.0f;
        x_advance = fontations_ffi::advance_width_or_zero(
                fBridgeFontRef, scale.y(), fBridgeNormalizedCoords, glyph.getGlyphID());
        // TODO(drott): y-advance?
        mx.advance = remainingMatrix.mapXY(x_advance, SkFloatToScalar(0.f));

        // The FreeType backend has a big switch here:
        // Scalable or bitmap, monochromatic or color, subpixel shifting bounds if needed.
        // For now: check if COLRv1, get clipbox, else -
        // get bounds from Skrifa scaler?

        if (fontations_ffi::has_colrv1_glyph(fBridgeFontRef, glyph.getGlyphID())) {
            SkRect boundsRect;
            if (fontations_ffi::get_colrv1_clip_box(fBridgeFontRef,
                                                    fBridgeNormalizedCoords,
                                                    glyph.getGlyphID(),
                                                    scale.y(),
                                                    boundsRect.fLeft,
                                                    boundsRect.fRight,
                                                    boundsRect.fBottom,
                                                    boundsRect.fTop)) {
                boundsRect.fBottom *= -1.0f;
                boundsRect.fTop *= -1.0f;
                mx.bounds = boundsRect.roundOut();
            }
        } else {
mx.computeFromPath = true;
        }

        return mx;
    }

    void generateImage(const SkGlyph&, void*) override {
        SK_ABORT("Should have generated from path.");
    }

    bool generatePath(const SkGlyph& glyph, SkPath* path) override {
        SkVector scale;
        SkMatrix remainingMatrix;
        if (!fRec.computeMatrices(
                    SkScalerContextRec::PreMatrixScale::kVertical, &scale, &remainingMatrix)) {
            return false;
        }
        bool result = generateYScalePathForGlyphId(glyph.getGlyphID(), path, scale.y());
        if (!result) {
            return false;
        }

        *path = path->makeTransform(remainingMatrix);
        return true;
    }

    // TODO: Hinting control here.
    bool generateYScalePathForGlyphId(uint16_t glyphId, SkPath* path, float yScale) {
        sk_fontations::PathGeometrySink pathWrapper;
        fontations_ffi::BridgeScalerMetrics scalerMetrics;

        if (!fontations_ffi::get_path(fBridgeFontRef,
                                      glyphId,
                                      yScale,
                                      fBridgeNormalizedCoords,
                                      pathWrapper,
                                      scalerMetrics)) {
            return false;
        }
        *path = std::move(pathWrapper).into_inner();
        if (scalerMetrics.has_overlaps) {
            // See SkScalerContext_FreeType_Base::generateGlyphPath.
            Simplify(*path, path);
            AsWinding(*path, path);
        }

        return true;
    }

    bool drawCOLRv1Glyph(const SkGlyph& glyph, SkSpan<SkColor> palette, SkCanvas* canvas) {
        uint16_t upem = fontations_ffi::units_per_em_or_zero(fBridgeFontRef);
        if (upem == 0) {
            return false;
        }

        SkMatrix canvasMatrix;
        fRec.getSingleMatrix(&canvasMatrix);
        canvas->save();
        canvasMatrix.postScale(1.f / upem, 1.f / upem);
        canvas->concat(canvasMatrix);
        sk_fontations::ColorPainter color_painter(*this, *canvas, palette, upem);
        bool result = fontations_ffi::draw_colr_glyph(
                fBridgeFontRef, fBridgeNormalizedCoords, glyph.getGlyphID(), color_painter);
        canvas->restore();
        return result;
    }

    sk_sp<SkDrawable> generateDrawable(const SkGlyph& glyph) override {
        // TODO: SkTypeface FreeType uses the ScalerContext extraBits to denote glyph type during
        // the call to generateMetrics().

        //         if (glyph.extraBits() == ScalerContextBits::COLRv0 ||
        //     glyph.extraBits() == ScalerContextBits::COLRv1 ||
        //     glyph.extraBits() == ScalerContextBits::SVG     )
        // {
        // SkSpan<SkColor> palette(fFaceRec->fSkPalette.get(), fFaceRec->fFTPaletteEntryCount);
        SkPictureRecorder recorder;
        SkCanvas* recordingCanvas = recorder.beginRecording(SkRect::Make(glyph.mask().fBounds));
        SkColor one_color[10] = {SK_ColorBLUE,
                                 SK_ColorRED,
                                 SK_ColorYELLOW,
                                 SK_ColorBLUE,
                                 SK_ColorRED,
                                 SK_ColorYELLOW,
                                 SK_ColorBLUE,
                                 SK_ColorRED,
                                 SK_ColorYELLOW,
                                 SK_ColorBLUE};
        if (drawCOLRv1Glyph(glyph, one_color, recordingCanvas)) {
            return recorder.finishRecordingAsDrawable();
        }

        return nullptr;
    }

    void generateFontMetrics(SkFontMetrics* out_metrics) override {
        fontations_ffi::Metrics metrics = fontations_ffi::get_skia_metrics(
                fBridgeFontRef, fMatrix.getScaleY(), fBridgeNormalizedCoords);
        out_metrics->fTop = -metrics.top;
        out_metrics->fAscent = -metrics.ascent;
        out_metrics->fDescent = -metrics.descent;
        out_metrics->fBottom = -metrics.bottom;
        out_metrics->fLeading = -metrics.leading;
        out_metrics->fAvgCharWidth = metrics.avg_char_width;
        out_metrics->fMaxCharWidth = metrics.max_char_width;
        out_metrics->fXMin = metrics.x_min;
        out_metrics->fXMax = metrics.x_max;
        out_metrics->fXHeight = -metrics.x_height;
        out_metrics->fCapHeight = -metrics.cap_height;
        out_metrics->fFlags = 0;
        // TODO(drott): Is it necessary to transform metrics with remaining parts of matrix?
    }

private:
    SkMatrix fMatrix;
    sk_sp<SkData> fFontData = nullptr;
    const fontations_ffi::BridgeFontRef& fBridgeFontRef;
    const fontations_ffi::BridgeNormalizedCoords& fBridgeNormalizedCoords;
    friend class sk_fontations::ColorPainter;
};

std::unique_ptr<SkStreamAsset> SkTypeface_Fontations::onOpenStream(int* ttcIndex) const {
    *ttcIndex = fTtcIndex;
    return std::make_unique<SkMemoryStream>(fFontData);
}

sk_sp<SkTypeface> SkTypeface_Fontations::onMakeClone(const SkFontArguments& args) const {
    return MakeFromData(fFontData, args);
}

std::unique_ptr<SkScalerContext> SkTypeface_Fontations::onCreateScalerContext(
        const SkScalerContextEffects& effects, const SkDescriptor* desc) const {
    return std::make_unique<SkFontationsScalerContext>(
            sk_ref_sp(const_cast<SkTypeface_Fontations*>(this)), effects, desc);
}

void SkTypeface_Fontations::onGetFontDescriptor(SkFontDescriptor* desc, bool* serialize) const {
    SkString familyName;
    onGetFamilyName(&familyName);
    desc->setFamilyName(familyName.c_str());
    desc->setStyle(this->fontStyle());
    desc->setFactoryId(FactoryId);
    *serialize = true;
}

size_t SkTypeface_Fontations::onGetTableData(SkFontTableTag tag,
                                             size_t offset,
                                             size_t length,
                                             void* data) const {
    rust::Slice<uint8_t> dataSlice;
    if (data) {
        dataSlice = rust::Slice<uint8_t>(reinterpret_cast<uint8_t*>(data), length);
    }
    size_t copied = fontations_ffi::table_data(*fBridgeFontRef, tag, offset, dataSlice);
    // If data is nullptr, the Rust side doesn't see a length limit.
    return std::min(copied, length);
}

int SkTypeface_Fontations::onGetTableTags(SkFontTableTag tags[]) const {
    uint16_t numTables = fontations_ffi::table_tags(*fBridgeFontRef, rust::Slice<uint32_t>());
    if (!tags) {
        return numTables;
    }
    rust::Slice<uint32_t> copyToTags(tags, numTables);
    return fontations_ffi::table_tags(*fBridgeFontRef, copyToTags);
}

int SkTypeface_Fontations::onGetVariationDesignPosition(
        SkFontArguments::VariationPosition::Coordinate coordinates[], int coordinateCount) const {
    rust::Slice<fontations_ffi::SkiaDesignCoordinate> copyToCoordinates;
    if (coordinates) {
        copyToCoordinates = rust::Slice<fontations_ffi::SkiaDesignCoordinate>(
                reinterpret_cast<fontations_ffi::SkiaDesignCoordinate*>(coordinates),
                coordinateCount);
    }
    return fontations_ffi::variation_position(*fBridgeNormalizedCoords, copyToCoordinates);
}

int SkTypeface_Fontations::onGetVariationDesignParameters(
        SkFontParameters::Variation::Axis parameters[], int parameterCount) const {
    sk_fontations::AxisWrapper axisWrapper(parameters, parameterCount);
    return fontations_ffi::populate_axes(*fBridgeFontRef, axisWrapper);
}

namespace sk_fontations {

namespace {
// https://learn.microsoft.com/en-us/typography/opentype/spec/colr#format-32-paintcomposite
inline SkBlendMode ToSkBlendMode(uint16_t colrV1CompositeMode) {
    switch (colrV1CompositeMode) {
        case 0:
            return SkBlendMode::kClear;
        case 1:
            return SkBlendMode::kSrc;
        case 2:
            return SkBlendMode::kDst;
        case 3:
            return SkBlendMode::kSrcOver;
        case 4:
            return SkBlendMode::kDstOver;
        case 5:
            return SkBlendMode::kSrcIn;
        case 6:
            return SkBlendMode::kDstIn;
        case 7:
            return SkBlendMode::kSrcOut;
        case 8:
            return SkBlendMode::kDstOut;
        case 9:
            return SkBlendMode::kSrcATop;
        case 10:
            return SkBlendMode::kDstATop;
        case 11:
            return SkBlendMode::kXor;
        case 12:
            return SkBlendMode::kPlus;
        case 13:
            return SkBlendMode::kScreen;
        case 14:
            return SkBlendMode::kOverlay;
        case 15:
            return SkBlendMode::kDarken;
        case 16:
            return SkBlendMode::kLighten;
        case 17:
            return SkBlendMode::kColorDodge;
        case 18:
            return SkBlendMode::kColorBurn;
        case 19:
            return SkBlendMode::kHardLight;
        case 20:
            return SkBlendMode::kSoftLight;
        case 21:
            return SkBlendMode::kDifference;
        case 22:
            return SkBlendMode::kExclusion;
        case 23:
            return SkBlendMode::kMultiply;
        case 24:
            return SkBlendMode::kHue;
        case 25:
            return SkBlendMode::kSaturation;
        case 26:
            return SkBlendMode::kColor;
        case 27:
            return SkBlendMode::kLuminosity;
        default:
            return SkBlendMode::kDst;
    }
}
}  // namespace

ColorPainter::ColorPainter(SkFontationsScalerContext& scaler_context,
                           SkCanvas& canvas,
                           SkSpan<SkColor>& palette,
                           uint16_t upem)
        : fScalerContext(scaler_context), fCanvas(canvas), fPalette(palette), fUpem(upem) {}

void ColorPainter::push_transform(float xx, float xy, float yx, float yy, float dx, float dy) {
    fCanvas.save();
    SkMatrix transform = SkMatrix::MakeAll(xx, yx, dx, xy, yy, dy, 0.f, 0.f, 1.0f);
    fCanvas.concat(transform);
}

void ColorPainter::pop_transform() { fCanvas.restore(); }

// TODO: Need to pass something that is able to extrac paths.
void ColorPainter::push_clip_glyph(uint16_t glyph_id) {
    fCanvas.save();
    SkPath path;
    fScalerContext.generateYScalePathForGlyphId(glyph_id, &path, fUpem);
    fCanvas.clipPath(path, true /* doAntialias */);
}

void ColorPainter::pop_clip() { fCanvas.restore(); }

void ColorPainter::fill_solid(uint16_t palette_index, float alpha) {
    SkPaint paint;
    SkColor4f color = SkColor4f::FromColor(fPalette[palette_index]);
    color.fA *= alpha;
    paint.setShader(nullptr);
    paint.setColor(color);
    fCanvas.drawPaint(paint);
}

void ColorPainter::push_layer(uint16_t compositeMode) {
    SkPaint paint;
    paint.setBlendMode(ToSkBlendMode(compositeMode));
    fCanvas.saveLayer(nullptr, &paint);
}

void ColorPainter::pop_layer() { fCanvas.restore(); }

}  // namespace sk_fontations
