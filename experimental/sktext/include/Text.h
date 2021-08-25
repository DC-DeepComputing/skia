// Copyright 2021 Google LLC.
#ifndef Processor_DEFINED
#define Processor_DEFINED
#include <string>
#include "experimental/sktext/include/Types.h"
#include "experimental/sktext/src/Line.h"
#include "experimental/sktext/src/LogicalRun.h"
#include "experimental/sktext/src/VisualRun.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkFontMgr.h"
#include "include/core/SkFontStyle.h"
#include "include/core/SkPaint.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkTextBlob.h"
#include "modules/skshaper/include/SkShaper.h"
#include "modules/skunicode/include/SkUnicode.h"

namespace skia {
namespace text {

class ShapedText;
class UnicodeText {
public:
    std::unique_ptr<ShapedText> shape(SkSpan<FontBlock> blocks, TextDirection textDirection);

    UnicodeText(std::unique_ptr<SkUnicode> unicode, SkSpan<uint16_t> utf16)
        : fUnicode(std::move(unicode))
        , fText16(std::u16string((char16_t*)utf16.data(), utf16.size())) {
        SkASSERT(unicode);
        initialize(utf16);
    }

    UnicodeText(std::unique_ptr<SkUnicode> unicode, const SkString& utf8)
        : fUnicode(std::move(unicode)) {
        SkASSERT(unicode);
        std::u16string utf16 = unicode->convertUtf8ToUtf16(utf8);
        initialize(SkSpan<uint16_t>((uint16_t*)utf16.data(), utf16.size()));
    }

    bool hasProperty(TextIndex index, CodeUnitFlags flag) const {
        return (fCodeUnitProperties[index] & flag) == flag;
    }
    bool isHardLineBreak(TextIndex index) const {
        return this->hasProperty(index, CodeUnitFlags::kHardLineBreakBefore);
    }
    bool isSoftLineBreak(TextIndex index) const {
        return index != 0 && this->hasProperty(index, CodeUnitFlags::kSoftLineBreakBefore);
    }
    bool isWhitespaces(TextRange range) const {
        if (range.leftToRight()) {
            for (auto i = range.fStart; i < range.fEnd; ++i) {
                if (!this->hasProperty(i, CodeUnitFlags::kPartOfWhiteSpace)) {
                    return false;
                }
            }
        } else {
            for (auto i = range.fStart; i > range.fEnd; --i) {
                if (!this->hasProperty(i, CodeUnitFlags::kPartOfWhiteSpace)) {
                    return false;
                }
            }
        }
        return true;
    }

    SkUnicode* getUnicode() const { return fUnicode.get(); }
    SkSpan<const char16_t> getText16() const { return SkSpan<const char16_t>(fText16.data(), fText16.size());}

private:
    ~UnicodeText() = default;
    void initialize(SkSpan<uint16_t> utf16);

    SkTArray<CodeUnitFlags, true> fCodeUnitProperties;
    std::u16string fText16;
    std::unique_ptr<SkUnicode> fUnicode;
};

class WrappedText;
class ShapedText : public SkShaper::RunHandler {
public:
    std::unique_ptr<WrappedText> wrap(UnicodeText* unicodeText, float width, float height);

    ShapedText()
    : fCurrentRun(nullptr)
    , fParagraphTextStart(0)
    , fRunGlyphStart(0.0f)
    , fTextHeight(0.0f) { }

    void beginLine() override {}
    void runInfo(const RunInfo&) override {}
    void commitRunInfo() override {}
    void commitLine() override {}
    void commitRunBuffer(const RunInfo&) override {
        fCurrentRun->commit();
        fLogicalRuns.emplace_back(std::move(*fCurrentRun));
        fRunGlyphStart += fCurrentRun->width();
    }
    Buffer runBuffer(const RunInfo& info) override {
        fCurrentRun = std::make_unique<LogicalRun>(info, fParagraphTextStart, fRunGlyphStart);
        return fCurrentRun->newRunBuffer();
    }
private:
    friend class UnicodeText;

    void addLine(WrappedText* wrappedText, SkUnicode* unicode, Stretch& stretch, Stretch& spaces, bool hardLineBreak);

    TextIndex fParagraphTextStart;
    SkScalar fRunGlyphStart;
    std::unique_ptr<LogicalRun> fCurrentRun;
    SkTArray<LogicalRun, false> fLogicalRuns;
    SkScalar fTextHeight;
};

class Visitor {
public:
    virtual ~Visitor() = default;
    virtual void onBeginLine(size_t index, TextRange lineText) { }
    virtual void onEndLine(size_t index, TextRange lineText, GlyphPos whitespaces) { }
    virtual void onGlyphRun(const SkFont& font,
                            TextRange textRange,
                            SkRect boundingRect,
                            int glyphCount,
                            int trailingSpacesStart,
                            const uint16_t glyphs[],
                            const SkPoint positions[]) { }
    virtual void onPlaceholder(TextRange, const SkRect& bounds) { }
};

class DrawableText;
class SelectableText;
class WrappedText {
public:
    std::unique_ptr<DrawableText> prepareToDraw(PositionType positionType, SkSpan<TextIndex> blocks) const;
    std::unique_ptr<SelectableText> prepareToEdit(UnicodeText* unicodeText) const;
    void format(TextAlign textAlign, TextDirection textDirection);
    void decorate(UnicodeText* unicodeText, SkSpan<TextIndex> chunks);
    SkSize actualSize() const { return fActualSize; }
    size_t countLines() const { return fVisualLines.size(); }

    void visit(Visitor*) const;
    void visit(Visitor*, PositionType positionType, SkSpan<TextIndex> blocks) const;

    GlyphRange textToGlyphs(PositionType positionType, TextRange textRange) const;

private:
    friend class ShapedText;
    WrappedText() : fActualSize(SkSize::MakeEmpty()), fAligned(TextAlign::kNothing) { }
    SkTArray<VisualRun, true> fVisualRuns;    // Broken by lines
    SkTArray<VisualLine, false> fVisualLines;
    SkSize fActualSize;
    TextAlign fAligned;
};

class DrawableText : public Visitor {
public:

    virtual void onGlyphRun(const SkFont& font,
                            TextRange textRange,
                            SkRect boundingRect,
                            int glyphCount,
                            const uint16_t glyphs[],
                            const SkPoint positions[],
                            const SkPoint offsets[]) {
        SkTextBlobBuilder builder;
        const auto& blobBuffer = builder.allocRunPos(font, SkToInt(glyphCount));
        sk_careful_memcpy(blobBuffer.glyphs, glyphs, glyphCount * sizeof(uint16_t));
        sk_careful_memcpy(blobBuffer.points(), positions, glyphCount * sizeof(SkPoint));
        fTextBlobs.emplace_back(builder.make());
    }

    std::vector<sk_sp<SkTextBlob>>& getTextBlobs() { return fTextBlobs; }

    DrawableText() = default;

private:
    std::vector<sk_sp<SkTextBlob>> fTextBlobs;
};

struct Position {
    Position(PositionType positionType, size_t lineIndex, GlyphRange glyphRange, TextRange textRange, SkRect rect)
        : fPositionType(positionType)
        , fLineIndex(lineIndex)
        , fGlyphRange(glyphRange)
        , fTextRange(textRange)
        , fBoundaries(rect) { }

    Position(PositionType positionType)
        : Position(positionType, EMPTY_INDEX, EMPTY_RANGE, EMPTY_RANGE, SkRect::MakeEmpty()) { }

    PositionType fPositionType;
    size_t fLineIndex;
    GlyphRange fGlyphRange;
    TextRange fTextRange;
    SkRect fBoundaries;
};

class SelectableText {
public:
    // Find the glyph by the screen point and adjust it by positionType
    Position adjustedPosition(PositionType positionType, SkPoint point) const;

    // Find the glyph by the text index and adjust it by positionType
    Position adjustedPosition(PositionType positionType, TextIndex textIndex) const;

    Position previousElement(Position element) const;
    Position nextElement(Position current) const;
    Position firstElement(PositionType positionType) const;
    Position lastElement(PositionType positionType) const;

    bool isFirstOnTheLine(Position element) const {
        return (element.fGlyphRange.fStart == 0);
    }
    bool isLastOnTheLine(Position element) const {
        return (element.fGlyphRange.fEnd == fBoxLines.back().fBoxGlyphs.size());
    }

    size_t countLines() const { return fBoxLines.size(); }

    bool hasProperty(TextIndex index, GlyphUnitFlags flag) const {
        return (fGlyphUnitProperties[index] & flag) == flag;
    }

    // Just in theory a random glyph range can be represented by multiple text ranges (because of LTR/RTL)
    std::vector<TextRange> glyphsToText(PositionType positionType, GlyphRange glyphRange) const;

private:

    struct BoxLine {
        SkTArray<SkRect, true> fBoxGlyphs;
        SkTArray<GlyphIndex, true> fGlyphByText;
        SkTArray<TextIndex, true> fTextByGlyph; // by glyph cluster
        GlyphIndex fTextEnd;
        GlyphIndex fTrailingSpacesEnd;
        TextRange fTextRange;
        SkRect fBounds;
        size_t fIndex;

    };

    Position findPosition(PositionType positionType, const BoxLine& line, TextIndex textIndex) const;
    Position findPosition(PositionType positionType, const BoxLine& line, SkPoint point) const;

    SelectableText() = default;
    SkTArray<BoxLine, true> fBoxLines;
    SkTArray<GlyphUnitFlags, true> fGlyphUnitProperties;
    SkSize fActualSize;
};

}  // namespace text
}  // namespace skia

#endif  // Processor_DEFINED
