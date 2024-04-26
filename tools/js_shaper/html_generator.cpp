/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include <TestFontCollection.h>
#include "modules/skunicode/include/SkUnicode.h"
#include "modules/skunicode/include/SkUnicode_icu.h"
#include "modules/skparagraph/include/FontCollection.h"
#include "modules/skparagraph/include/ParagraphStyle.h"
#include "modules/skparagraph/include/ParagraphBuilder.h"
#include "modules/skparagraph/include/Paragraph.h"
#include "modules/skparagraph/src/ParagraphImpl.h"
#include "modules/skparagraph/src/TextLine.h"
#include "tools/fonts/FontToolUtils.h"
#include "tools/Resources.h"
#include "include/core/SkString.h"
#include "include/core/SkStream.h"
#include "src/utils/SkOSPath.h"

using namespace skia::textlayout;

SkString generateRow(const char* unitType, const char* classes, int index1, int index2, const char* text, SkRect rect) {
        SkString tr;
        tr.appendf("<tr classes='%s'>", classes);
        if (index2 >= 0) {
            tr.appendf("<th scope='row'>%s[%d/%d]</th>", unitType, index1, index2);
        } else {
            tr.appendf("<th scope='row'>%s[%d]</th>", unitType, index1);
        }
        tr.appendf("<td>%s</td>", text);
        tr.appendf("<td>%4.2f</td>", rect.left());
        tr.appendf("<td>%4.2f</td>", rect.top());
        tr.appendf("<td>%4.2f</td>", rect.right());
        tr.appendf("<td>%4.2f</td>", rect.bottom());
        tr.append("</tr>");
        return tr;
}

int  main()
{
    auto fontCollection = sk_make_sp<TestFontCollection>("../../resources/fonts", true, true);
    sk_sp<SkFontMgr> fontManager = ToolUtils::TestFontMgr();
    fontCollection->setDefaultFontManager(fontManager);
    fontCollection->disableFontFallback();

    constexpr int width = 200;

    SkPaint textPaint;
    textPaint.setColor(SK_ColorBLACK);

    TextStyle style;
    style.setForegroundColor(textPaint);
    style.setFontFamilies({SkString("Roboto")});
    style.setFontSize(20);

    ParagraphStyle paraStyle;
    paraStyle.setTextStyle(style);
    paraStyle.setTextAlign(TextAlign::kLeft);

    sk_sp<SkUnicode> unicode = SkUnicodes::ICU::Make();
    if (!unicode) {
      printf("Could not load unicode data\n");
      return 1;
    }

    std::unique_ptr<ParagraphBuilder> builder =
        ParagraphBuilder::make(paraStyle, fontCollection, unicode);
    builder->addText("World domination is such an ugly phrase - I prefer to call it world optimisation");

    std::unique_ptr<Paragraph> paragraph = builder->Build();
    paragraph->layout(width - 20);
    auto impl = static_cast<ParagraphImpl*>(paragraph.get());

    SkFILEWStream textFile("generated/text.html");
    textFile.writeText(impl->text().data());

    SkFILEWStream htmlFile("generated/skia.html");
    htmlFile.writeText("<tbody id='tbody'>");
    auto l = 0;
    for (auto& line : impl->lines()) {
        const auto rect = SkRect::MakeLTRB(line.offset().fX, line.offset().fY, line.width(), line.height());
        htmlFile.writeText(generateRow("line", "line", l, -1, "",rect).data());
        auto r = 0;
        line.iterateThroughVisualRuns(true,
            [&](const Run* run, SkScalar runOffsetInLine, TextRange textRange, SkScalar* runWidthInLine) {
                *runWidthInLine = line.iterateThroughSingleRunByStyles(
                TextLine::TextAdjustment::GlyphCluster, run, runOffsetInLine, textRange, StyleType::kBackground,
                [&](TextRange textRange, const TextStyle& style, const TextLine::ClipContext& context) {
                    // Line broken glyph runs - exactly what we need
                    SkString fontName;
                    context.run->font().getTypeface()->getFamilyName(&fontName);
                    SkString font;
                    font.appendf(" %4.0fpx %s", context.run->font().getSize(), fontName.c_str());
                    htmlFile.writeText(generateRow("run", "run", r, -1, font.c_str(), context.clip).data());
                    int start = context.pos;
                    for (auto g = context.pos; g <= context.size; ++g) {
                        const auto clusterEnd =  context.run->clusterIndex(g);
                        const auto clusterStart = context.run->clusterIndex(start);
                        if (clusterStart != clusterEnd) {
                            SkString text(impl->text().data() + clusterStart, clusterEnd - clusterStart);
                            auto clipBox = line.measureTextInsideOneRun(TextRange(
                                                                            clusterStart,
                                                                            clusterEnd),
                                                                        context.run,
                                                                        runOffsetInLine,
                                                                        0,
                                                                        true,
                                                                        TextLine::TextAdjustment::GlyphCluster);
                            clipBox.clip.offset(-clipBox.fTextShift, 0);
                            if (text.equals(SkString(" ", clusterEnd - clusterStart))) {
                                // Whitespaces
                                htmlFile.writeText(generateRow("whitespaces", "whitespaces", g - 1, -1, "", clipBox.clip).data());
                            } else {
                                htmlFile.writeText(generateRow("grapheme", "grapheme", g - 1, -1, text.c_str(), clipBox.clip).data());
                            }

                            start = g;
                        }
                    }
                    r += 1;
                });
            return true;
            });

        l += 1;
    }
    htmlFile.writeText("</tbody>");
}