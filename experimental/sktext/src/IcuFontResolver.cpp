// Copyright 2021 Google LLC.
#include "experimental/sktext/include/Text.h"
#include "experimental/sktext/src/LogicalRun.h"
#include <memory>
#include <stack>

#include "include/core/SkFont.h"
#include "include/core/SkFontArguments.h"
#include "include/core/SkFontTypes.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSpan.h"
#include "include/core/SkStream.h"
#include "include/core/SkTypeface.h"
#include "include/core/SkTypes.h"
#include "include/private/SkBitmaskEnum.h"
#include "include/private/SkMalloc.h"
#include "include/private/SkMutex.h"
#include "include/private/SkTArray.h"
#include "include/private/SkTemplates.h"
#include "include/private/SkTo.h"
#include "modules/skunicode/include/SkUnicode.h"
#include "src/core/SkTDPQueue.h"
#include "src/utils/SkUTF.h"

namespace skia {
namespace text {

bool IcuFontResolvedText::resolveChain(UnicodeText* unicodeText, SkSpan<const char16_t> text, TextDirection textDirection, const char* locale, const FontChain& fontChain) {

    std::deque<TextRange> unresolvedTexts;
    unresolvedTexts.emplace_back(0, text.size());
    auto startForFontChain = fResolvedFonts.size(); // results for font chain start from here
    for (auto fontIndex = 0; fontIndex < fontChain.count(); ++fontIndex) {
        auto typeface = fontChain[fontIndex];

        std::deque<TextRange> newUnresolvedTexts;
        // Check all text range that have not been resolved yet
        while (!unresolvedTexts.empty()) {
            // Take the first unresolved
            auto unresolvedText = unresolvedTexts.front();
            unresolvedTexts.pop_front();
            auto startForText = newUnresolvedTexts.size(); // results for each unresolvedText start from here

            // Resolve font for the entire grapheme
            unicodeText->forEachGrapheme(unresolvedText, [&](TextRange grapheme) {
                if (unicodeText->isControl(grapheme)) {
                    return;
                }
                auto count = typeface->textToGlyphs(unicodeText->getText16().data() + grapheme.fStart, grapheme.width() * 2, SkTextEncoding::kUTF16, nullptr, 0);
                SkAutoTArray<SkGlyphID> glyphs(count);
                typeface->textToGlyphs(unicodeText->getText16().data() + grapheme.fStart, grapheme.width() * 2, SkTextEncoding::kUTF16, glyphs.data(), count);
                bool resolved = true;
                for (auto i = 0; i < count; ++i) {
                    if (glyphs[i] == 0) {
                        resolved = false;
                        break;
                    }
                }
                if (!resolved) {
                    // Grapheme is not resolved but it can have resolved canonically equivalent form
                    if (graphemeCanBeResolved(unicodeText, typeface, grapheme)) {
                        // Keep going
                    } else if (newUnresolvedTexts.empty() || newUnresolvedTexts.back().fEnd < grapheme.fStart) {
                        // It's a new unresolved block
                        newUnresolvedTexts.push_back(grapheme);
                    } else {
                        // Let's extend the last unresolved block
                        newUnresolvedTexts.back().fEnd = grapheme.fEnd;
                    }
                }
            });

            // Let's fill the resolved blocks with the current font
            addResolved(unresolvedText, newUnresolvedTexts, startForText, fontChain, fontIndex);
        }

        // Try the next font in chain
        SkASSERT(unresolvedTexts.empty());
        unresolvedTexts = std::move(newUnresolvedTexts);
    }

    if (startForFontChain == fResolvedFonts.size()) {
        fResolvedFonts.emplace_back(TextRange(0, text.size()), fontChain[0], fontChain.fontSize(), SkFontStyle::Normal());
        return true;
    }
    return unresolvedTexts.empty();
}

bool IcuFontResolvedText::graphemeCanBeResolved(UnicodeText* unicodeText, sk_sp<SkTypeface> typeface, TextRange grapheme) {
    if (!unicodeText->graphemeHasMarks(grapheme)) {
        return false;
    }
    std::vector<SkUnichar> codepoints = unicodeText->decompose(grapheme);
    for (auto i = 1ul; i < codepoints.size(); ++i) {
        auto baseRange = SkSpan<SkUnichar>(codepoints.data(), i + 1);

        // Check if it's good decomposed
        bool resolved = true;
        for (auto j = 0; j <= i; ++j) {
            if (typeface->unicharToGlyph(codepoints[j]) == 0) {
                resolved = false;
                break;
            }
        }
        if (!resolved) {
            // Check if it's good composed
            SkUnichar baseCodepoint;
            if (!unicodeText->composeCodepoints(baseRange, &baseCodepoint)) {
                continue;
            }
            if (typeface->unicharToGlyph(baseCodepoint) == 0) {
                continue;
            }
            resolved = true;
        }

        // We managed to resolved codepoints so far; let's check the rest
        for (auto j = i + 1; j < codepoints.size(); ++j) {
            if (typeface->unicharToGlyph(codepoints[j]) == 0) {
                resolved = false;
                break;
            }
        }

        if (resolved) {
            return true;
        }
    }
    return false;
}

}
}
