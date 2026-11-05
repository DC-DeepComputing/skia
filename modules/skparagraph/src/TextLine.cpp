// Copyright 2019 Google LLC

#include "modules/skparagraph/src/TextLine.h"

#include "include/core/SkBlurTypes.h"
#include "include/core/SkFont.h"
#include "include/core/SkFontArguments.h"
#include "include/core/SkFontMetrics.h"
#include "include/core/SkFontParameters.h"
#include "include/core/SkMaskFilter.h"
#include "include/core/SkPaint.h"
#include "include/core/SkSpan.h"
#include "include/core/SkString.h"
#include "include/core/SkTextBlob.h"
#include "include/core/SkTypeface.h"
#include "include/core/SkTypes.h"
#include "include/private/SkTemplates.h"
#include "include/private/SkTo.h"
#include "modules/skparagraph/include/DartTypes.h"
#include "modules/skparagraph/include/Metrics.h"
#include "modules/skparagraph/include/ParagraphPainter.h"
#include "modules/skparagraph/include/ParagraphStyle.h"
#include "modules/skparagraph/include/TextShadow.h"
#include "modules/skparagraph/include/TextStyle.h"
#include "modules/skparagraph/src/Decorations.h"
#include "modules/skparagraph/src/ParagraphImpl.h"
#include "modules/skparagraph/src/ParagraphPainterImpl.h"
#include "modules/skshaper/include/SkShaper.h"
#include "modules/skshaper/include/SkShaper_harfbuzz.h"
#include "modules/skshaper/include/SkShaper_skunicode.h"
#include "modules/skunicode/include/SkUnicode.h"
#include "src/core/SkUTF.h"

#include <algorithm>
#include <iterator>
#include <limits>
#include <map>
#include <memory>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

using namespace skia_private;

namespace skia {
namespace textlayout {

namespace {

// Whether `text` resolves to a single bidi run when its base direction is `dir`.
bool isSingleBidiRun(SkUnicode* unicode, const SkString& text, SkUnicode::TextDirection dir) {
    std::vector<SkUnicode::BidiRegion> regions;
    return unicode->getBidiRegions(text.c_str(), SkToInt(text.size()), dir, &regions) &&
           regions.size() <= 1;
}

// TODO: deal with all the intersection functionality
TextRange intersected(const TextRange& a, const TextRange& b) {
    if (a.start == b.start && a.end == b.end) return a;
    auto begin = std::max(a.start, b.start);
    auto end = std::min(a.end, b.end);
    return end >= begin ? TextRange(begin, end) : EMPTY_TEXT;
}

SkScalar littleRound(SkScalar a) {
    // This rounding is done to match Flutter tests. Must be removed..
  return SkScalarRoundToScalar(a * 100.0)/100.0;
}

TextRange operator*(const TextRange& a, const TextRange& b) {
    if (a.start == b.start && a.end == b.end) return a;
    auto begin = std::max(a.start, b.start);
    auto end = std::min(a.end, b.end);
    return end > begin ? TextRange(begin, end) : EMPTY_TEXT;
}

int compareRound(SkScalar a, SkScalar b, bool applyRoundingHack) {
    // There is a rounding error that gets bigger when maxWidth gets bigger
    // VERY long zalgo text (> 100000) on a VERY long line (> 10000)
    // Canvas scaling affects it
    // Letter spacing affects it
    // It has to be relative to be useful
    auto base = std::max(SkScalarAbs(a), SkScalarAbs(b));
    auto diff = SkScalarAbs(a - b);
    if (nearlyZero(base) || diff / base < 0.001f) {
        return 0;
    }

    auto ra = a;
    auto rb = b;

    if (applyRoundingHack) {
        ra = littleRound(a);
        rb = littleRound(b);
    }
    if (ra < rb) {
        return -1;
    } else {
        return 1;
    }
}

bool shouldIsolateCluster(const ParagraphImpl* owner, const Cluster* cluster) {
    int graphemes = 0;
    for (size_t i = cluster->textRange().start; i < cluster->textRange().end; ++i) {
        if (owner->codeUnitHasProperty(i, SkUnicode::CodeUnitFlags::kGraphemeStart)) {
            ++graphemes;
        }
    }
    return graphemes <= 1;
}

std::pair<size_t, size_t> isolateClusterGlyphs(const ParagraphImpl* owner,
                                               const Run* run,
                                               const Cluster* cluster,
                                               TextRange originalTextRange,
                                               bool isStart) {
    const char* textStr = owner->text().data();
    const char* ptr = textStr + cluster->textRange().start;
    const char* endPtr = textStr + cluster->textRange().end;
    if (cluster->size() > 1 &&
        cluster->size() == SkToSizeT(SkUTF::CountUTF8(ptr, cluster->textRange().width()))) {
        size_t g0 = cluster->endPos();
        size_t g1 = cluster->startPos();
        for (size_t k = 0; ptr < endPtr; ++k) {
            size_t idx = ptr - textStr;
            SkUTF::NextUTF8(&ptr, endPtr);
            if (idx < originalTextRange.start || idx >= originalTextRange.end) {
                continue;
            }
            size_t g = run->leftToRight() ? (cluster->startPos() + k)
                                          : (cluster->endPos() - 1 - k);
            g0 = std::min(g0, g);
            g1 = std::max(g1, g + 1);
        }
        if (g0 < g1) {
            return {g0, g1};
        }
    } else if (cluster->textRange().start >= originalTextRange.start) {
        return {cluster->startPos(), cluster->endPos()};
    }
    size_t fallback =
            (isStart == run->leftToRight()) ? cluster->endPos() : cluster->startPos();
    return {fallback, fallback};
}

// Reads big-endian scalars from raw TrueType tables without unaligned pointer casts
uint16_t readU16BE(const uint8_t* p) {
    return static_cast<uint16_t>((uint16_t(p[0]) << 8) | uint16_t(p[1]));
}

int16_t readS16BE(const uint8_t* p) {
    return static_cast<int16_t>(readU16BE(p));
}

uint32_t readU32BE(const uint8_t* p) {
    return (uint32_t(p[0]) << 24) | (uint32_t(p[1]) << 16) | (uint32_t(p[2]) << 8) |
           uint32_t(p[3]);
}

}  // namespace

TextLine::TextLine(ParagraphImpl* owner,
                   SkVector offset,
                   SkVector advance,
                   BlockRange blocks,
                   TextRange textExcludingSpaces,
                   TextRange text,
                   TextRange textIncludingNewlines,
                   ClusterRange clusters,
                   ClusterRange clustersWithGhosts,
                   SkScalar widthWithSpaces,
                   InternalLineMetrics sizes)
        : fOwner(owner)
        , fBlockRange(blocks)
        , fTextExcludingSpaces(textExcludingSpaces)
        , fText(text)
        , fTextIncludingNewlines(textIncludingNewlines)
        , fClusterRange(clusters)
        , fGhostClusterRange(clustersWithGhosts)
        , fRunsInVisualOrder()
        , fAdvance(advance)
        , fOffset(offset)
        , fShift(0.0)
        , fWidthWithSpaces(widthWithSpaces)
        , fEllipsis(nullptr)
        , fSizes(sizes)
        , fHasBackground(false)
        , fHasShadows(false)
        , fHasDecorations(false)
        , fAscentStyle(LineMetricStyle::CSS)
        , fDescentStyle(LineMetricStyle::CSS)
        , fTextBlobCachePopulated(false) {
    // Reorder visual runs
    auto& start = owner->cluster(fGhostClusterRange.start);
    auto& end = owner->cluster(fGhostClusterRange.end - 1);
    size_t numRuns = end.runIndex() - start.runIndex() + 1;

    for (BlockIndex index = fBlockRange.start; index < fBlockRange.end; ++index) {
        auto b = fOwner->styles().begin() + index;
        if (b->fStyle.hasBackground()) {
            fHasBackground = true;
        }
        if (b->fStyle.getDecorationType() != TextDecoration::kNoDecoration) {
            fHasDecorations = true;
        }
        if (b->fStyle.getShadowNumber() > 0) {
            fHasShadows = true;
        }
    }

    // Get the logical order

    // This is just chosen to catch the common/fast cases. Feel free to tweak.
    constexpr int kPreallocCount = 4;
    AutoSTArray<kPreallocCount, SkUnicode::BidiLevel> runLevels(numRuns);
    size_t runLevelsIndex = 0;
    for (auto runIndex = start.runIndex(); runIndex <= end.runIndex(); ++runIndex) {
        auto& run = fOwner->run(runIndex);
        runLevels[runLevelsIndex++] = run.fBidiLevel;
        fMaxRunMetrics.add(
            InternalLineMetrics(run.correctAscent(), run.correctDescent(), run.fFontMetrics.fLeading));
    }
    SkASSERT(runLevelsIndex == numRuns);

    AutoSTArray<kPreallocCount, int32_t> logicalOrder(numRuns);

    // TODO: hide all these logic in SkUnicode?
    fOwner->getUnicode()->reorderVisual(runLevels.data(), numRuns, logicalOrder.data());
    auto firstRunIndex = start.runIndex();
    for (auto index : logicalOrder) {
        fRunsInVisualOrder.push_back(firstRunIndex + index);
    }

    // TODO: This is the fix for flutter. Must be removed...
    for (auto cluster = &start; cluster <= &end; ++cluster) {
        if (!cluster->run().isPlaceholder() && !cluster->run().isCursiveScript()) {
            fShift += cluster->getHalfLetterSpacing();
            break;
        }
    }
}

void TextLine::paint(ParagraphPainter* painter, SkScalar x, SkScalar y) {
    if (fHasBackground) {
        this->iterateThroughVisualRuns(false,
            [painter, x, y, this]
            (const Run* run, SkScalar runOffsetInLine, TextRange textRange, SkScalar* runWidthInLine) {
                *runWidthInLine = this->iterateThroughSingleRunByStyles(
                TextAdjustment::GlyphCluster, run, runOffsetInLine, textRange, StyleType::kBackground,
                [painter, x, y, this](TextRange textRange, const TextStyle& style, const ClipContext& context) {
                    this->paintBackground(painter, x, y, textRange, style, context);
                });
            return true;
            });
    }

    if (fHasShadows) {
        this->iterateThroughVisualRuns(false,
            [painter, x, y, this]
            (const Run* run, SkScalar runOffsetInLine, TextRange textRange, SkScalar* runWidthInLine) {
            *runWidthInLine = this->iterateThroughSingleRunByStyles(
                TextAdjustment::GlyphCluster, run, runOffsetInLine, textRange, StyleType::kShadow,
                [painter, x, y, this]
                (TextRange textRange, const TextStyle& style, const ClipContext& context) {
                    this->paintShadow(painter, x, y, textRange, style, context);
                });
            return true;
            });
    }

    this->ensureTextBlobCachePopulated();

    for (auto& record : fTextBlobCache) {
        record.paint(painter, x, y);
    }

    if (fHasDecorations) {
        this->iterateThroughVisualRuns(false,
            [painter, x, y, this]
            (const Run* run, SkScalar runOffsetInLine, TextRange textRange, SkScalar* runWidthInLine) {
                *runWidthInLine = this->iterateThroughSingleRunByStyles(
                TextAdjustment::GlyphCluster, run, runOffsetInLine, textRange, StyleType::kDecorations,
                [painter, x, y, this]
                (TextRange textRange, const TextStyle& style, const ClipContext& context) {
                    this->paintDecorations(painter, x, y, textRange, style, context);
                });
                return true;
        });
    }
}

void TextLine::ensureTextBlobCachePopulated() {
    if (fTextBlobCachePopulated) {
        return;
    }
    if (fBlockRange.width() == 1 && fRunsInVisualOrder.size() == 1 && fEllipsis == nullptr &&
        fHyphen == nullptr && fOwner->run(fRunsInVisualOrder[0]).placeholderStyle() == nullptr) {
        if (fClusterRange.width() == 0) {
            return;
        }
        // Most common and most simple case
        const auto& style = fOwner->block(fBlockRange.start).fStyle;
        const auto& run = fOwner->run(fRunsInVisualOrder[0]);
        auto clip = SkRect::MakeXYWH(0.0f, this->sizes().runTop(&run, this->fAscentStyle),
                                     fAdvance.fX,
                                     run.calculateHeight(this->fAscentStyle, this->fDescentStyle));

        auto& start = fOwner->cluster(fClusterRange.start);
        auto& end = fOwner->cluster(fClusterRange.end - 1);
        SkASSERT(start.runIndex() == end.runIndex());
        GlyphRange glyphs;
        if (run.leftToRight()) {
            glyphs = GlyphRange(start.startPos(),
                                end.isHardBreak() ? end.startPos() : end.endPos());
        } else {
            glyphs = GlyphRange(end.startPos(),
                                start.isHardBreak() ? start.startPos() : start.endPos());
        }
        ClipContext context = {/*run=*/&run,
                               /*pos=*/glyphs.start,
                               /*size=*/glyphs.width(),
                               /*fTextShift=*/-run.positionX(glyphs.start), // starting position
                               /*clip=*/clip,                               // entire line
                               /*fExcludedTrailingSpaces=*/0.0f,            // no need for that
                               /*clippingNeeded=*/false};                   // no need for that
        this->buildTextBlob(fTextExcludingSpaces, style, context);
    } else {
        this->iterateThroughVisualRuns(false,
           [this](const Run* run,
                  SkScalar runOffsetInLine,
                  TextRange textRange,
                  SkScalar* runWidthInLine) {
               if (run->placeholderStyle() != nullptr) {
                   *runWidthInLine = run->advance().fX;
                   return true;
               }
               *runWidthInLine = this->iterateThroughSingleRunByStyles(
                   TextAdjustment::GlyphCluster,
                   run,
                   runOffsetInLine,
                   textRange,
                   StyleType::kForeground,
                   [this](TextRange textRange, const TextStyle& style, const ClipContext& context) {
                       this->buildTextBlob(textRange, style, context);
                   });
               return true;
           });
    }
    fTextBlobCachePopulated = true;
}

void TextLine::format(TextAlign align, SkScalar maxWidth) {
    SkScalar delta = maxWidth - this->width();
    if (delta <= 0) {
        return;
    }

    // We do nothing for left align
    if (align == TextAlign::kJustify) {
        if (!this->endsWithHardLineBreak()) {
            this->justify(maxWidth);
        } else if (fOwner->paragraphStyle().getTextDirection() == TextDirection::kRtl) {
            // Justify -> Right align
            fShift = delta;
        }
    } else if (align == TextAlign::kRight) {
        fShift = delta;
    } else if (align == TextAlign::kCenter) {
        fShift = delta / 2;
    }
}

void TextLine::scanStyles(StyleType styleType, const RunStyleVisitor& visitor) {
    if (this->empty()) {
        return;
    }

    this->iterateThroughVisualRuns(
            false,
            [this, visitor, styleType](
                    const Run* run, SkScalar runOffset, TextRange textRange, SkScalar* width) {
                *width = this->iterateThroughSingleRunByStyles(
                        TextAdjustment::GlyphCluster,
                        run,
                        runOffset,
                        textRange,
                        styleType,
                        [visitor](TextRange textRange,
                                  const TextStyle& style,
                                  const ClipContext& context) {
                            visitor(textRange, style, context);
                        });
                return true;
            });
}

SkRect TextLine::extendHeight(const ClipContext& context) const {
    SkRect result = context.clip;
    result.fBottom += std::max(this->fMaxRunMetrics.height() - this->height(), 0.0f);
    return result;
}

// Decomposes a 1-glyph single-grapheme cluster into TrueType glyf composite sub-glyphs mapped to
// visible code points, caching the result on the line so paint and shadow share one parse
int TextLine::decomposeClusterComponents(const Run* run, const Cluster* cluster) const {
    if (cluster->size() != 1 || cluster->isHardBreak() || cluster->isSoftHyphen() ||
        !shouldIsolateCluster(fOwner, cluster)) {
        return -1;
    }
    ClusterIndex clusterIdx = fOwner->clusterIndex(cluster->textRange().start);
    for (int i = 0; i < fDecomposedClusters.size(); ++i) {
        if (fDecomposedClusters[i].fClusterIndex == clusterIdx) {
            return fDecomposedClusters[i].fValid ? i : -1;
        }
    }
    int entryIdx = fDecomposedClusters.size();
    fDecomposedClusters.push_back({clusterIdx, false, {}});

    SkUnicode* unicode = fOwner->getUnicode().get();
    if (!unicode) {
        return -1;
    }
    constexpr int kMaxComponents = 8;
    struct VisibleCP {
        SkUnichar fCP;
        TextIndex fStart;
    };
    STArray<kMaxComponents, VisibleCP, true> visibleCPs;
    const char* textBase = fOwner->text().data();
    const char* ptr = textBase + cluster->textRange().start;
    const char* endPtr = textBase + cluster->textRange().end;
    while (ptr < endPtr) {
        TextIndex cpStart = SkToSizeT(ptr - textBase);
        SkUnichar cp = SkUTF::NextUTF8(&ptr, endPtr);
        if (cp < 0 || unicode->isRegionalIndicator(cp)) {
            return -1;
        }
        // Skip hidden joiners, variation selectors, skin-tone modifiers, and controls so
        // 1-codepoint composites like '±' + ZWJ do not masquerade as base + diacritic
        if (unicode->isDefaultIgnorable(cp) || unicode->isControl(cp) ||
            unicode->isEmojiModifier(cp)) {
            continue;
        }
        if (visibleCPs.size() >= kMaxComponents) {
            return -1;
        }
        visibleCPs.push_back({cp, cpStart});
    }
    if (visibleCPs.size() < 2) {
        return -1;
    }

    const SkTypeface* typeface = run->font().getTypeface();
    if (!typeface) {
        return -1;
    }
    // Color fonts draw multi-layer palettes or bitmaps rather than monochrome glyf components
    if (typeface->getTableSize(SkSetFourByteTag('C', 'O', 'L', 'R')) > 0 ||
        typeface->getTableSize(SkSetFourByteTag('S', 'V', 'G', ' ')) > 0 ||
        typeface->getTableSize(SkSetFourByteTag('s', 'b', 'i', 'x')) > 0 ||
        typeface->getTableSize(SkSetFourByteTag('C', 'B', 'D', 'T')) > 0) {
        return -1;
    }
    // Non-default variable-font instances can move composite components via gvar deltas
    int posCount = typeface->getVariationDesignPosition({});
    if (posCount > 0) {
        AutoSTArray<8, SkFontArguments::VariationPosition::Coordinate> coords(posCount);
        int paramCount = typeface->getVariationDesignParameters({});
        if (paramCount <= 0 || typeface->getVariationDesignPosition(coords) != posCount) {
            return -1;
        }
        AutoSTArray<8, SkFontParameters::Variation::Axis> axes(paramCount);
        if (typeface->getVariationDesignParameters(axes) != paramCount) {
            return -1;
        }
        for (int i = 0; i < posCount; ++i) {
            bool atDefault = false;
            for (int j = 0; j < paramCount; ++j) {
                if (axes[j].tag == coords[i].axis) {
                    atDefault = SkScalarNearlyEqual(coords[i].value, axes[j].def);
                    break;
                }
            }
            if (!atDefault) {
                return -1;
            }
        }
    } else if (posCount < 0 && typeface->getTableSize(SkSetFourByteTag('g', 'v', 'a', 'r')) > 0) {
        return -1;
    }

    int upem = typeface->getUnitsPerEm();
    int numGlyphs = typeface->countGlyphs();
    if (upem <= 0 || numGlyphs <= 0) {
        return -1;
    }
    SkGlyphID compositeGid = run->glyphs()[cluster->startPos()];
    if (compositeGid == 0 || compositeGid >= SkToUInt(numGlyphs)) {
        return -1;
    }
    uint8_t headFmt[2];
    if (typeface->getTableData(SkSetFourByteTag('h', 'e', 'a', 'd'), 50, 2, headFmt) != 2) {
        return -1;
    }
    int16_t locaFmt = readS16BE(headFmt);
    if (locaFmt != 0 && locaFmt != 1) {
        return -1;
    }
    constexpr SkFontTableTag kLocaTag = SkSetFourByteTag('l', 'o', 'c', 'a');
    constexpr SkFontTableTag kGlyfTag = SkSetFourByteTag('g', 'l', 'y', 'f');
    const size_t locaSize = typeface->getTableSize(kLocaTag);
    const size_t glyfSize = typeface->getTableSize(kGlyfTag);
    const size_t requiredLoca = (SkToSizeT(numGlyphs) + 1) * (locaFmt == 0 ? 2 : 4);
    if (glyfSize == 0 || locaSize < requiredLoca) {
        return -1;
    }

    // Read small loca/glyf slices into stack buffers to avoid heap-copying full font tables
    auto readGlyphSpan = [&](SkGlyphID gid, uint32_t* outOff0, uint32_t* outOff1) -> bool {
        if (gid == 0 || gid >= SkToUInt(numGlyphs)) {
            return false;
        }
        if (locaFmt == 0) {
            uint8_t pair[4];
            if (typeface->getTableData(kLocaTag, size_t(gid) * 2, 4, pair) != 4) {
                return false;
            }
            *outOff0 = uint32_t(readU16BE(pair)) * 2;
            *outOff1 = uint32_t(readU16BE(pair + 2)) * 2;
        } else {
            uint8_t pair[8];
            if (typeface->getTableData(kLocaTag, size_t(gid) * 4, 8, pair) != 8) {
                return false;
            }
            *outOff0 = readU32BE(pair);
            *outOff1 = readU32BE(pair + 4);
        }
        return *outOff0 < *outOff1 && *outOff1 <= glyfSize && *outOff1 - *outOff0 >= 10;
    };

    auto readGlyphHeader = [&](SkGlyphID gid,
                               int16_t* outContours,
                               int16_t* outYMin,
                               int16_t* outYMax) -> bool {
        uint32_t off0 = 0;
        uint32_t off1 = 0;
        uint8_t hdr[10];
        if (!readGlyphSpan(gid, &off0, &off1) ||
            typeface->getTableData(kGlyfTag, off0, sizeof(hdr), hdr) != sizeof(hdr)) {
            return false;
        }
        *outContours = readS16BE(hdr);
        if (outYMin) {
            *outYMin = readS16BE(hdr + 4);
        }
        if (outYMax) {
            *outYMax = readS16BE(hdr + 8);
        }
        return true;
    };

    auto isCompositeGlyph = [&](SkGlyphID gid) -> bool {
        int16_t contours = 0;
        return readGlyphHeader(gid, &contours, nullptr, nullptr) && contours < 0;
    };

    struct RawComp {
        SkGlyphID fGlyphId;
        uint16_t fFlags;
        int32_t fDx;
        int32_t fDy;
    };
    constexpr uint16_t kArg1And2AreWords = 0x0001;
    constexpr uint16_t kArgsAreXYValues = 0x0002;
    constexpr uint16_t kWeHaveAScale = 0x0008;
    constexpr uint16_t kMoreComponents = 0x0020;
    constexpr uint16_t kWeHaveAnXAndYScale = 0x0040;
    constexpr uint16_t kWeHaveATwoByTwo = 0x0080;
    constexpr uint16_t kUseMyMetrics = 0x0200;

    auto parseComposite = [&](SkGlyphID gid,
                              STArray<kMaxComponents, RawComp, true>* out) -> bool {
        uint32_t off0 = 0;
        uint32_t off1 = 0;
        if (!readGlyphSpan(gid, &off0, &off1)) {
            return false;
        }
        uint8_t glyfBuf[10 + kMaxComponents * 8];
        const size_t sliceLen = std::min<size_t>(off1 - off0, sizeof(glyfBuf));
        if (typeface->getTableData(kGlyfTag, off0, sliceLen, glyfBuf) != sliceLen ||
            readS16BE(glyfBuf) >= 0) {
            return false;
        }
        size_t pos = 10;
        for (int step = 0; step < kMaxComponents; ++step) {
            if (pos + 4 > sliceLen) {
                return false;
            }
            uint16_t flags = readU16BE(glyfBuf + pos);
            SkGlyphID subGid = readU16BE(glyfBuf + pos + 2);
            pos += 4;
            if (subGid == 0 || subGid >= SkToUInt(numGlyphs)) {
                return false;
            }
            // SkTextBlob positions glyphs by (x, y) translation only, so reject point-matched
            // or scaled/sheared sub-glyphs
            if ((flags & kArgsAreXYValues) == 0 ||
                (flags & (kWeHaveAScale | kWeHaveAnXAndYScale | kWeHaveATwoByTwo)) != 0) {
                return false;
            }
            int32_t dx = 0;
            int32_t dy = 0;
            if ((flags & kArg1And2AreWords) != 0) {
                if (pos + 4 > sliceLen) {
                    return false;
                }
                dx = readS16BE(glyfBuf + pos);
                dy = readS16BE(glyfBuf + pos + 2);
                pos += 4;
            } else {
                if (pos + 2 > sliceLen) {
                    return false;
                }
                dx = static_cast<int8_t>(glyfBuf[pos]);
                dy = static_cast<int8_t>(glyfBuf[pos + 1]);
                pos += 2;
            }
            out->push_back({subGid, flags, dx, dy});
            if ((flags & kMoreComponents) == 0) {
                return true;
            }
        }
        return false;
    };

    STArray<kMaxComponents, RawComp, true> comps;
    if (!parseComposite(compositeGid, &comps)) {
        return -1;
    }
    if (comps.size() < visibleCPs.size()) {
        STArray<kMaxComponents, RawComp, true> flatComps;
        bool overflow = false;
        for (int i = 0; i < comps.size(); ++i) {
            const RawComp& c = comps[i];
            STArray<kMaxComponents, RawComp, true> subComps;
            if (c.fGlyphId != compositeGid && parseComposite(c.fGlyphId, &subComps)) {
                if (flatComps.size() + subComps.size() > kMaxComponents) {
                    overflow = true;
                    break;
                }
                bool anySubMetrics = false;
                for (int j = 0; j < subComps.size(); ++j) {
                    anySubMetrics |= (subComps[j].fFlags & kUseMyMetrics) != 0;
                }
                for (int j = 0; j < subComps.size(); ++j) {
                    uint16_t mergedFlags = subComps[j].fFlags;
                    if (!anySubMetrics && j == 0 && (c.fFlags & kUseMyMetrics) != 0) {
                        mergedFlags |= kUseMyMetrics;
                    }
                    flatComps.push_back(
                            {subComps[j].fGlyphId, mergedFlags, c.fDx + subComps[j].fDx,
                             c.fDy + subComps[j].fDy});
                }
            } else {
                if (flatComps.size() >= kMaxComponents) {
                    overflow = true;
                    break;
                }
                flatComps.push_back(c);
            }
        }
        if (!overflow && flatComps.size() == visibleCPs.size()) {
            comps = std::move(flatComps);
        }
    }
    if (comps.size() != visibleCPs.size()) {
        return -1;
    }

    int baseCompIdx = -1;
    SkGlyphID baseCmap = typeface->unicharToGlyph(visibleCPs[0].fCP);
    // Soft-dotted 'i' and 'j' replace their base glyph with dotless 'ı' (U+0131) or 'ȷ' (U+0237)
    SkGlyphID altBaseCmap = 0;
    if (visibleCPs[0].fCP == 'i') {
        altBaseCmap = typeface->unicharToGlyph(0x0131);
    } else if (visibleCPs[0].fCP == 'j') {
        altBaseCmap = typeface->unicharToGlyph(0x0237);
    }
    int matches = 0;
    for (int i = 0; i < comps.size(); ++i) {
        SkGlyphID gid = comps[i].fGlyphId;
        if ((baseCmap != 0 && gid == baseCmap) || (altBaseCmap != 0 && gid == altBaseCmap)) {
            baseCompIdx = i;
            ++matches;
        }
    }
    if (matches != 1) {
        baseCompIdx = -1;
        int candIdx = -1;
        int metricMatches = 0;
        for (int i = 0; i < comps.size(); ++i) {
            if ((comps[i].fFlags & kUseMyMetrics) != 0) {
                candIdx = i;
                ++metricMatches;
            }
        }
        if (metricMatches != 1) {
            candIdx = -1;
        }
        if (candIdx < 0 && comps.size() == 2 && comps[0].fGlyphId != comps[1].fGlyphId &&
            comps[0].fDx == 0 && comps[0].fDy == 0 && (comps[1].fDx != 0 || comps[1].fDy != 0)) {
            candIdx = 0;
        }
        // Reject heuristic base selection when either visibleCPs[0] or candIdx is itself a
        // composite (e.g. precomposed 'ẹ' + U+0302 -> nested [gid('ê'), gid(dotbelow)])
        if (candIdx >= 0 && (baseCmap == 0 || !isCompositeGlyph(baseCmap)) &&
            !isCompositeGlyph(comps[candIdx].fGlyphId)) {
            baseCompIdx = candIdx;
        }
    }
    if (baseCompIdx < 0) {
        return -1;
    }

    int markToComp[kMaxComponents];
    bool compUsed[kMaxComponents] = {};
    for (int i = 0; i < kMaxComponents; ++i) {
        markToComp[i] = -1;
    }
    compUsed[baseCompIdx] = true;

    if (comps.size() == 2) {
        markToComp[1] = 1 - baseCompIdx;
    } else {
        // Fonts often store spacing modifier clones (e.g. U+02C6 for U+0302) as composite parts
        auto markGlyphCandidates = [&](SkUnichar cp, SkGlyphID out[3]) -> int {
            int n = 0;
            auto addCp = [&](SkUnichar c) {
                if (c == 0) return;
                SkGlyphID g = typeface->unicharToGlyph(c);
                if (g != 0) {
                    for (int k = 0; k < n; ++k) {
                        if (out[k] == g) return;
                    }
                    out[n++] = g;
                }
            };
            addCp(cp);
            switch (cp) {
                case 0x0300: addCp(0x0060); addCp(0x02CB); break;
                case 0x0301: addCp(0x00B4); addCp(0x02CA); break;
                case 0x0302: addCp(0x02C6); addCp(0x005E); break;
                case 0x0303: addCp(0x02DC); addCp(0x007E); break;
                case 0x0304: addCp(0x00AF); addCp(0x02C9); break;
                case 0x0306: addCp(0x02D8); break;
                case 0x0307: addCp(0x02D9); break;
                case 0x0308: addCp(0x00A8); break;
                case 0x030A: addCp(0x02DA); addCp(0x00B0); break;
                case 0x030B: addCp(0x02DD); break;
                case 0x030C: addCp(0x02C7); break;
                case 0x0327: addCp(0x00B8); break;
                case 0x0328: addCp(0x02DB); break;
                default: break;
            }
            return n;
        };

        auto compMatchesMark = [&](SkGlyphID gid, SkUnichar cp) {
            SkGlyphID cands[3];
            int n = markGlyphCandidates(cp, cands);
            for (int k = 0; k < n; ++k) {
                if (cands[k] == gid) return true;
            }
            return false;
        };

        for (int round = 0; round < kMaxComponents; ++round) {
            bool progressed = false;
            for (int m = 1; m < visibleCPs.size(); ++m) {
                if (markToComp[m] >= 0) continue;
                int hitComp = -1;
                int hitCount = 0;
                for (int i = 0; i < comps.size(); ++i) {
                    if (!compUsed[i] && compMatchesMark(comps[i].fGlyphId, visibleCPs[m].fCP)) {
                        hitComp = i;
                        ++hitCount;
                    }
                }
                if (hitCount != 1) continue;
                bool rival = false;
                for (int m2 = 1; m2 < visibleCPs.size(); ++m2) {
                    if (m2 != m && markToComp[m2] < 0 &&
                        compMatchesMark(comps[hitComp].fGlyphId, visibleCPs[m2].fCP)) {
                        rival = true;
                        break;
                    }
                }
                if (!rival) {
                    markToComp[m] = hitComp;
                    compUsed[hitComp] = true;
                    progressed = true;
                }
            }
            if (!progressed) break;
        }

        int remMarks[kMaxComponents];
        int remComps[kMaxComponents];
        int remM = 0;
        int remC = 0;
        for (int m = 1; m < visibleCPs.size(); ++m) {
            if (markToComp[m] < 0) remMarks[remM++] = m;
        }
        for (int i = 0; i < comps.size(); ++i) {
            if (!compUsed[i]) remComps[remC++] = i;
        }
        if (remM == 1 && remC == 1) {
            markToComp[remMarks[0]] = remComps[0];
            remM = 0;
        } else if (remM == 2 && remC == 2) {
            // Pair two unencoded marks only when they occupy strictly distinct vertical tiers
            // (below-base vs above-base, or Thai above-vowel vs upper tone mark)
            auto markVerticalTier = [](SkUnichar cp) -> int {
                if ((cp >= 0x0316 && cp <= 0x0333) || (cp >= 0x0339 && cp <= 0x033C) ||
                    cp == 0x0345 || (cp >= 0x0E38 && cp <= 0x0E3A)) {
                    return 0;
                }
                if ((cp >= 0x0300 && cp <= 0x0314) || (cp >= 0x033D && cp <= 0x0344) ||
                    cp == 0x0346 || (cp >= 0x034A && cp <= 0x034C) || cp == 0x0E31 ||
                    (cp >= 0x0E34 && cp <= 0x0E37) || cp == 0x0E47 ||
                    (cp >= 0x0E4D && cp <= 0x0E4E)) {
                    return 1;
                }
                if (cp >= 0x0E48 && cp <= 0x0E4C) {
                    return 2;
                }
                return -1;
            };
            int tier0 = markVerticalTier(visibleCPs[remMarks[0]].fCP);
            int tier1 = markVerticalTier(visibleCPs[remMarks[1]].fCP);
            int16_t cont0 = 0, yMin0 = 0, yMax0 = 0;
            int16_t cont1 = 0, yMin1 = 0, yMax1 = 0;
            if (tier0 >= 0 && tier1 >= 0 && tier0 != tier1 &&
                readGlyphHeader(comps[remComps[0]].fGlyphId, &cont0, &yMin0, &yMax0) &&
                readGlyphHeader(comps[remComps[1]].fGlyphId, &cont1, &yMin1, &yMax1) &&
                cont0 >= 0 && cont1 >= 0) {
                float y0 = 0.5f * float(yMin0 + yMax0) + float(comps[remComps[0]].fDy);
                float y1 = 0.5f * float(yMin1 + yMax1) + float(comps[remComps[1]].fDy);
                if (std::abs(y0 - y1) > 1.0f) {
                    bool firstCompLower = y0 < y1;
                    bool firstMarkLower = tier0 < tier1;
                    markToComp[remMarks[0]] =
                            (firstMarkLower == firstCompLower) ? remComps[0] : remComps[1];
                    markToComp[remMarks[1]] =
                            (firstMarkLower == firstCompLower) ? remComps[1] : remComps[0];
                    remM = 0;
                }
            }
        }
        if (remM != 0) {
            return -1;
        }
    }

    const SkFont& font = run->font();
    SkScalar unitScale = font.getSize() / SkIntToScalar(upem);
    auto makeOffset = [&](const RawComp& c) {
        // TrueType y-up component offsets also shift horizontally under synthetic oblique skewX
        SkScalar sx = SkIntToScalar(c.fDx) * unitScale;
        SkScalar sy = SkIntToScalar(c.fDy) * unitScale;
        return SkPoint::Make(sx * font.getScaleX() - sy * font.getSkewX(), -sy);
    };

    DecomposedCluster& outEntry = fDecomposedClusters[entryIdx];
    outEntry.fComponents.push_back(
            {comps[baseCompIdx].fGlyphId, makeOffset(comps[baseCompIdx]), visibleCPs[0].fStart});
    for (int m = 1; m < visibleCPs.size(); ++m) {
        const RawComp& mc = comps[markToComp[m]];
        outEntry.fComponents.push_back({mc.fGlyphId, makeOffset(mc), visibleCPs[m].fStart});
    }
    outEntry.fValid = true;
    return entryIdx;
}

// Replaces boundary 1-glyph composite clusters in builder with the sub-glyphs owned by
// textRange; returns false without touching builder when no boundary composite is split
bool TextLine::buildDecomposedBlob(const Run* run,
                                   TextRange textRange,
                                   const ClipContext& context,
                                   SkScalar correctedBaseline,
                                   SkTextBlobBuilder& builder,
                                   TArray<uint32_t, true>* outClusterIndexes,
                                   SkRect* inoutLocalClip) const {
    if (textRange.empty() || run->placeholderStyle() != nullptr || run->isEllipsis()) {
        return false;
    }
    const Cluster* startCluster = &fOwner->cluster(fOwner->clusterIndex(textRange.start));
    const Cluster* endCluster = &fOwner->cluster(fOwner->clusterIndex(textRange.end - 1));
    if (startCluster->runIndex() != run->index() || endCluster->runIndex() != run->index()) {
        return false;
    }
    bool startSplit = textRange.start > startCluster->textRange().start;
    bool endSplit = textRange.end < endCluster->textRange().end;
    if (!startSplit && !endSplit) {
        return false;
    }

    int startIdx = -1;
    int endIdx = -1;
    if (startCluster == endCluster) {
        startIdx = this->decomposeClusterComponents(run, startCluster);
        if (startIdx < 0) {
            return false;
        }
    } else {
        if (startSplit) {
            startIdx = this->decomposeClusterComponents(run, startCluster);
        }
        if (endSplit) {
            endIdx = this->decomposeClusterComponents(run, endCluster);
        }
        if (startIdx < 0 && endIdx < 0) {
            return false;
        }
    }

    const bool ltr = run->leftToRight();
    const Cluster* leftCluster = nullptr;
    const DecomposedCluster* leftDecomp = nullptr;
    const Cluster* rightCluster = nullptr;
    const DecomposedCluster* rightDecomp = nullptr;
    if (startCluster == endCluster) {
        leftCluster = startCluster;
        leftDecomp = &fDecomposedClusters[startIdx];
    } else {
        int leftIdx = ltr ? startIdx : endIdx;
        int rightIdx = ltr ? endIdx : startIdx;
        if (leftIdx >= 0) {
            leftCluster = ltr ? startCluster : endCluster;
            leftDecomp = &fDecomposedClusters[leftIdx];
        }
        if (rightIdx >= 0) {
            rightCluster = ltr ? endCluster : startCluster;
            rightDecomp = &fDecomposedClusters[rightIdx];
        }
    }

    size_t midPos = context.pos;
    size_t midEnd = context.pos + context.size;
    if (leftCluster != nullptr && midPos < midEnd && midPos == leftCluster->startPos()) {
        ++midPos;
    }
    if (rightCluster != nullptr && midPos < midEnd && midEnd - 1 == rightCluster->startPos()) {
        --midEnd;
    }

    auto computeBasePos = [&](const Cluster* c) {
        size_t cg = c->startPos();
        SkASSERT(cg < run->size());
        SkPoint basePos = run->positions()[cg];
        if (!run->fJustificationShifts.empty()) {
            // When isolateClusterGlyphs advances context.pos to cg + 1, fTextShift subtracts
            // fJustificationShifts[cg + 1].fY instead of fJustificationShifts[cg].fX
            basePos.fX += (context.pos == cg + 1) ? run->fJustificationShifts[cg + 1].fY
                                                  : run->fJustificationShifts[cg].fX;
        }
        if (!run->offsets().empty()) {
            basePos += run->offsets()[cg];
        }
        return basePos;
    };

    struct EmittedComp {
        SkGlyphID fGlyphId;
        SkPoint fPos;
        uint32_t fClusterIndex;
    };
    STArray<8, EmittedComp, true> leftEmitted;
    STArray<8, EmittedComp, true> rightEmitted;
    auto collectDecomp = [&](const Cluster* c,
                             const DecomposedCluster* d,
                             STArray<8, EmittedComp, true>* out) {
        if (!d) {
            return;
        }
        SkPoint basePos = computeBasePos(c);
        int n = d->fComponents.size();
        for (int step = 0; step < n; ++step) {
            int idx = ltr ? step : (n - 1 - step);
            const DecomposedComponent& comp = d->fComponents[idx];
            if (comp.fTextStart >= textRange.start && comp.fTextStart < textRange.end) {
                out->push_back({comp.fGlyphId, basePos + comp.fGlyphOffset,
                                SkToU32(comp.fTextStart)});
            }
        }
    };
    collectDecomp(leftCluster, leftDecomp, &leftEmitted);
    collectDecomp(rightCluster, rightDecomp, &rightEmitted);

    const size_t midCount = midEnd - midPos;
    const size_t totalGlyphs =
            SkToSizeT(leftEmitted.size()) + midCount + SkToSizeT(rightEmitted.size());
    if (totalGlyphs == 0) {
        return true;
    }

    const auto& blobBuf = builder.allocRunPos(run->font(), SkToInt(totalGlyphs));
    if (outClusterIndexes) {
        outClusterIndexes->reset(SkToInt(totalGlyphs + 1));
    }
    size_t outIdx = 0;
    for (int i = 0; i < leftEmitted.size(); ++i) {
        blobBuf.glyphs[outIdx] = leftEmitted[i].fGlyphId;
        blobBuf.points()[outIdx] = leftEmitted[i].fPos;
        if (outClusterIndexes) {
            (*outClusterIndexes)[SkToInt(outIdx)] = leftEmitted[i].fClusterIndex;
        }
        ++outIdx;
    }
    const SkSpan<const SkGlyphID> runGlyphs = run->glyphs();
    const SkSpan<const SkPoint> runPositions = run->positions();
    const SkSpan<const SkPoint> runOffsets = run->offsets();
    for (size_t g = midPos; g < midEnd; ++g) {
        blobBuf.glyphs[outIdx] = runGlyphs[g];
        SkPoint pt = runPositions[g];
        if (!run->fJustificationShifts.empty()) {
            pt.fX += run->fJustificationShifts[g].fX;
        }
        if (!runOffsets.empty()) {
            pt += runOffsets[g];
        }
        blobBuf.points()[outIdx] = pt;
        if (outClusterIndexes) {
            (*outClusterIndexes)[SkToInt(outIdx)] = SkToU32(run->globalClusterIndex(g));
        }
        ++outIdx;
    }
    for (int i = 0; i < rightEmitted.size(); ++i) {
        blobBuf.glyphs[outIdx] = rightEmitted[i].fGlyphId;
        blobBuf.points()[outIdx] = rightEmitted[i].fPos;
        if (outClusterIndexes) {
            (*outClusterIndexes)[SkToInt(outIdx)] = rightEmitted[i].fClusterIndex;
        }
        ++outIdx;
    }
    if (outClusterIndexes) {
        (*outClusterIndexes)[SkToInt(totalGlyphs)] =
                SkToU32(ltr ? textRange.end : textRange.start);
    }

    if (context.clippingNeeded && inoutLocalClip != nullptr) {
        // Keep active multi-grapheme ligature cuts intact while expanding the opposite edge so
        // overhanging decomposed diacritic ink (e.g. negative-dx 'Î') is not clipped away
        const bool leftLigatureCut =
                startCluster != endCluster &&
                (ltr ? (startSplit && leftDecomp == nullptr)
                     : (endSplit && leftDecomp == nullptr));
        const bool rightLigatureCut =
                startCluster != endCluster &&
                (ltr ? (endSplit && rightDecomp == nullptr)
                     : (startSplit && rightDecomp == nullptr));
        auto expandClip = [&](const STArray<8, EmittedComp, true>& emitted) {
            for (int i = 0; i < emitted.size(); ++i) {
                SkRect gb = run->font().getBounds(emitted[i].fGlyphId, nullptr);
                gb.outset(1.0f, 1.0f);
                gb.offset(context.fTextShift + emitted[i].fPos.fX,
                          correctedBaseline + emitted[i].fPos.fY);
                if (!leftLigatureCut) {
                    inoutLocalClip->fLeft = std::min(inoutLocalClip->fLeft, gb.fLeft);
                }
                if (!rightLigatureCut) {
                    inoutLocalClip->fRight = std::max(inoutLocalClip->fRight, gb.fRight);
                }
                inoutLocalClip->fTop = std::min(inoutLocalClip->fTop, gb.fTop);
                inoutLocalClip->fBottom = std::max(inoutLocalClip->fBottom, gb.fBottom);
            }
        };
        expandClip(leftEmitted);
        expandClip(rightEmitted);
    }
    return true;
}

void TextLine::buildTextBlob(TextRange textRange,
                             const TextStyle& style,
                             const ClipContext& context) {
    if (context.run->placeholderStyle() != nullptr) {
        return;
    }

    fTextBlobCache.emplace_back();
    TextBlobRecord& record = fTextBlobCache.back();

    if (style.hasForeground()) {
        record.fPaint = style.getForegroundPaintOrID();
    } else {
        std::get<SkPaint>(record.fPaint).setColor(style.getColor());
    }
    record.fVisitor_Run = context.run;
    record.fVisitor_Pos = context.pos;

    SkASSERT(nearlyEqual(context.run->baselineShift(), style.getBaselineShift()));
    SkScalar correctedBaseline =
            SkScalarFloorToScalar(this->baseline() + style.getBaselineShift() + 0.5);

    // TODO: This is the change for flutter, must be removed later
    SkTextBlobBuilder builder;
    SkRect localClip = context.clippingNeeded ? extendHeight(context) : context.clip;
    // Split 1-glyph TrueType composites emit per-style sub-glyphs; all other runs copy as-is
    if (!this->buildDecomposedBlob(context.run, textRange, context, correctedBaseline, builder,
                                   &record.fClusterIndexes, &localClip)) {
        context.run->copyTo(builder, SkToU32(context.pos), context.size);
    }
    record.fClippingNeeded = context.clippingNeeded;
    record.fClipRect = localClip.makeOffset(this->offset());

    record.fBlob = builder.make();
    if (record.fBlob != nullptr) {
        record.fBounds.joinPossiblyEmptyRect(record.fBlob->bounds());
    }

    record.fOffset = SkPoint::Make(this->offset().fX + context.fTextShift,
                                   this->offset().fY + correctedBaseline);
}

void TextLine::TextBlobRecord::paint(ParagraphPainter* painter, SkScalar x, SkScalar y) {
    if (fClippingNeeded) {
        painter->save();
        painter->clipRect(fClipRect.makeOffset(x, y));
    }
    painter->drawTextBlob(fBlob, x + fOffset.x(), y + fOffset.y(), fPaint);
    if (fClippingNeeded) {
        painter->restore();
    }
}

void TextLine::paintBackground(ParagraphPainter* painter,
                               SkScalar x,
                               SkScalar y,
                               TextRange textRange,
                               const TextStyle& style,
                               const ClipContext& context) const {
    if (style.hasBackground()) {
        painter->drawRect(context.clip.makeOffset(this->offset() + SkPoint::Make(x, y)),
                          style.getBackgroundPaintOrID());
    }
}

void TextLine::paintShadow(ParagraphPainter* painter,
                           SkScalar x,
                           SkScalar y,
                           TextRange textRange,
                           const TextStyle& style,
                           const ClipContext& context) const {
    SkScalar correctedBaseline =
            SkScalarFloorToScalar(this->baseline() + style.getBaselineShift() + 0.5);

    for (TextShadow shadow : style.getShadows()) {
        if (!shadow.hasShadow()) continue;

        SkTextBlobBuilder builder;
        SkRect localClip = context.clippingNeeded ? extendHeight(context) : SkRect::MakeEmpty();
        // Match buildTextBlob composite splitting so per-style mark shadows draw at mark offsets
        if (!this->buildDecomposedBlob(
                    context.run, textRange, context, correctedBaseline, builder, nullptr,
                    &localClip)) {
            context.run->copyTo(builder, context.pos, context.size);
        }

        if (context.clippingNeeded) {
            painter->save();
            localClip.offset(x, y);
            localClip.offset(this->offset());
            painter->clipRect(localClip);
        }
        auto blob = builder.make();
        painter->drawTextShadow(blob,
            x + this->offset().fX + shadow.fOffset.x() + context.fTextShift,
            y + this->offset().fY + shadow.fOffset.y() + correctedBaseline,
            shadow.fColor,
            SkDoubleToScalar(shadow.fBlurSigma));
        if (context.clippingNeeded) {
            painter->restore();
        }
    }
}

void TextLine::paintDecorations(ParagraphPainter* painter, SkScalar x, SkScalar y, TextRange textRange, const TextStyle& style, const ClipContext& context) const {
    ParagraphPainterAutoRestore ppar(painter);
    painter->translate(x + this->offset().fX, y + this->offset().fY + style.getBaselineShift());
    Decorations decorations;
    // Gaps are cut where glyphs cross the decoration, so they must be measured against the
    // baseline buildTextBlob() draws the text at, relative to the already shifted origin
    SkScalar correctedBaseline =
            SkScalarFloorToScalar(this->baseline() + style.getBaselineShift() + 0.5) -
            style.getBaselineShift();
    decorations.paint(painter, style, context, correctedBaseline);
}

void TextLine::justify(SkScalar maxWidth) {
    if (fHyphen != nullptr) {
        // The rendered soft hyphen follows the text, so the text fills the rest of the line
        maxWidth -= fHyphen->advance().fX;
    }
    int whitespacePatches = 0;
    SkScalar textLen = 0;
    SkScalar whitespaceLen = 0;
    bool whitespacePatch = false;
    // Take leading whitespaces width but do not increment a whitespace patch number; start true so
    // any leading soft hyphen before the first visible cluster does not open a leading patch
    bool leadingWhitespaces = true;
    this->iterateThroughClustersInGlyphsOrder(false, false,
        [&](const Cluster* cluster, ClusterIndex index, bool ghost) {
            if (cluster->isSoftHyphen()) {
                // Soft-hyphen clusters are unrendered in the text run and must not split an
                // adjacent ideographic or whitespace justification patch
            } else if (cluster->isWhitespaceBreak()) {
                if (!whitespacePatch && !leadingWhitespaces) {
                    // We only count patches BETWEEN words, not before
                    ++whitespacePatches;
                }
                whitespacePatch = !leadingWhitespaces;
                whitespaceLen += cluster->width();
            } else if (cluster->isIdeographic()) {
                // Whitespace break before and after, except before the first visible cluster
                if (!whitespacePatch && !leadingWhitespaces) {
                    // We only count patches BETWEEN words, not before
                    ++whitespacePatches; // before
                }
                whitespacePatch = true;
                leadingWhitespaces = false;
                ++whitespacePatches;    // after
            } else {
                whitespacePatch = false;
                leadingWhitespaces = false;
            }
            textLen += cluster->width();
            return true;
        });

    if (whitespacePatch) {
        // We only count patches BETWEEN words, not after
        --whitespacePatches;
    }
    if (whitespacePatches == 0) {
        if (fOwner->paragraphStyle().getTextDirection() == TextDirection::kRtl) {
            // Justify -> Right align
            fShift = maxWidth - textLen;
        }
        return;
    }

    SkScalar step = (maxWidth - textLen + whitespaceLen) / whitespacePatches;
    SkScalar shift = 0.0f;
    SkScalar prevShift = 0.0f;

    // Deal with the ghost spaces
    auto ghostShift = maxWidth - this->fAdvance.fX;
    // Spread the extra whitespaces
    whitespacePatch = false;
    // Do not break on leading whitespaces or leading soft hyphens
    leadingWhitespaces = true;
    this->iterateThroughClustersInGlyphsOrder(false, true, [&](const Cluster* cluster, ClusterIndex index, bool ghost) {

        if (ghost) {
            if (cluster->run().leftToRight()) {
                this->shiftCluster(cluster, ghostShift, ghostShift);
            }
            return true;
        }

        if (cluster->isSoftHyphen()) {
            // Keep whitespacePatch and leadingWhitespaces intact across soft hyphens
        } else if (cluster->isWhitespaceBreak()) {
            if (!whitespacePatch && !leadingWhitespaces) {
                shift += step;
                whitespacePatch = true;
                --whitespacePatches;
            }
            shift -= cluster->width();
        } else if (cluster->isIdeographic()) {
            // Only insert a before-patch once a visible cluster has preceded this ideograph
            if (!whitespacePatch && !leadingWhitespaces) {
                shift += step;
               --whitespacePatches;
            }
            whitespacePatch = false;
            leadingWhitespaces = false;
        } else {
            whitespacePatch = false;
            leadingWhitespaces = false;
        }
        this->shiftCluster(cluster, shift, prevShift);
        // shiftCluster still records the run-end shift for a trailing soft hyphen, but an
        // unrendered soft hyphen must not advance prevShift for the following visible cluster
        if (!cluster->isSoftHyphen()) {
            prevShift = shift;
        }
        // We skip ideographic whitespaces
        if (!cluster->isWhitespaceBreak() && cluster->isIdeographic()) {
            shift += step;
            whitespacePatch = true;
            --whitespacePatches;
        }
        return true;
    });

    if (whitespacePatch && whitespacePatches < 0) {
        whitespacePatches++;
        shift -= step;
    }

    SkAssertResult(nearlyEqual(shift, maxWidth - textLen));
    SkASSERT(whitespacePatches == 0);

    this->fWidthWithSpaces += ghostShift;
    this->fAdvance.fX = maxWidth;
}

void TextLine::shiftCluster(const Cluster* cluster, SkScalar shift, SkScalar prevShift) {

    auto& run = cluster->run();
    auto start = cluster->startPos();
    auto end = cluster->endPos();

    if (end == run.size()) {
        // Set the same shift for the fake last glyph (to avoid all extra checks)
        ++end;
    }

    if (run.fJustificationShifts.empty()) {
        // Do not fill this array until needed
        run.fJustificationShifts.reserve_exact(run.size() + 1);
        run.fJustificationShifts.push_back_n(run.size() + 1, { 0, 0 });
    }

    for (size_t pos = start; pos < end; ++pos) {
        run.fJustificationShifts[pos] = { shift, prevShift };
    }
}

void TextLine::createEllipsis(SkScalar maxWidth, const SkString& ellipsis, bool) {
    // Replace some clusters with the ellipsis
    // Go through the clusters in the reverse logical order
    // taking off cluster by cluster until the ellipsis fits
    SkScalar width = fAdvance.fX;
    RunIndex lastRun = EMPTY_RUN;
    std::unique_ptr<Run> ellipsisRun;
    for (auto clusterIndex = fGhostClusterRange.end; clusterIndex > fGhostClusterRange.start; --clusterIndex) {
        auto& cluster = fOwner->cluster(clusterIndex - 1);
        // Shape the ellipsis if the run has changed
        if (lastRun != cluster.runIndex()) {
            ellipsisRun = this->shapeEllipsis(ellipsis, &cluster);
            if (ellipsisRun->advance().fX > maxWidth) {
                // Ellipsis is bigger than the entire line; no way we can add it at all
                // BUT! We can keep scanning in case the next run will give us better results
                lastRun = EMPTY_RUN;
                continue;
            } else {
                // We may need to continue
                lastRun = cluster.runIndex();
            }
        }
        // See if it fits
        if (width + ellipsisRun->advance().fX > maxWidth) {
            width -= cluster.width();
            // Continue if the ellipsis does not fit
            continue;
        }
        // We found enough room for the ellipsis
        fAdvance.fX = width;
        fEllipsis = std::move(ellipsisRun);
        fEllipsis->setOwner(fOwner);

        // Let's update the line
        fClusterRange.end = clusterIndex;
        fGhostClusterRange.end = fClusterRange.end;
        fEllipsis->fClusterStart = cluster.textRange().start;
        fText.end = cluster.textRange().end;
        fTextIncludingNewlines.end = cluster.textRange().end;
        fTextExcludingSpaces.end = cluster.textRange().end;
        break;
    }

    if (!fEllipsis) {
        // Weird situation: ellipsis does not fit; no ellipsis then
        fClusterRange.end = fClusterRange.start;
        fGhostClusterRange.end = fClusterRange.start;
        fText.end = fText.start;
        fTextIncludingNewlines.end = fTextIncludingNewlines.start;
        fTextExcludingSpaces.end = fTextExcludingSpaces.start;
        fAdvance.fX = 0;
    }
}

void TextLine::createSoftHyphen() {
    if (fEllipsis) {
        return;
    }
    if (fClusterRange.width() == 0) {
        return;
    }

    auto& lastCluster = fOwner->cluster(fClusterRange.end - 1);
    if (!lastCluster.isSoftHyphen()) {
        return;
    }
    // Only draw the hyphen if the line actually broke at the soft hyphen. It
    // didn't if spaces or a hard line break follow it, which are trimmed into
    // the ghost clusters, or if the soft hyphen ends the paragraph.
    if (fClusterRange.end != fGhostClusterRange.end ||
        fTextIncludingNewlines.end >= fOwner->text().size()) {
        return;
    }

    const SkString& hyphenStr = fOwner->paragraphStyle().getSoftHyphenString();
    if (hyphenStr.isEmpty()) {
        return;
    }
    fHyphen = this->shapeEllipsis(hyphenStr, &lastCluster);
    if (fHyphen) {
        fHyphen->fClusterStart = lastCluster.textRange().start;
        // iterateThroughSingleRunByStyles builds testRange as
        // [fClusterStart, fClusterStart + fTextRange.width()), so sizing fTextRange to
        // lastCluster maps fHyphen directly to the U+00AD source range regardless of
        // hyphenStr's UTF-8 byte length and removes the need for Run::isHyphen()
        fHyphen->fTextRange = TextRange(0, lastCluster.textRange().width());
    }
}

std::unique_ptr<Run> TextLine::shapeEllipsis(const SkString& ellipsis, const Cluster* cluster) {
    return ShapeString(fOwner, fBlockRange, ellipsis, cluster);
}

std::unique_ptr<Run> TextLine::ShapeString(ParagraphImpl* owner,
                                           BlockRange blockRange,
                                           const SkString& ellipsis,
                                           const Cluster* cluster) {

    class ShapeHandler final : public SkShaper::RunHandler {
    public:
        ShapeHandler(SkScalar lineHeight,
                     bool useHalfLeading,
                     SkScalar baselineShift,
                     const SkString& ellipsis)
            : fRun(nullptr)
            , fLineHeight(lineHeight)
            , fUseHalfLeading(useHalfLeading)
            , fBaselineShift(baselineShift)
            , fEllipsis(ellipsis) {}
        std::unique_ptr<Run> run() & { return std::move(fRun); }

    private:
        void beginLine() override {}

        void runInfo(const RunInfo&) override {}

        void commitRunInfo() override {}

        Buffer runBuffer(const RunInfo& info) override {
            SkASSERT(!fRun);
            fRun = std::make_unique<Run>(nullptr, info, 0, fLineHeight, fUseHalfLeading, fBaselineShift, 0, 0);
            return fRun->newRunBuffer(&fBuffer, info.glyphCount);
        }

        void commitRunBuffer(const RunInfo& info) override {
            fRun->commit(fBuffer);
            fRun->fAdvance.fX = info.fAdvance.fX;
            fRun->fAdvance.fY = fRun->advance().fY;
            fRun->fPlaceholderIndex = std::numeric_limits<size_t>::max();
            fRun->fEllipsis = true;
        }

        void commitLine() override {}

        std::unique_ptr<Run> fRun;
        Run::ShapingBuffer fBuffer;
        SkScalar fLineHeight;
        bool fUseHalfLeading;
        SkScalar fBaselineShift;
        SkString fEllipsis;
    };

    const Run& run = cluster->run();
    TextStyle textStyle = owner->paragraphStyle().getTextStyle();
    for (auto i = blockRange.start; i < blockRange.end; ++i) {
        auto& block = owner->block(i);
        if (run.leftToRight() && cluster->textRange().end <= block.fRange.end) {
            textStyle = block.fStyle;
            break;
        } else if (!run.leftToRight() && cluster->textRange().start <= block.fRange.end) {
            textStyle = block.fStyle;
            break;
        }
    }

    // Shape the affix in the surrounding run's direction, so neutral and weak
    // characters in it (like a "!" after Arabic letters, or the default "-" and
    // "…") take that direction instead of LTR. ShapeHandler only supports a
    // single run, so if the affix would split in that direction (like
    // "Read more…" in RTL text), shape it in the opposite direction instead.
    SkBidiIterator::Level bidiLevel = run.bidiLevel();
    if (sk_sp<SkUnicode> unicode = owner->getUnicode()) {
        const bool ltr = run.leftToRight();
        const auto runDir = ltr ? SkUnicode::TextDirection::kLTR : SkUnicode::TextDirection::kRTL;
        const auto otherDir = ltr ? SkUnicode::TextDirection::kRTL : SkUnicode::TextDirection::kLTR;
        if (!isSingleBidiRun(unicode.get(), ellipsis, runDir) &&
            isSingleBidiRun(unicode.get(), ellipsis, otherDir)) {
            bidiLevel = ltr ? SkBidiIterator::kRTL : SkBidiIterator::kLTR;
        }
    }

    auto shaped = [&](sk_sp<SkTypeface> typeface, sk_sp<SkFontMgr> fallback) -> std::unique_ptr<Run> {
        ShapeHandler handler(
                run.heightMultiplier(), run.useHalfLeading(), run.baselineShift(), ellipsis);
        SkFont font(std::move(typeface), textStyle.getFontSize());
        font.setEdging(textStyle.getFontEdging());
        font.setHinting(textStyle.getFontHinting());
        font.setSubpixel(textStyle.getSubpixel());

        std::unique_ptr<SkShaper> shaper = SkShapers::HB::ShapeDontWrapOrReorder(
                owner->getUnicode(), fallback ? fallback : SkFontMgr::RefEmpty());

        const char* utf8 = ellipsis.c_str();
        size_t utf8Bytes = ellipsis.size();

        std::unique_ptr<SkShaper::BiDiRunIterator> bidi = SkShapers::unicode::BidiRunIterator(
                owner->getUnicode(), utf8, utf8Bytes, bidiLevel);
        SkASSERT(bidi);

        std::unique_ptr<SkShaper::LanguageRunIterator> language =
                SkShaper::MakeStdLanguageRunIterator(utf8, utf8Bytes);
        SkASSERT(language);

        std::unique_ptr<SkShaper::ScriptRunIterator> script =
                SkShapers::HB::ScriptRunIterator(utf8, utf8Bytes);
        SkASSERT(script);

        std::unique_ptr<SkShaper::FontRunIterator> fontRuns = SkShaper::MakeFontMgrRunIterator(
                utf8, utf8Bytes, font, fallback ? fallback : SkFontMgr::RefEmpty());
        SkASSERT(fontRuns);

        shaper->shape({ utf8, utf8Bytes },
                      *fontRuns,
                      *bidi,
                      *script,
                      *language,
                      {},
                      {},
                      &handler);
        auto ellipsisRun = handler.run();
        ellipsisRun->fTextRange = TextRange(0, ellipsis.size());
        ellipsisRun->fOwner = owner;
        return ellipsisRun;
    };

    // Check the current font
    auto ellipsisRun = shaped(run.fFont.refTypeface(), nullptr);
    if (ellipsisRun->isResolved()) {
        return ellipsisRun;
    }

    // Check all allowed fonts
    std::vector<sk_sp<SkTypeface>> typefaces = owner->fontCollection()->findTypefaces(
            textStyle.getFontFamilies(), textStyle.getFontStyle(), textStyle.getFontArguments());
    for (const auto& typeface : typefaces) {
        ellipsisRun = shaped(typeface, nullptr);
        if (ellipsisRun->isResolved()) {
            return ellipsisRun;
        }
    }

    // Try the fallback
    if (owner->fontCollection()->fontFallbackEnabled()) {
        const char* ch = ellipsis.c_str();
      SkUnichar unicode = SkUTF::NextUTF8WithReplacement(&ch,
                                                         ellipsis.c_str()
                                                             + ellipsis.size());
        // We do not expect emojis in ellipsis so if they appeat there
        // they will not be resolved with the pretiest color emoji font
      auto typeface = owner->fontCollection()->defaultFallback(unicode,
                                                               textStyle.getFontFamilies(),
                                                               textStyle.getFontStyle(),
                                                               textStyle.getLocale(),
                                                               textStyle.getFontArguments());
      if (typeface) {
          ellipsisRun = shaped(typeface, owner->fontCollection()->getFallbackManager());
          if (ellipsisRun->isResolved()) {
              return ellipsisRun;
          }
      }
    }
    return ellipsisRun;
}

TextLine::ClipContext TextLine::measureTextInsideOneRun(TextRange textRange,
                                                        const Run* run,
                                                        SkScalar runOffsetInLine,
                                                        SkScalar textOffsetInRunInLine,
                                                        bool includeGhostSpaces,
                                                        TextAdjustment textAdjustment) const {
    ClipContext result = { run, 0, run->size(), 0, SkRect::MakeEmpty(), 0, false };

    if (run->fEllipsis) {
        // Both ellipsis and placeholders can only be measured as one glyph
        result.fTextShift = runOffsetInLine;
        result.clip = SkRect::MakeXYWH(runOffsetInLine,
                                       sizes().runTop(run, this->fAscentStyle),
                                       run->advance().fX,
                                       run->calculateHeight(this->fAscentStyle,this->fDescentStyle));
        return result;
    } else if (run->isPlaceholder()) {
        // Placeholder glyph positions retain their origin from shaping. Translate
        // that origin to the visual run position, just as for regular text runs.
        result.fTextShift = runOffsetInLine - run->positionX(0);
        if (SkIsFinite(run->fFontMetrics.fAscent)) {
          result.clip = SkRect::MakeXYWH(runOffsetInLine,
                                         sizes().runTop(run, this->fAscentStyle),
                                         run->advance().fX,
                                         run->calculateHeight(this->fAscentStyle,this->fDescentStyle));
        } else {
            result.clip = SkRect::MakeXYWH(runOffsetInLine, run->fFontMetrics.fAscent, run->advance().fX, 0);
        }
        return result;
    } else if (textRange.empty()) {
        return result;
    }

    TextRange originalTextRange(textRange); // We need it for proportional measurement
    // Find [start:end] clusters for the text
    while (true) {
        // Update textRange by cluster edges (shift start up to the edge of the cluster)
        // TODO: remove this limitation?
        TextRange updatedTextRange;
        bool found;
        std::tie(found, updatedTextRange.start, updatedTextRange.end) =
                                        run->findLimitingGlyphClusters(textRange);
        if (!found) {
            return result;
        }

        if ((textAdjustment & TextAdjustment::Grapheme) == 0) {
            textRange = updatedTextRange;
            break;
        }

        // Update text range by grapheme edges (shift start up to the edge of the grapheme)
        std::tie(found, updatedTextRange.start, updatedTextRange.end) =
                                    run->findLimitingGraphemes(updatedTextRange);
        if (updatedTextRange == textRange) {
            break;
        }

        // Some clusters are inside graphemes and we need to adjust them
        //SkDebugf("Correct range: [%d:%d) -> [%d:%d)\n", textRange.start, textRange.end, startIndex, endIndex);
        textRange = updatedTextRange;

        // Move the start until it's on the grapheme edge (and glypheme, too)
    }
    Cluster* start = &fOwner->cluster(fOwner->clusterIndex(textRange.start));
    Cluster* end = &fOwner->cluster(fOwner->clusterIndex(textRange.end - (textRange.width() == 0 ? 0 : 1)));

    if (!run->leftToRight()) {
        std::swap(start, end);
    }
    result.pos = start->startPos();
    result.size = (end->isHardBreak() ? end->startPos() : end->endPos()) - start->startPos();
    auto textStartInRun = run->positionX(start->startPos());
    auto textStartInLine = runOffsetInLine + textOffsetInRunInLine;
    if (!run->leftToRight()) {
        std::swap(start, end);
    }
/*
    if (!run->fJustificationShifts.empty()) {
        SkDebugf("Justification for [%d:%d)\n", textRange.start, textRange.end);
        for (auto i = result.pos; i < result.pos + result.size; ++i) {
            auto j = run->fJustificationShifts[i];
            SkDebugf("[%d] = %f %f\n", i, j.fX, j.fY);
        }
    }
*/
    // Calculate the clipping rectangle for the text with cluster edges
    // There are 2 cases:
    // EOL (when we expect the last cluster clipped without any spaces)
    // Anything else (when we want the cluster width contain all the spaces -
    // coming from letter spacing or word spacing or justification)
    result.clip =
            SkRect::MakeXYWH(0,
                             sizes().runTop(run, this->fAscentStyle),
                             run->calculateWidth(result.pos, result.pos + result.size, false),
                             run->calculateHeight(this->fAscentStyle,this->fDescentStyle));

    // When a style boundary splits a grapheme cluster (base + combining marks), isolate
    // the glyphs belonging to originalTextRange instead of geometrically slicing the
    // cluster's bounding box; proportional clipping is reserved for multi-grapheme ligatures
    SkScalar leftCorrection = 0;
    SkScalar rightCorrection = 0;
    bool startSplit = originalTextRange.start > start->textRange().start;
    bool endSplit = originalTextRange.end < end->textRange().end;
    if (startSplit || endSplit) {
        size_t startG0 = start->startPos();
        size_t startG1 = start->endPos();
        size_t endG0 = end->startPos();
        size_t endG1 = end->isHardBreak() ? end->startPos() : end->endPos();
        bool isolated = false;

        if (startSplit) {
            if (shouldIsolateCluster(fOwner, start)) {
                std::tie(startG0, startG1) =
                        isolateClusterGlyphs(fOwner, run, start, originalTextRange, true);
                isolated = true;
            } else {
                leftCorrection = start->sizeToChar(originalTextRange.start);
            }
        }
        if (endSplit) {
            if (start == end && isolated) {
                endG0 = startG0;
                endG1 = startG1;
            } else if (shouldIsolateCluster(fOwner, end)) {
                std::tie(endG0, endG1) =
                        isolateClusterGlyphs(fOwner, run, end, originalTextRange, false);
                isolated = true;
            } else {
                rightCorrection = end->sizeFromChar(originalTextRange.end - 1);
            }
        }

        if (isolated) {
            result.pos = run->leftToRight() ? startG0 : endG0;
            size_t endPos = run->leftToRight() ? endG1 : startG1;
            result.size = endPos > result.pos ? (endPos - result.pos) : 0;
            result.clip = SkRect::MakeXYWH(
                    0,
                    sizes().runTop(run, this->fAscentStyle),
                    run->calculateWidth(result.pos, result.pos + result.size, false),
                    run->calculateHeight(this->fAscentStyle, this->fDescentStyle));
            textStartInRun = run->positionX(result.pos);
        }
    }

    /*
    SkDebugf("[%d: %d) => [%d: %d), @%d, %d: [%f:%f) + [%f:%f) = ", // جَآَهُ
             originalTextRange.start, originalTextRange.end, textRange.start, textRange.end,
             result.pos, result.size,
             result.clip.fLeft, result.clip.fRight, leftCorrection, rightCorrection);
     */
    result.clippingNeeded = leftCorrection != 0 || rightCorrection != 0;
    if (run->leftToRight()) {
        result.clip.fLeft += leftCorrection;
        result.clip.fRight -= rightCorrection;
        textStartInLine -= leftCorrection;
    } else {
        result.clip.fRight -= leftCorrection;
        result.clip.fLeft += rightCorrection;
        textStartInLine -= rightCorrection;
    }

    result.clip.offset(textStartInLine, 0);
    //SkDebugf("@%f[%f:%f)\n", textStartInLine, result.clip.fLeft, result.clip.fRight);

    if (compareRound(result.clip.fRight, fAdvance.fX, fOwner->getApplyRoundingHack()) > 0 && !includeGhostSpaces) {
        // There are few cases when we need it.
        // The most important one: we measure the text with spaces at the end (or at the beginning in RTL)
        // and we should ignore these spaces
        if (fOwner->paragraphStyle().getTextDirection() == TextDirection::kLtr) {
            // We only use this member for LTR
            result.fExcludedTrailingSpaces = std::max(result.clip.fRight - fAdvance.fX, 0.0f);
            result.clippingNeeded = true;
            result.clip.fRight = fAdvance.fX;
        }
    }

    if (result.clip.width() < 0) {
        // Weird situation when glyph offsets move the glyph to the left
        // (happens with zalgo texts, for instance)
        result.clip.fRight = result.clip.fLeft;
    }

    // The text must be aligned with the lineOffset
    result.fTextShift = textStartInLine - textStartInRun;

    return result;
}

void TextLine::iterateThroughClustersInGlyphsOrder(bool reversed,
                                                   bool includeGhosts,
                                                   const ClustersVisitor& visitor) const {
    // Walk through the clusters in the logical order (or reverse)
    SkSpan<const size_t> runs(fRunsInVisualOrder.data(), fRunsInVisualOrder.size());
    bool ignore = false;
    ClusterIndex index = 0;
    directional_for_each(runs, !reversed, [&](decltype(runs[0]) r) {
        if (ignore) return;
        auto run = this->fOwner->run(r);
        auto trimmedRange = fClusterRange.intersection(run.clusterRange());
        auto trailedRange = fGhostClusterRange.intersection(run.clusterRange());
        if (trailedRange.start >= trailedRange.end) {
            // The ellipsis removed all clusters of this run
            return;
        }
        // A run of trailing whitespaces only has no trimmed clusters
        trimmedRange.end = std::max(trimmedRange.end, trimmedRange.start);
        SkASSERT(trimmedRange.start == trailedRange.start);

        auto trailed = fOwner->clusters(trailedRange);
        auto trimmed = fOwner->clusters(trimmedRange);
        directional_for_each(trailed, reversed != run.leftToRight(), [&](Cluster& cluster) {
            if (ignore) return;
            bool ghost =  &cluster >= trimmed.data() + trimmed.size();
            if (!includeGhosts && ghost) {
                return;
            }
            if (!visitor(&cluster, index++, ghost)) {

                ignore = true;
                return;
            }
        });
    });
}

SkScalar TextLine::iterateThroughSingleRunByStyles(TextAdjustment textAdjustment,
                                                   const Run* run,
                                                   SkScalar runOffset,
                                                   TextRange textRange,
                                                   StyleType styleType,
                                                   const RunStyleVisitor& visitor) const {
    auto correctContext = [&](TextRange textRange, SkScalar textOffsetInRun) -> ClipContext {
        auto result = this->measureTextInsideOneRun(
                textRange, run, runOffset, textOffsetInRun, false, textAdjustment);
        if (styleType == StyleType::kDecorations) {
            // Decorations are drawn based on the real font metrics (regardless of styles and strut)
            result.clip.fTop = this->sizes().runTop(run, LineMetricStyle::CSS);
            result.clip.fBottom = result.clip.fTop +
                                  run->calculateHeight(LineMetricStyle::CSS, LineMetricStyle::CSS);
        }
        return result;
    };

    if (run->fEllipsis) {
        // Extra efforts to get the ellipsis text style
        ClipContext clipContext = correctContext(run->textRange(), 0.0f);
        TextRange testRange(run->fClusterStart, run->fClusterStart + run->textRange().width());
        for (BlockIndex index = fBlockRange.start; index < fBlockRange.end; ++index) {
           auto block = fOwner->styles().begin() + index;
           auto intersect = intersected(block->fRange, testRange);
           if (intersect.width() > 0) {
               visitor(testRange, block->fStyle, clipContext);
               return run->advance().fX;
           }
        }
        SkASSERT(false);
    }

    if (styleType == StyleType::kNone) {
        ClipContext clipContext = correctContext(textRange, 0.0f);
        // The placehoder can have height=0 or (exclusively) width=0 and still be a thing
        if (clipContext.clip.height() > 0.0f || clipContext.clip.width() > 0.0f) {
            visitor(textRange, TextStyle(), clipContext);
            return clipContext.clip.width();
        } else {
            return 0;
        }
    }

    TextIndex start = EMPTY_INDEX;
    size_t size = 0;
    const TextStyle* prevStyle = nullptr;
    SkScalar textOffsetInRun = 0;

    const BlockIndex blockRangeSize = fBlockRange.end - fBlockRange.start;
    for (BlockIndex index = 0; index <= blockRangeSize; ++index) {

        TextRange intersect;
        TextStyle* style = nullptr;
        if (index < blockRangeSize) {
            auto block = fOwner->styles().begin() +
                 (run->leftToRight() ? fBlockRange.start + index : fBlockRange.end - index - 1);

            // Get the text
            intersect = intersected(block->fRange, textRange);
            if (intersect.width() == 0) {
                if (start == EMPTY_INDEX) {
                    // This style is not applicable to the text yet
                    continue;
                } else {
                    // We have found all the good styles already
                    // but we need to process the last one of them
                    intersect = TextRange(start, start + size);
                    index = fBlockRange.end;
                }
            } else {
                // Get the style
                style = &block->fStyle;
                if (start != EMPTY_INDEX && style->matchOneAttribute(styleType, *prevStyle)) {
                    size += intersect.width();
                    // RTL text intervals move backward
                    start = std::min(intersect.start, start);
                    continue;
                } else if (start == EMPTY_INDEX ) {
                    // First time only
                    prevStyle = style;
                    size = intersect.width();
                    start = intersect.start;
                    continue;
                }
            }
        } else if (prevStyle != nullptr) {
            // This is the last style
        } else {
            break;
        }

        // We have the style and the text
        auto runStyleTextRange = TextRange(start, start + size);
        ClipContext clipContext = correctContext(runStyleTextRange, textOffsetInRun);
        textOffsetInRun += clipContext.clip.width();
        if (clipContext.clip.height() == 0) {
            continue;
        }
        visitor(runStyleTextRange, *prevStyle, clipContext);

        // Start all over again
        prevStyle = style;
        start = intersect.start;
        size = intersect.width();
    }
    return textOffsetInRun;
}

void TextLine::iterateThroughVisualRuns(bool includingGhostSpaces, const RunVisitor& visitor) const {

    // Walk through all the runs that intersect with the line in visual order
    SkScalar width = 0;
    SkScalar runOffset = 0;
    SkScalar totalWidth = 0;
    auto textRange = includingGhostSpaces ? this->textWithNewlines() : this->trimmedText();

    // In RTL, the line-end affix sits at the left edge and is visited before text runs;
    // honor a false return (e.g. from getGlyphPositionAtCoordinate when dx hits the affix)
    // so subsequent runs do not overwrite the visitor's result
    if (this->ellipsis() != nullptr && fOwner->paragraphStyle().getTextDirection() == TextDirection::kRtl) {
        runOffset = this->ellipsis()->offset().fX;
        if (!visitor(ellipsis(), runOffset, ellipsis()->textRange(), &width)) {
            return;
        }
    }

    if (this->hyphen() != nullptr &&
        fOwner->paragraphStyle().getTextDirection() == TextDirection::kRtl) {
        runOffset = this->hyphen()->offset().fX;
        if (!visitor(hyphen(), runOffset, hyphen()->textRange(), &width)) {
            return;
        }
    }

    for (auto& runIndex : fRunsInVisualOrder) {

        const auto run = &this->fOwner->run(runIndex);
        auto lineIntersection = intersected(run->textRange(), textRange);
        if (lineIntersection.width() == 0 && this->width() != 0) {
            // TODO: deal with empty runs in a better way
            continue;
        }
        if (!run->leftToRight() && runOffset == 0 && includingGhostSpaces) {
            // runOffset does not take in account a possibility
            // that RTL run could start before the line (trailing spaces)
            // so we need to do runOffset -= "trailing whitespaces length"
            TextRange whitespaces = intersected(
                    TextRange(fTextExcludingSpaces.end, fTextIncludingNewlines.end), run->fTextRange);
            if (whitespaces.width() > 0) {
                auto whitespacesLen = measureTextInsideOneRun(whitespaces, run, runOffset, 0, true, TextAdjustment::GlyphCluster).clip.width();
                runOffset -= whitespacesLen;
            }
        }
        runOffset += width;
        totalWidth += width;
        if (!visitor(run, runOffset, lineIntersection, &width)) {
            return;
        }
    }

    runOffset += width;
    totalWidth += width;

    if (this->ellipsis() != nullptr && fOwner->paragraphStyle().getTextDirection() == TextDirection::kLtr) {
        if (visitor(ellipsis(), runOffset, ellipsis()->textRange(), &width)) {
            totalWidth += width;
        }
    }

    if (this->hyphen() != nullptr &&
        fOwner->paragraphStyle().getTextDirection() == TextDirection::kLtr) {
        if (visitor(hyphen(), runOffset, hyphen()->textRange(), &width)) {
            totalWidth += width;
        }
    }

    if (!includingGhostSpaces && compareRound(totalWidth, this->width(), fOwner->getApplyRoundingHack()) != 0) {
    // This is a very important assert!
    // It asserts that 2 different ways of calculation come with the same results
        SkDEBUGFAILF("ASSERT: %f != %f\n", totalWidth, this->width());
    }
}

SkVector TextLine::offset() const {
    return fOffset + SkVector::Make(fShift, 0);
}

LineMetrics TextLine::getMetrics() const {
    LineMetrics result;
    SkASSERT(fOwner);

    // Fill out the metrics
    fOwner->ensureUTF16Mapping();
    result.fStartIndex = fOwner->getUTF16Index(fTextExcludingSpaces.start);
    result.fEndExcludingWhitespaces = fOwner->getUTF16Index(fTextExcludingSpaces.end);
    result.fEndIndex = fOwner->getUTF16Index(fText.end);
    result.fEndIncludingNewline = fOwner->getUTF16Index(fTextIncludingNewlines.end);
    result.fHardBreak = endsWithHardLineBreak();
    result.fAscent = - fMaxRunMetrics.ascent();
    result.fDescent = fMaxRunMetrics.descent();
    result.fUnscaledAscent = - fMaxRunMetrics.ascent(); // TODO: implement
    result.fHeight = fAdvance.fY;
    result.fWidth = fAdvance.fX;
    if (fOwner->getApplyRoundingHack()) {
        result.fHeight = littleRound(result.fHeight);
        result.fWidth = littleRound(result.fWidth);
    }
    result.fLeft = this->offset().fX;
    // This is Flutter definition of a baseline
    result.fBaseline = this->offset().fY + this->height() - this->sizes().descent();
    result.fLineNumber = this - fOwner->lines().data();

    // Fill out the style parts
    this->iterateThroughVisualRuns(false,
        [this, &result]
        (const Run* run, SkScalar runOffsetInLine, TextRange textRange, SkScalar* runWidthInLine) {
        if (run->placeholderStyle() != nullptr) {
            *runWidthInLine = run->advance().fX;
            return true;
        }
        *runWidthInLine = this->iterateThroughSingleRunByStyles(
        TextAdjustment::GlyphCluster, run, runOffsetInLine, textRange, StyleType::kForeground,
        [&result, &run](TextRange textRange, const TextStyle& style, const ClipContext& context) {
            SkFontMetrics fontMetrics;
            run->fFont.getMetrics(&fontMetrics);
            StyleMetrics styleMetrics(&style, fontMetrics);
            result.fLineMetrics.emplace(textRange.start, styleMetrics);
        });
        return true;
    });

    return result;
}

bool TextLine::isFirstLine() const {
    return this == &fOwner->lines().front();
}

bool TextLine::isLastLine() const {
    return this == &fOwner->lines().back();
}

bool TextLine::endsWithHardLineBreak() const {
    // TODO: For some reason Flutter imagines a hard line break at the end of the last line.
    //  To be removed...
    return (fGhostClusterRange.width() > 0 && fOwner->cluster(fGhostClusterRange.end - 1).isHardBreak()) ||
           fEllipsis != nullptr ||
           fGhostClusterRange.end == fOwner->clusters().size() - 1;
}

void TextLine::getRectsForRange(TextRange textRange0,
                                RectHeightStyle rectHeightStyle,
                                RectWidthStyle rectWidthStyle,
                                std::vector<TextBox>& boxes) const
{
    const Run* lastRun = nullptr;
    auto startBox = boxes.size();
    this->iterateThroughVisualRuns(true,
        [textRange0, rectHeightStyle, rectWidthStyle, &boxes, &lastRun, startBox, this]
        (const Run* run, SkScalar runOffsetInLine, TextRange textRange, SkScalar* runWidthInLine) {
        // For fHyphen (which has fEllipsis = true), iterateThroughSingleRunByStyles
        // always returns run->advance().fX for *runWidthInLine and passes the U+00AD
        // cluster's source range as textRange to the visitor below
        *runWidthInLine = this->iterateThroughSingleRunByStyles(
        TextAdjustment::GraphemeGluster, run, runOffsetInLine, textRange, StyleType::kNone,
        [run, runOffsetInLine, textRange0, rectHeightStyle, rectWidthStyle, &boxes, &lastRun, startBox, this]
        (TextRange textRange, const TextStyle& style, const TextLine::ClipContext& lineContext) {
            if (fHyphen != nullptr && !run->isEllipsis()) {
                // Exclude the line-final U+00AD cluster from normal text runs: its visual
                // glyph is emitted by fHyphen, while the unrendered U+00AD cluster in the
                // text run would emit a second box (breaking fallback fonts, letterSpacing,
                // and BiDi lines where fHyphen and the text run are not adjacent)
                textRange = textRange * TextRange(fTextIncludingNewlines.start,
                                                  fHyphen->fClusterStart);
            }
            auto intersect = textRange * textRange0;
            if (intersect.empty()) {
                return true;
            }

            auto paragraphStyle = fOwner->paragraphStyle();

            // Found a run that intersects with the text
            auto context = this->measureTextInsideOneRun(
                    intersect, run, runOffsetInLine, 0, true, TextAdjustment::GraphemeGluster);
            SkRect clip = context.clip;
            clip.offset(lineContext.fTextShift - context.fTextShift, 0);

            switch (rectHeightStyle) {
                case RectHeightStyle::kMax:
                    // TODO: Change it once flutter rolls into google3
                    //  (probably will break things if changed before)
                    clip.fBottom = this->height();
                    clip.fTop = this->sizes().delta();
                    break;
                case RectHeightStyle::kIncludeLineSpacingTop: {
                    clip.fBottom = this->height();
                    clip.fTop = this->sizes().delta();
                    auto verticalShift = this->sizes().rawAscent() - this->sizes().ascent();
                    if (isFirstLine()) {
                        clip.fTop += verticalShift;
                    }
                    break;
                }
                case RectHeightStyle::kIncludeLineSpacingMiddle: {
                    clip.fBottom = this->height();
                    clip.fTop = this->sizes().delta();
                    auto verticalShift = this->sizes().rawAscent() - this->sizes().ascent();
                    clip.offset(0, verticalShift / 2.0);
                    if (isFirstLine()) {
                        clip.fTop += verticalShift / 2.0;
                    }
                    if (isLastLine()) {
                        clip.fBottom -= verticalShift / 2.0;
                    }
                    break;
                 }
                case RectHeightStyle::kIncludeLineSpacingBottom: {
                    clip.fBottom = this->height();
                    clip.fTop = this->sizes().delta();
                    auto verticalShift = this->sizes().rawAscent() - this->sizes().ascent();
                    clip.offset(0, verticalShift);
                    if (isLastLine()) {
                        clip.fBottom -= verticalShift;
                    }
                    break;
                }
                case RectHeightStyle::kStrut: {
                    const auto& strutStyle = paragraphStyle.getStrutStyle();
                    if (strutStyle.getStrutEnabled()
                        && strutStyle.getFontSize() > 0) {
                        auto strutMetrics = fOwner->strutMetrics();
                        auto top = this->baseline();
                        clip.fTop = top + strutMetrics.ascent();
                        clip.fBottom = top + strutMetrics.descent();
                    }
                }
                break;
                case RectHeightStyle::kTight: {
                    if (run->fHeightMultiplier <= 0) {
                        break;
                    }
                    const auto effectiveBaseline = this->baseline() + this->sizes().delta();
                    clip.fTop = effectiveBaseline + run->ascent();
                    clip.fBottom = effectiveBaseline + run->descent();
                }
                break;
                default:
                    SkASSERT(false);
                break;
            }

            // Separate trailing spaces and move them in the default order of the paragraph
            // in case the run order and the paragraph order don't match
            SkRect trailingSpaces = SkRect::MakeEmpty();
            if (this->trimmedText().end <this->textWithNewlines().end && // Line has trailing space
                this->textWithNewlines().end == intersect.end &&         // Range is at the end of the line
                this->trimmedText().end > intersect.start)               // Range has more than just spaces
            {
                auto delta = this->spacesWidth();
                trailingSpaces = SkRect::MakeXYWH(0, 0, 0, 0);
                // There are trailing spaces in this run
                if (paragraphStyle.getTextAlign() == TextAlign::kJustify && isLastLine())
                {
                    // TODO: this is just a patch. Make it right later (when it's clear what and how)
                    trailingSpaces = clip;
                    if(run->leftToRight()) {
                        trailingSpaces.fLeft = this->width();
                        clip.fRight = this->width();
                    } else {
                        trailingSpaces.fRight = 0;
                        clip.fLeft = 0;
                    }
                } else if (paragraphStyle.getTextDirection() == TextDirection::kRtl &&
                    !run->leftToRight())
                {
                    // Split
                    trailingSpaces = clip;
                    trailingSpaces.fLeft = - delta;
                    trailingSpaces.fRight = 0;
                    clip.fLeft += delta;
                } else if (paragraphStyle.getTextDirection() == TextDirection::kLtr &&
                    run->leftToRight())
                {
                    // Split
                    trailingSpaces = clip;
                    trailingSpaces.fLeft = this->width();
                    trailingSpaces.fRight = trailingSpaces.fLeft + delta;
                    clip.fRight -= delta;
                }
            }

            clip.offset(this->offset());
            if (trailingSpaces.width() > 0) {
                trailingSpaces.offset(this->offset());
            }

            // Check if we can merge two boxes instead of adding a new one
            auto merge = [&lastRun, &context, &boxes](SkRect clip) {
                bool mergedBoxes = false;
                if (!boxes.empty() &&
                    lastRun != nullptr &&
                    context.run->leftToRight() == lastRun->leftToRight() &&
                    lastRun->placeholderStyle() == nullptr &&
                    context.run->placeholderStyle() == nullptr &&
                    nearlyEqual(lastRun->heightMultiplier(),
                                context.run->heightMultiplier()) &&
                    lastRun->font() == context.run->font())
                {
                    auto& lastBox = boxes.back();
                    if (nearlyEqual(lastBox.rect.fTop, clip.fTop) &&
                        nearlyEqual(lastBox.rect.fBottom, clip.fBottom) &&
                            (nearlyEqual(lastBox.rect.fLeft, clip.fRight) ||
                             nearlyEqual(lastBox.rect.fRight, clip.fLeft)))
                    {
                        lastBox.rect.fLeft = std::min(lastBox.rect.fLeft, clip.fLeft);
                        lastBox.rect.fRight = std::max(lastBox.rect.fRight, clip.fRight);
                        mergedBoxes = true;
                    }
                }
                lastRun = context.run;
                return mergedBoxes;
            };

            if (!merge(clip)) {
                boxes.emplace_back(clip, context.run->getTextDirection());
            }
            if (!nearlyZero(trailingSpaces.width()) && !merge(trailingSpaces)) {
                boxes.emplace_back(trailingSpaces, paragraphStyle.getTextDirection());
            }

            if (rectWidthStyle == RectWidthStyle::kMax && !isLastLine()) {
                // Align the very left/right box horizontally
                auto lineStart = this->offset().fX;
                auto lineEnd = this->offset().fX + this->width();
                auto left = boxes[startBox];
                auto right = boxes.back();
                if (left.rect.fLeft > lineStart && left.direction == TextDirection::kRtl) {
                    left.rect.fRight = left.rect.fLeft;
                    left.rect.fLeft = 0;
                    boxes.insert(boxes.begin() + startBox + 1, left);
                }
                if (right.direction == TextDirection::kLtr &&
                    right.rect.fRight >= lineEnd &&
                    right.rect.fRight < fOwner->widthWithTrailingSpaces()) {
                    right.rect.fLeft = right.rect.fRight;
                    right.rect.fRight = fOwner->widthWithTrailingSpaces();
                    boxes.emplace_back(right);
                }
            }

            return true;
        });
        return true;
    });
    if (fOwner->getApplyRoundingHack()) {
        for (auto& r : boxes) {
            r.rect.fLeft = littleRound(r.rect.fLeft);
            r.rect.fRight = littleRound(r.rect.fRight);
            r.rect.fTop = littleRound(r.rect.fTop);
            r.rect.fBottom = littleRound(r.rect.fBottom);
        }
    }
}

PositionWithAffinity TextLine::getGlyphPositionAtCoordinate(SkScalar dx) {

    if (SkScalarNearlyZero(this->width()) && SkScalarNearlyZero(this->spacesWidth())) {
        // TODO: this is one of the flutter changes that have to go away eventually
        //  Empty line is a special case in txtlib (but only when there are no spaces, too)
        auto utf16Index = fOwner->getUTF16Index(this->fTextExcludingSpaces.end);
        return { SkToS32(utf16Index) , kDownstream };
    }

    PositionWithAffinity result(0, Affinity::kDownstream);
    this->iterateThroughVisualRuns(true,
        [this, dx, &result]
        (const Run* run, SkScalar runOffsetInLine, TextRange textRange, SkScalar* runWidthInLine) {
            if (run->isEllipsis()) {
                // In LTR the affix is visited last (only reached when dx is past all text
                // runs); in RTL it is visited first at the left edge, so report its width
                // to offset subsequent runs and only stop if dx is within/left of the affix
                *runWidthInLine = run->advance().fX;
                if (fOwner->paragraphStyle().getTextDirection() == TextDirection::kLtr ||
                    dx <= this->offset().fX + runOffsetInLine + run->advance().fX) {
                    // Like any other end of line, the position belongs to this line, not to
                    // the start of the next one
                    auto utf16Index = fOwner->getUTF16Index(this->fText.end);
                    result = {SkToS32(utf16Index), utf16Index > 0 ? kUpstream : kDownstream};
                    return false;
                }
                return true;
            }

            bool keepLooking = true;
            *runWidthInLine = this->iterateThroughSingleRunByStyles(
            TextAdjustment::GraphemeGluster, run, runOffsetInLine, textRange, StyleType::kNone,
            [this, run, dx, &result, &keepLooking]
            (TextRange textRange, const TextStyle& style, const TextLine::ClipContext& context0) {

                SkScalar offsetX = this->offset().fX;
                ClipContext context = context0;

                // Correct the clip size because libtxt counts trailing spaces
                if (run->leftToRight()) {
                    context.clip.fRight += context.fExcludedTrailingSpaces; // extending clip to the right
                } else {
                    // Clip starts from 0; we cannot extend it to the left from that
                }
                // However, we need to offset the clip
                context.clip.offset(offsetX, 0.0f);

                // This patch will help us to avoid a floating point error
                if (SkScalarNearlyEqual(context.clip.fRight, dx, 0.01f)) {
                    context.clip.fRight = dx;
                }

                if (dx <= context.clip.fLeft) {
                    // All the other runs are placed right of this one
                    auto utf16Index = fOwner->getUTF16Index(context.run->globalClusterIndex(context.pos));
                    if (run->leftToRight()) {
                        result = { SkToS32(utf16Index), kDownstream};
                        keepLooking = false;
                    } else {
                        result = { SkToS32(utf16Index + 1), kUpstream};
                        // If we haven't reached the end of the run we need to keep looking
                        keepLooking = context.pos != 0;
                    }
                    // For RTL we go another way
                    return !run->leftToRight();
                }

                if (dx >= context.clip.fRight) {
                    // We have to keep looking ; just in case keep the last one as the closest
                    auto utf16Index = fOwner->getUTF16Index(context.run->globalClusterIndex(context.pos + context.size));
                    if (run->leftToRight()) {
                        result = {SkToS32(utf16Index), kUpstream};
                    } else {
                        result = {SkToS32(utf16Index), kDownstream};
                    }
                    // For RTL we go another way
                    return run->leftToRight();
                }

                // So we found the run that contains our coordinates
                // Find the glyph position in the run that is the closest left of our point
                // TODO: binary search
                size_t found = context.pos;
                for (size_t index = context.pos; index < context.pos + context.size; ++index) {
                    // TODO: this rounding is done to match Flutter tests. Must be removed..
                    auto end = context.run->positionX(index) + context.fTextShift + offsetX;
                    if (fOwner->getApplyRoundingHack()) {
                        end = littleRound(end);
                    }
                    if (end > dx) {
                        break;
                    } else if (end == dx && !context.run->leftToRight()) {
                        // When we move RTL variable end points to the beginning of the code point which is included
                        found = index;
                        break;
                    }
                    found = index;
                }

                SkScalar glyphemePosLeft = context.run->positionX(found) + context.fTextShift + offsetX;
                SkScalar glyphemesWidth = context.run->positionX(found + 1) - context.run->positionX(found);

                // Find the grapheme range that contains the point
                auto clusterIndex8 = context.run->globalClusterIndex(found);
                auto clusterEnd8 = context.run->globalClusterIndex(found + 1);
                auto graphemes = fOwner->countSurroundingGraphemes({clusterIndex8, clusterEnd8});

                SkScalar center = glyphemePosLeft + glyphemesWidth / 2;
                if (graphemes.size() > 1) {
                    // Calculate the position proportionally based on grapheme count
                    SkScalar averageGraphemeWidth = glyphemesWidth / graphemes.size();
                    SkScalar delta = dx - glyphemePosLeft;
                    int graphemeIndex = SkScalarNearlyZero(averageGraphemeWidth)
                                                ? 0
                                                : SkScalarFloorToInt(delta / averageGraphemeWidth);
                    // Justification shifts coordinates beyond glyph bounds;
                    // clamp to valid graphemes
                    graphemeIndex = std::clamp(graphemeIndex, 0, SkToInt(graphemes.size()) - 1);
                    auto graphemeCenter = glyphemePosLeft + graphemeIndex * averageGraphemeWidth +
                                          averageGraphemeWidth / 2;
                    auto graphemeUtf8Index = graphemes[graphemeIndex];
                    if ((dx < graphemeCenter) == context.run->leftToRight()) {
                        size_t utf16Index = fOwner->getUTF16Index(graphemeUtf8Index);
                        result = { SkToS32(utf16Index), kDownstream };
                    } else {
                        size_t utf16Index = fOwner->getUTF16Index(graphemeUtf8Index + 1);
                        result = { SkToS32(utf16Index), kUpstream };
                    }
                    // Keep UTF16 index as is
                } else if ((dx < center) == context.run->leftToRight()) {
                    size_t utf16Index = fOwner->getUTF16Index(clusterIndex8);
                    result = { SkToS32(utf16Index), kDownstream };
                } else {
                    size_t utf16Index = context.run->leftToRight()
                                                ? fOwner->getUTF16Index(clusterEnd8)
                                                : fOwner->getUTF16Index(clusterIndex8) + 1;
                    result = { SkToS32(utf16Index), kUpstream };
                }

                return keepLooking = false;

            });
            return keepLooking;
        }
    );
    return result;
}

void TextLine::getRectsForPlaceholders(std::vector<TextBox>& boxes) {
    std::vector<std::pair<TextIndex, TextBox>> placeholders;
    this->iterateThroughVisualRuns(
            true,
            [&placeholders, this](
                    const Run* run, SkScalar runOffset, TextRange textRange, SkScalar* width) {
                auto context = this->measureTextInsideOneRun(
                        textRange, run, runOffset, 0, true, TextAdjustment::GraphemeGluster);
                *width = context.clip.width();

                if (textRange.width() == 0) {
                    return true;
                }
                if (!run->isPlaceholder()) {
                    return true;
                }

                SkRect clip = context.clip;
                clip.offset(this->offset());

                if (fOwner->getApplyRoundingHack()) {
                    clip.fLeft = littleRound(clip.fLeft);
                    clip.fRight = littleRound(clip.fRight);
                    clip.fTop = littleRound(clip.fTop);
                    clip.fBottom = littleRound(clip.fBottom);
                }
                placeholders.emplace_back(run->textRange().start,
                                          TextBox(clip, run->getTextDirection()));
                return true;
            });

    // Measure in visual order, but return boxes in the order placeholders were added.
    // Sorting by their text offsets also handles mixed-direction runs within a line.
    std::sort(placeholders.begin(), placeholders.end(), [](const auto& a, const auto& b) {
        return a.first < b.first;
    });
    for (const auto& placeholder : placeholders) {
        boxes.push_back(placeholder.second);
    }
}
}  // namespace textlayout
}  // namespace skia
