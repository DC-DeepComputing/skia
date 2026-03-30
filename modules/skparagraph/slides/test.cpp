#include <include/core/SkFontMgr.h>
#include <include/ports/SkFontMgr_fontconfig.h>
#include <include/ports/SkFontScanner_FreeType.h>
#include <modules/skparagraph/include/ParagraphBuilder.h>
#include <modules/skparagraph/include/FontCollection.h>

using namespace skia::textlayout;

int main() {
    sk_sp<SkFontMgr> fontMgr = SkFontMgr_New_FontConfig((_FcConfig*)nullptr, SkFontScanner_Make_FreeType());

    sk_sp<FontCollection> collection = sk_make_sp<FontCollection>();

    ParagraphStyle paragraphStyle;
    collection->enableFontFallback();
    return 0;
}


class ParagraphSlidePlaceholders : public ParagraphSlide_Base {
public:
    ParagraphSlidePlaceholders() { fName = "ParagraphSlidePlaceholders"; }
    void draw(SkCanvas* canvas) override {
        canvas->drawColor(SK_ColorWHITE);
        auto fontCollection = sk_make_sp<FontCollection>();
        fontCollection->setDefaultFontManager(ToolUtils::TestFontMgr(), std::vector<SkString>());
        fontCollection->enableFontFallback();

        ParagraphStyle paragraphStyle;
        TextStyle defaultTextStyle;
        defaultTextStyle.setFontFamilies({SkString("Roboto")});
        defaultTextStyle.setFontSize(50);
        defaultTextStyle.setColor(SkColor(0xFF000000));
        paragraphStyle.setTextStyle(defaultTextStyle);

        TextStyle textStyle1;
        textStyle1.setFontFamilies({SkString("Roboto")});
        textStyle1.setFontSize(20);
        textStyle1.setColor(SkColor(0xFF000000));

        TextStyle textStyle2;
        textStyle2.setFontFamilies({SkString("Roboto")});
        textStyle2.setFontSize(20);


        ParagraphBuilderImpl builder(paragraphStyle, fontCollection, get_unicode());

        builder.pushStyle(textStyle1);
        builder.addText("Alphabetic 20 on baseline:");
        builder.pop();

        builder.pushStyle(textStyle2);
        PlaceholderStyle placeholderStyle1;
        placeholderStyle1.fWidth = 20;
        placeholderStyle1.fHeight = 20;
        placeholderStyle1.fAlignment = PlaceholderAlignment::kBaseline;
        placeholderStyle1.fBaselineOffset = 10.0;
        placeholderStyle1.fBaseline = TextBaseline::kAlphabetic;
        builder.addPlaceholder(placeholderStyle1);
        builder.pop();

        builder.pushStyle(textStyle1);
        builder.addText("\nAlphabetic 20 above baseline:");
        builder.pop();

        builder.pushStyle(textStyle2);
        PlaceholderStyle placeholderStyle2;
        placeholderStyle2.fWidth = 20;
        placeholderStyle2.fHeight = 20;
        placeholderStyle2.fAlignment = PlaceholderAlignment::kAboveBaseline;
        placeholderStyle2.fBaselineOffset = 0.0;
        placeholderStyle2.fBaseline = TextBaseline::kAlphabetic;
        builder.addPlaceholder(placeholderStyle2);
        builder.pop();

        builder.pushStyle(textStyle1);
        builder.addText("\nAlphabetic 20 below baseline:");
        builder.pop();

        builder.pushStyle(textStyle2);
        PlaceholderStyle placeholderStyle3;
        placeholderStyle3.fWidth = 20;
        placeholderStyle3.fHeight = 20;
        placeholderStyle3.fAlignment = PlaceholderAlignment::kBelowBaseline;
        placeholderStyle3.fBaselineOffset = 0.0;
        placeholderStyle3.fBaseline = TextBaseline::kAlphabetic;
        builder.addPlaceholder(placeholderStyle3);
        builder.pop();

        builder.pushStyle(textStyle1);
        builder.addText("\nAlphabetic 20 middle:");
        builder.pop();

        builder.pushStyle(textStyle2);
        PlaceholderStyle placeholderStyle4;
        placeholderStyle4.fWidth = 20;
        placeholderStyle4.fHeight = 20;
        placeholderStyle4.fAlignment = PlaceholderAlignment::kMiddle;
        placeholderStyle4.fBaselineOffset = 0.0;
        placeholderStyle4.fBaseline = TextBaseline::kAlphabetic;
        builder.addPlaceholder(placeholderStyle4);
        builder.pop();

        builder.pushStyle(textStyle1);
        builder.addText("\nAlphabetic 20 top:");
        builder.pop();

        builder.pushStyle(textStyle2);
        PlaceholderStyle placeholderStyle5;
        placeholderStyle5.fWidth = 20;
        placeholderStyle5.fHeight = 20;
        placeholderStyle5.fAlignment = PlaceholderAlignment::kTop;
        placeholderStyle5.fBaselineOffset = 0.0;
        placeholderStyle5.fBaseline = TextBaseline::kAlphabetic;
        builder.addPlaceholder(placeholderStyle5);
        builder.pop();

        builder.pushStyle(textStyle1);
        builder.addText("\nAlphabetic 20 bottom:");
        builder.pop();

        builder.pushStyle(textStyle2);
        PlaceholderStyle placeholderStyle6;
        placeholderStyle6.fWidth = 20;
        placeholderStyle6.fHeight = 20;
        placeholderStyle6.fAlignment = PlaceholderAlignment::kBottom;
        placeholderStyle6.fBaselineOffset = 0.0;
        placeholderStyle6.fBaseline = TextBaseline::kAlphabetic;
        builder.addPlaceholder(placeholderStyle6);
        builder.pop();

        auto p = builder.Build();
        auto paragraph = static_cast<ParagraphImpl*>(p.get());

        paragraph->layout(500);
        paragraph->paint(canvas, 0, 0);

        auto rects = paragraph->getRectsForPlaceholders();
        paragraph->getRectsForRange(0, paragraph->text().size(), RectHeightStyle::kTight, RectWidthStyle::kTight);

        SkPaint bluePaint;
        bluePaint.setColor(SK_ColorBLUE);
        bluePaint.setStyle(SkPaint::kStroke_Style);
        bluePaint.setAntiAlias(true);
        bluePaint.setStrokeWidth(1);
        for (auto& rect : rects) {
            canvas->drawRect(rect.rect, bluePaint);
        }
    }
};

