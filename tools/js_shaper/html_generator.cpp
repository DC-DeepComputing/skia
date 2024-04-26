/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "include/core/SkString.h"
#include "include/core/SkStream.h"
#include "include/private/base/SkTemplates.h"
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
#include "src/utils/SkOSPath.h"

using namespace skia::textlayout;

SkString generateRow(const char* unitType, const char* classes, int index1, int index2, const char* text, SkRect rect) {
        SkString tr;
        tr.appendf("<tr class='%s'>", classes);
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
    auto fontCollection = sk_make_sp<TestFontCollection>("../../resources/fonts/noto_sans", true, true);

    constexpr int width = 800;

    SkPaint textPaint;
    textPaint.setColor(SK_ColorBLACK);

    TextStyle style;
    style.setForegroundColor(textPaint);
  style.setFontFamilies({SkString("Noto Sans"),
                         SkString("Noto Sans Emoji"),
                         SkString("Noto Sans Arabic"),
                         SkString("Noto Sans Hebrew"),
                         SkString("Noto Sans Bengali"),});
    style.setFontSize(20);

    ParagraphStyle paraStyle;
    paraStyle.setTextStyle(style);
    paraStyle.setTextAlign(TextAlign::kLeft);

    sk_sp<SkUnicode> unicode = SkUnicodes::ICU::Make();
    if (!unicode) {
      printf("Could not load unicode data\n");
      return 1;
    }

    SkFILEStream textFile("generated/text.html");
    int len = textFile.skip(10000);
    if (!textFile.isAtEnd()) {
        SkDebugf("Text is too long (< 1000)\n");
        return -1;
    }

    SkString buffer(len);
    textFile.rewind();
    textFile.read(buffer.data(), len);
    while (true) {
        int pos = buffer.find("<br/>");
        if (pos < 0) {
            break;
        }
        buffer.remove(pos, 5);
    }

    std::unique_ptr<ParagraphBuilder> builder =
        ParagraphBuilder::make(paraStyle, fontCollection, unicode);
    builder->addText(buffer.data());

    std::unique_ptr<Paragraph> paragraph = builder->Build();
    paragraph->layout(width);
    auto impl = static_cast<ParagraphImpl*>(paragraph.get());

    SkFILEWStream htmlFile("generated/skia.html");
    htmlFile.writeText("<tbody id='tbody'>");
    auto l = 0;
    auto glyphIndex = 0;
    for (auto& line : impl->lines()) {
        const auto rect = SkRect::MakeXYWH(line.offset().fX, line.offset().fY, line.width(), line.height());
        htmlFile.writeText(generateRow("line", "line", l, -1, "",rect).data());
        auto r = 0;
        line.iterateThroughVisualRuns(true,
            [&](const Run* run, SkScalar runOffsetInLine, TextRange textRange, SkScalar* runWidthInLine) {
                *runWidthInLine = line.iterateThroughSingleRunByStyles(
                TextLine::TextAdjustment::GlyphCluster, run, runOffsetInLine, textRange, StyleType::kBackground,
                [&](TextRange textRange, const TextStyle& style, const TextLine::ClipContext& lineContext) {
                    // Line broken glyph runs - exactly what we need
                    SkString fontName;
                    lineContext.run->font().getTypeface()->getFamilyName(&fontName);
                    SkString font;
                    font.appendf(" %4.0fpx %s", lineContext.run->font().getSize(), fontName.c_str());
                    htmlFile.writeText(generateRow("run", "run", r, -1, font.c_str(), lineContext.clip).data());
                    int start = lineContext.pos;
                    for (auto g = lineContext.pos; g <= lineContext.pos + lineContext.size; ++g) {
                        auto clusterEnd =  lineContext.run->globalClusterIndex(g);
                        auto clusterStart = lineContext.run->globalClusterIndex(start);
                        auto graphemeEdge = impl->codeUnitHasProperty(clusterEnd, SkUnicode::CodeUnitFlags::kGraphemeStart);
                        if (graphemeEdge && (clusterStart != clusterEnd)) {
                            if (clusterStart > clusterEnd) {
                                std::swap(clusterStart, clusterEnd);
                            }
                            SkString text(impl->text().data() + clusterStart, clusterEnd - clusterStart);
                            auto context = line.measureTextInsideOneRun(TextRange( clusterStart, clusterEnd),
                                                                        lineContext.run,
                                                                        runOffsetInLine,
                                                                        0,
                                                                        true,
                                                                        TextLine::TextAdjustment::GlyphCluster);
                            context.clip.offset(lineContext.fTextShift - context.fTextShift, 0);
                            if (text.equals(SkString(" ", clusterEnd - clusterStart))) {
                                // Whitespaces
                                htmlFile.writeText(generateRow("whitespaces", "whitespaces word grapheme", glyphIndex, -1, "", context.clip).data());
                            } else {
                                htmlFile.writeText(generateRow("grapheme", "grapheme", glyphIndex, -1, text.c_str(), context.clip).data());
                            }

                            start = g;
                            glyphIndex += 1;
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