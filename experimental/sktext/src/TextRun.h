// Copyright 2021 Google LLC.
#ifndef TextRun_DEFINED
#define TextRun_DEFINED

#include "experimental/sktext/include/Types.h"
#include "modules/skshaper/include/SkShaper.h"

namespace skia {
namespace text {

class Position;
class TextRun {
 public:
  TextRun(const SkShaper::RunHandler::RunInfo& info);
  SkShaper::RunHandler::Buffer newRunBuffer();
  void commit();

  SkScalar calculateWidth(GlyphRange glyphRange) const;
  SkScalar calculateWidth(GlyphIndex start, GlyphIndex end) const {
      return calculateWidth(GlyphRange(start, end));
  }

  bool leftToRight() const { return fBidiLevel % 2 == 0; }
  uint8_t bidiLevel() const { return fBidiLevel; }
  GlyphIndex findGlyph(TextIndex textIndex) const;
  size_t size() const { return fGlyphs.size(); }

 private:
  friend class UnicodeText;
  friend class ShapedText;
  friend class FormattedText;
  SkFont fFont;

  SkVector fAdvance;
  SkShaper::RunHandler::Range fUtf8Range;
  TextRange fUtf16Range;
  SkSTArray<128, SkGlyphID, true> fGlyphs;
  SkSTArray<128, SkPoint, true> fPositions;
  SkSTArray<128, SkPoint, true> fOffsets;
  SkSTArray<128, uint32_t, true> fClusters;
  SkSTArray<128, SkRect, true> fBounds;

  uint8_t fBidiLevel;
};

// This is a input/output range within one run
class Line;
struct Position {
    Position(PositionType positionType, size_t lineIndex, const TextRun* run, GlyphRange glyphRange, TextRange textRange, SkRect rect)
        : fPositionType(positionType)
        , fLineIndex(lineIndex)
        , fRun(run)
        , fGlyphRange(glyphRange)
        , fTextRange(textRange)
        , fBoundaries(rect) { }

    Position(PositionType positionType)
        : Position(positionType, EMPTY_INDEX, nullptr, EMPTY_RANGE, EMPTY_RANGE, SkRect::MakeEmpty()) { }

    PositionType fPositionType;
    size_t fLineIndex;
    const TextRun* fRun;
    GlyphRange fGlyphRange;
    TextRange fTextRange;
    SkRect fBoundaries;
};
} // namespace text
} // namespace skia
#endif
