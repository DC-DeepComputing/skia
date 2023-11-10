/*
 * Copyright 2023 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/ports/SkTypeface_fontations.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkData.h"
#include "include/core/SkFontMetrics.h"
#include "include/core/SkPictureRecorder.h"
#include "include/core/SkStream.h"
#include "include/effects/SkGradientShader.h"
#include "include/pathops/SkPathOps.h"
#include "src/core/SkFontDescriptor.h"
#include "src/core/SkFontPriv.h"
#include "src/ports/SkTypeface_fontations_priv.h"

#include "src/ports/fontations/src/skpath_bridge.h"

namespace {

[[maybe_unused]] static inline const constexpr bool kSkShowTextBlitCoverage = false;

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
        GlyphMetrics mx(glyph.maskFormat());

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

        // TODO: synthetic bold metrics change?

        if (fontations_ffi::has_colrv1_glyph(fBridgeFontRef, glyph.getGlyphID())) {
            SkRect boundsRect = SkRect::MakeEmpty();
            if (fontations_ffi::get_colrv1_clip_box(fBridgeFontRef,
                                                    fBridgeNormalizedCoords,
                                                    glyph.getGlyphID(),
                                                    scale.y(),
                                                    boundsRect.fLeft,
                                                    boundsRect.fRight,
                                                    boundsRect.fTop,
                                                    boundsRect.fBottom)) {
                boundsRect.fTop *= -1.0f;
                // TODO: Apply remaining matrics, compute bounding box of that.
                mx.bounds = boundsRect.roundOut();
            }
            mx.maskFormat = SkMask::kARGB32_Format;
        } else {
            // TODO: Retrieve from read_fonts and Skrifa - TrueType bbox or from path with hinting?
            mx.computeFromPath = true;
        }

        return mx;
    }

    void generateImage(const SkGlyph& glyph, void* imageBuffer) override {
        // TODO: For monochromatic contour glyphs, this needs to call back into the SkScalerContext
        // functionality to generate stuff from path.

        SkASSERT(glyph.maskFormat() == SkMask::kARGB32_Format);
        SkBitmap dstBitmap;
        // TODO: mark this as sRGB when the blits will be sRGB.
        dstBitmap.setInfo(
                SkImageInfo::Make(
                        glyph.width(), glyph.height(), kN32_SkColorType, kPremul_SkAlphaType),
                glyph.rowBytes());
        dstBitmap.setPixels(imageBuffer);

        SkCanvas canvas(dstBitmap);
        if constexpr (kSkShowTextBlitCoverage) {
            canvas.clear(0x33FF0000);
        } else {
            canvas.clear(SK_ColorTRANSPARENT);
        }
        canvas.translate(-glyph.left(), -glyph.top());

        SkColor some_colors[14] = {SkColorSetARGB(0xFF, 0xFF, 0x00, 0x00),
                                   SkColorSetARGB(0xFF, 0xFF, 0xA5, 0x00),
                                   SkColorSetARGB(0xFF, 0xFF, 0xFF, 0x00),
                                   SkColorSetARGB(0xFF, 0x00, 0x80, 0x00),
                                   SkColorSetARGB(0xFF, 0x00, 0x00, 0xFF),
                                   SkColorSetARGB(0xFF, 0x4B, 0x00, 0x82),
                                   SkColorSetARGB(0xFF, 0xEE, 0x82, 0xEE),
                                   SkColorSetARGB(0xFF, 0xFA, 0xF0, 0xE6),
                                   SkColorSetARGB(0xFF, 0x2F, 0x4F, 0x4F),
                                   SkColorSetARGB(0xFF, 0xFF, 0xFF, 0xFF),
                                   SkColorSetARGB(0xFF, 0x00, 0x00, 0x00),
                                   SkColorSetARGB(0xFF, 0x68, 0xC7, 0xE8),
                                   SkColorSetARGB(0xFF, 0xFF, 0xDC, 0x01),
                                   SkColorSetARGB(0xFF, 0x80, 0x80, 0x80)};

        drawCOLRv1Glyph(glyph, some_colors, &canvas);
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
        sk_fontations::ColorPainter colorPainter(*this, *canvas, palette, upem);
        bool result = fontations_ffi::draw_colr_glyph(
                fBridgeFontRef, fBridgeNormalizedCoords, glyph.getGlyphID(), colorPainter);
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

SkColor4f lerpSkColor(SkColor4f c0, SkColor4f c1, float t) {
    // Due to the floating point calculation in the caller, when interpolating between very narrow
    // stops, we may get values outside the interpolation range, guard against these.
    if (t < 0) {
        return c0;
    }
    if (t > 1) {
        return c1;
    }

    const auto c0_4f = skvx::float4::Load(c0.vec());
    const auto c1_4f = skvx::float4::Load(c1.vec());
    const auto c_4f = c0_4f + (c1_4f - c0_4f) * t;

    SkColor4f l;
    c_4f.store(l.vec());
    return l;
}

enum TruncateStops { TruncateStart, TruncateEnd };

// Truncate a vector of color stops at a previously computed stop position and insert at that
// position the color interpolated between the surrounding stops.
void truncateToStopInterpolating(SkScalar zeroRadiusStop,
                                 std::vector<SkColor4f>& colors,
                                 std::vector<SkScalar>& stops,
                                 TruncateStops truncateStops) {
    if (stops.size() <= 1u || zeroRadiusStop < stops.front() || stops.back() < zeroRadiusStop) {
        return;
    }

    size_t afterIndex =
            (truncateStops == TruncateStart)
                    ? std::lower_bound(stops.begin(), stops.end(), zeroRadiusStop) - stops.begin()
                    : std::upper_bound(stops.begin(), stops.end(), zeroRadiusStop) - stops.begin();

    const float t =
            (zeroRadiusStop - stops[afterIndex - 1]) / (stops[afterIndex] - stops[afterIndex - 1]);
    SkColor4f lerpColor = lerpSkColor(colors[afterIndex - 1], colors[afterIndex], t);

    if (truncateStops == TruncateStart) {
        stops.erase(stops.begin(), stops.begin() + afterIndex);
        colors.erase(colors.begin(), colors.begin() + afterIndex);
        stops.insert(stops.begin(), 0);
        colors.insert(colors.begin(), lerpColor);
    } else {
        stops.erase(stops.begin() + afterIndex, stops.end());
        colors.erase(colors.begin() + afterIndex, colors.end());
        stops.insert(stops.end(), 1);
        colors.insert(colors.end(), lerpColor);
    }
}

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

inline SkTileMode ToSkTileMode(uint8_t extendMode) {
    switch (extendMode) {
        case 1:
            return SkTileMode::kRepeat;
        case 2:
            return SkTileMode::kMirror;
        default:
            return SkTileMode::kClamp;
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

void ColorPainter::fill_radial(float x0,
                               float y0,
                               float startRadius,
                               float x1,
                               float y1,
                               float endRadius,
                               const fontations_ffi::ColorStop* color_stops,
                               size_t num_color_stops,
                               uint8_t extend_mode) {
    SkPaint paint;

    SkPoint start = SkPoint::Make(x0, y0);
    SkPoint end = SkPoint::Make(x1, y1);

    std::vector<SkScalar> stops;
    std::vector<SkColor4f> colors;

    stops.resize(num_color_stops);
    colors.resize(num_color_stops);

    for (size_t i = 0; i < num_color_stops; ++i) {
        stops[i] = SkFloatToScalar(color_stops[i].stop);
        colors[i] = SkColor4f::FromColor(fPalette[color_stops[i].palette_index]);
    }

    if (stops.size() == 1) {
        paint.setColor(colors[0]);
        return;
    }

    SkScalar colorStopRange = stops.back() - stops.front();
    SkTileMode tileMode = ToSkTileMode(extend_mode);

    if (colorStopRange == 0.f) {
        if (tileMode != SkTileMode::kClamp) {
            paint.setColor(SK_ColorTRANSPARENT);
            // return true;
            return;
        } else {
            // Insert duplicated fake color stop in pad case at +1.0f to enable the projection
            // of circles for an originally 0-length color stop range. Adding this stop will
            // paint the equivalent gradient, because: All font specified color stops are in the
            // same spot, mode is pad, so everything before this spot is painted with the first
            // color, everything after this spot is painted with the last color. Not adding this
            // stop will skip the projection and result in specifying non-normalized color stops
            // to the shader.
            stops.push_back(stops.back() + 1.0f);
            colors.push_back(colors.back());
            colorStopRange = 1.0f;
        }
    }
    SkASSERT(colorStopRange != 0.f);

    // If the colorStopRange is 0 at this point, the default behavior of the shader is to
    // clamp to 1 color stops that are above 1, clamp to 0 for color stops that are below 0,
    // and repeat the outer color stops at 0 and 1 if the color stops are inside the
    // range. That will result in the correct rendering.
    if (colorStopRange != 1 || stops.front() != 0.f) {
        // For the Skia two-point caonical shader to understand the
        // COLRv1 color stops we need to scale stops to 0 to 1 range and
        // interpolate new centers and radii. Otherwise the shader
        // clamps stops outside the range to 0 and 1 (larger interval)
        // or repeats the outer stops at 0 and 1 if the (smaller
        // interval).
        SkVector startToEnd = end - start;
        SkScalar radiusDiff = endRadius - startRadius;
        SkScalar scaleFactor = 1 / colorStopRange;
        SkScalar stopsStartOffset = stops.front();

        SkVector startOffset = startToEnd;
        startOffset.scale(stops.front());
        SkVector endOffset = startToEnd;
        endOffset.scale(stops.back());

        // The order of the following computations is important in order to avoid
        // overwriting start or startRadius before the second reassignment.
        end = start + endOffset;
        start = start + startOffset;
        endRadius = startRadius + radiusDiff * stops.back();
        startRadius = startRadius + radiusDiff * stops.front();

        for (auto& stop : stops) {
            stop = (stop - stopsStartOffset) * scaleFactor;
        }
    }

    // For negative radii, interpolation is needed to prepare parameters suitable
    // for invoking the shader. Implementation below as resolution discussed in
    // https://github.com/googlefonts/colr-gradients-spec/issues/367.
    // Truncate to manually interpolated color for tile mode clamp, otherwise
    // calculate positive projected circles.
    if (startRadius < 0 || endRadius < 0) {
        if (startRadius == endRadius && startRadius < 0) {
            paint.setColor(SK_ColorTRANSPARENT);
            // return true;
            return;
        }

        if (tileMode == SkTileMode::kClamp) {
            SkVector startToEnd = end - start;
            SkScalar radiusDiff = endRadius - startRadius;
            SkScalar zeroRadiusStop = 0.f;
            TruncateStops truncateSide = TruncateStart;
            if (startRadius < 0) {
                truncateSide = TruncateStart;

                // Compute color stop position where radius is = 0.  After the scaling
                // of stop positions to the normal 0,1 range that we have done above,
                // the size of the radius as a function of the color stops is: r(x) = r0
                // + x*(r1-r0) Solving this function for r(x) = 0, we get: x = -r0 /
                // (r1-r0)
                zeroRadiusStop = -startRadius / (endRadius - startRadius);
                startRadius = 0.f;
                SkVector startEndDiff = end - start;
                startEndDiff.scale(zeroRadiusStop);
                start = start + startEndDiff;
            }

            if (endRadius < 0) {
                truncateSide = TruncateEnd;
                zeroRadiusStop = -startRadius / (endRadius - startRadius);
                endRadius = 0.f;
                SkVector startEndDiff = end - start;
                startEndDiff.scale(1 - zeroRadiusStop);
                end = end - startEndDiff;
            }

            if (!(startRadius == 0 && endRadius == 0)) {
                truncateToStopInterpolating(zeroRadiusStop, colors, stops, truncateSide);
            } else {
                // If both radii have become negative and where clamped to 0, we need to
                // produce a single color cone, otherwise the shader colors the whole
                // plane in a single color when two radii are specified as 0.
                if (radiusDiff > 0) {
                    end = start + startToEnd;
                    endRadius = radiusDiff;
                    colors.erase(colors.begin(), colors.end() - 1);
                    stops.erase(stops.begin(), stops.end() - 1);
                } else {
                    start -= startToEnd;
                    startRadius = -radiusDiff;
                    colors.erase(colors.begin() + 1, colors.end());
                    stops.erase(stops.begin() + 1, stops.end());
                }
            }
        } else {
            if (startRadius < 0 || endRadius < 0) {
                auto roundIntegerMultiple = [](SkScalar factorZeroCrossing, SkTileMode tileMode) {
                    int roundedMultiple = factorZeroCrossing > 0 ? ceilf(factorZeroCrossing)
                                                                 : floorf(factorZeroCrossing) - 1;
                    if (tileMode == SkTileMode::kMirror && roundedMultiple % 2 != 0) {
                        roundedMultiple += roundedMultiple < 0 ? -1 : 1;
                    }
                    return roundedMultiple;
                };

                SkVector startToEnd = end - start;
                SkScalar radiusDiff = endRadius - startRadius;
                SkScalar factorZeroCrossing = (startRadius / (startRadius - endRadius));
                bool inRange = 0.f <= factorZeroCrossing && factorZeroCrossing <= 1.0f;
                SkScalar direction = inRange && radiusDiff < 0 ? -1.0f : 1.0f;
                SkScalar circleProjectionFactor =
                        roundIntegerMultiple(factorZeroCrossing * direction, tileMode);
                startToEnd.scale(circleProjectionFactor);
                startRadius += circleProjectionFactor * radiusDiff;
                endRadius += circleProjectionFactor * radiusDiff;
                start += startToEnd;
                end += startToEnd;
            }
        }
    }

    // An opaque color is needed to ensure the gradient is not modulated by alpha.
    paint.setColor(SK_ColorBLACK);

    paint.setShader(SkGradientShader::MakeTwoPointConical(
            start,
            startRadius,
            end,
            endRadius,
            colors.data(),
            SkColorSpace::MakeSRGB(),
            stops.data(),
            stops.size(),
            tileMode,
            SkGradientShader::Interpolation{SkGradientShader::Interpolation::InPremul::kNo,
                                            SkGradientShader::Interpolation::ColorSpace::kSRGB,
                                            SkGradientShader::Interpolation::HueMethod::kShorter},
            nullptr));

    fCanvas.drawPaint(paint);
}

void ColorPainter::push_layer(uint8_t compositeMode) {
    SkPaint paint;
    paint.setBlendMode(ToSkBlendMode(compositeMode));
    fCanvas.saveLayer(nullptr, &paint);
}

void ColorPainter::pop_layer() { fCanvas.restore(); }

}  // namespace sk_fontations
