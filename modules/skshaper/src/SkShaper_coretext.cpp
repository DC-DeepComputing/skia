/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "modules/skshaper/include/SkShaper.h"
#include "modules/skshaper/include/SkShaper_coretext.h"
#include "modules/skshaper/include/SkShaper_skunicode.h"
#include "modules/skunicode/include/SkUnicode.h"

#ifdef SK_BUILD_FOR_MAC
#import <ApplicationServices/ApplicationServices.h>
#endif

#ifdef SK_BUILD_FOR_IOS
#include <CoreText/CoreText.h>
#include <CoreText/CTFontManager.h>
#include <CoreGraphics/CoreGraphics.h>
#include <CoreFoundation/CoreFoundation.h>
#endif

#include "include/core/SkFont.h"
#include "include/core/SkFontStyle.h"
#include "include/core/SkFontMgr.h"
#include "include/core/SkTypeface.h"
#include "include/ports/SkTypeface_mac.h"
#include "include/private/SkTFitsIn.h"
#include "include/private/SkTemplates.h"
#include "include/private/SkTo.h"
#include "src/core/SkFontPriv.h"
#include "src/core/SkUTF.h"
#include "src/utils/mac/SkCGBase.h"
#include "src/utils/mac/SkUniqueCFRef.h"

#include <algorithm>
#include <memory>
#include <unordered_map>
#include <utility>
#include <vector>

using namespace skia_private;

class TypefaceCache {
public:
    sk_sp<SkTypeface> getOrCreate(CTFontRef ctFont) {
        if (!ctFont) {
            return nullptr;
        }
        auto it = fCache.find(ctFont);
        if (it != fCache.end()) {
            return it->second;
        }
        sk_sp<SkTypeface> tf = SkMakeTypefaceFromCTFont(ctFont);
        fCache[ctFont] = tf;
        return tf;
    }

private:
    std::unordered_map<CTFontRef, sk_sp<SkTypeface>> fCache;
};

class SkShaper_CoreText : public SkShaper {
public:
    explicit SkShaper_CoreText(SkShapers::CT::LineBreakMode lbm) : fLineBreakMode(lbm) {}

private:
#if !defined(SK_DISABLE_LEGACY_SKSHAPER_FUNCTIONS)
    void shape(const char* utf8, size_t utf8Bytes,
               const SkFont& srcFont,
               bool leftToRight,
               SkScalar width,
               RunHandler*) const override;

    void shape(const char* utf8, size_t utf8Bytes,
               FontRunIterator&,
               BiDiRunIterator&,
               ScriptRunIterator&,
               LanguageRunIterator&,
               SkScalar width,
               RunHandler*) const override;
#endif

    void shape(const char* utf8, size_t utf8Bytes,
               FontRunIterator&,
               BiDiRunIterator&,
               ScriptRunIterator&,
               LanguageRunIterator&,
               const Feature*, size_t featureSize,
               SkScalar width,
               RunHandler*) const override;

    void shape(const char* utf8, size_t utf8Bytes,
               FontRunIterator&,
               BiDiRunIterator&,
               ScriptRunIterator&,
               LanguageRunIterator&,
               const Feature*, size_t featureSize,
               SkScalar width,
               float textTracking,
               RunHandler*) const override;

    const SkShapers::CT::LineBreakMode fLineBreakMode;
};

// CTFramesetter/CTFrame can do this, but require version 10.14
class LineBreakIter {
    const CTTypesetterRef fTypesetter;
    const double          fWidth;
    CFIndex               fStart = 0;
    const SkUniqueCFRef<CFStringTokenizerRef> fWordTokenizer;

    static SkUniqueCFRef<CFStringTokenizerRef> MakeWordTokenizer(CFStringRef txt,
                                                                 SkShapers::CT::LineBreakMode lbm) {
        if (lbm != SkShapers::CT::LineBreakMode::kStrict) {
            return nullptr;
        }

        SkUniqueCFRef<CFLocaleRef> locale(CFLocaleCopyCurrent());

        CFRange full_range = CFRangeMake(0, CFStringGetLength(txt));
        SkUniqueCFRef<CFStringTokenizerRef> tokenizer(
            CFStringTokenizerCreate(kCFAllocatorDefault, txt,
                                    full_range, kCFStringTokenizerUnitWord, locale.get()));

        // Advance to the first word.
        CFStringTokenizerAdvanceToNextToken(tokenizer.get());

        return tokenizer;
    }

    // If the line range happens to break a word, return the range padding needed to include
    // the full word. This is only done for LineBreakMode::kStrict.
    CFIndex computeWordBreakPadding(const CFRange line_range) {
        if (!fWordTokenizer) {
            return 0;
        }

        CFRange word_range = CFStringTokenizerGetCurrentTokenRange(fWordTokenizer.get());
        if (word_range.location == kCFNotFound) {
            // No words left.
            return 0;
        }

        // Consume words until past current line.
        const CFIndex line_end = line_range.location + line_range.length;
        while (word_range.location < line_end) {
            // Do we have a word break at the end of the current line?
            const CFIndex word_end = word_range.location + word_range.length;
            if (word_range.location < line_end && line_end < word_end) {
                // We should never see a word break for a word starting on the previous line.
                SkASSERT(word_range.location >= line_range.location);

                // Pad the current line with the rest of this word.
                return word_end - line_end;
            }

            if (CFStringTokenizerAdvanceToNextToken(fWordTokenizer.get()) ==
                kCFStringTokenizerTokenNone) {
                // all words consumed, this is the end of the string
                break;
            }
            word_range = CFStringTokenizerGetCurrentTokenRange(fWordTokenizer.get());
        }

        // No word breaks -> no padding.
        return 0;
    }

public:
    LineBreakIter(CFStringRef txt, CTTypesetterRef ts, SkScalar width,
                  SkShapers::CT::LineBreakMode lbm)
        : fTypesetter(ts), fWidth(width), fWordTokenizer(MakeWordTokenizer(txt, lbm)) {}

    SkUniqueCFRef<CTLineRef> nextLine() {
        CFRange line_range {fStart, CTTypesetterSuggestLineBreak(fTypesetter, fStart, fWidth)};
        // Pad to avoid word breaks if needed.
        line_range.length += computeWordBreakPadding(line_range);

        if (!line_range.length) {
            return nullptr;
        }

        fStart += line_range.length;

        return SkUniqueCFRef<CTLineRef>(CTTypesetterCreateLine(fTypesetter, line_range));
    }
};

[[maybe_unused]] static void dict_add_double(CFMutableDictionaryRef d, const void* name, double value) {
    SkUniqueCFRef<CFNumberRef> number(
            CFNumberCreate(kCFAllocatorDefault, kCFNumberDoubleType, &value));
    CFDictionaryAddValue(d, name, number.get());
}

static SkUniqueCFRef<CTFontRef> create_ctfont_from_font(const SkFont& font) {
    auto typeface = font.getTypeface();
    auto ctfont = SkTypeface_GetCTFontRef(typeface);
    if (!ctfont) {
        return SkUniqueCFRef<CTFontRef>(
                CTFontCreateUIFontForLanguage(kCTFontUIFontSystem, font.getSize(), nullptr));
    }
    return SkUniqueCFRef<CTFontRef>(
            CTFontCreateCopyWithAttributes(ctfont, font.getSize(), nullptr, nullptr));
}

static SkFont run_to_font(CTRunRef run, const SkFont& orig, TypefaceCache& cache) {
    CFDictionaryRef attr = CTRunGetAttributes(run);
    CTFontRef ct = (CTFontRef)CFDictionaryGetValue(attr, kCTFontAttributeName);
    if (!ct) {
        return orig;
    }
    SkFont font(orig);
    sk_sp<SkTypeface> tf = cache.getOrCreate(ct);
    if (tf) {
        font.setTypeface(std::move(tf));
    }

    // Detect synthetic bold / italic matrix applied by CoreText
    CGAffineTransform matrix = CTFontGetMatrix(ct);
    if (matrix.c != 0) {
        font.setSkewX(-matrix.c);
    }
    CTFontSymbolicTraits traits = CTFontGetSymbolicTraits(ct);
    if ((traits & kCTFontTraitBold) && font.getTypeface() && font.getTypeface()->fontStyle().weight() < SkFontStyle::kBold_Weight) {
        font.setEmbolden(true);
    }

    return font;
}

namespace {
class UTF16ToUTF8IndicesMap {
public:
    /** Builds a UTF-16 to UTF-8 indices map; the text is not retained
     * @return true if successful
     */
    bool setUTF8(const char* utf8, size_t size) {
        SkASSERT(utf8 != nullptr);

        if (!SkTFitsIn<int32_t>(size)) {
            SkDEBUGF("UTF16ToUTF8IndicesMap: text too long");
            return false;
        }

        auto utf16Size = SkUTF::UTF8ToUTF16(nullptr, 0, utf8, size);
        if (utf16Size < 0) {
            SkDEBUGF("UTF16ToUTF8IndicesMap: Invalid utf8 input");
            return false;
        }

        // utf16Size+1 to also store the size
        fUtf16ToUtf8Indices = std::vector<size_t>(utf16Size + 1);
        auto utf16 = fUtf16ToUtf8Indices.begin();
        auto utf8Begin = utf8, utf8End = utf8 + size;
        while (utf8Begin < utf8End) {
            *utf16 = utf8Begin - utf8;
            utf16 += SkUTF::ToUTF16(SkUTF::NextUTF8(&utf8Begin, utf8End), nullptr);
        }
        *utf16 = size;

        return true;
    }

    size_t mapIndex(size_t index) const {
        SkASSERT(index < fUtf16ToUtf8Indices.size());
        return fUtf16ToUtf8Indices[index];
    }

    std::pair<size_t, size_t> mapRange(size_t start, size_t size) const {
        auto utf8Start = mapIndex(start);
        return {utf8Start, mapIndex(start + size) - utf8Start};
    }
private:
    std::vector<size_t> fUtf16ToUtf8Indices;
};
} // namespace

// kCTTrackingAttributeName not available until 10.12
const CFStringRef kCTTracking_AttributeName = CFSTR("CTTracking");

#if !defined(SK_DISABLE_LEGACY_SKSHAPER_FUNCTIONS)
void SkShaper_CoreText::shape(const char* utf8,
                              size_t utf8Bytes,
                              FontRunIterator& font,
                              BiDiRunIterator& bidi,
                              ScriptRunIterator& script,
                              LanguageRunIterator& lang,
                              SkScalar width,
                              RunHandler* handler) const {
    return this->shape(utf8, utf8Bytes, font, bidi, script, lang, nullptr, 0, width, handler);
}

void SkShaper_CoreText::shape(const char* utf8,
                              size_t utf8Bytes,
                              const SkFont& font,
                              bool,
                              SkScalar width,
                              RunHandler* handler) const {
    TrivialFontRunIterator fontRuns(font, utf8Bytes);
    TrivialBiDiRunIterator bidi{0, utf8Bytes};
    TrivialScriptRunIterator script{0, utf8Bytes};
    TrivialLanguageRunIterator lang{nullptr, utf8Bytes};
    return this->shape(utf8, utf8Bytes, fontRuns, bidi, script, lang, nullptr, 0, width, handler);
}
#endif

void SkShaper_CoreText::shape(const char* utf8,
                              size_t utf8Bytes,
                              FontRunIterator& fontRuns,
                              BiDiRunIterator& bidi,
                              ScriptRunIterator& script,
                              LanguageRunIterator& lang,
                              const Feature* features, size_t featuresSize,
                              SkScalar width,
                              RunHandler* handler) const {
    return this->shape(utf8, utf8Bytes, fontRuns, bidi, script, lang, features, featuresSize,
                       width, /*textTracking=*/0, handler);
}

void SkShaper_CoreText::shape(const char* utf8,
                              size_t utf8Bytes,
                              FontRunIterator& fontRuns,
                              BiDiRunIterator& bidiRuns,
                              ScriptRunIterator&,
                              LanguageRunIterator&,
                              const Feature*,
                              size_t,
                              SkScalar width,
                              float textTracking,
                              RunHandler* handler) const {
    SkFont font;
    if (!fontRuns.atEnd()) {
        fontRuns.consume();
        font = fontRuns.currentFont();
    }

    if (utf8Bytes == 0) {
        handler->beginLine();
        handler->commitRunInfo();
        handler->commitLine();
        return;
    }

    // Pre-extract BiDi levels per byte
    std::vector<uint8_t> bidiLevels(utf8Bytes, 0);
    size_t bidiPos = 0;
    while (!bidiRuns.atEnd() && bidiPos < utf8Bytes) {
        bidiRuns.consume();
        size_t end = std::min(bidiRuns.endOfCurrentRun(), utf8Bytes);
        uint8_t level = bidiRuns.currentLevel();
        while (bidiPos < end) {
            bidiLevels[bidiPos++] = level;
        }
    }

    SkUniqueCFRef<CFStringRef> textString(
            CFStringCreateWithBytes(kCFAllocatorDefault, (const uint8_t*)utf8, utf8Bytes,
                                    kCFStringEncodingUTF8, false));
    if (!textString) {
        return;
    }

    UTF16ToUTF8IndicesMap utf8IndicesMap;
    if (!utf8IndicesMap.setUTF8(utf8, utf8Bytes)) {
        return;
    }

    SkUniqueCFRef<CTFontRef> ctfont = create_ctfont_from_font(font);
    if (!ctfont) {
        return;
    }

    SkUniqueCFRef<CFMutableDictionaryRef> attr(
            CFDictionaryCreateMutable(kCFAllocatorDefault, 0,
                                      &kCFTypeDictionaryKeyCallBacks,
                                      &kCFTypeDictionaryValueCallBacks));
    CFDictionaryAddValue(attr.get(), kCTFontAttributeName, ctfont.get());
    if (textTracking != 0) {
        dict_add_double(attr.get(), kCTTracking_AttributeName, textTracking * font.getSize());
    }

    SkUniqueCFRef<CFAttributedStringRef> attrString(
            CFAttributedStringCreate(kCFAllocatorDefault, textString.get(), attr.get()));
    if (!attrString) {
        return;
    }

    SkUniqueCFRef<CTTypesetterRef> typesetter(
            CTTypesetterCreateWithAttributedString(attrString.get()));
    if (!typesetter) {
        return;
    }

    TypefaceCache typefaceCache;
    std::vector<SkFont> fontStorage;
    std::vector<SkShaper::RunHandler::RunInfo> infos;

    LineBreakIter iter(textString.get(), typesetter.get(), width, fLineBreakMode);
    while (SkUniqueCFRef<CTLineRef> line = iter.nextLine()) {
        CFArrayRef run_array = CTLineGetGlyphRuns(line.get());
        CFIndex runCount = CFArrayGetCount(run_array);
        if (runCount == 0) {
            continue;
        }
        handler->beginLine();
        fontStorage.clear();
        fontStorage.reserve(runCount);
        infos.clear();
        for (CFIndex j = 0; j < runCount; ++j) {
            CTRunRef run = (CTRunRef)CFArrayGetValueAtIndex(run_array, j);
            CFIndex runGlyphs = CTRunGetGlyphCount(run);

            SkASSERT(sizeof(CGGlyph) == sizeof(uint16_t));

            AutoSTArray<4096, CGSize> advances(runGlyphs);
            CTRunGetAdvances(run, {0, runGlyphs}, advances.data());
            SkScalar adv = 0;
            for (CFIndex k = 0; k < runGlyphs; ++k) {
                adv += advances[k].width;
            }

            CFRange cfRange = CTRunGetStringRange(run);
            auto range = utf8IndicesMap.mapRange(cfRange.location, cfRange.length);
            uint8_t runBidiLevel = (range.first < bidiLevels.size()) ? bidiLevels[range.first] : 0;
            CTRunStatus status = CTRunGetStatus(run);
            bool isRTL = (status & kCTRunStatusRightToLeft) != 0;
            if (isRTL && !(runBidiLevel & 1)) {
                runBidiLevel |= 1;
            } else if (!isRTL && (runBidiLevel & 1)) {
                runBidiLevel &= ~1;
            }

            fontStorage.push_back(run_to_font(run, font, typefaceCache));
            infos.push_back(SkShaper::RunHandler::RunInfo{
                fontStorage.back(),
                runBidiLevel,
                0,
                "",
                {adv, 0},
                (size_t)runGlyphs,
                {range.first, range.second},
            });
            handler->runInfo(infos.back());
        }
        handler->commitRunInfo();

        // Fill in the glyph buffers and cluster mappings
        SkScalar lineAdvance = 0;
        for (CFIndex j = 0; j < runCount; ++j) {
            const auto& info = infos[j];
            auto buffer = handler->runBuffer(info);

            CTRunRef run = (CTRunRef)CFArrayGetValueAtIndex(run_array, j);
            CFIndex runGlyphs = info.glyphCount;
            SkASSERT(CTRunGetGlyphCount(run) == (CFIndex)info.glyphCount);

            CTRunGetGlyphs(run, {0, runGlyphs}, buffer.glyphs);

            AutoSTArray<4096, CGPoint> positions(runGlyphs);
            CTRunGetPositions(run, {0, runGlyphs}, positions.data());
            AutoSTArray<4096, CFIndex> indices;
            if (buffer.clusters) {
                indices.reset(runGlyphs);
                CTRunGetStringIndices(run, {0, runGlyphs}, indices.data());
            }

            for (CFIndex k = 0; k < runGlyphs; ++k) {
                buffer.positions[k] = {
                    buffer.point.fX + SkScalarFromCGFloat(positions[k].x) - lineAdvance,
                    buffer.point.fY - SkScalarFromCGFloat(positions[k].y),
                };
                if (buffer.offsets) {
                    buffer.offsets[k] = {0, -SkScalarFromCGFloat(positions[k].y)};
                }
                if (buffer.clusters) {
                    buffer.clusters[k] = utf8IndicesMap.mapIndex(indices[k]);
                }
            }
            handler->commitRunBuffer(info);
            lineAdvance += info.fAdvance.fX;
        }
        handler->commitLine();
    }
}

namespace {
class CoreTextFactory final : public SkShapers::Factory {
public:
    CoreTextFactory(sk_sp<SkUnicode> unicode, SkShapers::CT::LineBreakMode lbm)
        : fUnicode(std::move(unicode)), fLineBreakMode(lbm) {}

    std::unique_ptr<SkShaper> makeShaper(sk_sp<SkFontMgr>) override {
        return SkShapers::CT::CoreText(fLineBreakMode);
    }
    std::unique_ptr<SkShaper::BiDiRunIterator> makeBidiRunIterator(
            const char* utf8, size_t utf8Bytes, uint8_t bidiLevel) override {
        return SkShapers::unicode::BidiRunIterator(fUnicode, utf8, utf8Bytes, bidiLevel);
    }
    std::unique_ptr<SkShaper::ScriptRunIterator> makeScriptRunIterator(
            const char* utf8, size_t utf8Bytes, SkFourByteTag script) override {
        return std::make_unique<SkShaper::TrivialScriptRunIterator>(script, utf8Bytes);
    }

    SkUnicode* getUnicode() override {
        return fUnicode.get();
    }

private:
    sk_sp<SkUnicode> fUnicode;
    SkShapers::CT::LineBreakMode fLineBreakMode;
};
}  // namespace

namespace SkShapers::CT {
std::unique_ptr<SkShaper> CoreText(LineBreakMode lbm) {
    return std::make_unique<SkShaper_CoreText>(lbm);
}

sk_sp<SkShapers::Factory> Factory(sk_sp<SkUnicode> unicode, LineBreakMode lbm) {
    return sk_make_sp<CoreTextFactory>(std::move(unicode), lbm);
}
}  // namespace SkShapers::CT
