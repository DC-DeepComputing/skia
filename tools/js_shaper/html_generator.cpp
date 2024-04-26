/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "include/core/SkString.h"
#include "include/core/SkStream.h"
#include <TestFontCollection.h>
#include "modules/skunicode/include/SkUnicode.h"
#include "modules/skunicode/include/SkUnicode_icu.h"
#include "modules/skparagraph/include/ParagraphStyle.h"
#include "modules/skparagraph/include/ParagraphBuilder.h"
#include "modules/skparagraph/include/Paragraph.h"
#include "modules/skparagraph/src/ParagraphImpl.h"
#include "modules/skparagraph/src/TextLine.h"
#include "tools/fonts/FontToolUtils.h"

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
        tr.append("</tr>\n");
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
      auto r = 0;
      auto w = 0;
      auto g = 0;

      std::vector<size_t> words;
      impl->getUnicode()->getWords(impl->text().data(), impl->text().size(), nullptr, &words);

      auto properties = impl->getProperties();
      for (auto word : words) {
          properties[word] = properties[word] | SkUnicode::CodeUnitFlags::kWordBreak;
      }

      ClusterRange currentRun;
      TextDirection currentTextDirection = paraStyle.getTextDirection();
      ClusterRange currentWord;
      TextIndex currentClusterIndex = EMPTY_INDEX;
      ClusterRange currentGraphemeCluster;
      for (auto& line : impl->lines()) {

          line.iterateThroughClustersInGlyphsOrder(false, true,
              [&](const Cluster* cluster, ClusterIndex index, bool ghost) {

              // Detect a new grapheme cluster
              bool startNewGrapheme = (properties[cluster->textRange().start] & SkUnicode::CodeUnitFlags::kGraphemeStart) == SkUnicode::CodeUnitFlags::kGraphemeStart;
              bool startNewWord = (properties[cluster->textRange().start] & SkUnicode::CodeUnitFlags::kWordBreak) == SkUnicode::CodeUnitFlags::kWordBreak;
              bool endOfLine = cluster->textRange().end == line.text().end;
              bool printWord = true;
              if (startNewGrapheme) {
                  // 1. Close the previous grapheme cluster
                  if (currentGraphemeCluster.width() > 0) {
                      auto textRange = impl->textRange(currentGraphemeCluster);
                      SkString text(impl->text(textRange).data(), textRange.width());
                      std::vector<TextBox> boxes;
                      line.getRectsForRange(impl->textRange(currentGraphemeCluster),
                                            RectHeightStyle::kTight,
                                            RectWidthStyle::kTight,
                                            boxes);
                      // This grapheme could also be a word or a whitespaces
                      auto singleCluster =  currentGraphemeCluster.width() == 1 ? &impl->cluster(currentGraphemeCluster.start) : nullptr;
                      SkString type("grapheme");
                      SkString classes("grapheme");
                      int secondIndex = -1;
                      if (singleCluster != nullptr && singleCluster->isIntraWordBreak()) {
                          classes.append(" word");
                          type.append("/word");
                          secondIndex = w;
                          w += 1;
                          printWord = false;
                      }
                      if (singleCluster != nullptr && singleCluster->isWhitespaceBreak()) {
                          classes.append(" whitespaces");
                          type = "whitespaces";
                          if (!singleCluster->isIntraWordBreak() || currentWord.width() != 1) {
                              secondIndex = w;
                              w += 1;
                          }
                          printWord = false;
                      }
                      // In theory the run could be covered by multiple rectangles of different font features
                      SkASSERT(!boxes.empty());
                      SkRect runGrapheme(boxes[0].rect);
                      for (auto& box : boxes) {
                          runGrapheme.joinPossiblyEmptyRect(box.rect);
                      }
                      htmlFile.writeText(generateRow(type.c_str(), classes.c_str(), g, secondIndex, text.c_str(), runGrapheme).data());
                      g += 1;
                  }

                  // Detect a new line
                  if (cluster->isHardBreak()) {
                      htmlFile.writeText(generateRow("newline", "newline", g, -1, "", SkRect::MakeXYWH(0,0,0,0)).data());
                      return false;
                  }

                  // 2. Create a new (empty) grapheme cluster
                  currentGraphemeCluster.start = index;
                  currentClusterIndex = index;
              }
              // Extend the grapheme cluster
              currentGraphemeCluster.end = index + 1;

              // Detect the word break
              if (cluster->isIntraWordBreak()) {
                  // 1. Close the previous word
                  if (printWord) {
                      SkString type("word");
                      SkString classes("word");
                      auto textRange = impl->textRange(currentWord);
                      SkString text(impl->text(textRange).data(), textRange.width());
                      std::vector<TextBox> boxes;
                      line.getRectsForRange(impl->textRange(currentWord),
                                            RectHeightStyle::kTight,
                                            RectWidthStyle::kTight,
                                            boxes);
                      SkASSERT(!boxes.empty());
                      SkRect wordRect(boxes[0].rect);
                      for (auto& box : boxes) {
                          wordRect.joinPossiblyEmptyRect(box.rect);
                      }
                      if (impl->isWhitespaceRange(currentWord)) {
                          classes.append(" whitespaces");
                          type = "whitespaces";
                      }
                      htmlFile.writeText(generateRow(type.c_str(), classes.c_str(), w, -1, text.c_str(), wordRect).data());
                      w += 1;
                  }

                  // 2. Start the new (empty) word
                  currentWord.start = index;
              }
              // Extend the word
              currentWord.end = index + 1;

              if (cluster->run().getTextDirection() != currentTextDirection) {
                  // 1. Close the previous run
                  if (currentRun.width() > 0) {
                      SkString text(currentTextDirection == TextDirection::kLtr ? "Left to Right" : "Right to Left");
                      std::vector<TextBox> boxes;
                      line.getRectsForRange(impl->textRange(currentRun),
                                            RectHeightStyle::kTight,
                                            RectWidthStyle::kTight,
                                            boxes);
                      // In theory the run could be covered by multiple rectangles of different font features
                      SkASSERT(!boxes.empty());
                      SkRect runRect(boxes[0].rect);
                      for (auto& box : boxes) {
                          runRect.joinPossiblyEmptyRect(box.rect);
                      }
                      htmlFile.writeText(generateRow("run", "run", r, -1, text.c_str(), runRect).data());
                      r += 1;
                  }

                  // 2. Start the new (empty) run
                  currentRun.start = index;
                  currentTextDirection = cluster->run().getTextDirection();
              }
              // Extend the run
              currentRun.end = index + 1;

              return true;
          });

          // 1. Close the previous line
          if (!line.isFirstLine() || line.isLastLine()) {
              const auto rect = SkRect::MakeXYWH(line.offset().fX, line.offset().fY, line.width(), line.height());
              htmlFile.writeText(generateRow("line", "line", l, -1, "", rect).data());
              l += 1;
          }
      }
    htmlFile.writeText("</tbody>");
}