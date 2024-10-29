// Copyright 2019 Google LLC.
#include "include/core/SkCanvas.h"
#include "include/core/SkPath.h"
#include "include/core/SkPictureRecorder.h"
#include "include/core/SkTypeface.h"
#include "include/private/base/SkTFitsIn.h"
#include "modules/skparagraph/src/ParagraphImpl.h"
#include "modules/skparagraph/src/Run.h"
#include "src/base/SkUTF.h"
#include "src/core/SkTextBlobPriv.h"

#include <algorithm>
#include <cfloat>
#include <cmath>
#include <utility>

//#ifdef SK_BUILD_FOR_CORETEXT
//#import "/Library/Developer/CommandLineTools/SDKs/MacOSX14.5.sdk/System/Library/Frameworks/ApplicationServices.framework/Versions/A/Headers/ApplicationServices.h"
//#import "/Library/Developer/CommandLineTools/SDKs/MacOSX.sdk/System/Library/Frameworks/ApplicationServices.framework/Versions/A/Headers/ApplicationServices.h"
#import <ApplicationServices/ApplicationServices.h>
#include <ApplicationServices/ApplicationServices.h>
#include "include/ports/SkTypeface_mac.h"
#include "src/utils/mac/SkCGBase.h"
#include "src/utils/mac/SkUniqueCFRef.h"
//#endif

using namespace skia_private;

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

[[maybe_unused]] static void dict_add_double(CFMutableDictionaryRef d, const void* name, double value) {
    SkUniqueCFRef<CFNumberRef> number(
            CFNumberCreate(kCFAllocatorDefault, kCFNumberDoubleType, &value));
    CFDictionaryAddValue(d, name, number.get());
}

static SkUniqueCFRef<CTFontRef> create_ctfont_from_font(const SkFont& font) {
    auto typeface = font.getTypeface();
    auto ctfont = SkTypeface_GetCTFontRef(typeface);
    if (!ctfont) {
        return nullptr;
    }
    return SkUniqueCFRef<CTFontRef>(
            CTFontCreateCopyWithAttributes(ctfont, font.getSize(), nullptr, nullptr));
}

static SkFont run_to_font(CTRunRef run, const SkFont& orig) {
    CFDictionaryRef attr = CTRunGetAttributes(run);
    CTFontRef ct = (CTFontRef)CFDictionaryGetValue(attr, kCTFontAttributeName);
    if (!ct) {
        SkDebugf("no ctfont in Run Attributes\n");
        CFShow(attr);
        return orig;
    }
    // Do I need to add a local cache, or allow the caller to manage this lookup?
    SkFont font(orig);
    font.setTypeface(SkMakeTypefaceFromCTFont(ct));
    return font;
}
}

namespace skia {
namespace textlayout {

const CFStringRef kCTTracking_AttributeName = CFSTR("CTTracking");

class LineBreakIter {
    CTTypesetterRef fTypesetter;
    double          fWidth;
    CFIndex         fStart;

public:
    LineBreakIter(CTTypesetterRef ts, SkScalar width) : fTypesetter(ts), fWidth(width) {
        fStart = 0;
    }

    SkUniqueCFRef<CTLineRef> nextLine() {
        CFRange stringRange {fStart, CTTypesetterSuggestLineBreak(fTypesetter, fStart, fWidth)};
        if (stringRange.length == 0) {
            return nullptr;
        }
        fStart += stringRange.length;
        return SkUniqueCFRef<CTLineRef>(CTTypesetterCreateLine(fTypesetter, stringRange));
    }
};

bool ParagraphImpl::extractInfoFromCoreText() {
    fCodeUnitProperties.push_back_n(fText.size() + 1, SkUnicode::CodeUnitFlags::kNoCodeUnitFlag);

    // Grab the first font available
    if (this->fTextStyles.empty()) {
        SkDebugf("No text styles\n");
        return false;
    }
    auto firstTextStyle = this->fTextStyles[0].fStyle;
    auto typefaces = this->fontCollection()->findTypefaces(
                            firstTextStyle.getFontFamilies(),
                            firstTextStyle.getFontStyle(),
                            firstTextStyle.getFontArguments()
                        );
    if (typefaces.empty()) {
        SkDebugf("No typefaces\n");
        return false;
    }
    SkFont font(typefaces[0], firstTextStyle.getFontSize());

    // Create the text object
    SkUniqueCFRef<CFStringRef> textString(
            CFStringCreateWithBytes(kCFAllocatorDefault, (const uint8_t*)fText.c_str(), fText.size(),
                                    kCFStringEncodingUTF8, false));

    SkString fontFamily;
    typefaces[0]->getFamilyName(&fontFamily);
    SkDebugf("CoreText: '%s' with %s, %f\n", fText.c_str(), fontFamily.c_str(), font.getSize());

    // UTF8
    UTF16ToUTF8IndicesMap utf8IndicesMap;
    if (!utf8IndicesMap.setUTF8(fText.c_str(), fText.size())) {
        SkDebugf("No utf8 indices\n");
        return false;
    }

    // Create the font object
    SkUniqueCFRef<CTFontRef> ctfont = create_ctfont_from_font(font);
    if (!ctfont) {
        SkDebugf("No coretext font\n");
        return false;
    }

    // Create a parameters dictionary
    SkUniqueCFRef<CFMutableDictionaryRef> attr(
            CFDictionaryCreateMutable(kCFAllocatorDefault, 0,
                                      &kCFTypeDictionaryKeyCallBacks,
                                      &kCFTypeDictionaryValueCallBacks));
    CFDictionaryAddValue(attr.get(), kCTFontAttributeName, ctfont.get());
    if ((false)) {
        // trying to see what these affect
        dict_add_double(attr.get(), kCTTracking_AttributeName, 1);
        dict_add_double(attr.get(), kCTKernAttributeName, 0.0);
    }

    SkUniqueCFRef<CFAttributedStringRef> attrString(
            CFAttributedStringCreate(kCFAllocatorDefault, textString.get(), attr.get()));

    SkUniqueCFRef<CTTypesetterRef> typesetter(
            CTTypesetterCreateWithAttributedString(attrString.get()));

    // We have to compute RunInfos in a loop, and then reuse them in a 2nd loop,
    // so we store them in an array (we reuse the array's storage for each line).
    std::vector<SkFont> fontStorage;
    std::vector<SkShaper::RunHandler::RunInfo> infos;

    // Shape the text in one line
    LineBreakIter iter(typesetter.get(), SK_ScalarMax);
    SkUniqueCFRef<CTLineRef> line = iter.nextLine();

    CFArrayRef run_array = CTLineGetGlyphRuns(line.get());
    CFIndex runCount = CFArrayGetCount(run_array);
    if (runCount == 0) {
        SkDebugf("No GlyphRuns\n");
        return false;
    }
    // Begin the line
    fontStorage.clear();
    fontStorage.reserve(runCount); // ensure the refs won't get invalidated
    infos.clear();
    for (CFIndex j = 0; j < runCount; ++j) {
        // Begin the run
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

        fontStorage.push_back(run_to_font(run, font));
        infos.push_back({
            fontStorage.back(), // info just stores a ref to the font
            0,                  // need fBidiLevel
            {adv, 0},
            (size_t)runGlyphs,
            {range.first, range.second},
        });
        // End the run handler->runInfo(infos.back());
    }

    // Now loop through again and fill in the buffers
    SkScalar lineAdvance = 0;
    for (CFIndex j = 0; j < runCount; ++j) {
        const auto& info = infos[j];
        auto& textRun = fRuns.emplace_back(this,
                                           info,
                                           0,
                                           0.0f,
                                           0.0f,
                                           false,
                                           fRuns.size(),
                                           lineAdvance);
        SkDebugf("run[%d]: [%d:%d)\n", fRuns.size(), info.utf8Range.begin(), info.utf8Range.end());
        SkShaper::RunHandler::Buffer buffer = textRun.newRunBuffer();

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
                buffer.point.fY,
            };
            if (buffer.offsets) {
                buffer.offsets[k] = {0, 0}; // offset relative to the origin for this glyph
            }
            if (buffer.clusters) {
                buffer.clusters[k] = utf8IndicesMap.mapIndex(indices[k]);
            }
            // TODO: Remove this assumption (glyph cluster breaks == grapheme breaks)
            auto index8 = utf8IndicesMap.mapIndex(indices[k]);
            fCodeUnitProperties[index8]|= SkUnicode::CodeUnitFlags::kGraphemeStart;
        }
        // Track the run advance
        lineAdvance += info.fAdvance.fX;
    }
    // Bidi regions`
    fBidiRegions.emplace_back(0, fText.size(), 0);

    // Soft line breaks
    fCodeUnitProperties.push_back_n(fText.size() + 1, SkUnicode::CodeUnitFlags::kNoCodeUnitFlag);
    auto stringRange = CTLineGetStringRange(line.get());
    auto tokenizer = CFStringTokenizerCreate(kCFAllocatorDefault, textString.get(), stringRange,
                                            kCFStringTokenizerUnitWordBoundary,
                                            CFLocaleCopyCurrent());
    auto tokenType = CFStringTokenizerAdvanceToNextToken(tokenizer);
    while (tokenType != kCFStringTokenizerTokenNone) {
        CFRange range16 = CFStringTokenizerGetCurrentTokenRange(tokenizer);
        auto range8 = utf8IndicesMap.mapRange(range16.location, range16.length);
        fCodeUnitProperties[range8.first] |= SkUnicode::CodeUnitFlags::kSoftLineBreakBefore;
        fCodeUnitProperties[range8.first + range8.second] |= SkUnicode::CodeUnitFlags::kSoftLineBreakBefore;
        fWords.emplace_back(range16.location);
        tokenType = CFStringTokenizerAdvanceToNextToken(tokenizer);
    }
    fCodeUnitProperties[fText.size()] |= SkUnicode::CodeUnitFlags::kSoftLineBreakBefore;

    // Whitespaces
    CFCharacterSetRef whitespaces = CFCharacterSetGetPredefined(kCFCharacterSetWhitespace);
    CFCharacterSetRef newlines = CFCharacterSetGetPredefined(kCFCharacterSetNewline);
    CFCharacterSetRef controls = CFCharacterSetGetPredefined(kCFCharacterSetControl);
    CFIndex textLength = CFStringGetLength(textString.get());
    CFStringInlineBuffer buf;
    CFStringInitInlineBuffer(textString.get(), &buf, CFRangeMake(0, textLength));
    for (auto index = 0; index < textLength; ++index) {
        UniChar ch = CFStringGetCharacterFromInlineBuffer(&buf, index);
        auto before = utf8IndicesMap.mapIndex(index);
        auto after = utf8IndicesMap.mapIndex(index + 1);
        SkUnicode::CodeUnitFlags flags = SkUnicode::CodeUnitFlags::kNoCodeUnitFlag;
        if (CFCharacterSetIsCharacterMember(whitespaces, ch)) {
            flags |= SkUnicode::CodeUnitFlags::kPartOfWhiteSpaceBreak;
        }
        if (CFCharacterSetIsCharacterMember(newlines, ch)) {
            flags |= SkUnicode::CodeUnitFlags::kHardLineBreakBefore;
        }
        if (CFCharacterSetIsCharacterMember(controls, ch)) {
            flags |= SkUnicode::CodeUnitFlags::kControl;
        }
        for (auto i = before; i < after; ++i) {
            fCodeUnitProperties[i] |= flags;
        }
    }

    // Get some information about trailing spaces / hard line breaks
    fTrailingSpaces = fText.size();
    TextIndex firstWhitespace = EMPTY_INDEX;
    for (int i = 0; i < fCodeUnitProperties.size(); ++i) {
        auto flags = fCodeUnitProperties[i];
        if (SkUnicode::hasPartOfWhiteSpaceBreakFlag(flags)) {
            if (fTrailingSpaces  == fText.size()) {
                fTrailingSpaces = i;
            }
            if (firstWhitespace == EMPTY_INDEX) {
                firstWhitespace = i;
            }
        } else {
            fTrailingSpaces = fText.size();
        }
        if (SkUnicode::hasHardLineBreakFlag(flags)) {
            fHasLineBreaks = true;
        }
    }

    if (firstWhitespace < fTrailingSpaces) {
        fHasWhitespacesInside = true;
    }

    skia_private::TArray<size_t, true> fClustersIndexFromCodeUnit;
    skia_private::TArray<TextIndex, true> fUTF8IndexForUTF16Index;
    skia_private::TArray<size_t, true> fUTF16IndexForUTF8Index;
    fState = kShaped;
    this->applySpacingAndBuildClusterTable();
    return true;
}
}  // namespace textlayout
}  // namespace skia
