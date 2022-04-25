/*
 * Copyright 2022 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "samplecode/Sample.h"

#include "include/core/SkFontMetrics.h"
#include "include/core/SkTime.h"
#include "include/core/SkTypeface.h"
#include "modules/skottie/include/Skottie.h"
#include "modules/skottie/include/SkottieProperty.h"
#include "src/core/SkAdvancedTypefaceMetrics.h"
#include "src/core/SkFontDescriptor.h"
#include "src/core/SkFontPriv.h"
#include "src/core/SkOSFile.h"
#include "src/core/SkScalerContext.h"
#include "src/sfnt/SkOTUtils.h"
#include "src/utils/SkOSPath.h"
#include "tools/Resources.h"

#include <tuple>
#include <vector>

static constexpr char kSkottieResource[] = "skottie/skottie-textpath-06.json";
static constexpr char kSkottieFontDir [] = "/home/mali/skottie_font";

class SkottieTypeface;

class SkottieScalerContext final : public SkScalerContext {
public:
    SkottieScalerContext(sk_sp<SkTypeface>, const SkScalerContextEffects&, const SkDescriptor*);

private:
    SkottieTypeface* skottieTF() const;

    bool generateAdvance(SkGlyph* glyph) override;
    void generateMetrics(SkGlyph* glyph, SkArenaAlloc*) override;
    void generateFontMetrics(SkFontMetrics*) override;
    void generateImage(const SkGlyph& glyph) override;
    bool SK_WARN_UNUSED_RESULT generatePath(const SkGlyph&, SkPath*) override;

    SkMatrix fMatrix;

    using INHERITED = SkScalerContext;
};

class SkottieTypeface final : public SkTypeface {
public:
    static sk_sp<SkottieTypeface> Make() {
        SkOSFile::Iter iter(kSkottieFontDir, "json");

        std::vector<GlyphData> glyphs;

        // glyph 0 -> missing glyph
        glyphs.push_back({nullptr, {0,0}, 0, 0});

        static constexpr auto kAscentRatio = 0.7f,
                           kGlyphAnimRatio = 0.5f;
        float max_width  = 0,
              max_height = 0;

        SkString glyph_file;
        while (iter.next(&glyph_file)) {
            auto glyph_path = SkOSPath::Join(kSkottieFontDir, glyph_file.c_str());
            auto anim = skottie::Animation::MakeFromFile(glyph_path.c_str());
            if (!anim) {
                continue;
            }

            anim->seekFrame(0);

            SkASSERT(!glyph_file.isEmpty());

            const auto ascent = -anim->size().height()*kAscentRatio;
            const SkScalar advance = anim->size().width()*kGlyphAnimRatio;
            const SkPoint   origin = {-advance/2, -ascent};
            const SkUnichar    chr = glyph_file[0];

            max_width  = std::max(max_width , anim->size().width());
            max_height = std::max(max_height, anim->size().height());

            glyphs.push_back({std::move(anim), origin, advance, chr});
        }

        const SkFontMetrics metrics = {
            SkFontMetrics::kUnderlineThicknessIsValid_Flag         // fFlags
                | SkFontMetrics::kUnderlinePositionIsValid_Flag
                | SkFontMetrics::kStrikeoutThicknessIsValid_Flag
                | SkFontMetrics::kStrikeoutPositionIsValid_Flag,
            -max_height*kAscentRatio,                              // fTop
            -max_height*kAscentRatio,                              // fAscent
            max_height*(1 - kAscentRatio),                         // fDescent
            max_height*(1 - kAscentRatio),                         // fBottom
            max_height*(1 - kAscentRatio)*0.5f,                    // fLeading
            max_width,                                             // fAvgCharWidth
            max_width,                                             // fMaxCharWidth
            0,                                                     // fXMin
            max_width,                                             // fXMax
            0,                                                     // fXHeight
            0,                                                     // fCapHeight
            max_height*0.05f,                                      // fUnderlineThickness
            max_height*(1 - kAscentRatio)*0.1f,                    // fUnderlinePosition
            max_height*0.02f,                                      // fStrikeoutThickness
            -max_height*kAscentRatio*0.5f,                         // fStrikeoutPosition
        };

        SkDebugf("Loaded %zu animated glyphs.\n", glyphs.size());

        const auto upem = static_cast<int>(max_height*kGlyphAnimRatio);
        return sk_sp<SkottieTypeface>(new SkottieTypeface("skottieface", SkFontStyle::Normal(),
                                      upem, metrics, std::move(glyphs)));
    }

    sk_sp<SkottieTypeface> makeForTime(double t) {
        for (const auto& gd : fGlyphs) {
            if (gd.fAnimation) {
                gd.fAnimation->seek(std::fmod(t, gd.fAnimation->duration()));
            }
        }

        return sk_sp<SkottieTypeface>(new SkottieTypeface(fName.c_str(), this->fontStyle(), fUpem,
                                                          fFontMetrics, std::move(fGlyphs)));
    }

private:
    struct GlyphData {
        sk_sp<skottie::Animation> fAnimation;
        SkPoint                   fOrigin;
        SkScalar                  fAdvance;
        SkUnichar                 fChar;
    };

    SkottieTypeface(const char* name, const SkFontStyle& style, int upem,
                    const SkFontMetrics& metrics, std::vector<GlyphData>&& glyphs)
        : INHERITED(style, false)
        , fName(name)
        , fUpem(upem)
        , fFontMetrics(metrics)
        , fGlyphs(std::move(glyphs))
    {
        for (size_t i = 0; i < fGlyphs.size(); ++i) {
            fCMap.set(fGlyphs[i].fChar, SkTo<SkGlyphID>(i));
        }
    }

    void getAdvance(SkGlyph* glyph) const {
        SkGlyphID glyphID = glyph->getGlyphID();
        glyphID = glyphID < fGlyphs.size() ? glyphID : 0;

        glyph->fAdvanceX = fGlyphs[glyphID].fAdvance;
        glyph->fAdvanceY = 0;
    }

    sk_sp<SkTypeface> onMakeClone(const SkFontArguments&) const override { return sk_ref_sp(this); }

    std::unique_ptr<SkScalerContext> onCreateScalerContext(
            const SkScalerContextEffects& effects,
            const SkDescriptor* desc) const override {
        return std::make_unique<SkottieScalerContext>(sk_ref_sp(this), effects, desc);
    }

    void onFilterRec(SkScalerContextRec* rec) const override {
        rec->setHinting(SkFontHinting::kNone);
    }

    std::unique_ptr<SkAdvancedTypefaceMetrics> onGetAdvancedMetrics() const override {
        std::unique_ptr<SkAdvancedTypefaceMetrics> info(new SkAdvancedTypefaceMetrics);
        info->fFontName = fName;
        return info;
    }

    void getPostScriptGlyphNames(SkString*) const override {}

    void getGlyphToUnicodeMap(SkUnichar glyphToUnicode[]) const override {
        for (size_t i = 0; i < fGlyphs.size(); ++i) {
            glyphToUnicode[i] = fGlyphs[i].fChar;
        }
    }

    void onCharsToGlyphs(const SkUnichar* chars, int count, SkGlyphID glyphs[]) const override {
        for (int i = 0; i < count; ++i) {
            SkGlyphID* gid = fCMap.find(chars[i]);
            glyphs[i] = gid ? *gid : 0;
        }
    }

    std::unique_ptr<SkStreamAsset> onOpenStream(int* ttcIndex) const override {
        return nullptr;
    }

    bool onGlyphMaskNeedsCurrentColor() const override {
        return false;
    }

    int onGetVariationDesignPosition(SkFontArguments::VariationPosition::Coordinate[],
                                     int) const override {
        return 0;
    }
    int onGetVariationDesignParameters(SkFontParameters::Variation::Axis[], int) const override {
        return 0;
    }

    void onGetFontDescriptor(SkFontDescriptor* desc, bool* isLocal) const override {
        desc->setFamilyName(fName.c_str());
        desc->setStyle(this->fontStyle());
        *isLocal = false;
    }

    int onCountGlyphs() const override { return SkToInt(fGlyphs.size()); }

    int onGetUPEM() const override { return fUpem; }

    void onGetFamilyName(SkString* familyName) const override { *familyName = fName; }
    bool onGetPostScriptName(SkString*) const override { return false; }
    LocalizedStrings* onCreateFamilyNameIterator() const override {
        SkString familyName(fName);
        SkString language("und");  // undetermined
        return new SkOTUtils::LocalizedStrings_SingleName(familyName, language);
    }

    int onGetTableTags(SkFontTableTag tags[]) const override { return 0; }
    size_t onGetTableData(SkFontTableTag, size_t, size_t, void*) const override { return 0; }

    const SkString                   fName;
    const int                        fUpem;
    const SkFontMetrics              fFontMetrics;
    std::vector<GlyphData>     fGlyphs;
    SkTHashMap<SkUnichar, SkGlyphID> fCMap;

    friend class SkottieScalerContext;

    using INHERITED = SkTypeface;
};

SkottieScalerContext::SkottieScalerContext(sk_sp<SkTypeface> tface,
                                           const SkScalerContextEffects& effects,
                                           const SkDescriptor* desc)
    : INHERITED(std::move(tface), effects, desc)
{
    fRec.getSingleMatrix(&fMatrix);
    SkScalar upem = this->skottieTF()->fUpem;
    fMatrix.preScale(1.f / upem, 1.f / upem);
}

SkottieTypeface* SkottieScalerContext::skottieTF() const {
    return static_cast<SkottieTypeface*>(this->getTypeface());
}

bool SkottieScalerContext::generateAdvance(SkGlyph* glyph) {
    this->skottieTF()->getAdvance(glyph);
    const SkVector advance =
            fMatrix.mapXY(SkFloatToScalar(glyph->fAdvanceX), SkFloatToScalar(glyph->fAdvanceY));
    glyph->fAdvanceX = SkScalarToFloat(advance.fX);
    glyph->fAdvanceY = SkScalarToFloat(advance.fY);

    return true;
}

void SkottieScalerContext::generateMetrics(SkGlyph* glyph, SkArenaAlloc* alloc) {
    SkGlyphID glyphID = glyph->getGlyphID();
    glyphID           = glyphID < this->skottieTF()->fGlyphs.size() ? glyphID : 0;

    glyph->zeroMetrics();
    glyph->fMaskFormat = SkMask::kARGB32_Format;
    glyph->setPath(alloc, nullptr, false);
    this->generateAdvance(glyph);

    const auto& glyphData = this->skottieTF()->fGlyphs[glyphID];
    const auto  glyphSize = glyphData.fAnimation ? glyphData.fAnimation->size()
                                                 : SkSize::Make(0, 0);

    SkRect newBounds = SkRect::MakeXYWH(glyphData.fOrigin.fX,
                                       -glyphData.fOrigin.fY,
                                        glyphSize.fWidth,
                                        glyphSize.fHeight);
    fMatrix.mapRect(&newBounds);
    SkScalar dx = SkFixedToScalar(glyph->getSubXFixed());
    SkScalar dy = SkFixedToScalar(glyph->getSubYFixed());
    newBounds.offset(dx, dy);

    SkIRect ibounds;
    newBounds.roundOut(&ibounds);
    glyph->fLeft   = ibounds.fLeft;
    glyph->fTop    = ibounds.fTop;
    glyph->fWidth  = ibounds.width();
    glyph->fHeight = ibounds.height();
}

void SkottieScalerContext::generateFontMetrics(SkFontMetrics* metrics) {
    *metrics = this->skottieTF()->fFontMetrics;
    SkFontPriv::ScaleFontMetrics(metrics, fMatrix.getScaleY());
}

void SkottieScalerContext::generateImage(const SkGlyph& glyph) {
    SkGlyphID glyphID = glyph.getGlyphID();
    glyphID           = glyphID < this->skottieTF()->fGlyphs.size() ? glyphID : 0;

    SkBitmap bm;
    // TODO: this should be SkImageInfo::MakeS32 when that passes all the tests.
    bm.installPixels(SkImageInfo::MakeN32(glyph.fWidth, glyph.fHeight, kPremul_SkAlphaType),
                     glyph.fImage,
                     glyph.rowBytes());
    bm.eraseColor(0);

    const auto& glyphData = this->skottieTF()->fGlyphs[glyphID];

    SkScalar dx = SkFixedToScalar(glyph.getSubXFixed());
    SkScalar dy = SkFixedToScalar(glyph.getSubYFixed());

    SkCanvas canvas(bm);
    canvas.translate(-glyph.fLeft, -glyph.fTop);
    canvas.translate(dx, dy);
    canvas.concat(fMatrix);
    canvas.translate(glyphData.fOrigin.fX, -glyphData.fOrigin.fY);

    if (glyphData.fAnimation) {
        glyphData.fAnimation->render(&canvas);
    }
}

bool SkottieScalerContext::generatePath(const SkGlyph& glyph, SkPath* path) {
    // Should never get here since generateMetrics always sets the path to not exist.
    SK_ABORT("Path requested, but it should have been indicated that there isn't one.");
    path->reset();
    return false;
}

class SampleAnimatedFont final : public Sample {
public:
    SampleAnimatedFont()
        : fTextManager(sk_make_sp<TextManager>())
        , fTypeface(SkottieTypeface::Make())
    {
        auto stream = GetResourceAsStream(kSkottieResource);
        SkASSERT(stream);

        fAnimation = skottie::Animation::Builder()
                        .setPropertyObserver(fTextManager)
                        .make(stream.get());
        SkASSERT(fAnimation);
    }

private:
    class TextManager final : public skottie::PropertyObserver {
    public:
        TextManager() {}

        void updateTypeface(const sk_sp<SkTypeface>& tf) {
            for (const auto& tlayer : fTextLayers) {
                auto txt = tlayer->get();
                txt.fTypeface = tf;
                tlayer->set(txt);
            }
        }

    private:
        void onTextProperty(const char[],
                            const LazyHandle<skottie::TextPropertyHandle>& lh) override {
            fTextLayers.push_back(lh());
        }

        std::vector<std::unique_ptr<skottie::TextPropertyHandle>> fTextLayers;
    };

    SkString name() override { return SkString("AnimatedFont"); }

    bool onAnimate(double nanos) override {
        if (fAnimateFace) {
            const auto s = SkTime::GetSecs();
            if (!fFaceTimeBase) {
                fFaceTimeBase = s;
            }

            fTypeface = fTypeface->makeForTime(s - fFaceTimeBase);
            fTextManager->updateTypeface(fTypeface);
        }

        if (!fAnimTimeBase) {
            fAnimTimeBase = nanos;
        }

        const auto frame_count = fAnimation->duration() * fAnimation->fps(),
                         t_sec = (nanos - fAnimTimeBase) * 1e-9,
                         frame = std::fmod(t_sec * fAnimation->fps(), frame_count);

        fAnimation->seekFrame(frame);

        return true;
    }

    bool onChar(SkUnichar ch) override {
        if (ch == 'A') {
            fAnimateFace = !fAnimateFace;
            return true;
        }

        return false;
    }

    void onDrawContent(SkCanvas* canvas) override {
        const auto dst = SkRect::MakeWH(this->width(), this->height());
        fAnimation->render(canvas, &dst);
    }

    sk_sp<TextManager>        fTextManager;
    sk_sp<skottie::Animation> fAnimation;
    sk_sp<SkottieTypeface>    fTypeface;
    double                    fAnimTimeBase = 0,  // nanos
                              fFaceTimeBase = 0;  // secs
    bool                      fAnimateFace  = true;
};

DEF_SAMPLE( return new SampleAnimatedFont(); )
