/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "modules/skshaper/include/SkShaper.h"
#include "modules/skshaper/include/SkShaper_coretext.h"
#if defined(SK_SHAPER_UNICODE_AVAILABLE)
#include "modules/skshaper/include/SkShaper_skunicode.h"
#include "modules/skunicode/include/SkUnicode.h"
#endif

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
#include "include/core/SkFontMgr.h"
#include "include/core/SkFontStyle.h"
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

// Caches SkTypeface wrappers for CTRun CTFontRef instances within a line
// Avoids expensive font manager lookups and duplicate SkTypeface instantiations across runs
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
#endif  // !defined(SK_DISABLE_LEGACY_SKSHAPER_FUNCTIONS)

    void shape(const char* utf8, size_t utf8Bytes,
               FontRunIterator&,
               BiDiRunIterator&,
               ScriptRunIterator&,
               LanguageRunIterator&,
               const Feature*, size_t featureSize,
               SkScalar width,
               RunHandler*) const override;

    void shape(SkSpan<const char> utf8,
               FontRunIterator&,
               BiDiRunIterator&,
               ScriptRunIterator&,
               LanguageRunIterator&,
               SkSpan<const Feature>,
               const Options&,
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

        // TODO: plumb a locale arg for more accurate results with complex scripts?
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
    // the full word.  This is only done for LineBreakMode::kStrict.
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

// Identifies SkTypeface_SFSymbols virtual typefaces handling Plane 16 PUA codepoints
// These bypass CoreText font cascading because CT cannot resolve synthetic symbol glyphs
static bool is_sf_symbols_typeface(const SkTypeface* tf) {
    if (!tf) {
        return false;
    }
    SkString name;
    tf->getFamilyName(&name);
    return name.equals("SF Symbols");
}

// Matches San Francisco system font names (.AppleSystemUIFont, .SFNS, SF Pro, system-ui)
// Used to detect when Apple HIG dynamic tracking should be applied automatically
static bool is_apple_system_font(const SkTypeface* tf) {
    if (!tf) return true;
    SkString name;
    tf->getFamilyName(&name);
    if (!name.isEmpty()) {
        if (name.startsWith("System Font") || name.startsWith("SF Pro") ||
            name.startsWith("-apple-system") || name.equals("system-ui")) {
            return true;
        }
    }
    tf->getPostScriptName(&name);
    if (!name.isEmpty()) {
        if (name.startsWith(".AppleSystemUIFont") || name.startsWith(".SFNS") ||
            name.startsWith(".SFUI") || name.startsWith("SFPro")) {
            return true;
        }
    }
    return false;
}

// Apple HIG dynamic tracking (letter-spacing) specifications for San Francisco
// CoreText does not apply HIG tracking automatically at the CTTypesetter layer;
// higher-level frameworks (UIKit/AppKit) inject tracking via NSAttributedString
// Without explicit tracking, San Francisco text renders noticeably tighter than native UI
// SF Pro Text (<20pt) and SF Pro Display (>=20pt) use distinct tracking curves
// Values are specified in 1/1000 em and linearly interpolated between size breakpoints
struct AppleHIGTrackingPoint {
    float fontSize;
    float tracking; // in 1/1000 of an em
};

static constexpr AppleHIGTrackingPoint kAppleHIGTrackingText[] = {
    {  6.0f,  41.0f },
    {  7.0f,  34.0f },
    {  8.0f,  26.0f },
    {  9.0f,  19.0f },
    { 10.0f,  12.0f },
    { 11.0f,   6.0f },
    { 12.0f,   0.0f },
    { 13.0f,  -6.0f },
    { 14.0f, -11.0f },
    { 15.0f, -16.0f },
    { 16.0f, -20.0f },
    { 17.0f, -26.0f },
    { 18.0f, -25.0f },
    { 19.0f, -24.0f },
};

static constexpr AppleHIGTrackingPoint kAppleHIGTrackingDisplay[] = {
    { 20.0f,  19.0f },
    { 24.0f,  10.0f },
    { 28.0f,   3.0f },
    { 32.0f,  -2.0f },
    { 36.0f,  -6.0f },
    { 40.0f, -10.0f },
    { 48.0f, -16.0f },
    { 64.0f, -22.0f },
    { 80.0f, -24.0f },
};

template <size_t N>
static float interpolate_tracking_table(const AppleHIGTrackingPoint (&table)[N], float fontSize) {
    if (fontSize <= table[0].fontSize) {
        return table[0].tracking / 1000.0f;
    }
    if (fontSize >= table[N - 1].fontSize) {
        return table[N - 1].tracking / 1000.0f;
    }
    for (size_t i = 0; i < N - 1; ++i) {
        if (fontSize >= table[i].fontSize && fontSize <= table[i + 1].fontSize) {
            float t = (fontSize - table[i].fontSize) / (table[i + 1].fontSize - table[i].fontSize);
            float track = table[i].tracking + t * (table[i + 1].tracking - table[i].tracking);
            return track / 1000.0f;
        }
    }
    return 0.0f;
}

static float get_apple_hig_tracking(float fontSize) {
    if (fontSize < 20.0f) {
        return interpolate_tracking_table(kAppleHIGTrackingText, fontSize);
    }
    return interpolate_tracking_table(kAppleHIGTrackingDisplay, fontSize);
}

// Constructs CTFont matching the requested size, style, and system font cascade
// For system fonts, CTFontCreateUIFontForLanguage preserves native optical sizing
// and automatic fallback to Apple's system font cascade
static SkUniqueCFRef<CTFontRef> create_ctfont_from_font(const SkFont& font) {
    auto typeface = font.getTypeface();
    if (is_apple_system_font(typeface)) {
        CTFontRef sysFont = CTFontCreateUIFontForLanguage(kCTFontUIFontSystem, font.getSize(), nullptr);
        if (typeface) {
            auto ctfont = SkTypeface_GetCTFontRef(typeface);
            if (ctfont) {
                CTFontSymbolicTraits traits = CTFontGetSymbolicTraits(ctfont);
                if (traits & (kCTFontTraitBold | kCTFontTraitItalic)) {
                    CTFontRef styledFont = CTFontCreateCopyWithSymbolicTraits(sysFont, font.getSize(), nullptr, traits, traits);
                    if (styledFont) {
                        CFRelease(sysFont);
                        return SkUniqueCFRef<CTFontRef>(styledFont);
                    }
                }
            }
        }
        return SkUniqueCFRef<CTFontRef>(sysFont);
    }
    auto ctfont = SkTypeface_GetCTFontRef(typeface);
    if (!ctfont) {
        return SkUniqueCFRef<CTFontRef>(
                CTFontCreateUIFontForLanguage(kCTFontUIFontSystem, font.getSize(), nullptr));
    }
    return SkUniqueCFRef<CTFontRef>(
            CTFontCreateCopyWithAttributes(ctfont, font.getSize(), nullptr, nullptr));
}

// Reconstructs SkFont for a CTRun, preserving synthetic bold and oblique styling
// CoreText applies synthetic styling via CGAffineTransform matrix or font traits
// when a matching physical typeface variant is unavailable
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
#endif  // !defined(SK_DISABLE_LEGACY_SKSHAPER_FUNCTIONS)

void SkShaper_CoreText::shape(const char* utf8,
                              size_t utf8Bytes,
                              FontRunIterator& fontRuns,
                              BiDiRunIterator& bidi,
                              ScriptRunIterator& script,
                              LanguageRunIterator& lang,
                              const Feature* features, size_t featuresSize,
                              SkScalar width,
                              RunHandler* handler) const {
    return this->shape({utf8, utf8Bytes}, fontRuns, bidi, script, lang, {features, featuresSize},
                       { .width = width}, handler);
}

void SkShaper_CoreText::shape(SkSpan<const char> utf8,
                              FontRunIterator& fontRuns,
                              BiDiRunIterator& bidiRuns,
                              ScriptRunIterator&,
                              LanguageRunIterator&,
                              SkSpan<const Feature>,
                              const Options& opts,
                              RunHandler* handler) const {
    SkFont font;
    if (!fontRuns.atEnd()) {
        fontRuns.consume();
        font = fontRuns.currentFont();
    }

    if (utf8.size() == 0) {
        handler->beginLine();
        handler->commitRunInfo();
        handler->commitLine();
        return;
    }

    // Pre-extract per-byte BiDi embedding levels from iterator to assign run levels
    std::vector<uint8_t> bidiLevels(utf8.size(), 0);
    size_t bidiPos = 0;
    while (!bidiRuns.atEnd() && bidiPos < utf8.size()) {
        bidiRuns.consume();
        size_t end = std::min(bidiRuns.endOfCurrentRun(), utf8.size());
        uint8_t level = bidiRuns.currentLevel();
        while (bidiPos < end) {
            bidiLevels[bidiPos++] = level;
        }
    }

    if (is_sf_symbols_typeface(font.getTypeface())) {
        // Direct glyph mapping for virtual typefaces (e.g. SkTypeface_SFSymbols)
        // Bypasses CTTypesetter which cannot resolve synthetic non-CTFont symbol glyphs
        handler->beginLine();
        size_t count = SkUTF::CountUTF8(utf8.data(), utf8.size());
        if (count == 0) {
            handler->commitRunInfo();
            handler->commitLine();
            return;
        }
        AutoSTArray<64, SkGlyphID> glyphs(count);
        AutoSTArray<64, SkScalar> widths(count);
        AutoSTArray<64, size_t> clusters(count);

        const char* ptr = utf8.data();
        const char* end = ptr + utf8.size();
        size_t i = 0;
        SkScalar totalAdvance = 0;
        while (ptr < end && i < count) {
            clusters[i] = ptr - utf8.data();
            SkUnichar u = SkUTF::NextUTF8WithReplacement(&ptr, end);
            glyphs[i] = font.unicharToGlyph(u);
            font.getWidths(SkSpan(&glyphs[i], 1), SkSpan(&widths[i], 1));
            totalAdvance += widths[i];
            ++i;
        }

        SkShaper::RunHandler::RunInfo info{
            font,
            0,
            0,
            "",
            {totalAdvance, 0},
            count,
            {0, utf8.size()},
        };
        handler->runInfo(info);
        handler->commitRunInfo();

        auto buffer = handler->runBuffer(info);
        SkScalar curX = 0;
        for (size_t k = 0; k < count; ++k) {
            buffer.glyphs[k] = glyphs[k];
            buffer.positions[k] = {buffer.point.fX + curX, buffer.point.fY};
            if (buffer.offsets) {
                buffer.offsets[k] = {0, 0};
            }
            if (buffer.clusters) {
                buffer.clusters[k] = clusters[k];
            }
            curX += widths[k];
        }
        handler->commitRunBuffer(info);
        handler->commitLine();
        return;
    }

    SkUniqueCFRef<CFStringRef> textString(
            CFStringCreateWithBytes(kCFAllocatorDefault, (const uint8_t*)utf8.data(), utf8.size(),
                                    kCFStringEncodingUTF8, false));
    if (!textString) {
        return;
    }

    UTF16ToUTF8IndicesMap utf8IndicesMap;
    if (!utf8IndicesMap.setUTF8(utf8.data(), utf8.size())) {
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
    if (opts.tracking != 0.0f) {
        // Tracking is specified in em units, while CoreText expects absolute points
        dict_add_double(attr.get(), kCTTracking_AttributeName, opts.tracking * font.getSize());
    } else if (font.getSize() > 0.0f && is_apple_system_font(font.getTypeface())) {
        // Apply Apple HIG dynamic tracking automatically for San Francisco system fonts
        dict_add_double(attr.get(), kCTTracking_AttributeName,
                        get_apple_hig_tracking(font.getSize()) * font.getSize());
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

    LineBreakIter iter(textString.get(), typesetter.get(), opts.width, fLineBreakMode);
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
            // Reconcile iterator BiDi level with CTRun status to ensure proper LTR/RTL parity
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
            SkString runPSName;
            if (info.fFont.getTypeface()) {
                info.fFont.getTypeface()->getPostScriptName(&runPSName);
            }
            // CoreText maps missing glyphs to Apple's LastResort font instead of failing
            // Zero out glyph IDs so Skia font fallback mechanisms can handle missing glyphs
            if (runPSName.contains("LastResort")) {
                for (CFIndex k = 0; k < runGlyphs; ++k) {
                    buffer.glyphs[k] = 0;
                }
            } else {
                CTRunGetGlyphs(run, {0, runGlyphs}, buffer.glyphs);
            }

            AutoSTArray<4096, CGPoint> positions(runGlyphs);
            CTRunGetPositions(run, {0, runGlyphs}, positions.data());
            AutoSTArray<4096, CFIndex> indices;
            if (buffer.clusters) {
                indices.reset(runGlyphs);
                CTRunGetStringIndices(run, {0, runGlyphs}, indices.data());
            }

            // CoreText positions have inverted Y relative to Skia's coordinate system
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
// Bundles SkShaper_CoreText with SkUnicode for BiDi segmentation and line breaking
// Enables standalone Apple typography without ICU dependencies
class CoreTextFactory final : public SkShapers::Factory {
public:
#if defined(SK_SHAPER_UNICODE_AVAILABLE)
    CoreTextFactory(sk_sp<SkUnicode> unicode, SkShapers::CT::LineBreakMode lbm)
        : fUnicode(std::move(unicode)), fLineBreakMode(lbm) {}
#else
    CoreTextFactory(sk_sp<SkUnicode>, SkShapers::CT::LineBreakMode lbm)
        : fLineBreakMode(lbm) {}
#endif

    std::unique_ptr<SkShaper> makeShaper(sk_sp<SkFontMgr>) override {
        return SkShapers::CT::CoreText(fLineBreakMode);
    }
    std::unique_ptr<SkShaper::BiDiRunIterator> makeBidiRunIterator(
            const char* utf8, size_t utf8Bytes, uint8_t bidiLevel) override {
#if defined(SK_SHAPER_UNICODE_AVAILABLE)
        if (fUnicode) {
            return SkShapers::unicode::BidiRunIterator(fUnicode, utf8, utf8Bytes, bidiLevel);
        }
#endif
        return std::make_unique<SkShaper::TrivialBiDiRunIterator>(bidiLevel, utf8Bytes);
    }
    std::unique_ptr<SkShaper::ScriptRunIterator> makeScriptRunIterator(
            const char* utf8, size_t utf8Bytes, SkFourByteTag script) override {
        return std::make_unique<SkShaper::TrivialScriptRunIterator>(script, utf8Bytes);
    }

    SkUnicode* getUnicode() override {
#if defined(SK_SHAPER_UNICODE_AVAILABLE)
        return fUnicode.get();
#else
        return nullptr;
#endif
    }

private:
#if defined(SK_SHAPER_UNICODE_AVAILABLE)
    sk_sp<SkUnicode> fUnicode;
#endif
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
