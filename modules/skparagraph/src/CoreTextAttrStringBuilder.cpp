// Copyright 2026 Google LLC
#include "modules/skparagraph/src/CoreTextAttrStringBuilder.h"

#include "include/core/SkString.h"
#include "include/private/SkAttributes.h"
#include "include/private/SkTemplates.h"
#include "include/private/SkTo.h"
#include "modules/skparagraph/src/Run.h"
#include "src/core/SkUTF.h"

#include <algorithm>
#include <iterator>
#include <optional>
#include <vector>

namespace skia {
namespace textlayout {

namespace {

static const CFStringRef kStyleGroupAttributeName = CFSTR("SkParagraphStyleGroup");

// Styles that share all font, metric, rasterization, and shaping attributes can coalesce into
// one CFAttributedString style span so paint-only boundaries (color, decoration, shadows) do not
// split CTRuns or ligatures
bool can_coalesce_styles(const TextStyle& a, const TextStyle& b) {
    return a.matchOneAttribute(StyleType::kFont, b) &&
           a.getTypeface() == b.getTypeface() &&
           a.getFontFeatures() == b.getFontFeatures() &&
           SkScalarNearlyEqual(a.getLetterSpacing(), b.getLetterSpacing());
}

// The font restyled by attrs, or font itself when there is nothing to apply or CoreText
// rejects the descriptor
SkUniqueCFRef<CTFontRef> copy_with_attributes(CTFontRef font,
                                              CGFloat fontSize,
                                              CFDictionaryRef attrs) {
    // Callers pass a createFont result, null only if CTFontCreateUIFontForLanguage fails for the
    // system UI fallback; the CFRetain fallback below would crash on null
    SkASSERT(font);
    if (CFDictionaryGetCount(attrs) > 0) {
        SkUniqueCFRef<CTFontDescriptorRef> desc(CTFontDescriptorCreateWithAttributes(attrs));
        CTFontRef styled = CTFontCreateCopyWithAttributes(font, fontSize, nullptr, desc.get());
        if (styled) {
            return SkUniqueCFRef<CTFontRef>(styled);
        }
    }
    return SkUniqueCFRef<CTFontRef>((CTFontRef)CFRetain(font));
}

constexpr SkFourByteTag kLatn = SkSetFourByteTag('L', 'a', 't', 'n');

struct ScriptBlock {
    SkUnichar fFirst;
    SkUnichar fLast;
    SkFourByteTag fScript;
};

// Tag for letters of scripts kScriptBlocks does not list. Their only consumers are
// Run::isCursiveScript, which turns letterSpacing off, and the run merge in ParagraphImpl, so a
// non-cursive placeholder is enough; SkUnicode has no script API to name them exactly
constexpr SkFourByteTag kUnlistedLetterScript = SkSetFourByteTag('Z', 'z', 'z', 'z');

// Unicode blocks with their scripts, sorted for binary search. Every block of the scripts
// Run::isCursiveScript calls cursive is listed, so no cursive letter falls through to
// kUnlistedLetterScript. A 0 entry is a block of Common code points (punctuation, symbols,
// emoji) that joins the neighbouring run without the CoreFoundation lookup
constexpr ScriptBlock kScriptBlocks[] = {
        {0x00C0, 0x02AF, kLatn},
        {0x0370, 0x03FF, SkSetFourByteTag('G', 'r', 'e', 'k')},
        {0x0400, 0x052F, SkSetFourByteTag('C', 'y', 'r', 'l')},
        {0x0530, 0x058F, SkSetFourByteTag('A', 'r', 'm', 'n')},
        {0x0590, 0x05FF, SkSetFourByteTag('H', 'e', 'b', 'r')},
        {0x0600, 0x06FF, SkSetFourByteTag('A', 'r', 'a', 'b')},
        {0x0700, 0x074F, SkSetFourByteTag('S', 'y', 'r', 'c')},
        {0x0750, 0x077F, SkSetFourByteTag('A', 'r', 'a', 'b')},
        {0x07C0, 0x07FF, SkSetFourByteTag('N', 'k', 'o', 'o')},
        {0x0840, 0x085F, SkSetFourByteTag('M', 'a', 'n', 'd')},
        {0x0860, 0x086F, SkSetFourByteTag('S', 'y', 'r', 'c')},
        {0x0870, 0x089F, SkSetFourByteTag('A', 'r', 'a', 'b')},
        {0x08A0, 0x08FF, SkSetFourByteTag('A', 'r', 'a', 'b')},
        {0x0900, 0x097F, SkSetFourByteTag('D', 'e', 'v', 'a')},
        {0x0E00, 0x0E7F, SkSetFourByteTag('T', 'h', 'a', 'i')},
        {0x10A0, 0x10FF, SkSetFourByteTag('G', 'e', 'o', 'r')},
        {0x1100, 0x11FF, SkSetFourByteTag('H', 'a', 'n', 'g')},
        {0x1800, 0x18AF, SkSetFourByteTag('M', 'o', 'n', 'g')},
        {0x1E00, 0x1EFF, kLatn},
        {0x1F00, 0x1FFF, SkSetFourByteTag('G', 'r', 'e', 'k')},
        {0x2000, 0x2BFF, 0},
        {0x2D00, 0x2D2F, SkSetFourByteTag('G', 'e', 'o', 'r')},
        {0x2DE0, 0x2DFF, SkSetFourByteTag('C', 'y', 'r', 'l')},
        {0x3000, 0x303F, 0},
        {0x3040, 0x309F, SkSetFourByteTag('H', 'i', 'r', 'a')},
        {0x30A0, 0x30FF, SkSetFourByteTag('K', 'a', 'n', 'a')},
        {0x3130, 0x318F, SkSetFourByteTag('H', 'a', 'n', 'g')},
        {0x31F0, 0x31FF, SkSetFourByteTag('K', 'a', 'n', 'a')},
        {0x3400, 0x4DBF, SkSetFourByteTag('H', 'a', 'n', 'i')},
        {0x4E00, 0x9FFF, SkSetFourByteTag('H', 'a', 'n', 'i')},
        {0xA640, 0xA69F, SkSetFourByteTag('C', 'y', 'r', 'l')},
        {0xA840, 0xA87F, SkSetFourByteTag('P', 'h', 'a', 'g')},
        {0xA8E0, 0xA8FF, SkSetFourByteTag('D', 'e', 'v', 'a')},
        {0xAC00, 0xD7AF, SkSetFourByteTag('H', 'a', 'n', 'g')},
        {0xF900, 0xFAFF, SkSetFourByteTag('H', 'a', 'n', 'i')},
        {0xFB1D, 0xFB4F, SkSetFourByteTag('H', 'e', 'b', 'r')},
        {0xFB50, 0xFDFF, SkSetFourByteTag('A', 'r', 'a', 'b')},
        {0xFE00, 0xFE0F, 0},
        {0xFE70, 0xFEFF, SkSetFourByteTag('A', 'r', 'a', 'b')},
        {0xFFF0, 0xFFFF, 0},
        {0x10D00, 0x10D3F, SkSetFourByteTag('R', 'o', 'h', 'g')},
        {0x11660, 0x1167F, SkSetFourByteTag('M', 'o', 'n', 'g')},
        {0x1F000, 0x1FAFF, 0},
        {0x20000, 0x2EBEF, SkSetFourByteTag('H', 'a', 'n', 'i')},
        {0xE0000, 0xE007F, 0},
};

// Returns true when every kScriptBlocks entry has fFirst <= fLast and starts after the previous
// entry ends; find_block's binary search returns wrong blocks otherwise
constexpr bool blocks_sorted_and_disjoint() {
    for (size_t i = 0; i < std::size(kScriptBlocks); ++i) {
        if (kScriptBlocks[i].fFirst > kScriptBlocks[i].fLast ||
            (i > 0 && kScriptBlocks[i - 1].fLast >= kScriptBlocks[i].fFirst)) {
            return false;
        }
    }
    return true;
}
static_assert(blocks_sorted_and_disjoint(), "upper_bound in ResolveCodepointScript needs it");

// Classifies a code point outside kScriptBlocks: a combining mark still joins its base and a
// letter starts a non-cursive run instead of taking a cursive neighbour's script (0 and
// kUnlistedLetterScript). Never inlined: inlined into the decode loop it made the builder
// 1.17-1.28x slower on every bench text, ASCII too, against 1.01-1.02x as a call
SK_NEVER_INLINE SkFourByteTag classify_unlisted(SkUnichar c) {
    static const CFCharacterSetRef marks = CFCharacterSetGetPredefined(kCFCharacterSetNonBase);
    static const CFCharacterSetRef letters = CFCharacterSetGetPredefined(kCFCharacterSetLetter);
    if (CFCharacterSetIsLongCharacterMember(marks, static_cast<UTF32Char>(c))) {
        return 0;
    }
    return CFCharacterSetIsLongCharacterMember(letters, static_cast<UTF32Char>(c))
                   ? kUnlistedLetterScript
                   : 0;
}

// Returns the kScriptBlocks entry that holds c, or null when no listed block does
const ScriptBlock* find_block(SkUnichar c) {
    const ScriptBlock* next = std::upper_bound(
            std::begin(kScriptBlocks),
            std::end(kScriptBlocks),
            c,
            [](SkUnichar u, const ScriptBlock& block) { return u < block.fFirst; });
    return next != std::begin(kScriptBlocks) && c <= (next - 1)->fLast ? next - 1 : nullptr;
}

// Returns the script of c as ResolveCodepointScript defines it. lastBlock is the caller's last
// table hit, checked before the binary search and updated on a miss
SkFourByteTag resolve_script(SkUnichar c, const ScriptBlock** lastBlock) {
    // ASCII letters are the hot case and the only code points below U+00C0 the table calls Latin
    if (c < 0x80) {
        return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') ? kLatn : 0;
    }
    const ScriptBlock* block = *lastBlock;
    if (!block || c < block->fFirst || c > block->fLast) {
        block = find_block(c);
        *lastBlock = block;
    }
    return block ? block->fScript : classify_unlisted(c);
}

// Sets the base writing direction on fullRange of attrStr; it controls CoreText paragraph-level
// BiDi embedding. Leaves attrStr unchanged if CoreText fails to create the paragraph style
void set_base_direction(CFMutableAttributedStringRef attrStr,
                        CFRange fullRange,
                        TextDirection direction) {
    CTWritingDirection dir = (direction == TextDirection::kRtl) ? kCTWritingDirectionRightToLeft
                                                                : kCTWritingDirectionLeftToRight;
    CTParagraphStyleSetting dirSetting = {
            kCTParagraphStyleSpecifierBaseWritingDirection, sizeof(CTWritingDirection), &dir};
    SkUniqueCFRef<CTParagraphStyleRef> pStyle(CTParagraphStyleCreate(&dirSetting, 1));
    if (!pStyle) {
        return;
    }
    CFAttributedStringSetAttribute(
            attrStr, fullRange, kCTParagraphStyleAttributeName, pStyle.get());
}

// Tags cfRange of attrStr with group, a distinct value per non-coalesced span, so CoreText splits
// CTRuns at span boundaries even when two spans resolve to the same CTFont (e.g. LastResort
// fallback or differing SkFont edging/baselineShift)
void set_style_group(CFMutableAttributedStringRef attrStr, CFRange cfRange, int group) {
    SkUniqueCFRef<CFNumberRef> groupNum(
            CFNumberCreate(kCFAllocatorDefault, kCFNumberIntType, &group));
    if (!groupNum) {
        return;
    }
    CFAttributedStringSetAttribute(attrStr, cfRange, kStyleGroupAttributeName, groupNum.get());
}

// The typefaces and point size a style span shapes with
struct SpanFont {
    // Null when the style names no typeface and no family resolves; createFont then picks the
    // system UI font
    sk_sp<SkTypeface> fPrimary;
    // Resolved families in order; entries after the first form the cascade list
    std::vector<sk_sp<SkTypeface>> fTypefaces;
    CGFloat fSize;
};

// Returns the font of style: its explicit typeface, else the families resolved through
// fontCollection by CoreTextFontRegistry::findTypefaces, and a positive point size
SpanFont resolve_span_font(const TextStyle& style, FontCollection& fontCollection) {
    SpanFont result;
    result.fPrimary = style.refTypeface();
    // TextStyle accepts zero, negative and NaN sizes but CoreText needs a positive point size
    // (createFont asserts it), so such a span takes 14, the TextStyle default size
    result.fSize = style.getFontSize() > 0.0f ? style.getFontSize() : 14.0f;
    // Empty font families must bypass FontCollection fallback to allow CoreText to bind
    // .AppleSystemUIFont
    if (result.fPrimary || style.getFontFamilies().empty()) {
        return result;
    }
    // Clones carry the variation in their CTFontDescriptor, so the primary font and
    // every cascade entry below shape with the same axes as the HarfBuzz path
    result.fTypefaces = CoreTextFontRegistry::Instance().findTypefaces(fontCollection,
                                                                       style.getFontFamilies(),
                                                                       style.getFontStyle(),
                                                                       style.getFontArguments());
    if (!result.fTypefaces.empty()) {
        result.fPrimary = result.fTypefaces.front();
    }
    return result;
}

// Returns the descriptor of the LastResort font at size, or null if CoreText fails to create it
SkUniqueCFRef<CTFontDescriptorRef> last_resort_descriptor(CGFloat size) {
    SkUniqueCFRef<CTFontRef> lrFont(CTFontCreateWithName(CFSTR("LastResort"), size, nullptr));
    if (!lrFont) {
        return nullptr;
    }
    return SkUniqueCFRef<CTFontDescriptorRef>(CTFontCopyFontDescriptor(lrFont.get()));
}

// Sets kCTFontCascadeListAttribute in descAttrs to the descriptors of the span's typefaces after
// the first, then LastResort when fallback is disabled; the cascade list provides the font
// fallback chain for characters outside the primary typeface. No key when the list is empty
void add_cascade_list(CFMutableDictionaryRef descAttrs,
                      const SpanFont& spanFont,
                      bool fallbackDisabled) {
    if (spanFont.fTypefaces.size() <= 1 && !fallbackDisabled) {
        return;
    }
    SkUniqueCFRef<CFMutableArrayRef> cascadeList(
            CFArrayCreateMutable(kCFAllocatorDefault, 0, &kCFTypeArrayCallBacks));
    for (size_t i = 1; i < spanFont.fTypefaces.size(); ++i) {
        SkUniqueCFRef<CTFontDescriptorRef> desc =
                CoreTextFontRegistry::Instance().getDescriptor(spanFont.fTypefaces[i].get());
        if (desc) {
            CFArrayAppendValue(cascadeList.get(), desc.get());
        }
    }
    if (fallbackDisabled) {
        if (SkUniqueCFRef<CTFontDescriptorRef> lrDesc = last_resort_descriptor(spanFont.fSize)) {
            CFArrayAppendValue(cascadeList.get(), lrDesc.get());
        }
    }
    if (CFArrayGetCount(cascadeList.get()) == 0) {
        return;
    }
    CFDictionarySetValue(descAttrs, kCTFontCascadeListAttribute, cascadeList.get());
}

// Appends to featureArray the OpenType feature setting dictionary {tag: value}; tag is the
// 4-character ASCII feature tag
void append_feature(CFMutableArrayRef featureArray, const char* tag, int value) {
    SkUniqueCFRef<CFStringRef> tagStr(
            CFStringCreateWithCString(kCFAllocatorDefault, tag, kCFStringEncodingASCII));
    SkUniqueCFRef<CFNumberRef> valNum(
            CFNumberCreate(kCFAllocatorDefault, kCFNumberIntType, &value));
    const void* fKeys[] = {kCTFontOpenTypeFeatureTag, kCTFontOpenTypeFeatureValue};
    const void* fVals[] = {tagStr.get(), valNum.get()};
    SkUniqueCFRef<CFDictionaryRef> featDict(CFDictionaryCreate(kCFAllocatorDefault,
                                                               fKeys,
                                                               fVals,
                                                               2,
                                                               &kCFTypeDictionaryKeyCallBacks,
                                                               &kCFTypeDictionaryValueCallBacks));
    CFArrayAppendValue(featureArray, featDict.get());
}

// Sets kCTFontFeatureSettingsAttribute in descAttrs to the user features of style with a
// 4-character tag, then liga=0 when style has letterSpacing. Returns the non-empty user features
// without that liga=0 for a letter-spaced span, null otherwise
SkUniqueCFRef<CFArrayRef> add_feature_settings(CFMutableDictionaryRef descAttrs,
                                               const TextStyle& style) {
    // CoreText expects 4-character OpenType feature tags packed into descriptor settings
    const auto& features = style.getFontFeatures();
    const SkScalar letterSpacing = style.getLetterSpacing();
    // letterSpacing needs feature settings even without user features; the extra array
    // slot holds its liga=0
    if (features.empty() && letterSpacing == 0) {
        return nullptr;
    }
    SkUniqueCFRef<CFMutableArrayRef> featureArray(
            CFArrayCreateMutable(kCFAllocatorDefault, features.size() + 1, &kCFTypeArrayCallBacks));
    for (const auto& feat : features) {
        if (feat.fName.size() == 4) {
            append_feature(featureArray.get(), feat.fName.c_str(), feat.fValue);
        }
    }
    SkUniqueCFRef<CFArrayRef> userFeatures;
    // KEEPER-DEBT: OneLineShaper keeps liga for negative letterSpacing; native tracking and
    // CSS Text 3 drop optional ligatures for any nonzero spacing. Remove once it uses != 0.
    // Spacing goes between clusters, so a ligature would hide it; liga goes off after the
    // user features. Cursive runs get no spacing and keep liga, see setCursiveRunsFont
    if (letterSpacing != 0) {
        // Skip copying an empty array when the span sets letterSpacing without user features
        if (CFArrayGetCount(featureArray.get()) > 0) {
            userFeatures.reset(CFArrayCreateCopy(kCFAllocatorDefault, featureArray.get()));
        }
        append_feature(featureArray.get(), "liga", 0);
    }
    if (CFArrayGetCount(featureArray.get()) > 0) {
        CFDictionarySetValue(descAttrs, kCTFontFeatureSettingsAttribute, featureArray.get());
    }
    return userFeatures;
}

// Sets kCTFontVariationAttribute in descAttrs to the variation coordinates of fontArgs; no key
// when there are none
void add_variations(CFMutableDictionaryRef descAttrs,
                    const std::optional<FontArguments>& fontArgs) {
    // The registry rebuilds system fonts by name and drops cloned variations, so only the
    // requested axes are re-applied: CoreText merges them and keeps deriving opsz from the
    // point size, which a full design position would pin. Palette overrides have no
    // CoreText equivalent and stay HarfBuzz-only
    if (!fontArgs.has_value() || fontArgs->getVariationCoordinates().empty()) {
        return;
    }
    SkUniqueCFRef<CFMutableDictionaryRef> variation(
            CFDictionaryCreateMutable(kCFAllocatorDefault,
                                      0,
                                      &kCFTypeDictionaryKeyCallBacks,
                                      &kCFTypeDictionaryValueCallBacks));
    // css-fonts-4: the last value given for an axis wins, which SetValue provides
    for (const auto& coord : fontArgs->getVariationCoordinates()) {
        int64_t axisTag = coord.axis;
        double axisValue = coord.value;
        SkUniqueCFRef<CFNumberRef> tagNum(
                CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt64Type, &axisTag));
        SkUniqueCFRef<CFNumberRef> valNum(
                CFNumberCreate(kCFAllocatorDefault, kCFNumberDoubleType, &axisValue));
        if (tagNum && valNum) {
            CFDictionarySetValue(variation.get(), tagNum.get(), valNum.get());
        }
    }
    if (CFDictionaryGetCount(variation.get()) == 0) {
        return;
    }
    CFDictionarySetValue(descAttrs, kCTFontVariationAttribute, variation.get());
}

// The base CTFont of a style span and the descriptor attributes applied on top of it
struct SpanFontAttrs {
    SkUniqueCFRef<CTFontRef> fBase;
    SkUniqueCFRef<CFMutableDictionaryRef> fDescAttrs;
    // User features without the letterSpacing liga=0, see add_feature_settings
    SkUniqueCFRef<CFArrayRef> fUserFeatures;
};

// Creates the base font of spanFont with the font style of style, and the cascade list,
// feature settings and variations of the span as descriptor attributes, in that order
SpanFontAttrs make_span_font(const TextStyle& style,
                             const SpanFont& spanFont,
                             bool fallbackDisabled) {
    SpanFontAttrs result;
    result.fBase = CoreTextFontRegistry::Instance().createFont(
            spanFont.fPrimary.get(), style.getFontStyle(), spanFont.fSize);
    result.fDescAttrs.reset(CFDictionaryCreateMutable(kCFAllocatorDefault,
                                                      0,
                                                      &kCFTypeDictionaryKeyCallBacks,
                                                      &kCFTypeDictionaryValueCallBacks));
    add_cascade_list(result.fDescAttrs.get(), spanFont, fallbackDisabled);
    result.fUserFeatures = add_feature_settings(result.fDescAttrs.get(), style);
    add_variations(result.fDescAttrs.get(), style.getFontArguments());
    return result;
}

// Sets kCTLanguageAttributeName on cfRange of attrStr to locale; nothing for an empty locale
void set_language(CFMutableAttributedStringRef attrStr, CFRange cfRange, const SkString& locale) {
    // Han unification leaves one code point per ideograph, so CoreText picks the regional
    // cascade (and locl forms) from the string language; without the tag it follows the
    // machine's preferred languages, unlike the HarfBuzz path where
    // FontCollection::defaultFallback already receives this locale
    if (locale.isEmpty()) {
        return;
    }
    SkUniqueCFRef<CFStringRef> language(
            CFStringCreateWithCString(kCFAllocatorDefault, locale.c_str(), kCFStringEncodingUTF8));
    if (!language) {
        return;
    }
    CFAttributedStringSetAttribute(attrStr, cfRange, kCTLanguageAttributeName, language.get());
}

}  // namespace

SkFourByteTag CoreTextAttrStringBuilder::ResolveCodepointScript(SkUnichar c) {
    const ScriptBlock* lastBlock = nullptr;
    return resolve_script(c, &lastBlock);
}

CoreTextAttrStringBuilder::CoreTextAttrStringBuilder(
        const char* utf8, size_t utf8Bytes,
        TextDirection baseDirection,
        sk_sp<FontCollection> fontCollection)
    : fUtf8(utf8)
    , fUtf8Bytes(utf8Bytes)
    , fBaseDirection(baseDirection)
    , fFontCollection(std::move(fontCollection))
    , fUtf16(SkToInt(utf8Bytes)) {
    SkASSERT(fUtf8);
    SkASSERT(fFontCollection);

    SkFourByteTag current = 0;
    size_t runStart = 0;
    // Neighbouring code points mostly share a block, so the last hit skips the binary search:
    // with it the builder takes 0.89-1.00x the time of the callback version on the bench texts,
    // without it up to 1.04x on mixed text
    const ScriptBlock* lastBlock = nullptr;
    // One decode pass gives the index map, the UTF-16 text for build(), and the script runs
    // TODO(jlavrova): decode once per paragraph and give each segment a slice of that UTF-16
    // text and map. Blockers: SkUnicode::extractUtfConversionMapping breaks on invalid UTF-8,
    // and leading Common code points after '\n' would join the previous segment's script
    fIndexMap.start(fUtf8Bytes);
    const char* ptr = fUtf8;
    const char* const end = fUtf8 + fUtf8Bytes;
    // NextScalar consumes at least one byte, so the loop ends on malformed UTF-8 too
    while (ptr < end) {
        const size_t utf8Start = ptr - fUtf8;
        const SkUnichar u = CoreTextIndexMap::NextScalar(&ptr, end);
        const size_t utf16Start = fIndexMap.append(utf8Start, ptr - fUtf8, u);
        SkASSERT(utf16Start + SkUTF::ToUTF16(u) <= SkToSizeT(fUtf16.count()));
        SkUTF::ToUTF16(u, fUtf16.data() + utf16Start);
        SkFourByteTag script = resolve_script(u, &lastBlock);
        if (script == 0 || script == current) {
            continue;
        }
        if (current != 0) {
            fScriptRuns.push_back({TextRange(runStart, utf8Start), current});
            runStart = utf8Start;
        }
        current = script;
    }
    fIndexMap.finish();
    fScriptRuns.push_back({TextRange(runStart, fUtf8Bytes), current != 0 ? current : kLatn});
}

SkFourByteTag CoreTextAttrStringBuilder::scriptAt(size_t utf8Index) const {
    SkASSERT(!fScriptRuns.empty());
    for (const ScriptRun& run : fScriptRuns) {
        if (utf8Index < run.fRange.end) {
            return run.fScript;
        }
    }
    return fScriptRuns.back().fScript;
}

void CoreTextAttrStringBuilder::addStyle(TextRange utf8Range, const TextStyle& style) {
    if (utf8Range.start >= utf8Range.end || utf8Range.start >= fUtf8Bytes) {
        return;
    }
    size_t end = std::min(utf8Range.end, fUtf8Bytes);
    // Merge adjacent spans that differ only in paint attributes so ligatures and kerning are
    // preserved across paint-only Block boundaries
    if (!fStyles.empty() &&
        fStyles.back().fRange.end == utf8Range.start &&
        can_coalesce_styles(fStyles.back().fStyle, style)) {
        fStyles.back().fRange.end = end;
        return;
    }
    fStyles.push_back({TextRange(utf8Range.start, end), style});
}

SkUniqueCFRef<CFAttributedStringRef> CoreTextAttrStringBuilder::build() {
    // CFStringCreateWithBytes drops a leading U+FEFF from UTF-8 as a byte order mark, which
    // would shift every CFRange from CoreTextIndexMap; UTF-16 input keeps it as a character.
    // One U+FFFD per invalid byte, exactly as CoreTextIndexMap counts them
    const CFIndex utf16Length = SkTo<CFIndex>(fIndexMap.toUtf16Index(fUtf8Bytes));
    SkUniqueCFRef<CFStringRef> cfStr(CFStringCreateWithCharacters(
            kCFAllocatorDefault, reinterpret_cast<const UniChar*>(fUtf16.data()), utf16Length));
    if (!cfStr) {
        return nullptr;
    }
    // CFStringCreateWithCharacters copies exactly utf16Length code units
    SkASSERT(CFStringGetLength(cfStr.get()) == utf16Length);

    SkUniqueCFRef<CFMutableAttributedStringRef> attrStr(
            CFAttributedStringCreateMutable(kCFAllocatorDefault, utf16Length));
    CFAttributedStringReplaceString(attrStr.get(), CFRangeMake(0, 0), cfStr.get());

    if (utf16Length == 0) {
        return SkUniqueCFRef<CFAttributedStringRef>(attrStr.release());
    }

    set_base_direction(attrStr.get(), CFRangeMake(0, utf16Length), fBaseDirection);

    // The collection setting holds for the whole paragraph, so every span shares it
    const bool fallbackDisabled = !fFontCollection->fontFallbackEnabled();
    for (int spanIdx = 0; spanIdx < fStyles.size(); ++spanIdx) {
        const StyleSpan& span = fStyles[spanIdx];
        const CFRange cfRange = fIndexMap.utf8RangeToCFRange(span.fRange);
        if (cfRange.length <= 0) {
            continue;
        }
        if (spanIdx > 0) {
            set_style_group(attrStr.get(), cfRange, spanIdx);
        }
        const SpanFont spanFont = resolve_span_font(span.fStyle, *fFontCollection);
        const SpanFontAttrs attrs = make_span_font(span.fStyle, spanFont, fallbackDisabled);
        SkUniqueCFRef<CTFontRef> font =
                copy_with_attributes(attrs.fBase.get(), spanFont.fSize, attrs.fDescAttrs.get());
        CFAttributedStringSetAttribute(attrStr.get(), cfRange, kCTFontAttributeName, font.get());
        // Only a letter-spaced span has liga=0 to take back from its cursive runs
        if (span.fStyle.getLetterSpacing() != 0) {
            this->setCursiveRunsFont(attrStr.get(),
                                     span.fRange,
                                     attrs.fBase.get(),
                                     spanFont.fSize,
                                     attrs.fDescAttrs.get(),
                                     attrs.fUserFeatures.get());
        }
        set_language(attrStr.get(), cfRange, span.fStyle.getLocale());

        // Run::fBaselineShift is the sole carrier of the style's baselineShift: TextLine paints
        // each run at baseline + shift, so a kCTBaselineOffsetAttributeName (y-up) would land in
        // glyph fY with the opposite sign and cancel the visual shift
    }

    return SkUniqueCFRef<CFAttributedStringRef>(attrStr.release());
}

void CoreTextAttrStringBuilder::setCursiveRunsFont(CFMutableAttributedStringRef attrStr,
                                                   TextRange spanRange,
                                                   CTFontRef baseFont,
                                                   CGFloat fontSize,
                                                   CFDictionaryRef descAttrs,
                                                   CFArrayRef userFeatures) const {
    // Built on the first cursive run only, so letter-spaced Latin text pays nothing
    SkUniqueCFRef<CTFontRef> cursiveFont;
    for (const ScriptRun& run : fScriptRuns) {
        // Script runs ignore style spans, so only the part inside this span changes font
        const size_t start = std::max(run.fRange.start, spanRange.start);
        const size_t end = std::min(run.fRange.end, spanRange.end);
        if (start >= end || !Run::IsCursiveScript(run.fScript)) {
            continue;
        }
        if (!cursiveFont) {
            SkUniqueCFRef<CFMutableDictionaryRef> attrs(
                    CFDictionaryCreateMutableCopy(kCFAllocatorDefault, 0, descAttrs));
            // Cascade and variations stay; an empty feature list would still be a setting, so
            // the key goes away when the user gave no features
            if (userFeatures) {
                CFDictionarySetValue(attrs.get(), kCTFontFeatureSettingsAttribute, userFeatures);
            } else {
                CFDictionaryRemoveValue(attrs.get(), kCTFontFeatureSettingsAttribute);
            }
            cursiveFont = copy_with_attributes(baseFont, fontSize, attrs.get());
        }
        CFAttributedStringSetAttribute(attrStr,
                                       fIndexMap.utf8RangeToCFRange(TextRange(start, end)),
                                       kCTFontAttributeName,
                                       cursiveFont.get());
    }
}

}  // namespace textlayout
}  // namespace skia
