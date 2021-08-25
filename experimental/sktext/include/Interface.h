// Copyright 2021 Google LLC.
#ifndef Interface_DEFINED
#define Interface_DEFINED
#include <string>
#include "experimental/sktext/include/Types.h"
#include "experimental/sktext/src/Line.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkFontMgr.h"
#include "include/core/SkFontStyle.h"
#include "include/core/SkPaint.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkTextBlob.h"
#include "modules/skshaper/include/SkShaper.h"
#include "modules/skunicode/include/SkUnicode.h"

using namespace skia::text;
namespace skia {
namespace API {

class LogicalRun;
class ShapedText;
class UnicodeText : public SkRefCnt {
public:
    std::unique_ptr<ShapedText> shape(SkSpan<FontBlock> blocks, TextDirection textDirection);

    UnicodeText(std::unique_ptr<SkUnicode> unicode, SkSpan<uint16_t> utf16);
    UnicodeText(std::unique_ptr<SkUnicode> unicode, const SkString& utf8);

    bool hasProperty(TextIndex index, CodeUnitFlags flag) const;
    bool isHardLineBreak(TextIndex index) const;
    bool isSoftLineBreak(TextIndex index) const;
    bool isWhitespaces(TextRange range) const;

    SkUnicode* getUnicode() const;
    SkSpan<const char16_t> getText16() const;
private:
    SkTArray<CodeUnitFlags, true> fCodeUnitProperties;
    std::u16string fText16;
    std::unique_ptr<SkUnicode> fUnicode;
};

class WrappedText;
class ShapedText {
public:
    std::unique_ptr<WrappedText> wrap(sk_sp<UnicodeText> unicodeText, float width, float height);
    ShapedText(sk_sp<UnicodeText> unicodeText, SkTArray<LogicalRun, false> logicalRuns);
private:
    sk_sp<UnicodeText> fUnicodeText;
    SkTArray<LogicalRun, false> fLogicalRuns;
};

class PhysicalRun;
class PhysicalLine;
class DrawableText;
class EditableText;
class WrappedText {
public:
    std::unique_ptr<DrawableText> prepareToDraw(TextAlign textAlign, TextDirection textDirection);
    std::unique_ptr<EditableText> prepareToEdit();
    void format(TextAlign textAlign, TextDirection textDirection);
    void decorate(SkSpan<TextIndex> chunks);
    SkSize actualSize() const { return fActualSize; }
    size_t countLines() const { return fPhysicalLines.size(); }
private:
    // The result of wrapping cut by lines and sorted by visual order
    SkTArray<PhysicalRun, true> fPhysicalRuns;
    SkTArray<PhysicalLine, false> fPhysicalLines;
    SkSize fActualSize;
};

class VisualRun;
class DrawableText {
public:
    class Visitor {
    public:
        virtual ~Visitor() = default;
        virtual void onBeginLine(TextRange lineText) {}
        virtual void onEndLine(TextRange lineText) {}
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
            fParent->fTextBlobs.emplace_back(builder.make());
        }
        virtual void onPlaceholder(TextRange, const SkRect& bounds) {}

        void buildTextBlobs(DrawableText* drawableText) {
            fParent = drawableText;
            fParent->fTextBlobs.clear();
            fParent->visit(this);
        }

        DrawableText* fParent;
    };

    void visit(WrappedText*, Visitor*) const;
    void visit((WrappedText*,Visitor*, SkSpan<TextIndex> blocks) const;
    std::vector<sk_sp<SkTextBlob>>& getTextBlobs() const;
private:
    // To draw and redraw
    std::vector<sk_sp<SkTextBlob>> fTextBlobs;
};

struct Position {
    Position(PositionType positionType, size_t lineIndex, const VisualRun* visualRun, GlyphRange glyphRange, TextRange textRange, SkRect rect);
    Position(PositionType positionType);
    PositionType fPositionType;
    size_t fLineIndex;
    // TODO: Move to indexes
    const VisualRun* fVisualRun;
    GlyphRange fGlyphRange;
    TextRange fTextRange;
    SkRect fBoundaries;
};

using VisualLine = PhysicalLine;
class VisualRun;
class SelectableText {
public:
    // Operations starting from mouse/cursor require repositioning and adjusting by the screen
    Position adjustedPosition(PositionType positionType, SkPoint point) const;

    // Operations of the text (adding, removing) require repositioning and adjusting by the text
    Position adjustedPosition(PositionType positionType, TextIndex textIndex) const;

    bool recalculateBoundaries(Position& position) const;

    const VisualRun* visuallyPreviousRun(size_t lineIndex, const VisualRun* run) const;
    const VisualRun* visuallyNextRun(size_t lineIndex, const VisualRun* run) const;
    const VisualRun* visuallyFirstRun(size_t lineIndex) const;
    const VisualRun* visuallyLastRun(size_t lineIndex) const;
    bool isVisuallyFirst(size_t lineIndex, const VisualRun* run) const;
    bool isVisuallyLast(size_t lineIndex, const VisualRun* run) const;

    Position previousElement(Position element) const;
    Position nextElement(Position current) const;
    Position firstElement(PositionType positionType) const;
    Position lastElement(PositionType positionType) const;

    bool isFirstOnTheLine(Position element) const;
    bool isLastOnTheLine(Position element) const;

    size_t countLines() const { return fVisualLines.size(); }
};

}  // namespace text
}  // namespace skia

#endif  // Processor_DEFINED
