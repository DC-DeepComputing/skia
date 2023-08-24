/*
 * Copyright 2023 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkData.h"
#include "include/core/SkFont.h"
#include "include/core/SkPath.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkStream.h"
#include "include/core/SkString.h"
#include "include/core/SkTypeface.h"
#include "include/ports/SkTypeface_fontations.h"
#include "modules/skshaper/include/SkShaper.h"
#include "src/utils/SkJSON.h"
#include "tools/Resources.h"

#include <regex>
#include <string>

namespace skiagm {

namespace {

const char kDefaultFontsExp[] = "Noto Sans.*";
const char kDefaultLangsExp[] = "(dhi_Deva|de_Latn)";
const char kRasterTestJsonPath[] = "third_party/externals/raster_test_data/raster_test.json";

// Iterates over test data from the font tests CIPD, allows filtering for fonts and test samples
// languages using C++ regular expressions.
class RasterTestDataIterator {
public:
    struct LangSample {
        SkString langTag;
        SkString sampleShort;
        SkString sampleLong;
    };

    struct TestSet {
        SkString fontName;
        SkString fontFilename;
        std::vector<LangSample> langSamples;
    };

    RasterTestDataIterator(const char* fontFilterRegexp, const char* langFilterRegexp)
            : fFontFilter(fontFilterRegexp), fLangFilter(langFilterRegexp) {
        sk_sp<SkData> jsonTestData = SkData::MakeFromFileName(kRasterTestJsonPath);
        fJsonDom = std::make_unique<skjson::DOM>(
                reinterpret_cast<const char*>(jsonTestData->bytes()), jsonTestData->size());
        const skjson::ObjectValue& root = fJsonDom->root().as<skjson::ObjectValue>();
        fFonts = root["fonts"];
        fSamples = root["samples"];
        SkASSERT(fFonts && fFonts->size() == 1558);
        SkASSERT(fSamples);
    }

    bool next(TestSet* testSet) {
        while (testSet && fFontsIndex < fFonts->size()) {
            const skjson::ObjectValue* fontsEntry = (*fFonts)[fFontsIndex++];
            SkASSERT(fontsEntry);
            const skjson::StringValue* fontName = (*fontsEntry)["name"];
            SkASSERT(fontName);
            std::smatch match;
            std::string fontNameStr(fontName->str());
            if (std::regex_match(fontNameStr, match, fFontFilter)) {
                testSet->fontName = SkString(fontNameStr);
                const skjson::StringValue* fontFilename = (*fontsEntry)["path"];
                testSet->fontFilename =
                        SkString(fontFilename->str().data(), fontFilename->str().size());
                testSet->langSamples =
                        getLanguageSamples((*fontsEntry)["languages"].as<skjson::ArrayValue>());
                return true;
            }
        }
        return false;
    }

private:
    std::vector<LangSample> getLanguageSamples(const skjson::ArrayValue* languages) {
        std::vector<LangSample> samples;
        for (size_t i = 0; i < languages->size(); ++i) {
            const skjson::StringValue* langTag = (*languages)[i];
            std::string langTagStr(langTag->str());
            const skjson::ObjectValue* sample = (*fSamples)[langTagStr.c_str()];
            const skjson::StringValue* shortSample = (*sample)["short_sample"];
            const skjson::StringValue* longSample = (*sample)["long_sample"];
            SkString sampleShort(shortSample->str().data(), shortSample->str().size());
            SkString sampleLong(longSample->str().data(), longSample->str().size());
            samples.push_back({SkString(langTagStr), sampleShort, sampleLong});
        }
        return samples;
    }
    std::regex fFontFilter;
    std::regex fLangFilter;
    size_t fFontsIndex = 2;
    std::unique_ptr<skjson::DOM> fJsonDom;
    const skjson::ArrayValue* fFonts;
    const skjson::ObjectValue* fSamples;
};

constexpr int kGmWidth = 1000;
constexpr int kMargin = 60;

const char kReportFontName[] = "fonts/Roboto-Regular.ttf";
// const SkScalar kDumpFontSize = 20.0f;

#if 0
bool textBlobsAllPathsEqual(sk_sp<const SkTextBlob> blobA,
                            sk_sp<const SkTextBlob> blobB,
                            SkString fontFilename) {
    SkTextBlob::Iter iterA(*blobA);
    SkTextBlob::Iter iterB(*blobB);
    SkTextBlob::Iter::ExperimentalRun runAInfo;
    SkTextBlob::Iter::ExperimentalRun runBInfo;
    SkPath pathA, pathB;
    while (iterA.experimentalNext(&runAInfo)) {
        pathA.reset();
        pathB.reset();
        SkASSERT(iterB.experimentalNext(&runBInfo));
        SkASSERT(runAInfo.count == runBInfo.count);
        for (int i = 0; i < runAInfo.count; ++i) {
            runAInfo.font.getPath(runAInfo.glyphs[i], &pathA);
            runBInfo.font.getPath(runBInfo.glyphs[i], &pathB);
            SkDynamicMemoryWStream streamA, streamB;
            // Hacky use of the dump method to account for close(), vs. final lineto(),
            // The SkPath::dump method overcomes that.
            pathA.dump(&streamA, false);
            pathB.dump(&streamB, false);
            sk_sp<SkData> dataA = streamA.detachAsData();
            sk_sp<SkData> dataB = streamB.detachAsData();
            if (dataA->size() != dataB->size() ||
                memcmp(dataA->data(), dataB->data(), dataA->size() - 1)) {
                printf("Different path for glyph index: %d glyph id: %d, font: %s data sizes %ld "
                       "vs %ld\n",
                       i,
                       runAInfo.glyphs[i],
                       fontFilename.c_str(),
                       dataA->size(),
                       dataB->size());
                printf("Path A (Fontations): \n");
                pathA.dump();
                printf("Path B (FreeType): \n");
                pathB.dump();
                return false;
            }
        }
    }
    return true;
}
#endif

}  // namespace

class FontationsFtCompareGM : public GM {
public:
    FontationsFtCompareGM(const char* fontFilename,
                          const char* testPhrase,
                          const char* languageOrScript)
            : fTestFontFilename(fontFilename)
            , fTestPhrase(testPhrase)
            , fLanguageOrScript(languageOrScript) {
        this->setBGColor(SK_ColorWHITE);
    }

protected:
    void onOnceBeforeDraw() override {
        fReportTypeface =
                SkTypeface_Make_Fontations(GetResourceAsStream(kReportFontName), SkFontArguments());
        RasterTestDataIterator testIterator("Noto Sans.*", "");
        RasterTestDataIterator::TestSet testSet;
        while (testIterator.next(&testSet)) {
            printf("Font: %s filename %s\n",
                   testSet.fontName.c_str(),
                   testSet.fontFilename.c_str());
            for (auto& langEntry : testSet.langSamples) {
                printf("lang %s\n", langEntry.langTag.c_str());
            }
            printf("\n");
        }
    }

    SkString getName() const {
        return SkStringPrintf("fontations_compare_ft_%s", fLanguageOrScript.c_str());
    }

    SkISize getISize() { return SkISize::Make(kGmWidth, 200); }

    DrawResult onDraw(SkCanvas* canvas, SkString* errorMsg) override {
        SkPaint paint;
        paint.setColor(SK_ColorBLACK);

        fTestTypeface = SkTypeface_Make_Fontations(GetResourceAsStream(fTestFontFilename.c_str()),
                                                   SkFontArguments());
        fFtTypeface = SkFontMgr::RefDefault()->makeFromStream(
                GetResourceAsStream(fTestFontFilename.c_str()), 0);

        if (!fTestTypeface || !fFtTypeface) {
            *errorMsg = "Unable to initialize typeface.";
            return DrawResult::kSkip;
        }

        auto configureFont = [](SkFont& font) {
            font.setSize(24);
            font.setEdging(SkFont::Edging::kSubpixelAntiAlias);
            font.setSubpixel(true);
            font.setHinting(SkFontHinting::kNone);
        };

        SkFont font(fTestTypeface);
        configureFont(font);

        SkFont ftFont(fFtTypeface);
        configureFont(ftFont);

        SkScalar availableWidth = kGmWidth / 2 - (kMargin * 3);

        SkTextBlobBuilderRunHandler textBlobBuilder(fTestPhrase.c_str(), {0, 0});
        std::unique_ptr<SkShaper> shaper = SkShaper::Make();
        shaper->shape(fTestPhrase.c_str(),
                      fTestPhrase.size(),
                      font,
                      true,
                      availableWidth,
                      &textBlobBuilder);
        sk_sp<const SkTextBlob> blob = textBlobBuilder.makeBlob();

        SkTextBlobBuilderRunHandler ftTextBlobBuilder(fTestPhrase.c_str(), {0, 0});
        shaper->shape(fTestPhrase.c_str(),
                      fTestPhrase.size(),
                      ftFont,
                      true,
                      availableWidth,
                      &ftTextBlobBuilder);
        sk_sp<const SkTextBlob> ftBlob = ftTextBlobBuilder.makeBlob();

        canvas->drawTextBlob(blob.get(), SkDoubleToScalar(kMargin), SkDoubleToScalar(100), paint);
        // Comment out for FreeType results.
        // canvas->drawTextBlob(ftBlob.get(),
        //                      SkDoubleToScalar(kMargin),
        //                      SkDoubleToScalar(100),
        //                      paint);

        // No path comparison needed here.
        // SkASSERT(textBlobsAllPathsEqual(blob, ftBlob, fTestFontFilename));

        return DrawResult::kOk;
    }

private:
    using INHERITED = GM;

    const SkString fTestFontFilename;
    const SkString fTestPhrase;
    const SkString fLanguageOrScript;
    sk_sp<SkTypeface> fTestTypeface;
    sk_sp<SkTypeface> fFtTypeface;
    sk_sp<SkTypeface> fReportTypeface;
    std::unique_ptr<SkFontArguments::VariationPosition::Coordinate[]> fCoordinates;
};

// Sample phrases from https://r12a.github.io/scripts/samples/index.html, license CC-by-SA
// and fonts.google.com.
// Disable CFF font examples for now.
// DEF_GM(return new FontationsFtCompareGM(
//         "fonts/noto/NotoSansTC-Regular.otf",
//         "鑑於對人類家庭所有成員的固有尊嚴及其平等的和不移的權利的承認，乃是世界自由、正義與和平的基礎",
//         "traditional_chinese"));
// DEF_GM(return new FontationsFtCompareGM(
//         "fonts/noto/NotoSansSC-Regular.otf",
//         "人人生而自由，在尊嚴和權利上一律平等。他們賦有理性和良心，並應以兄弟關係的精神相對待"
//         "。",
//         "simplified_chinese"));
DEF_GM(return new FontationsFtCompareGM(
        "fonts/noto/NotoSansThai-Regular.ttf",
        "มนุษย์ทั้งหลายเกิดมามีอิสระและเสมอภาคกันในเกียรติศักด[เกียรติศักดิ์]และสิทธิ ต่างมีเหตุผลและมโนธรรม "
        "และควรปฏิบัติต่อกันด้วยเจตนารมณ์แห่งภราดรภาพ",
        "thai"));
DEF_GM(return new FontationsFtCompareGM(
        "fonts/noto/NotoSansMalayalam-Regular.ttf",
        "മനുഷ്യരെല്ലാവരും തുല്യാവകാശങ്ങളോടും അന്തസ്സോടും സ്വാതന്ത്ര്യത്തോടുംകൂടി ജനിച്ചിട്ടുള്ളവരാണ്‌. "
        "അന്യോന്യം "
        "ഭ്രാതൃഭാവത്തോടെ പെരുമാറുവാനാണ്‌ മനുഷ്യന്നു വിവേകബുദ്ധിയും "
        "മനസ്സാക്ഷിയും "
        "സിദ്ധമായിരിക്കുന്നത്‌.",
        "malay"));
DEF_GM(return new FontationsFtCompareGM(
        "fonts/noto/NotoSans-Regular.ttf",
        "Όλοι οι άνθρωποι γεννιούνται ελεύθεροι και ίσοι στην αξιοπρέπεια και τα δικαιώματα. "
        "Είναι προικισμένοι με λογική και συνείδηση, και οφείλουν να συμπεριφέρονται μεταξύ "
        "τους με πνεύμα αδελφοσύνης.",
        "greek"));
DEF_GM(return new FontationsFtCompareGM(
        "fonts/noto/NotoSans-Regular.ttf",
        "Всі люди народжуються вільними і рівними у своїй гідності та правах. Вони "
        "наділені розумом і совістю і повинні діяти у відношенні один до одного в дусі "
        "братерства.",
        "ukrainian"));
DEF_GM(return new FontationsFtCompareGM(
        "fonts/noto/NotoSansDevanagari-Regular.ttf",
        "सभी मनुष्यों को गौरव और अधिकारों के मामले में जन्मजात स्वतन्त्रता और समानता प्राप्त है । उन्हें बुद्धि और "
        "अन्तरात्मा की देन प्राप्त है और परस्पर उन्हें भाईचारे के भाव से बर्ताव करना चाहिए ।",
        "hindi"));
DEF_GM(return new FontationsFtCompareGM(
        "fonts/noto/NotoSansJP-Regular.ttf",
        "第１条 "
        "すべての人間は、生まれながらにして自由であり、かつ、尊厳と権利とについて平等である。人間は"
        "、理性と良心とを授けられており、互いに同胞の精神をもって行動しなければ"
        "ならない。",
        "japanese"));
DEF_GM(return new FontationsFtCompareGM("fonts/noto/NotoSans-Regular.ttf",
                                        "Whereas recognition of the inherent dignity",
                                        "english"));

}  // namespace skiagm
