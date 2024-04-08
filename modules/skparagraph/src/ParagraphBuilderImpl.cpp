// Copyright 2019 Google LLC.

#include "include/core/SkRefCnt.h"
#include "include/core/SkTypes.h"
#include "modules/skparagraph/include/FontCollection.h"
#include "modules/skparagraph/include/Paragraph.h"
#include "modules/skparagraph/include/ParagraphBuilder.h"
#include "modules/skparagraph/include/ParagraphStyle.h"
#include "modules/skparagraph/include/TextStyle.h"
#include "modules/skparagraph/src/ParagraphBuilderImpl.h"
#include "modules/skparagraph/src/ParagraphImpl.h"
#include "modules/skunicode/include/SkUnicode.h"
#include "modules/skunicode/utils/FactoryHelpers.h"
#include <memory>
#include <utility>
#include "src/core/SkStringUtils.h"

namespace skia {
namespace textlayout {

std::unique_ptr<ParagraphBuilder> ParagraphBuilder::make(const ParagraphStyle& style,
                                                             sk_sp<FontCollection> fontCollection) {
    return std::make_unique<ParagraphBuilderImpl>(style, std::move(fontCollection));
}

std::unique_ptr<ParagraphBuilder> ParagraphBuilderImpl::make(const ParagraphStyle& style,
                                                             sk_sp<FontCollection> fontCollection) {
    return std::make_unique<ParagraphBuilderImpl>(style, std::move(fontCollection));
}

std::unique_ptr<ParagraphBuilder> ParagraphBuilderImpl::make(const ParagraphStyle& style,
                                                             sk_sp<FontCollection> fontCollection,
                                                             sk_sp<SkUnicodes::Factory> factory) {
    if (nullptr == factory) {
        return nullptr;
    }
    return std::make_unique<ParagraphBuilderImpl>(style,
                                                  std::move(fontCollection),
                                                  std::move(factory));
}

ParagraphBuilderImpl::ParagraphBuilderImpl(
        const ParagraphStyle& style, sk_sp<FontCollection> fontCollection, sk_sp<SkUnicodes::Factory> factory)
        : ParagraphBuilder()
        , fUtf8()
        , fFontCollection(std::move(fontCollection))
        , fParagraphStyle(style)
        , fUnicodeFactory(std::move(factory))
        , fTextIsFinalized(false)
{
    SkASSERT(fFontCollection);
    startStyledBlock();
}

ParagraphBuilderImpl::ParagraphBuilderImpl(
        const ParagraphStyle& style, sk_sp<FontCollection> fontCollection)
      : ParagraphBuilderImpl(style, fontCollection, SkUnicodes::BestAvailable()) {
    SkASSERT(fFontCollection);
    this->startStyledBlock();
}

ParagraphBuilderImpl::~ParagraphBuilderImpl() = default;

void ParagraphBuilderImpl::pushStyle(const TextStyle& style) {
    fTextStyles.push_back(style);
    if (!fStyledBlocks.empty() && fStyledBlocks.back().fRange.end == fUtf8.size() &&
        fStyledBlocks.back().fStyle == style) {
        // Just continue with the same style
    } else {
        // Go with the new style
        startStyledBlock();
    }
}

void ParagraphBuilderImpl::pop() {
    if (!fTextStyles.empty()) {
        fTextStyles.pop_back();
    } else {
        // In this case we use paragraph style and skip Pop operation
        SkDEBUGF("SkParagraphBuilder.Pop() called too many times.\n");
    }

    this->startStyledBlock();
}

const TextStyle& ParagraphBuilderImpl::internalPeekStyle() {
    if (fTextStyles.empty()) {
        return fParagraphStyle.getTextStyle();
    } else {
        return fTextStyles.back();
    }
}

TextStyle ParagraphBuilderImpl::peekStyle() {
    return this->internalPeekStyle();
}

void ParagraphBuilderImpl::addText(const std::u16string& text) {
    auto utf8 = SkUnicode::convertUtf16ToUtf8(text);
    fUtf8.append(utf8);
}

void ParagraphBuilderImpl::addText(const char* text) {
    fUtf8.append(text);
}

void ParagraphBuilderImpl::addText(const char* text, size_t len) {
    fUtf8.append(text, len);
}

void ParagraphBuilderImpl::addPlaceholder(const PlaceholderStyle& placeholderStyle) {
    addPlaceholder(placeholderStyle, false);
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
    auto topStyle = internalPeekStyle();
    if (!lastOne) {
        pushStyle(topStyle.cloneForPlaceholder());
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

void ParagraphBuilderImpl::startStyledBlock() {
    endRunIfNeeded();
    fStyledBlocks.emplace_back(fUtf8.size(), fUtf8.size(), internalPeekStyle());
}

void ParagraphBuilderImpl::finalize() {
    if (fTextIsFinalized) {
        return;
    }
    if (!fUtf8.isEmpty()) {
        this->endRunIfNeeded();
    }

    fTextIsFinalized = true;
}

std::unique_ptr<Paragraph> ParagraphBuilderImpl::Build() {
    this->finalize();
    // Add one fake placeholder with the rest of the text
    this->addPlaceholder(PlaceholderStyle(), true);

    fUTF8IndexForUTF16Index.clear();
    fUTF16IndexForUTF8Index.clear();

    return std::make_unique<ParagraphImpl>(
        fUtf8,
        fParagraphStyle,
        fStyledBlocks,
        fPlaceholders,
        fFontCollection,
        fUnicodeFactory->MakeUnicode());
}

SkSpan<char> ParagraphBuilderImpl::getText() {
    this->finalize();
    return SkSpan<char>(fUtf8.isEmpty() ? nullptr : fUtf8.data(), fUtf8.size());
}

const ParagraphStyle& ParagraphBuilderImpl::getParagraphStyle() const {
    return fParagraphStyle;
}

void ParagraphBuilderImpl::ensureUTF16Mapping() {
    fillUTF16MappingOnce([&] {
        SkUnicode::extractUtfConversionMapping(
                this->getText(),
                [&](size_t index) { fUTF8IndexForUTF16Index.emplace_back(index); },
                [&](size_t index) { fUTF16IndexForUTF8Index.emplace_back(index); });
    });
}

void ParagraphBuilderImpl::setWordsUtf8(std::vector<SkUnicode::Position> wordsUtf8) {
    fUnicodeFactory->setWordsUtf8(wordsUtf8);
}

void ParagraphBuilderImpl::setWordsUtf16(std::vector<SkUnicode::Position> wordsUtf16) {
    this->ensureUTF16Mapping();
    std::vector<SkUnicode::Position> wordsUtf8;
    for (SkUnicode::Position indexUtf16: wordsUtf16) {
        wordsUtf8.emplace_back(fUTF8IndexForUTF16Index[indexUtf16]);
    }
    fUnicodeFactory->setGraphemeBreaksUtf8(std::move(wordsUtf8));
}

void ParagraphBuilderImpl::setGraphemeBreaksUtf8(std::vector<SkUnicode::Position> graphemeBreaksUtf8) {
    fUnicodeFactory->setGraphemeBreaksUtf8(std::move(graphemeBreaksUtf8));
}

void ParagraphBuilderImpl::setGraphemeBreaksUtf16(std::vector<SkUnicode::Position> graphemeBreaksUtf16) {
    this->ensureUTF16Mapping();
    std::vector<SkUnicode::Position> graphemeBreaksUtf8;
    for (SkUnicode::Position indexUtf16: graphemeBreaksUtf16) {
        graphemeBreaksUtf8.emplace_back(fUTF8IndexForUTF16Index[indexUtf16]);
    }
    fUnicodeFactory->setGraphemeBreaksUtf8(std::move(graphemeBreaksUtf8));
}

void ParagraphBuilderImpl::setLineBreaksUtf8(std::vector<SkUnicode::LineBreakBefore> lineBreaksUtf8) {
    fUnicodeFactory->setLineBreaksUtf8(std::move(lineBreaksUtf8));
}

void ParagraphBuilderImpl::setLineBreaksUtf16(std::vector<SkUnicode::LineBreakBefore> lineBreaksUtf16) {
    this->ensureUTF16Mapping();
    std::vector<SkUnicode::LineBreakBefore> lineBreaksUtf8;
    for (SkUnicode::LineBreakBefore lineBreakUtf16: lineBreaksUtf16) {
        lineBreaksUtf8.emplace_back(SkUnicode::LineBreakBefore(
                fUTF8IndexForUTF16Index[lineBreakUtf16.pos], lineBreakUtf16.breakType));
    }
    fUnicodeFactory->setLineBreaksUtf8(std::move(lineBreaksUtf8));
}

void ParagraphBuilderImpl::Reset() {

    fTextStyles.clear();
    fUtf8.reset();
    fStyledBlocks.clear();
    fPlaceholders.clear();
    fUTF8IndexForUTF16Index.clear();
    fUTF16IndexForUTF8Index.clear();
    fUnicodeFactory->clearData();
    fTextIsFinalized = false;
    startStyledBlock();
}

}  // namespace textlayout
}  // namespace skia
