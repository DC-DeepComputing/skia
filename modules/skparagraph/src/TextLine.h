// Copyright 2019 Google LLC.
#ifndef TextLine_DEFINED
#define TextLine_DEFINED

#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkScalar.h"
#include "include/private/base/SkTArray.h"
#include "modules/skparagraph/include/DartTypes.h"
#include "modules/skparagraph/include/Metrics.h"
#include "modules/skparagraph/include/ParagraphPainter.h"
#include "modules/skparagraph/include/TextStyle.h"
#include "modules/skparagraph/src/Run.h"
#include "src/base/SkBitmaskEnum.h"

#include <stddef.h>
#include <functional>
#include <memory>
#include <vector>
#include <optional>

class SkString;

namespace skia {
namespace textlayout {

class ParagraphImpl;

class TextLine {
public:

    struct ClipContext {
      const Run* run;
      size_t pos;
      size_t size;
      SkScalar fTextShift; // Shifts the text inside the run so it's placed at the right position
      SkRect clip;
      SkScalar fExcludedTrailingSpaces;
      bool clippingNeeded;
    };

    /**
     * @struct RightHalf
     * @brief Represents right half of a text line when split for middle ellipsis mode.
     *
     * This structure is used in conjunction with `TextLine` to handle text
     * truncation with middle ellipsis. It stores information about one
     * half (right) of the text line when it needs to be split in the middle due to
     * exceeding the available width.
     *
     * This structure likely contains properties related to the text content and its
     * layout for right half of the line.
     */
    struct RightHalf {
        SkVector fAdvance;
        ClusterRange fClusterRange;
        ClusterRange fGhostClusterRange;
        TextRange fText;
        TextRange fTextIncludingNewlines;
        TextRange fTextExcludingSpaces;
    };

    enum TextAdjustment {
        GlyphCluster = 0x01,    // All text producing glyphs pointing to the same ClusterIndex
        GlyphemeCluster = 0x02, // base glyph + all attached diacritics
        Grapheme = 0x04,        // Text adjusted to graphemes
        GraphemeGluster = 0x05, // GlyphCluster & Grapheme
    };

    TextLine() = default;
    TextLine(const TextLine&) = delete;
    TextLine& operator=(const TextLine&) = delete;
    TextLine(TextLine&&) = default;
    TextLine& operator=(TextLine&&) = default;
    ~TextLine() = default;

    TextLine(ParagraphImpl* owner,
             SkVector offset,
             SkVector advance,
             BlockRange blocks,
             TextRange textExcludingSpaces,
             TextRange text,
             TextRange textIncludingNewlines,
             ClusterRange clusters,
             ClusterRange clustersWithGhosts,
             SkScalar widthWithSpaces,
             InternalLineMetrics sizes);

    TextRange trimmedText() const { return fTextExcludingSpaces; }
    TextRange textWithNewlines() const { return fTextIncludingNewlines; }
    TextRange text() const { return fText; }
    ClusterRange clusters() const { return fClusterRange; }
    ClusterRange clustersWithSpaces() const { return fGhostClusterRange; }
    Run* ellipsis() const { return fEllipsis.get(); }
    InternalLineMetrics sizes() const { return fSizes; }
    bool empty() const { return fTextExcludingSpaces.empty(); }

    SkScalar spacesWidth() const { return fWidthWithSpaces - width(); }
    SkScalar height() const { return fAdvance.fY; }
    SkScalar width() const {
        return fAdvance.fX + (fEllipsis != nullptr ? fEllipsis->fAdvance.fX : 0) + (this->rightHalf().has_value() ? this->rightHalf().value().fAdvance.fX: 0) ;
    }
    SkScalar widthWithoutEllipsis() const { return fAdvance.fX; }
    SkVector offset() const;

    SkScalar alphabeticBaseline() const { return fSizes.alphabeticBaseline(); }
    SkScalar ideographicBaseline() const { return fSizes.ideographicBaseline(); }
    SkScalar baseline() const { return fSizes.baseline(); }

    using RunVisitor = std::function<bool(
            const Run* run, SkScalar runOffset, TextRange textRange, SkScalar* width)>;
    void iterateThroughVisualRuns(bool includingGhostSpaces, const RunVisitor& runVisitor) const;
    using RunStyleVisitor = std::function<void(
            TextRange textRange, const TextStyle& style, const ClipContext& context)>;
    SkScalar iterateThroughSingleRunByStyles(TextAdjustment textAdjustment,
                                             const Run* run,
                                             SkScalar runOffset,
                                             TextRange textRange,
                                             StyleType styleType,
                                             const RunStyleVisitor& visitor) const;

    using ClustersVisitor = std::function<bool(const Cluster* cluster, ClusterIndex index, bool ghost)>;
    void iterateThroughClustersInGlyphsOrder(bool reverse,
                                             bool includeGhosts,
                                             const ClustersVisitor& visitor) const;

    void format(TextAlign align, SkScalar maxWidth);
    /**
     * @brief Creates an ellipsis for the text line based on the specified mode and width.
     *
     * This function applies ellipsis truncation to the text line if its width exceeds
     * the provided `maxWidth`. The type of ellipsis applied (head, middle, tail, or clip)
     * is determined by the `ellipsisMode` parameter.
     *
     * @param maxWidth [in] The maximum allowed width for the text line.
     * @param ellipsis [in] The string containing the ellipsis glyph(s) to use for truncation.
     * @param ellipsisMode [in] The ellipsis mode specifying how to handle overflow.
     */
    void createEllipsisWithMode(const SkScalar& maxWidth, const SkString& ellipsis, const EllipsisMode& ellipsisMode);
    std::optional<RightHalf> rightHalf() const{
        return fRightHalf;
    }
    void paint(ParagraphPainter* painter, SkScalar x, SkScalar y);
    void visit(SkScalar x, SkScalar y);
    void ensureTextBlobCachePopulated();

    void createEllipsis(SkScalar maxWidth, const SkString& ellipsis, bool ltr);

    // For testing internal structures
    void scanStyles(StyleType style, const RunStyleVisitor& visitor);

    void setMaxRunMetrics(const InternalLineMetrics& metrics) { fMaxRunMetrics = metrics; }
    InternalLineMetrics getMaxRunMetrics() const { return fMaxRunMetrics; }

    bool isFirstLine() const;
    bool isLastLine() const;
    void getRectsForRange(TextRange textRange,
                          RectHeightStyle rectHeightStyle,
                          RectWidthStyle rectWidthStyle,
                          std::vector<TextBox>& boxes) const;
    void getRectsForPlaceholders(std::vector<TextBox>& boxes);
    PositionWithAffinity getGlyphPositionAtCoordinate(SkScalar dx);

    ClipContext measureTextInsideOneRun(TextRange textRange,
                                        const Run* run,
                                        SkScalar runOffsetInLine,
                                        SkScalar textOffsetInRunInLine,
                                        bool includeGhostSpaces,
                                        TextAdjustment textAdjustment) const;

    LineMetrics getMetrics() const;

    SkRect extendHeight(const ClipContext& context) const;

    void shiftVertically(SkScalar shift) { fOffset.fY += shift; }

    void setAscentStyle(LineMetricStyle style) { fAscentStyle = style; }
    void setDescentStyle(LineMetricStyle style) { fDescentStyle = style; }

    bool endsWithHardLineBreak() const;

private:
    std::unique_ptr<Run> shapeEllipsis(const SkString& ellipsis, const Cluster* cluster);
    void justify(SkScalar maxWidth);

    void buildTextBlob(TextRange textRange, const TextStyle& style, const ClipContext& context);
    void paintBackground(ParagraphPainter* painter,
                         SkScalar x,
                         SkScalar y,
                         TextRange textRange,
                         const TextStyle& style,
                         const ClipContext& context) const;
    void paintShadow(ParagraphPainter* painter,
                     SkScalar x,
                     SkScalar y,
                     TextRange textRange,
                     const TextStyle& style,
                     const ClipContext& context) const;
    void paintDecorations(ParagraphPainter* painter,
                          SkScalar x,
                          SkScalar y,
                          TextRange textRange,
                          const TextStyle& style,
                          const ClipContext& context) const;

    void shiftCluster(const Cluster* cluster, SkScalar shift, SkScalar prevShift);

    ParagraphImpl* fOwner;
    BlockRange fBlockRange;
    TextRange fTextExcludingSpaces;
    TextRange fText;
    TextRange fTextIncludingNewlines;
    ClusterRange fClusterRange;
    ClusterRange fGhostClusterRange;
    // Avoid the malloc/free in the common case of one run per line
    skia_private::STArray<1, size_t, true> fRunsInVisualOrder;
    SkVector fAdvance;                  // Text size
    SkVector fOffset;                   // Text position
    SkScalar fShift;                    // Let right
    SkScalar fWidthWithSpaces;
    std::unique_ptr<Run> fEllipsis;     // In case the line ends with the ellipsis
    InternalLineMetrics fSizes;                 // Line metrics as a max of all run metrics and struts
    InternalLineMetrics fMaxRunMetrics;         // No struts - need it for GetRectForRange(max height)
    bool fHasBackground;
    bool fHasShadows;
    bool fHasDecorations;

    // Adding this optional RightHalf for middle ellipsis mode. Since in case of middle ellipsis,
    // text line is split into two halfs. So when we have middle ellipsis mode,
    // this will be treated as right half and other similar properties as left half values.
    std::optional<RightHalf> fRightHalf;

    /**
     * @brief Finds the cluster nearest to the middle of a given width.
     *
     * This function searches for a cluster among potentially many clusters and returns
     * the cluster that is closest to the horizontal middle of a specified width.
     *
     * @param width [in] The width to consider when finding the nearest cluster.
     * @return The cluster nearest to the middle of the provided width, or an empty cluster
     *         if no suitable cluster is found.
     */
    Cluster findClusterNearToMiddle(const SkScalar& width);


    /**
     * This function applies head ellipsis truncation to the text line when its width
     * exceeds the provided `maxWidth`. Head ellipsis displays the end portion of the text
     * that fits within the container and indicates omitted text at the beginning using
     * ellipsis glyphs (`...`).
     * This function is called from `createEllipsisWithMode` when the
     * `ellipsisMode` is set to `EllipsisMode::kHead`.
     */
    void createHeadEllipsis(const SkScalar& maxWidth, const SkString& ellipsis);

    /**
     * This function applies middle ellipsis truncation to the text line when its width
     * exceeds the provided `maxWidth`. Middle ellipsis displays the beginning and end
     * portions of the text that fit within the container, while indicating omitted text
     * in the middle using ellipsis glyphs (`...`).
     * This function is called from `createEllipsisWithMode` when the
     * `ellipsisMode` is set to `EllipsisMode::kMiddle`.
     */
    void createMiddleEllipsis(const SkScalar& maxWidth, const SkString& ellipsis);

    /**
     * This function applies clip ellipsis truncation to the text line. Clip ellipsis
     * simply cuts off any text that extends beyond the provided `maxWidth` without
     * using ellipsis glyphs (`...`).
     * This function is called from `createEllipsisWithMode` when the
     * `ellipsisMode` is set to `EllipsisMode::kClip`.
     */
    void createClipEllipsis(const SkScalar& maxWidth, const SkString& ellipsis);

    LineMetricStyle fAscentStyle;
    LineMetricStyle fDescentStyle;

    struct TextBlobRecord {
        void paint(ParagraphPainter* painter, SkScalar x, SkScalar y);

        sk_sp<SkTextBlob> fBlob;
        SkPoint fOffset = SkPoint::Make(0.0f, 0.0f);
        ParagraphPainter::SkPaintOrID fPaint;
        SkRect fBounds = SkRect::MakeEmpty();
        bool fClippingNeeded = false;
        SkRect fClipRect = SkRect::MakeEmpty();

        // Extra fields only used for the (experimental) visitor
        const Run* fVisitor_Run;
        size_t     fVisitor_Pos;
    };
    bool fTextBlobCachePopulated;
public:
    std::vector<TextBlobRecord> fTextBlobCache;
};
}  // namespace textlayout
}  // namespace skia

namespace sknonstd {
    template <> struct is_bitmask_enum<skia::textlayout::TextLine::TextAdjustment> : std::true_type {};
}  // namespace sknonstd

#endif  // TextLine_DEFINED

