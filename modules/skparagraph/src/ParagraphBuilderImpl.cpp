// Copyright 2019 Google LLC.

#include "include/core/SkTypes.h"
#include "modules/skparagraph/include/FontCollection.h"
#include "modules/skparagraph/include/Paragraph.h"
#include "modules/skparagraph/include/ParagraphBuilder.h"
#include "modules/skparagraph/include/ParagraphStyle.h"
#include "modules/skparagraph/include/TextStyle.h"
#include "modules/skparagraph/src/ParagraphBuilderImpl.h"
#include "modules/skparagraph/src/ParagraphImpl.h"

#include <algorithm>
#include <utility>
#include "src/core/SkStringUtils.h"

namespace skia {
namespace textlayout {

std::unique_ptr<ParagraphBuilder> ParagraphBuilder::make(
        const ParagraphStyle& style, sk_sp<FontCollection> fontCollection) {
    return ParagraphBuilderImpl::make(style, fontCollection);
}

std::unique_ptr<ParagraphBuilder> ParagraphBuilderImpl::make(
        const ParagraphStyle& style, sk_sp<FontCollection> fontCollection) {
    auto unicode = SkUnicode::Make();
    if (nullptr == unicode) {
        return nullptr;
    }
    return std::make_unique<ParagraphBuilderImpl>(style, fontCollection);
}

std::unique_ptr<ParagraphBuilder> ParagraphBuilderImpl::make(
        const ParagraphStyle& style, sk_sp<FontCollection> fontCollection, std::unique_ptr<SkUnicode> unicode) {
    if (nullptr == unicode) {
        return nullptr;
    }
    return std::make_unique<ParagraphBuilderImpl>(style, fontCollection, std::move(unicode));
}

ParagraphBuilderImpl::ParagraphBuilderImpl(
        const ParagraphStyle& style, sk_sp<FontCollection> fontCollection, std::unique_ptr<SkUnicode> unicode)
        : ParagraphBuilder(style, fontCollection)
        , fUtf8()
        , fFontCollection(std::move(fontCollection))
        , fUnicode(std::move(unicode))
        , fUniqueId(0) {
    SkASSERT(fUnicode);

    static size_t gUniqueId = 0;
    fUniqueId = (++gUniqueId);

    if (fFontCollection->getTextTrace()) {
        traceParagraphStyle(style);
        SkDebugf("ParagraphBuilderImpl %s(%s, %s);\n"
                 , uniqueName(ObjectType::Builder).c_str()
                 , uniqueName(ObjectType::ParagraphStyle).c_str()
                 , uniqueName(ObjectType::FontCollection).c_str());
    }

    this->setParagraphStyle(style);
}

ParagraphBuilderImpl::ParagraphBuilderImpl(
        const ParagraphStyle& style, sk_sp<FontCollection> fontCollection)
        : ParagraphBuilderImpl(style, fontCollection, SkUnicode::Make())
{ }

ParagraphBuilderImpl::~ParagraphBuilderImpl() = default;

void ParagraphBuilderImpl::setParagraphStyle(const ParagraphStyle& style) {
    fParagraphStyle = style;
    fTextStyles.push(fParagraphStyle.getTextStyle());
    fStyledBlocks.emplace_back(fUtf8.size(), fUtf8.size(), fParagraphStyle.getTextStyle());
    /*
    if (fFontCollection->getTextTrace()) {
        traceParagraphStyle(style);
        SkDebugf("%s.setParagraphStyle(%s);\n"
                 , uniqueName(ObjectType::Builder).c_str()
                 , uniqueName(ObjectType::ParagraphStyle).c_str());
    }
    */
}

void ParagraphBuilderImpl::pushStyle(const TextStyle& style) {
    this->endRunIfNeeded();

    fTextStyles.push(style);
    if (!fStyledBlocks.empty() && fStyledBlocks.back().fRange.end == fUtf8.size() &&
        fStyledBlocks.back().fStyle == style) {
        // Just continue with the same style
    } else {
        // Go with the new style
        fStyledBlocks.emplace_back(fUtf8.size(), fUtf8.size(), fTextStyles.top());
    }

    if (fFontCollection->getTextTrace()) {
        SkDebugf("{\n");
        this->traceTextStyle(style, false);
        SkDebugf("    %s.pushStyle(%s);\n"
                 , uniqueName(ObjectType::Builder).c_str()
                 , uniqueName(ObjectType::TextStyle).c_str());
        SkDebugf("}\n");
    }
}

void ParagraphBuilderImpl::pop() {
    this->endRunIfNeeded();

    if (fTextStyles.size() > 1) {
        fTextStyles.pop();
    } else {
        // In this case we use paragraph style and skip Pop operation
        SkDEBUGF("SkParagraphBuilder.Pop() called too many times.\n");
    }

    auto top = fTextStyles.top();
    fStyledBlocks.emplace_back(fUtf8.size(), fUtf8.size(), top);

    if (fFontCollection->getTextTrace()) {
        SkDebugf("%s.pop();\n"
                 , uniqueName(ObjectType::Builder).c_str());
    }
}

TextStyle ParagraphBuilderImpl::peekStyle() {
    this->endRunIfNeeded();

    if (!fTextStyles.empty()) {
        return fTextStyles.top();
    } else {
        SkDebugf("SkParagraphBuilder._styles is empty.\n");
        return fParagraphStyle.getTextStyle();
    }
}

void ParagraphBuilderImpl::addText(const std::u16string& text) {
    auto utf8 = fUnicode->convertUtf16ToUtf8(text);
    fUtf8.append(utf8);
    if (fFontCollection->getTextTrace()) {
        SkDebugf("%s.addText(%s);\n"
                 , uniqueName(ObjectType::Builder).c_str()
                 , ParagraphBuilderImpl::text(utf8.c_str(), utf8.size()).c_str());
    }
}

void ParagraphBuilderImpl::addText(const char* str) {
    fUtf8.append(str);
    if (fFontCollection->getTextTrace()) {
        SkDebugf("%s.addText(%s);\n"
                 , uniqueName(ObjectType::Builder).c_str()
                 , ParagraphBuilderImpl::text(str, strlen(str)).c_str());
    }
}

void ParagraphBuilderImpl::addText(const char* str, size_t len) {
    fUtf8.append(str, len);
    if (fFontCollection->getTextTrace()) {
        SkString modified = ParagraphBuilderImpl::text(str, len);
        SkDebugf("%s.addText(%s, %d);\n"
                 , uniqueName(ObjectType::Builder).c_str()
                 , modified.c_str(), modified.size());
    }
}

void ParagraphBuilderImpl::addPlaceholder(const PlaceholderStyle& placeholderStyle) {
    addPlaceholder(placeholderStyle, false);
    if (fFontCollection->getTextTrace()) {
        SkDebugf("{\n");
        tracePlaceholder(placeholderStyle);
        SkDebugf("%s.addPlaceholder(%s);\n"
                 , uniqueName(ObjectType::Builder).c_str()
                 , uniqueName(ObjectType::Placeholder).c_str());
        SkDebugf("}\n");
    }
}

void ParagraphBuilderImpl::addPlaceholder(const PlaceholderStyle& placeholderStyle, bool lastOne) {
    if (!fUtf8.isEmpty() && !lastOne) {
        // We keep the very last text style
        this->endRunIfNeeded();
    }

    BlockRange stylesBefore(fPlaceholders.empty() ? 0 : fPlaceholders.back().fBlocksBefore.end + 1,
                            fStyledBlocks.size());
    TextRange textBefore(fPlaceholders.empty() ? 0 : fPlaceholders.back().fRange.end,
                            fUtf8.size());
    auto start = fUtf8.size();
    auto topStyle = fTextStyles.top();
    if (!lastOne) {
        pushStyle(TextStyle(topStyle, true));
        addText(std::u16string(1ull, 0xFFFC));
        pop();
    }
    auto end = fUtf8.size();
    fPlaceholders.emplace_back(start, end, placeholderStyle, topStyle, stylesBefore, textBefore);
}

void ParagraphBuilderImpl::endRunIfNeeded() {
    if (fStyledBlocks.empty()) {
        return;
    }

    auto& last = fStyledBlocks.back();
    if (last.fRange.start == fUtf8.size()) {
        fStyledBlocks.pop_back();
    } else {
        last.fRange.end = fUtf8.size();
    }
}

std::unique_ptr<Paragraph> ParagraphBuilderImpl::Build() {
    if (!fUtf8.isEmpty()) {
        this->endRunIfNeeded();
    }

    if (fFontCollection->getTextTrace()) {
        SkDebugf("auto %s = %s.Build();\n"
                 , uniqueName(ObjectType::Paragraph).c_str()
                 , uniqueName(ObjectType::Builder).c_str());
    }

    // Add one fake placeholder with the rest of the text
    addPlaceholder(PlaceholderStyle(), true);
    return std::make_unique<ParagraphImpl>(
            fUtf8, fParagraphStyle, fStyledBlocks, fPlaceholders, fFontCollection, fUnicode, fUniqueId);
}

void ParagraphBuilderImpl::Reset() {
    std::stack<TextStyle> text_styles;
    fTextStyles.swap(text_styles);
    fUtf8.reset();
    fStyledBlocks.reset();
    fPlaceholders.reset();

    this->setParagraphStyle(fParagraphStyle);

    if (fFontCollection->getTextTrace()) {
        SkDebugf("%s.reset();\n"
                 , uniqueName(ObjectType::Builder).c_str());
    }
}

SkString ParagraphBuilderImpl::uniqueName(ObjectType type) const {
    const char* object = "UnknownType";
    switch (type) {
        case ObjectType::Paragraph: object = "paragraph"; break;
        case ObjectType::Builder: object = "builder"; break;
        case ObjectType::ParagraphStyle: object = "style"; break;
        case ObjectType::TextStyle: object = "textStyle"; break;
        case ObjectType::DefaultTextStyle: object = "defaultTextStyle"; break;
        case ObjectType::StrutStyle: object = "strutStyle"; break;
        case ObjectType::Placeholder: object = "placeholder"; break;
        case ObjectType::Background: object = "background"; break;
        case ObjectType::Foreground: object = "foreground"; break;
        case ObjectType::FontCollection: return SkString("fontCollection");
    }
    return SkStringPrintf("%s%d", object, fUniqueId);
}

void ParagraphBuilderImpl::traceParagraphStyle(const ParagraphStyle& style) const {
    SkDebugf("ParagraphStyle %s;\n"
             , uniqueName(ObjectType::ParagraphStyle).c_str());
    SkDebugf("{\n");
    traceStrutStyle(style.getStrutStyle());
    SkDebugf("    %s.setStrutStyle(%s);\n"
             , uniqueName(ObjectType::ParagraphStyle).c_str()
             , uniqueName(ObjectType::StrutStyle).c_str());
    SkDebugf("}\n");
    SkDebugf("{\n");
    traceTextStyle(style.getTextStyle(), true);
    SkDebugf("    %s.setTextStyle(%s);\n"
             , uniqueName(ObjectType::ParagraphStyle).c_str()
             , uniqueName(ObjectType::DefaultTextStyle).c_str());
    SkDebugf("}\n");
    ParagraphStyle defaultStyle;
    if (style.getTextAlign() != defaultStyle.getTextAlign()) {
        SkDebugf("%s.setTextAlign(%s);\n"
                 , uniqueName(ObjectType::ParagraphStyle).c_str()
                 , ParagraphBuilderImpl::toString(style.getTextAlign()));
    }
    if (style.getTextDirection() != defaultStyle.getTextDirection()) {
        SkDebugf("%s.setTextDirection(%s);\n"
                 , uniqueName(ObjectType::ParagraphStyle).c_str()
                 , ParagraphBuilderImpl::toString(style.getTextDirection()));
    }
    if (style.getMaxLines() != defaultStyle.getMaxLines()) {
        SkDebugf("%s.setMaxLines(%zu);\n"
                 , uniqueName(ObjectType::ParagraphStyle).c_str()
                 , style.getMaxLines());
    }
    if (style.ellipsized()) {
        SkDebugf("%s.setEllipsis(\"%s\");\n"
                 , uniqueName(ObjectType::ParagraphStyle).c_str()
                 , style.getEllipsis().c_str());
    }
    if (style.getHeight() != defaultStyle.getHeight()) {
        SkDebugf("%s.setHeight(%f);\n"
                 , uniqueName(ObjectType::ParagraphStyle).c_str()
                 , style.getHeight());
    }
    if (style.getTextHeightBehavior() != defaultStyle.getTextHeightBehavior()) {
        SkDebugf("%s.setTextHeightBehavior(%s);\n"
                 , uniqueName(ObjectType::ParagraphStyle).c_str()
                 , ParagraphBuilderImpl::toString(style.getTextHeightBehavior()));
    }
}

void ParagraphBuilderImpl::traceStrutStyle(const StrutStyle& style) const {
    SkDebugf("    StrutStyle %s;\n"
             , uniqueName(ObjectType::StrutStyle).c_str());

    SkDebugf("    %s.setFontFamilies({"
             , uniqueName(ObjectType::StrutStyle).c_str());
    for (auto& ff : style.getFontFamilies()) {
        SkDebugf("SkString(\"%s\")", ff.c_str());
    }
    SkDebugf("});\n");

    StrutStyle defaultStyle;
    if (!(style.getFontStyle() == defaultStyle.getFontStyle())) {
        SkDebugf("    %s.setFontStyle(%s);\n"
                 , uniqueName(ObjectType::StrutStyle).c_str()
                 , ParagraphBuilderImpl::toString(style.getFontStyle()));
    }
    if (!(style.getFontSize() == defaultStyle.getFontSize())) {
        SkDebugf("    %s.setFontSize(%f);\n"
                 , uniqueName(ObjectType::StrutStyle).c_str()
                 , style.getFontSize());
    }
    if (!(style.getHeight() == defaultStyle.getHeight())) {
        SkDebugf("    %s.setHeight(%f);\n"
                 , uniqueName(ObjectType::StrutStyle).c_str()
                 , style.getHeight());
    }
    if (!(style.getLeading() == defaultStyle.getLeading())) {
        SkDebugf("    %s.setLeading(%f);\n"
                 , uniqueName(ObjectType::StrutStyle).c_str()
                 , style.getLeading());
    }
    if (!(style.getForceStrutHeight() == defaultStyle.getForceStrutHeight())) {
        SkDebugf("    %s.setForceStrutHeight(%s);\n"
                 , uniqueName(ObjectType::StrutStyle).c_str()
                 , style.getForceStrutHeight() ? "true" : "false");
    }
    if (!(style.getStrutEnabled() == defaultStyle.getStrutEnabled())) {
        SkDebugf("    %s.setStrutEnabled(%s);\n"
                 , uniqueName(ObjectType::StrutStyle).c_str()
                 , style.getStrutEnabled() ? "true" : "false");
    }
    if (!(style.getHeightOverride() == defaultStyle.getHeightOverride())) {
        SkDebugf("    %s.setHeightOverride(%s);\n"
                 , uniqueName(ObjectType::StrutStyle).c_str()
                 , style.getHeightOverride() ? "true" : "false");
    }
    if (!(style.getHalfLeading() == defaultStyle.getHalfLeading())) {
        SkDebugf("    %s.setHalfLeading(%s);\n"
                 , uniqueName(ObjectType::StrutStyle).c_str()
                 , style.getHalfLeading() ? "true" : "false");
    }
}

void ParagraphBuilderImpl::traceTextStyle(const TextStyle& style, bool isDefault) const {
    SkString name(uniqueName(isDefault
                                ? ObjectType::DefaultTextStyle
                                : ObjectType::TextStyle).c_str());

    SkDebugf("    TextStyle %s;\n", name.c_str());
    TextStyle defaultTextStyle;
    if (!(style.getFontStyle() == defaultTextStyle.getFontStyle())) {
        SkDebugf("    %s.setFontStyle(%s);\n"
                 , name.c_str()
                 , ParagraphBuilderImpl::toString(style.getFontStyle()));
    }
    SkDebugf("    %s.setFontFamilies({"
             , name.c_str());
    for (auto& ff : style.getFontFamilies()) {
        SkDebugf("SkString(\"%s\")", ff.c_str());
    }
    SkDebugf("});\n");
    if (style.getFontSize() != defaultTextStyle.getFontSize()) {
        SkDebugf("    %s.setFontSize(%f);\n"
                 , name.c_str()
                 , style.getFontSize());
    }
    if (style.getHeight() != defaultTextStyle.getHeight()) {
        SkDebugf("    %s.setHeight(%f);\n"
                 , name.c_str()
                 , style.getHeight());
    }
    if (style.getHeightOverride() != defaultTextStyle.getHeightOverride()) {
        SkDebugf("%s.setHeightOverride(%s);\n"
                 , name.c_str()
                 , style.getHeightOverride() ? "true" : "false");
    }
    if (style.getBaselineShift() != defaultTextStyle.getBaselineShift()) {
        SkDebugf("    %s.setBaselineShift(%f);\n"
                 , name.c_str()
                 , style.getBaselineShift());
    }
    if (style.getHalfLeading() != defaultTextStyle.getHalfLeading()) {
        SkDebugf("    %s.setHalfLeading(%s);\n"
                 , name.c_str()
                 , style.getHalfLeading() ? "true" : "false");
    }
    if (style.getLocale() != defaultTextStyle.getLocale()) {
        SkDebugf("%s.setLocale(\"%s\");\n"
                 , name.c_str()
                 , style.getLocale().c_str());
    }
    if (style.getLetterSpacing() != defaultTextStyle.getLetterSpacing()) {
        SkDebugf("    %s.setLetterSpacing(%f);\n"
                 , name.c_str()
                 , style.getLetterSpacing());
    }
    if (style.getWordSpacing() != defaultTextStyle.getWordSpacing()) {
        SkDebugf("    %s.setWordSpacing(%f);\n"
                 , name.c_str()
                 , style.getWordSpacing());
    }
    if (style.getTextBaseline() != defaultTextStyle.getTextBaseline()) {
        SkDebugf("    %s.setTextBaseline(%s);\n"
                 , name.c_str()
                 , ParagraphBuilderImpl::toString(style.getTextBaseline()));
    }
    if (style.getColor() != defaultTextStyle.getColor()) {
        SkDebugf("    %s.setColor(%s);\n"
                 , name.c_str()
                 , this->colorToString(style.getColor()));
    }
    if (style.hasBackground() != defaultTextStyle.hasBackground()) {
        SkDebugf("    SkPaint %s(%s);\n"
                 , this->uniqueName(ObjectType::Background).c_str()
                 , this->colorToString(style.getBackground().getColor()));
        SkDebugf("    %s.setBackgroundColor(%s);\n"
                 , name.c_str()
                 , this->uniqueName(ObjectType::Background).c_str());
    }
    if (style.hasForeground() != defaultTextStyle.hasForeground()) {
        SkDebugf("    SkPaint %s(%s);\n"
                 , this->uniqueName(ObjectType::Foreground).c_str()
                 , this->colorToString(style.getForeground().getColor()));
        SkDebugf("    %s.setForegroundColor(%s);\n"
                 , name.c_str()
                 , this->uniqueName(ObjectType::Foreground).c_str());
    }
    for (auto& ff : style.getFontFeatures()) {
        SkDebugf("    %s.addFontFeature(SkString(\"%s\"), %d);\n"
                 , name.c_str()
                 , ff.fName.c_str()
                 , ff.fValue);
    }
}

void ParagraphBuilderImpl::tracePlaceholder(const PlaceholderStyle& style) const {
    SkDebugf("    PlaceholderStyle %s;\n"
             , uniqueName(ObjectType::Placeholder).c_str());

    PlaceholderStyle defaultStyle;
    if (!(style.fWidth == defaultStyle.fWidth)) {
        SkDebugf("    %s.fWidth = %f;\n"
                 , uniqueName(ObjectType::Placeholder).c_str()
                 , style.fWidth);
    }
    if (!(style.fHeight == defaultStyle.fHeight)) {
        SkDebugf("    %s.fHeight = %f;\n"
                 , uniqueName(ObjectType::Placeholder).c_str()
                 , style.fHeight);
    }
    if (!(style.fAlignment == defaultStyle.fAlignment)) {
        SkDebugf("    %s.fAlignment = %s;\n"
                 , uniqueName(ObjectType::Placeholder).c_str()
                 , ParagraphBuilderImpl::toString(style.fAlignment));
    }
    if (!(style.fBaseline == defaultStyle.fBaseline)) {
        SkDebugf("    %s.fBaseline = %s;\n"
                 , uniqueName(ObjectType::Placeholder).c_str()
                 , ParagraphBuilderImpl::toString(style.fBaseline));
    }
    if (!(style.fBaselineOffset == defaultStyle.fBaselineOffset)) {
        SkDebugf("    %s.fBaselineOffset = %f;\n"
                 , uniqueName(ObjectType::Placeholder).c_str()
                 , style.fBaselineOffset);
    }
}

const char* ParagraphBuilderImpl::toString(TextAlign textAlign) {
    const char* table[] = {
        "TextAlign::kLeft",
        "TextAlign::kRight",
        "TextAlign::kCenter",
        "TextAlign::kJustify",
        "TextAlign::kStart",
        "TextAlign::kEnd",
    };
    return table[(int)textAlign];
}

const char* ParagraphBuilderImpl::toString(TextDirection textDirection) {
    return textDirection == TextDirection::kLtr ? "TextDirection::kLtr" : "TextDirection::kRtr";
}

const char* ParagraphBuilderImpl::toString(TextHeightBehavior textHeightBehavior) {
    const char* table[] = {
        "TextHeightBehavior::kAll",
        "TextHeightBehavior::kDisableFirstAscent",
        "TextHeightBehavior::kDisableLastDescent",
        "TextHeightBehavior::kDisableAll",
    };
    return table[(int)textHeightBehavior];
}

const char* ParagraphBuilderImpl::toString(SkFontStyle fontStyle) {
    return SkStringPrintf("SkFontStyle(%d, %d, %s)"
                          , fontStyle.weight()
                          , fontStyle.width()
                          , fontStyle.slant() == SkFontStyle::Slant::kItalic_Slant
                            ? "SkFontStyle::Slant::kItalic_Slant"
                            : "SkFontStyle::Slant::kUpright_Slant").c_str();
}

const char* ParagraphBuilderImpl::toString(PlaceholderAlignment placeholderAlignment) {
    const char* table[] = {
        "PlaceholderAlignment::kBaseline",
        "PlaceholderAlignment::kAboveBaseline",
        "PlaceholderAlignment::kBelowBaseline",
        "PlaceholderAlignment::kTop",
        "PlaceholderAlignment::kBottom",
        "PlaceholderAlignment::kMiddle",
    };
    return table[(int)placeholderAlignment];
}

const char* ParagraphBuilderImpl::toString(TextBaseline textBaseline) {
    return textBaseline == TextBaseline::kAlphabetic
            ? "TextBaseline::kAlphabetic"
            : "TextBaseline::kIdeographic";
}

const char* ParagraphBuilderImpl::colorToString(SkColor color) {

    switch (color) {
        case SK_ColorTRANSPARENT: return "SkColors::kTransient";
        case SK_ColorBLACK: return "SkColors::kBlack";
        case SK_ColorDKGRAY: return "SkColors::kDkGray";
        case SK_ColorGRAY: return "SkColors::kGray";
        case SK_ColorLTGRAY: return "SkColors::kLtGray";
        case SK_ColorWHITE: return "SkColors::kWhite";
        case SK_ColorRED: return "SkColors::kRed";
        case SK_ColorGREEN: return "SkColors::kGreen";
        case SK_ColorBLUE: return "SkColors::kBlue";
        case SK_ColorYELLOW: return "SkColors::kYellow";
        case SK_ColorCYAN: return "SkColors::kCyan";
        case SK_ColorMAGENTA: return "SkColors::kMagenta";
        default:
            return SkStringPrintf("0x%x", color).c_str();
    }
}

SkString ParagraphBuilderImpl::text(const char* str, size_t len) {
    SkTArray<SkString> lines;
    SkStrSplit(SkString(str, len).c_str(), "\n", kStrict_SkStrSplitMode, &lines);
    SkString result;
    if (lines.size() == 1) {
        result.append("\"");
        result.append(lines[0]);
        result.append("\"");
        return result;
    } else if (lines.size() > 1) {
        bool first = true;
        for (auto& l : lines) {
            result.append("\"");
            if (!first) result.append("\\" "n");
            result.append(l);
            result.append("\"");
            first = false;
        }
        return result;
    } else {
        return SkString("\"\"");
    }
}


}  // namespace textlayout
}  // namespace skia
