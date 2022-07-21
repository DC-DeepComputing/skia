/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/utils/SkCustomTypeface.h"

#include "include/core/SkCanvas.h"
#include "include/core/SkData.h"
#include "include/core/SkDrawable.h"
#include "include/core/SkFontArguments.h"
#include "include/core/SkFontMetrics.h"
#include "include/core/SkFontParameters.h"
#include "include/core/SkFontStyle.h"
#include "include/core/SkFontTypes.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPath.h"
#include "include/core/SkPathTypes.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkStream.h"
#include "include/core/SkString.h"
#include "include/core/SkTypeface.h"
#include "include/core/SkTypes.h"
#include "include/private/SkFloatingPoint.h"
#include "include/private/SkMalloc.h"
#include "include/private/SkTo.h"
#include "src/core/SkAdvancedTypefaceMetrics.h" // IWYU pragma: keep
#include "src/core/SkAutoMalloc.h"
#include "src/core/SkGlyph.h"
#include "src/core/SkPathPriv.h"
#include "src/core/SkScalerContext.h"

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

class SkArenaAlloc;
class SkDescriptor;
class SkFontDescriptor;

static SkFontMetrics scale_fontmetrics(const SkFontMetrics& src, float sx, float sy) {
    SkFontMetrics dst = src;

    #define SCALE_X(field)  dst.field *= sx
    #define SCALE_Y(field)  dst.field *= sy

    SCALE_X(fAvgCharWidth);
    SCALE_X(fMaxCharWidth);
    SCALE_X(fXMin);
    SCALE_X(fXMax);

    SCALE_Y(fTop);
    SCALE_Y(fAscent);
    SCALE_Y(fDescent);
    SCALE_Y(fBottom);
    SCALE_Y(fLeading);
    SCALE_Y(fXHeight);
    SCALE_Y(fCapHeight);
    SCALE_Y(fUnderlineThickness);
    SCALE_Y(fUnderlinePosition);
    SCALE_Y(fStrikeoutThickness);
    SCALE_Y(fStrikeoutPosition);

    #undef SCALE_X
    #undef SCALE_Y

    return dst;
}

class SkUserTypeface final : public SkTypeface {
private:
    friend class SkCustomTypefaceBuilder;
    friend class SkUserScalerContext;

    explicit SkUserTypeface(SkFontStyle style, std::vector<SkPath>&& paths,
                            std::vector<SkCustomTypefaceBuilder::DrawableRec>&& drawables,
                            std::vector<float>&& advances,
                            const SkFontMetrics& metrics)
        : SkTypeface(style)
        , fPaths(std::move(paths))
        , fDrawables(std::move(drawables))
        , fAdvances(std::move(advances))
        , fMetrics(metrics)
    {}

    const std::vector<SkPath>                               fPaths;
    const std::vector<SkCustomTypefaceBuilder::DrawableRec> fDrawables;
    const std::vector<float>                                fAdvances;
    const SkFontMetrics                                     fMetrics;

    std::unique_ptr<SkScalerContext> onCreateScalerContext(const SkScalerContextEffects&,
                                                           const SkDescriptor* desc) const override;
    void onFilterRec(SkScalerContextRec* rec) const override;
    void getGlyphToUnicodeMap(SkUnichar* glyphToUnicode) const override;
    std::unique_ptr<SkAdvancedTypefaceMetrics> onGetAdvancedMetrics() const override;

    void onGetFontDescriptor(SkFontDescriptor* desc, bool* isLocal) const override;

    void onCharsToGlyphs(const SkUnichar* chars, int count, SkGlyphID glyphs[]) const override;

    void onGetFamilyName(SkString* familyName) const override;
    bool onGetPostScriptName(SkString*) const override;
    SkTypeface::LocalizedStrings* onCreateFamilyNameIterator() const override;

    std::unique_ptr<SkStreamAsset> onOpenStream(int*) const override;

    // trivial

    sk_sp<SkTypeface> onMakeClone(const SkFontArguments& args) const override {
        return sk_ref_sp(this);
    }
    int onCountGlyphs() const override { return this->glyphCount(); }
    int onGetUPEM() const override { return 2048; /* ?? */ }
    bool onComputeBounds(SkRect* bounds) const override {
        bounds->setLTRB(fMetrics.fXMin, fMetrics.fTop, fMetrics.fXMax, fMetrics.fBottom);
        return true;
    }

    // noops

    void getPostScriptGlyphNames(SkString*) const override {}
    bool onGlyphMaskNeedsCurrentColor() const override { return false; }
    int onGetVariationDesignPosition(SkFontArguments::VariationPosition::Coordinate[],
                                     int) const override { return 0; }
    int onGetVariationDesignParameters(SkFontParameters::Variation::Axis[],
                                       int) const override { return 0; }
    int onGetTableTags(SkFontTableTag tags[]) const override { return 0; }
    size_t onGetTableData(SkFontTableTag, size_t, size_t, void*) const override { return 0; }

    int glyphCount() const {
        SkASSERT(fPaths.size() == fAdvances.size());
        SkASSERT(fDrawables.size() == fAdvances.size());
        return SkToInt(fPaths.size());
    }
};

SkCustomTypefaceBuilder::SkCustomTypefaceBuilder() {
    sk_bzero(&fMetrics, sizeof(fMetrics));
}

void SkCustomTypefaceBuilder::setMetrics(const SkFontMetrics& fm, float scale) {
    fMetrics = scale_fontmetrics(fm, scale, scale);
}

void SkCustomTypefaceBuilder::setFontStyle(SkFontStyle style) {
    fStyle = style;
}

void SkCustomTypefaceBuilder::ensureStorage(SkGlyphID index) {
    SkASSERT(fPaths.size() == fAdvances.size());
    SkASSERT(fPaths.size() == fDrawables.size());
    if (index >= fPaths.size()) {
           fPaths.resize(SkToSizeT(index) + 1);
       fDrawables.resize(SkToSizeT(index) + 1);
        fAdvances.resize(SkToSizeT(index) + 1);
    }
}

void SkCustomTypefaceBuilder::setGlyph(SkGlyphID index, float advance, const SkPath& path) {
    this->ensureStorage(index);

    fAdvances [index] = advance;
    fPaths    [index] = path;
    fDrawables[index].fDraw = nullptr;
}

void SkCustomTypefaceBuilder::setGlyph(SkGlyphID index, float advance,
                                       DrawableRec drawable) {
    this->ensureStorage(index);

    fAdvances [index] = advance;
    fDrawables[index] = std::move(drawable);
    fPaths    [index].reset();
}

sk_sp<SkTypeface> SkCustomTypefaceBuilder::detach() {
    SkASSERT(fPaths.size() == fAdvances.size());
    SkASSERT(fPaths.size() == fDrawables.size());
    if (fPaths.empty()) return nullptr;

    // initially inverted, so that any "union" will overwrite the first time
    SkRect bounds = {SK_ScalarMax, SK_ScalarMax, -SK_ScalarMax, -SK_ScalarMax};

    for (const auto& path : fPaths) {
        if (!path.isEmpty()) {
            bounds.join(path.getBounds());
        }
    }

    for (const auto& drawable : fDrawables) {
        if (drawable.fDraw) {
            bounds.join(drawable.fBounds);
        }
    }

    fMetrics.fTop    = bounds.top();
    fMetrics.fBottom = bounds.bottom();
    fMetrics.fXMin   = bounds.left();
    fMetrics.fXMax   = bounds.right();

    return sk_sp<SkUserTypeface>(new SkUserTypeface(fStyle,
                                                    std::move(fPaths),
                                                    std::move(fDrawables),
                                                    std::move(fAdvances),
                                                    fMetrics));
}

/////////////

void SkUserTypeface::onFilterRec(SkScalerContextRec* rec) const {
    rec->setHinting(SkFontHinting::kNone);
}

void SkUserTypeface::getGlyphToUnicodeMap(SkUnichar* glyphToUnicode) const {
    for (int gid = 0; gid < this->glyphCount(); ++gid) {
        glyphToUnicode[gid] = SkTo<SkUnichar>(gid);
    }
}

std::unique_ptr<SkAdvancedTypefaceMetrics> SkUserTypeface::onGetAdvancedMetrics() const {
    return nullptr;
}

void SkUserTypeface::onGetFontDescriptor(SkFontDescriptor* desc, bool* isLocal) const {
    *isLocal = true;
}

void SkUserTypeface::onCharsToGlyphs(const SkUnichar uni[], int count, SkGlyphID glyphs[]) const {
    for (int i = 0; i < count; ++i) {
        glyphs[i] = uni[i] < this->glyphCount() ? SkTo<SkGlyphID>(uni[i]) : 0;
    }
}

void SkUserTypeface::onGetFamilyName(SkString* familyName) const {
    *familyName = "";
}

bool SkUserTypeface::onGetPostScriptName(SkString*) const {
    return false;
}

SkTypeface::LocalizedStrings* SkUserTypeface::onCreateFamilyNameIterator() const {
    return nullptr;
}

//////////////

class SkUserScalerContext : public SkScalerContext {
public:
    SkUserScalerContext(sk_sp<SkUserTypeface>           face,
                        const SkScalerContextEffects& effects,
                        const SkDescriptor*           desc)
            : SkScalerContext(std::move(face), effects, desc) {
        fRec.getSingleMatrix(&fMatrix);
        this->forceGenerateImageFromPath();
    }

    const SkUserTypeface* userTF() const {
        return static_cast<SkUserTypeface*>(this->getTypeface());
    }

protected:
    bool generateAdvance(SkGlyph* glyph) override {
        const SkUserTypeface* tf = this->userTF();
        auto advance = fMatrix.mapXY(tf->fAdvances[glyph->getGlyphID()], 0);

        glyph->fAdvanceX = advance.fX;
        glyph->fAdvanceY = advance.fY;
        return true;
    }

    void generateMetrics(SkGlyph* glyph, SkArenaAlloc* alloc) override {
        glyph->zeroMetrics();
        this->generateAdvance(glyph);

        const auto& drec = this->userTF()->fDrawables[glyph->getGlyphID()];
        if (drec.fDraw) {
            glyph->fMaskFormat = SkMask::kARGB32_Format;

            SkRect bounds = fMatrix.mapRect(drec.fBounds);
            bounds.offset(SkFixedToScalar(glyph->getSubXFixed()),
                          SkFixedToScalar(glyph->getSubYFixed()));

            SkIRect ibounds;
            bounds.roundOut(&ibounds);
            glyph->fLeft   = ibounds.fLeft;
            glyph->fTop    = ibounds.fTop;
            glyph->fWidth  = ibounds.width();
            glyph->fHeight = ibounds.height();

            glyph->setDrawable(alloc, drec.fDraw);
        }
    }

    void generateImage(const SkGlyph& glyph) override {
        const auto& drec = this->userTF()->fDrawables[glyph.getGlyphID()];
        SkASSERTF(drec.fDraw, "Only drawable-backed glyphs should reach generateImage.");

        auto canvas = SkCanvas::MakeRasterDirectN32(glyph.fWidth, glyph.fHeight,
                                                    static_cast<SkPMColor*>(glyph.fImage),
                                                    glyph.rowBytes());
        canvas->clear(SK_ColorTRANSPARENT);
        canvas->translate(-glyph.fLeft, -glyph.fTop);
        canvas->drawDrawable(drec.fDraw.get(), &fMatrix);
    }

    bool generatePath(const SkGlyph& glyph, SkPath* path) override {
        const auto& glyph_path = this->userTF()->fPaths[glyph.getGlyphID()];

        if (glyph_path.isEmpty()) {
            return false;
        }

        glyph_path.transform(fMatrix, path);

        return true;
    }

    sk_sp<SkDrawable> generateDrawable(const SkGlyph& glyph) override {
        const auto& glyph_drawable = this->userTF()->fDrawables[glyph.getGlyphID()].fDraw;
        if (!glyph_drawable) {
            return nullptr;
        }

        // TODO: transform wrapper
        return glyph_drawable;
    }

    void generateFontMetrics(SkFontMetrics* metrics) override {
        auto [sx, sy] = fMatrix.mapXY(1, 1);
        *metrics = scale_fontmetrics(this->userTF()->fMetrics, sx, sy);
    }

private:
    SkMatrix fMatrix;
};

std::unique_ptr<SkScalerContext> SkUserTypeface::onCreateScalerContext(
    const SkScalerContextEffects& effects, const SkDescriptor* desc) const
{
    return std::make_unique<SkUserScalerContext>(
            sk_ref_sp(const_cast<SkUserTypeface*>(this)), effects, desc);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

static void write_scaled_float_to_16(SkWStream* stream, float x, float scale) {
    stream->write16(SkToS16(sk_float_round2int(x * scale)) & 0xFFFF);
}

enum PVerb {
    kMove,
    kLine,
    kCurve,
    kClose,
};

static void compress_write(SkWStream* stream, const SkPath& path, int upem) {
    int pCount = 0;
    std::vector<PVerb> verbs;
    for (auto [v, p, w] : SkPathPriv::Iterate(path)) {
        switch (v) {
            default: break;
            case SkPathVerb::kMove: verbs.push_back(kMove); pCount += 1; break;
            case SkPathVerb::kQuad: verbs.push_back(kCurve); pCount += 2; break;
            case SkPathVerb::kLine: verbs.push_back(kLine); pCount += 1; break;
            case SkPathVerb::kClose: verbs.push_back(kClose); break;
        }
    }

    int vCount = verbs.size();

    stream->write16(upem);      // share w/ other paths?
    stream->write16(vCount);
    stream->write16(pCount);
    for (int i = 0; i < (vCount & ~3); i += 4) {
        stream->write8((verbs[i+0]<<6) | (verbs[i+1]<<4) | (verbs[i+2]<<2) | verbs[i+3]);
    }
    if (vCount & 3) {
        uint8_t b = 0;
        int shift = 6;
        for (int i = vCount & ~3; i < vCount; ++i) {
            b |= verbs[i] << shift;
            shift >>= 2;
        }
        stream->write8(b);
    }
    if (vCount & 1) {
        stream->write8(0);
    }

    const float scale = (float)upem;
    auto write_pts = [&](const SkPoint pts[], int count) {
        for (int i = 0; i < count; ++i) {
            write_scaled_float_to_16(stream, pts[i].fX, scale);
            write_scaled_float_to_16(stream, pts[i].fY, scale);
        }
    };

    for (auto [v, p, w] : SkPathPriv::Iterate(path)) {
        switch (v) {
            default: break;
            case SkPathVerb::kMove: write_pts(&p[0], 1); break;
            case SkPathVerb::kQuad: write_pts(&p[1], 2); break;
            case SkPathVerb::kLine: write_pts(&p[1], 1); break;
            case SkPathVerb::kClose:                     break;
        }
    }
}

static constexpr int kMaxGlyphCount = 65536;
static constexpr size_t kHeaderSize = 16;
static const char gHeaderString[] = "SkUserTypeface01";
static_assert(sizeof(gHeaderString) == 1 + kHeaderSize, "need header to be 16 bytes");

enum GlyphType : uint32_t { kPath, kDrawable };

std::unique_ptr<SkStreamAsset> SkUserTypeface::onOpenStream(int* ttcIndex) const {
    SkDynamicMemoryWStream wstream;

    wstream.write(gHeaderString, kHeaderSize);

    wstream.write(&fMetrics, sizeof(fMetrics));

    SkFontStyle style = this->fontStyle();
    wstream.write(&style, sizeof(style));

    // just hacking around -- this makes the serialized font 1/2 size
    const bool use_compression = false;

    wstream.write32(this->glyphCount());

    if (use_compression) {
        for (float a : fAdvances) {
            write_scaled_float_to_16(&wstream, a, 2048);
        }
    } else {
        wstream.write(fAdvances.data(), this->glyphCount() * sizeof(float));
    }

    auto write_data = [](SkWStream& str, const sk_sp<SkData>& data) {
        const size_t sz = data->size();
        SkASSERT(sz);

        str.write(&sz, sizeof(sz));
        str.write(data->data(), sz);
    };

    for (size_t i = 0; i < SkToSizeT(this->glyphCount()); ++i) {
        const auto& drec = fDrawables[i];
        if (drec.fDraw) {
            // drawable glyph
            SkASSERT(fPaths[i].isEmpty());
            wstream.write32(GlyphType::kDrawable);
            wstream.write(&drec.fBounds, sizeof(drec.fBounds));
            write_data(wstream, drec.fDraw->serialize());
        } else {
            // path glyph
            wstream.write32(GlyphType::kPath);

            const auto& p = fPaths[i];
            if (use_compression) {
                compress_write(&wstream, p, 2048);
            } else {
                write_data(wstream, p.serialize());
            }
        }
    }
//    SkDebugf("%d glyphs, %d bytes\n", fGlyphCount, wstream.bytesWritten());
    *ttcIndex = 0;
    return wstream.detachAsStream();
}

class AutoRestorePosition {
    SkStream* fStream;
    size_t fPosition;
public:
    AutoRestorePosition(SkStream* stream) : fStream(stream) {
        fPosition = stream->getPosition();
    }

    ~AutoRestorePosition() {
        if (fStream) {
            fStream->seek(fPosition);
        }
    }

    // So we don't restore the position
    void markDone() { fStream = nullptr; }
};

sk_sp<SkTypeface> SkCustomTypefaceBuilder::Deserialize(SkStream* stream) {
    AutoRestorePosition arp(stream);

    char header[kHeaderSize];
    if (stream->read(header, kHeaderSize) != kHeaderSize ||
        0 != memcmp(header, gHeaderString, kHeaderSize))
    {
        return nullptr;
    }

    SkFontMetrics metrics;
    if (stream->read(&metrics, sizeof(metrics)) != sizeof(metrics)) {
        return nullptr;
    }

    SkFontStyle style;
    if (stream->read(&style, sizeof(style)) != sizeof(style)) {
        return nullptr;
    }

    int glyphCount;
    if (!stream->readS32(&glyphCount) || glyphCount < 0 || glyphCount > kMaxGlyphCount) {
        return nullptr;
    }

    SkCustomTypefaceBuilder builder;

    builder.setMetrics(metrics);
    builder.setFontStyle(style);

    std::vector<float> advances(glyphCount);
    if (stream->read(advances.data(), glyphCount * sizeof(float)) != glyphCount * sizeof(float)) {
        return nullptr;
    }

    // SkPath and SkDrawable cannot read from a stream, so we have to page them into ram
    auto read_data = [](SkStream* str) -> sk_sp<SkData> {
        size_t sz;
        if (str->read(&sz, sizeof(sz)) != sizeof(sz)) {
            return nullptr;
        }

        auto data = SkData::MakeUninitialized(sz);
        if (str->read(data->writable_data(), sz) != sz) {
            return nullptr;
        }

        return data;
    };

    for (int i = 0; i < glyphCount; ++i) {
        uint32_t gtype;
        if (!stream->readU32(&gtype)) {
            return nullptr;
        }

        switch (gtype) {
        case GlyphType::kDrawable: {
            SkRect bounds;
            if (stream->read(&bounds, sizeof(bounds)) != sizeof(bounds)) {
                return nullptr;
            }

            auto data = read_data(stream);
            if (!data) {
                return nullptr;
            }

            auto drawable = SkDrawable::Deserialize(data->data(), data->size());
            if (!drawable) {
                return nullptr;
            }
            builder.setGlyph(i, advances[i], { std::move(drawable), bounds });
        } break;
        case GlyphType::kPath: {
            auto data = read_data(stream);
            if (!data) {
                return nullptr;
            }

            SkPath path;
            if (path.readFromMemory(data->data(), data->size()) != data->size()) {
                return nullptr;
            }

            builder.setGlyph(i, advances[i], path);
        } break;
        default:
            return nullptr;
        }
    }

    arp.markDone();
    return builder.detach();
}
