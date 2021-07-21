// Copyright 2021 Google LLC.
#ifndef Processor_DEFINED
#define Processor_DEFINED
#include <string>
#include "experimental/sktext/include/Types.h"
#include "experimental/sktext/src/Line.h"
#include "experimental/sktext/src/TextRun.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkFontMgr.h"
#include "include/core/SkFontStyle.h"
#include "include/core/SkPaint.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkTextBlob.h"
#include "modules/skshaper/include/SkShaper.h"
#include "modules/skshaper/src/SkUnicode.h"

namespace skia {
namespace text {

const TextDirection DEFAULT_TEXT_DIRECTION = TextDirection::kLtr;
const TextAlign DEFAULT_TEXT_ALIGN = TextAlign::kLeft;
const SkColor DEFAULT_FOREGROUND = SK_ColorBLACK;
const SkScalar DEFAULT_CURSOR_WIDTH = 2;
const SkColor DEFAULT_CURSOR_COLOR = SK_ColorGRAY;
const SkColor DEFAULT_SELECTION_COLOR = SK_ColorCYAN;
const SkScalar DEFAULT_STATUS_HEIGHT = 20;

class UnicodeText;
class Text {
public:
    static std::unique_ptr<UnicodeText> parse(SkSpan<uint16_t> utf16);
};

class ShapedText;
class UnicodeText : public SkShaper::RunHandler {

public:
    std::unique_ptr<ShapedText> shape(SkSpan<Block> blocks,
                                      TextDirection textDirection);

    bool hasProperty(size_t index, CodeUnitFlags flag) {
        return (fCodeUnitProperties[index] & flag) == flag;
    }
    bool isHardLineBreak(size_t index) {
        return this->hasProperty(index, CodeUnitFlags::kHardLineBreakBefore);
    }
    bool isSoftLineBreak(size_t index) {
        return index != 0 && this->hasProperty(index, CodeUnitFlags::kSoftLineBreakBefore);
    }
    SkUnicode* getUnicode() const { return fUnicode.get(); }

private:
    friend class Text;
    UnicodeText() : fCurrentRun(nullptr) { }
    void beginLine() override {}
    void runInfo(const RunInfo&) override {}
    void commitRunInfo() override {}
    void commitLine() override {}
    void commitRunBuffer(const RunInfo&) override;
    Buffer runBuffer(const RunInfo& info) override {
        fCurrentRun = std::make_unique<TextRun>(info);
        return fCurrentRun->newRunBuffer();
    }

    SkFont createFont(const Block& block);

    SkTArray<size_t, true> fUTF16FromUTF8;
    SkTArray<CodeUnitFlags, true> fCodeUnitProperties;
    SkString fText8;
    std::unique_ptr<SkUnicode> fUnicode;
    std::unique_ptr<TextRun> fCurrentRun;
    std::unique_ptr<ShapedText> fShapedText;
};

class WrappedText;
class ShapedText {
public:
    std::unique_ptr<WrappedText> wrap(float width, float height, SkUnicode* unicode);
    bool isClusterEdge(size_t index) const {
        return (fGlyphUnitProperties[index] & GlyphUnitFlags::kGlyphClusterStart) == GlyphUnitFlags::kGlyphClusterStart;
    }
    void adjustLeft(size_t* index) const {
        SkASSERT(index != nullptr);
        while (*index != 0) {
            if (isClusterEdge(*index)) {
                return;
            }
            --index;
        }
    }
    void adjustRight(size_t* index) const {
        SkASSERT(index != nullptr);
        while (*index < this->fGlyphUnitProperties.size()) {
            if (isClusterEdge(*index)) {
                return;
            }
            ++index;
        }
    }
    bool hasProperty(size_t index, GlyphUnitFlags flag) {
        return (fGlyphUnitProperties[index] & flag) == flag;
    }
    bool isHardLineBreak(size_t index) {
        return this->hasProperty(index, GlyphUnitFlags::kHardLineBreakBefore);
    }
    bool isSoftLineBreak(size_t index) {
        return index != 0 && this->hasProperty(index, GlyphUnitFlags::kSoftLineBreakBefore);
    }
    bool isWhitespaces(TextRange range) {
        if (range.leftToRight()) {
            for (auto i = range.fStart; i < range.fEnd; ++i) {
                if (!this->hasProperty(i, GlyphUnitFlags::kPartOfWhiteSpace)) {
                    return false;
                }
            }
        } else {
            for (auto i = range.fStart; i > range.fEnd; --i) {
                if (!this->hasProperty(i, GlyphUnitFlags::kPartOfWhiteSpace)) {
                    return false;
                }
            }
        }
        return true;
}

private:
    friend class UnicodeText;
    ShapedText() { }
    SkTArray<TextRun, false> fRuns;
    SkTArray<GlyphUnitFlags, true> fGlyphUnitProperties;
};

class FormattedText;
class WrappedText {
public:
    sk_sp<FormattedText> format(TextAlign, TextDirection);
    SkSize size() const { return fSize; }
    size_t countLines() const { return fLines.size(); }
private:
    friend class ShapedText;
    WrappedText() : fSize(SkSize::MakeEmpty()) { }
    void addLine(Stretch& stretch, Stretch& spaces, SkUnicode* unicode);
    SkTArray<TextRun, false> fRuns;
    SkTArray<Line, false> fLines;
    SkTArray<GlyphUnitFlags, true> fGlyphUnitProperties;
    SkSize fSize;
};

class FormattedText : public SkRefCnt {
public:
    SkSize  size() const { return fSize; }

    Position adjustedPosition(PositionType positionType, SkPoint point) const;

    bool recalculateBoundaries(Position& position) const;

    const TextRun* visuallyPreviousRun(size_t lineIndex, const TextRun* run) const;
    const TextRun* visuallyNextRun(size_t lineIndex, const TextRun* run) const;
    const TextRun* visuallyFirstRun(size_t lineIndex) const;
    const TextRun* visuallyLastRun(size_t lineIndex) const;
    bool isVisuallyFirst(size_t lineIndex, const TextRun* run) const;
    bool isVisuallyLast(size_t lineIndex, const TextRun* run) const;

    Position previousElement(Position element) const;
    Position nextElement(Position current) const;
    Position firstElement(PositionType positionType) const;
    Position lastElement(PositionType positionType) const;

    size_t lineIndex(const Line* line) const { return line - fLines.data(); }
    size_t countLines() const { return fLines.size(); }
    const Line* line(size_t lineIndex) const { return fLines.empty() ? nullptr : &fLines[lineIndex]; }
    size_t runIndex(const TextRun* run) const { return run == nullptr ? EMPTY_INDEX : run - fRuns.data(); }

    bool hasProperty(size_t index, GlyphUnitFlags flag) const {
        return (fGlyphUnitProperties[index] & flag) == flag;
    }

    class Visitor {
    public:
        virtual ~Visitor() = default;
        virtual void onBeginLine(TextRange, float baselineY) = 0;
        virtual void onEndLine(TextRange, float baselineY) = 0;
        virtual void onGlyphRun(SkFont font,
                                TextRange textRange,
                                int glyphCount,
                                const uint16_t glyphs[],
                                const SkPoint  positions[],
                                const SkPoint offsets[]) = 0;
        virtual void onPlaceholder(TextRange, const SkRect& bounds) = 0;
    };

    // Visit runs as is by lines
    void visit(Visitor*) const;
    // Visit chunked runs
    void visit(Visitor*, SkSpan<size_t> blocks) const;

private:

    void adjustTextRange(Position* position) const;

    friend class WrappedText;
    FormattedText() { }
    SkTArray<TextRun, false> fRuns;
    SkTArray<Line, false> fLines;
    SkTArray<GlyphUnitFlags, true> fGlyphUnitProperties;
    SkSize fSize;
};

class SimpleText {
public:
    SimpleText(SkScalar width) {
        fSize = SkSize::Make(width, SK_ScalarInfinity);
        fText = u"";
        fBlocks = SkSpan<Block>(nullptr, 0);
        fUnicodeText = nullptr;
        fShapedText = nullptr;
        fWrappedText = nullptr;
        fFormattedText = nullptr;
    }

    SimpleText(std::u16string text, SkSize size, SkSpan<Block> fontBlocks, TextDirection textDirection, TextAlign textAlign) {
        fSize = size;
        fBlocks = fontBlocks;
        fText = std::move(text);
        fUnicodeText = skia::text::Text::parse(SkSpan<uint16_t>((uint16_t*)fText.data(), fText.size()));
        fShapedText = fUnicodeText->shape(fontBlocks, DEFAULT_TEXT_DIRECTION);
        fWrappedText = fShapedText->wrap(size.width(), size.height(), fUnicodeText->getUnicode());
        fFormattedText = fWrappedText->format(DEFAULT_TEXT_ALIGN, DEFAULT_TEXT_DIRECTION);
    }

    bool update(std::u16string text) {
        if (!this->fBlocks.empty()) {
            SkASSERT(this->fBlocks.size() == 1);
            this->fBlocks[0].charCount = text.size();
        }
        this->fText = std::move(text);
        this->fUnicodeText = skia::text::Text::parse(SkSpan<uint16_t>((uint16_t*)this->fText.data(), this->fText.size()));
        this->fShapedText = fUnicodeText->shape(this->fBlocks, DEFAULT_TEXT_DIRECTION);
        this->fWrappedText = fShapedText->wrap(this->fSize.fWidth, this->fSize.fHeight, this->fUnicodeText->getUnicode());
        this->fFormattedText = fWrappedText->format(DEFAULT_TEXT_ALIGN, DEFAULT_TEXT_DIRECTION);
        return true;
    }

    std::u16string fText;
    std::unique_ptr<UnicodeText> fUnicodeText;
    std::unique_ptr<ShapedText> fShapedText;
    std::unique_ptr<WrappedText> fWrappedText;
    sk_sp<FormattedText> fFormattedText;
    SkSize fSize;
    SkSpan<Block> fBlocks;
};

}  // namespace text
}  // namespace skia

#endif  // Processor_DEFINED
