/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef sparse_strips_corners_h
#define sparse_strips_corners_h

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkFont.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "include/core/SkPathBuilder.h"
#include "include/core/SkPathEffect.h"
#include "include/core/SkTypeface.h"
#include "include/effects/SkDashPathEffect.h"
#include "src/gpu/graphite/sparse_strips/Flatten.h"
#include "src/gpu/graphite/sparse_strips/MSAA_LUT.h"
#include "src/gpu/graphite/sparse_strips/Polyline.h"
#include "src/gpu/graphite/sparse_strips/Strip.h"
#include "src/gpu/graphite/sparse_strips/Tiler.h"
#include "tools/fonts/FontToolUtils.h"

#include <array>
#include <cmath>
#include <cstdint>
#include <limits>
#include <vector>

namespace skgpu::graphite {

static const float kSubX[8] = {
    (0 + 0.5f) / 8.0f,
    (5 + 0.5f) / 8.0f,
    (3 + 0.5f) / 8.0f,
    (7 + 0.5f) / 8.0f,
    (1 + 0.5f) / 8.0f,
    (4 + 0.5f) / 8.0f,
    (6 + 0.5f) / 8.0f,
    (2 + 0.5f) / 8.0f
};
static const float kSubY[8] = {
    (0 + 0.5f) / 8.0f,
    (1 + 0.5f) / 8.0f,
    (2 + 0.5f) / 8.0f,
    (3 + 0.5f) / 8.0f,
    (4 + 0.5f) / 8.0f,
    (5 + 0.5f) / 8.0f,
    (6 + 0.5f) / 8.0f,
    (7 + 0.5f) / 8.0f
};

struct SubsampleInfo {
    int originalLutBit;
    int truncatedLutBit;
    int bumped;
    int lutMinusBumped;
    int canonicalY;
    int untruncatedCanonicalY;
    int fillLeft;
    int crossTop;
};

struct PixelInfo {
    SubsampleInfo subsamples[8];
    int highlightType = 0; // 0: none, 1: normal, 2: start, 3: end
};

struct TruncationInfo {
    bool active = false;
    float yVal = 0.0f;
    bool isStart = true;
};

struct TileInfo {
    PixelInfo pixels[4][4];
    TruncationInfo truncations[4][4];
};

struct MockedTilePair {
    Line line1;
    Tile tile1;
    Line line2;
    Tile tile2;
};

inline MockedTilePair setupCase(const SkPath& path, bool forceLeftOnSecond = true) {
    // Translate path by +4.0 in X to avoid negative coordinate underflow in tiler
    SkPath shiftedPath = path.makeTransform(SkMatrix::Translate(4.0f, 0.0f));

    Flatten flattener;
    Polyline polyline;
    flattener.processPaths<FlattenMode::kScalar>(shiftedPath, SkMatrix(), 12.0f, 8.0f, &polyline);

    Tiles<4, 4> tiler;
    tiler.makeTilesMSAA(polyline, 12, 8);

    MockedTilePair pair;
    pair.tile1 = Tile(1, 0, 0, 0);
    pair.tile2 = Tile(1, 0, 1, 0);

    int lineCount = 0;
    for (auto it = polyline.begin(); it != polyline.end(); ++it) {
        auto [line, idx] = *it;
        // Shift line back to local tile space of tile (1, 0)
        Line shiftedLine = line;
        shiftedLine.p0.fX -= 4.0f;
        shiftedLine.p1.fX -= 4.0f;

        if (lineCount == 0) {
            pair.line1 = shiftedLine;
        } else if (lineCount == 1) {
            pair.line2 = shiftedLine;
        }
        lineCount++;
    }

    for (const auto& tile : tiler.getTiles()) {
        if (tile.x == 1 && tile.y == 0) {
            if (tile.lineIdx() == 0) {
                pair.tile1 = tile;
            } else if (tile.lineIdx() == 1) {
                pair.tile2 = tile;
            }
        }
    }

    if (pair.line1.p0.fX == 0.0f || pair.line1.p1.fX == 0.0f) {
        pair.tile1 = Tile(pair.tile1.x, pair.tile1.y, 0, pair.tile1.intersectionMask() | IntersectionBits::L);
    }
    if (forceLeftOnSecond && (pair.line2.p0.fX == 0.0f || pair.line2.p1.fX == 0.0f)) {
        pair.tile2 = Tile(pair.tile2.x, pair.tile2.y, 1, pair.tile2.intersectionMask() | IntersectionBits::L);
    }
    return pair;
}

inline TileInfo computeLineTileInfo(const Line& line, const Tile& tile, const SkTDArray<uint8_t>& maskLut) {
    TileInfo info;
    for (int r = 0; r < 4; ++r) {
        for (int c = 0; c < 4; ++c) {
            info.pixels[r][c].highlightType = 0;
            for (int k = 0; k < 8; ++k) {
                info.pixels[r][c].subsamples[k] = {0, 0, 0, 0, 0, 0, 0, 0};
            }
            info.truncations[r][c] = {false, 0.0f, true};
        }
    }

    if (line.p0 == line.p1) {
        return info;
    }

    bool canonicalXDir = line.p1.fX >= line.p0.fX;
    bool canonicalYDir = line.p1.fY >= line.p0.fY;

    float dx = line.p1.fX - line.p0.fX;
    float dy = line.p1.fY - line.p0.fY;
    float idx = (std::abs(dx) <= Strip::kStripEpsilon) ? 0.0f : 1.0f / dx;
    float idy = (std::abs(dy) <= Strip::kStripEpsilon) ? 0.0f : 1.0f / dy;
    float dxdy = dx * idy;

    std::array<float, 4> derivs = {dx, dy, idx, idy};
    std::array<float, 4> tileBounds = {0.0f, 0.0f, 4.0f, 4.0f};

    auto clipped = Tile::ClipToTile<4, 4>(
            line, tileBounds, derivs, tile.intersectionMask(), canonicalXDir, canonicalYDir);
    SkPoint pTop = clipped[0];
    SkPoint pBot = clipped[1];

    if (tile.hasLeftIntersection()) {
        float yEdge = pTop.fX < pBot.fX ? pTop.fY : pBot.fY;
        int32_t startY_fill = static_cast<int32_t>(std::ceil(yEdge));
        int32_t val = canonicalXDir ? -1 : 1;
        for (int32_t row = startY_fill; row < 4; ++row) {
            for (int32_t column = 0; column < 4; ++column) {
                for (int32_t k = 0; k < 8; ++k) {
                    info.pixels[row][column].subsamples[k].fillLeft += val;
                }
            }
        }
    }

    if (std::abs(dy) < Strip::kStripEpsilon && pTop.fY == std::floor(pTop.fY)) {
        return info;
    }

    int32_t startY = static_cast<int32_t>(std::floor(pTop.fY));
    int32_t endY = static_cast<int32_t>(std::ceil(pBot.fY));

    float normalX = dy;
    float normalY = -dx;
    if (normalX < 0.0f) {
        normalX = -normalX;
        normalY = -normalY;
    }
    float manhattanDistance = normalX + std::abs(normalY);
    float invManhattanDistance =
            (manhattanDistance < Strip::kStripEpsilon) ? 0.0f : 1.0f / manhattanDistance;
    bool hasPositiveSlope = normalY <= 0.0f;
    float localLineConstant = normalX * pTop.fX + normalY * pTop.fY;
    float normalizedSlope = std::abs(normalY) * invManhattanDistance;
    int lutRowOffset = std::clamp(
            static_cast<int>(std::floor(normalizedSlope * (Strip::kLutMaskHeight / 2))),
            0,
            (Strip::kLutMaskHeight / 2) - 1);
    int lutRow = hasPositiveSlope ? (lutRowOffset + Strip::kLutMaskHeight / 2) : lutRowOffset;
    float stepX = normalX * invManhattanDistance;
    float stepY = normalY * invManhattanDistance;
    float tBase =
            ((hasPositiveSlope ? normalX : manhattanDistance) - localLineConstant) *
            invManhattanDistance;
    bool sortedXDir = pTop.fX <= pBot.fX;

    std::array<SkPoint, 5> rows;
    rows.fill(SkPoint::Make(std::numeric_limits<float>::quiet_NaN(),
                            std::numeric_limits<float>::quiet_NaN()));
    rows[startY] = pTop;
    for (int32_t r = startY + 1; r < endY; ++r) {
        float gy = static_cast<float>(r);
        float gx = pTop.fX + (gy - pTop.fY) * dxdy;
        rows[r] = {gx, gy};
    }
    rows[endY] = pBot;

    for (int32_t row = startY; row < endY; ++row) {
        float pTopX = rows[row].fX;
        float pTopY = rows[row].fY;
        float pBotX = rows[row + 1].fX;
        float pBotY = rows[row + 1].fY;

        if (std::isnan(pTopX) || std::isnan(pBotX)) {
            continue;
        }

        float xMax = std::fmax(pTopX, pBotX);
        int32_t xStart = std::clamp(static_cast<int32_t>(std::floor(std::fmin(pTopX, pBotX))), 0, 3);
        int32_t xEnd = std::clamp(static_cast<int32_t>(std::floor(xMax)), 0, 3);

        float tVal = tBase + stepY * static_cast<float>(row) +
                     stepX * static_cast<float>(xStart);

        bool isStartY = (row == startY);
        bool isEndYMinus1 = (row == endY - 1);

        bool pixelTopTouch = (pTopY != std::floor(pTopY));
        bool defaultBumped = pixelTopTouch && sortedXDir;

        int canonicalStartX = sortedXDir ? xStart : xEnd;
        int canonicalEndX = sortedXDir ? xEnd : xStart;

        uint8_t startMaskVal = 0xff;
        bool startBumped = defaultBumped;
        if (isStartY) {
            bool touchLeft = pTopX == 0.0f;
            startBumped = pixelTopTouch && touchLeft;
            if (!touchLeft) {
                uint32_t shift = static_cast<uint32_t>(
                        std::round(8.0f * (pTopY - static_cast<float>(row))));
                startMaskVal = static_cast<uint8_t>(0xff << shift);
            }
        }

        uint8_t endMaskVal = 0xff;
        if (isEndYMinus1 && pBotX != 0.0f) {
            uint32_t shift = static_cast<uint32_t>(
                    std::round(8.0f * (pBotY - static_cast<float>(row))));
            endMaskVal = static_cast<uint8_t>(~(0xff << shift));
        }

        for (int32_t column = xStart; column <= xEnd; ++column) {
            info.pixels[row][column].highlightType = 1; // Default intersection (yellow)

            int u = std::clamp(static_cast<int>(std::floor(tVal * Strip::kLutMaskWidthF)),
                               0,
                               Strip::kLutMaskWidthExcl);
            uint8_t maskVal = maskLut[lutRow * Strip::kLutMaskWidth + u];

            int originalLutVal = maskVal;

            uint8_t edgeMask = 0xff;
            if (column == canonicalStartX) edgeMask &= startMaskVal;
            if (column == canonicalEndX) edgeMask &= endMaskVal;
            maskVal &= edgeMask;

            int truncatedLutVal = maskVal;

            if (column == canonicalStartX && isStartY) {
                if (startMaskVal != 0xff) {
                    info.truncations[row][column] = {true, pTopY, true};
                }
                info.pixels[row][column].highlightType = 2; // Start (Orange)
            }
            if (column == canonicalEndX && isEndYMinus1) {
                if (endMaskVal != 0xff) {
                    info.truncations[row][column] = {true, pBotY, false};
                }
                info.pixels[row][column].highlightType = 3; // End (Magenta)
            }

            bool bumped =
                    (isStartY && column == canonicalStartX) ? startBumped : defaultBumped;

            for (int32_t k = 0; k < 8; ++k) {
                int originalLutBit = (originalLutVal & (1 << k)) ? 1 : 0;
                int truncatedLutBit = (truncatedLutVal & (1 << k)) ? 1 : 0;
                int bumped_val = bumped ? -1 : 0;
                int lutMinusBumped = truncatedLutBit + bumped_val;
                int canonicalY = canonicalYDir ? lutMinusBumped : -lutMinusBumped;
                int untruncatedLutMinusBumped = originalLutBit + bumped_val;
                int untruncatedCanonicalY = canonicalYDir ? untruncatedLutMinusBumped : -untruncatedLutMinusBumped;

                info.pixels[row][column].subsamples[k] = {
                    originalLutBit,
                    truncatedLutBit,
                    bumped_val,
                    lutMinusBumped,
                    canonicalY,
                    untruncatedCanonicalY,
                    info.pixels[row][column].subsamples[k].fillLeft,
                    0
                };
            }

            tVal += stepX;
        }
    }

    for (int32_t row = startY; row < endY; ++row) {
        float pTopX = rows[row].fX;
        float pTopY = rows[row].fY;
        float pBotX = rows[row + 1].fX;

        if (std::isnan(pTopX) || std::isnan(pBotX)) {
            continue;
        }

        float xMax = std::fmax(pTopX, pBotX);
        int32_t xEnd = std::clamp(static_cast<int32_t>(std::floor(xMax)), 0, 3);

        bool crossedTop = (row > startY) || (pTopY == std::floor(pTopY));
        if (crossedTop) {
            int32_t val = canonicalYDir ? 1 : -1;
            for (int32_t column = xEnd + 1; column < 4; ++column) {
                for (int32_t k = 0; k < 8; ++k) {
                    info.pixels[row][column].subsamples[k].crossTop += val;
                }
            }
        }
    }

    return info;
}

inline TileInfo combineTileInfos(const TileInfo& t1, const TileInfo& t2, bool useTruncation) {
    TileInfo combined;
    for (int r = 0; r < 4; ++r) {
        for (int c = 0; c < 4; ++c) {
            combined.pixels[r][c].highlightType = std::max(t1.pixels[r][c].highlightType, t2.pixels[r][c].highlightType);
            for (int k = 0; k < 8; ++k) {
                int val1 = useTruncation ?
                           (t1.pixels[r][c].subsamples[k].canonicalY + t1.pixels[r][c].subsamples[k].fillLeft + t1.pixels[r][c].subsamples[k].crossTop) :
                           (t1.pixels[r][c].subsamples[k].untruncatedCanonicalY + t1.pixels[r][c].subsamples[k].fillLeft + t1.pixels[r][c].subsamples[k].crossTop);
                int val2 = useTruncation ?
                           (t2.pixels[r][c].subsamples[k].canonicalY + t2.pixels[r][c].subsamples[k].fillLeft + t2.pixels[r][c].subsamples[k].crossTop) :
                           (t2.pixels[r][c].subsamples[k].untruncatedCanonicalY + t2.pixels[r][c].subsamples[k].fillLeft + t2.pixels[r][c].subsamples[k].crossTop);
                int sum = val1 + val2;

                combined.pixels[r][c].subsamples[k].originalLutBit = 0;
                combined.pixels[r][c].subsamples[k].truncatedLutBit = 0;
                combined.pixels[r][c].subsamples[k].bumped = 0;
                combined.pixels[r][c].subsamples[k].lutMinusBumped = 0;
                combined.pixels[r][c].subsamples[k].canonicalY = sum;
                combined.pixels[r][c].subsamples[k].untruncatedCanonicalY = sum;
                combined.pixels[r][c].subsamples[k].fillLeft = 0;
                combined.pixels[r][c].subsamples[k].crossTop = 0;
            }
        }
    }
    return combined;
}

inline void drawChevron(SkCanvas* canvas, SkPoint p0, SkPoint p1, float S, const SkPaint& paint) {
    if (p0 == p1) return;

    float dx = p1.fX - p0.fX;
    float dy = p1.fY - p0.fY;
    float length = std::sqrt(dx * dx + dy * dy);
    if (length < 1e-4f) return;

    float ux = dx / length;
    float uy = dy / length;

    SkPoint mid = { (p0.fX + p1.fX) / 2.0f, (p0.fY + p1.fY) / 2.0f };

    float H = 6.0f;

    float nx = -uy;
    float ny = ux;

    SkPoint w1 = {
        mid.fX - 0.866f * H * ux + 0.5f * H * nx,
        mid.fY - 0.866f * H * uy + 0.5f * H * ny
    };
    SkPoint w2 = {
        mid.fX - 0.866f * H * ux - 0.5f * H * nx,
        mid.fY - 0.866f * H * uy - 0.5f * H * ny
    };

    canvas->drawLine(w1.fX, w1.fY, mid.fX, mid.fY, paint);
    canvas->drawLine(w2.fX, w2.fY, mid.fX, mid.fY, paint);
}

inline void drawProjectedLine(SkCanvas* canvas, float x, float y, float S, const Line& l, const SkPaint& paint) {
    if (l.p0 == l.p1) return;

    float dx = l.p1.fX - l.p0.fX;
    float dy = l.p1.fY - l.p0.fY;
    float length = std::sqrt(dx * dx + dy * dy);
    if (length < 1e-4f) return;

    // Find all intersections with tile boundary box [0, 4] x [0, 4]
    std::vector<float> ts;

    if (std::abs(dx) > 1e-4f) {
        float t_x0 = -l.p0.fX / dx;
        float y_x0 = l.p0.fY + t_x0 * dy;
        if (y_x0 >= -1e-4f && y_x0 <= 4.0001f) ts.push_back(t_x0);

        float t_x4 = (4.0f - l.p0.fX) / dx;
        float y_x4 = l.p0.fY + t_x4 * dy;
        if (y_x4 >= -1e-4f && y_x4 <= 4.0001f) ts.push_back(t_x4);
    }
    if (std::abs(dy) > 1e-4f) {
        float t_y0 = -l.p0.fY / dy;
        float x_y0 = l.p0.fX + t_y0 * dx;
        if (x_y0 >= -1e-4f && x_y0 <= 4.0001f) ts.push_back(t_y0);

        float t_y4 = (4.0f - l.p0.fY) / dy;
        float x_y4 = l.p0.fX + t_y4 * dx;
        if (x_y4 >= -1e-4f && x_y4 <= 4.0001f) ts.push_back(t_y4);
    }

    if (ts.size() < 2) return;

    float tMin = ts[0];
    float tMax = ts[0];
    for (float t : ts) {
        if (t < tMin) tMin = t;
        if (t > tMax) tMax = t;
    }

    SkPoint entry = { l.p0.fX + tMin * dx, l.p0.fY + tMin * dy };
    SkPoint exit  = { l.p0.fX + tMax * dx, l.p0.fY + tMax * dy };

    SkPoint entryScreen = { x + entry.fX * S, y + entry.fY * S };
    SkPoint exitScreen  = { x + exit.fX * S, y + exit.fY * S };
    SkPoint p0Screen    = { x + l.p0.fX * S, y + l.p0.fY * S };
    SkPoint p1Screen    = { x + l.p1.fX * S, y + l.p1.fY * S };

    if (tMin < -1e-4f) {
        canvas->drawLine(entryScreen.fX, entryScreen.fY, p0Screen.fX, p0Screen.fY, paint);
    }
    if (tMax > 1.0001f) {
        canvas->drawLine(p1Screen.fX, p1Screen.fY, exitScreen.fX, exitScreen.fY, paint);
    }
}

inline void drawLines(SkCanvas* canvas, float x, float y, float S, const Line& l1, const Line& l2, bool draw, bool projectInfinite = false) {
    if (!draw) return;
    SkPaint linePaint;
    linePaint.setColor(SkColorSetRGB(64, 196, 255));
    linePaint.setStyle(SkPaint::kStroke_Style);
    linePaint.setStrokeWidth(2.0f);
    linePaint.setAntiAlias(true);

    if (l1.p0 != l1.p1) {
        float x1 = x + l1.p0.fX * S;
        float y1 = y + l1.p0.fY * S;
        float x2 = x + l1.p1.fX * S;
        float y2 = y + l1.p1.fY * S;
        canvas->drawLine(x1, y1, x2, y2, linePaint);
        drawChevron(canvas, {x1, y1}, {x2, y2}, S, linePaint);

        if (projectInfinite) {
            SkPaint dashPaint;
            dashPaint.setColor(SkColorSetRGB(64, 196, 255));
            dashPaint.setStyle(SkPaint::kStroke_Style);
            dashPaint.setStrokeWidth(1.5f);
            const SkScalar intervals[] = {4.0f, 3.0f};
            dashPaint.setPathEffect(SkDashPathEffect::Make(intervals, 0.0f));
            dashPaint.setAntiAlias(true);
            drawProjectedLine(canvas, x, y, S, l1, dashPaint);
        }
    }
    if (l2.p0 != l2.p1) {
        float x1 = x + l2.p0.fX * S;
        float y1 = y + l2.p0.fY * S;
        float x2 = x + l2.p1.fX * S;
        float y2 = y + l2.p1.fY * S;
        canvas->drawLine(x1, y1, x2, y2, linePaint);
        drawChevron(canvas, {x1, y1}, {x2, y2}, S, linePaint);

        if (projectInfinite) {
            SkPaint dashPaint;
            dashPaint.setColor(SkColorSetRGB(64, 196, 255));
            dashPaint.setStyle(SkPaint::kStroke_Style);
            dashPaint.setStrokeWidth(1.5f);
            const SkScalar intervals[] = {4.0f, 3.0f};
            dashPaint.setPathEffect(SkDashPathEffect::Make(intervals, 0.0f));
            dashPaint.setAntiAlias(true);
            drawProjectedLine(canvas, x, y, S, l2, dashPaint);
        }
    }
}

inline void drawGrid(SkCanvas* canvas, float x, float y, const float S, const TileInfo& info, int stepType, const Line& line, bool drawLine) {
    SkPaint bgPaint;
    bgPaint.setColor(SkColorSetRGB(38, 38, 42));
    bgPaint.setStyle(SkPaint::kFill_Style);
    canvas->drawRect(SkRect::MakeXYWH(x, y, 4 * S, 4 * S), bgPaint);

    SkPaint gridPaint;
    gridPaint.setColor(SkColorSetRGB(65, 65, 70));
    gridPaint.setStyle(SkPaint::kStroke_Style);
    gridPaint.setStrokeWidth(1.0f);
    for (int i = 0; i <= 4; ++i) {
        canvas->drawLine(x + i * S, y, x + i * S, y + 4 * S, gridPaint);
        canvas->drawLine(x, y + i * S, x + 4 * S, y + i * S, gridPaint);
    }

    // Intersected cell outlines (color-coded)
    SkPaint outlinePaint;
    outlinePaint.setStyle(SkPaint::kStroke_Style);
    outlinePaint.setStrokeWidth(1.5f);
    outlinePaint.setAntiAlias(true);
    for (int row = 0; row < 4; ++row) {
        for (int col = 0; col < 4; ++col) {
            int highlight = info.pixels[row][col].highlightType;
            if (highlight > 0) {
                if (highlight == 1) {
                    outlinePaint.setColor(SkColorSetRGB(255, 235, 59)); // Yellow
                } else if (highlight == 2) {
                    outlinePaint.setColor(SkColorSetRGB(255, 152, 0)); // Start (Orange)
                } else if (highlight == 3) {
                    outlinePaint.setColor(SkColorSetRGB(233, 30, 99)); // End (Magenta)
                }
                canvas->drawRect(SkRect::MakeXYWH(x + col * S, y + row * S, S, S), outlinePaint);
            }
        }
    }

    // Draw Truncation dashes in Truncated LUT column
    if (stepType == 1) {
        for (int row = 0; row < 4; ++row) {
            for (int col = 0; col < 4; ++col) {
                const auto& trunc = info.truncations[row][col];
                if (trunc.active) {
                    float px = x + col * S;
                    float py = y + row * S;
                    float yLocal = trunc.yVal - static_cast<float>(row);

                    SkPaint dashPaint;
                    dashPaint.setColor(trunc.isStart ? SkColorSetRGB(255, 152, 0) : SkColorSetRGB(233, 30, 99));
                    dashPaint.setStyle(SkPaint::kStroke_Style);
                    dashPaint.setStrokeWidth(1.5f);
                    const SkScalar intervals[] = {4.0f, 3.0f};
                    dashPaint.setPathEffect(SkDashPathEffect::Make(intervals, 0.0f));
                    dashPaint.setAntiAlias(true);

                    canvas->drawLine(px, py + yLocal * S, px + S, py + yLocal * S, dashPaint);
                }
            }
        }
    }

    SkFont font(ToolUtils::DefaultTypeface(), 7.0f);
    SkPaint greenPaint, redPaint, grayPaint;
    greenPaint.setColor(SkColorSetRGB(76, 175, 80));
    redPaint.setColor(SkColorSetRGB(244, 67, 54));
    grayPaint.setColor(SkColorSetRGB(110, 110, 115));

    for (int row = 0; row < 4; ++row) {
        for (int col = 0; col < 4; ++col) {
            float px = x + col * S;
            float py = y + row * S;

            for (int k = 0; k < 8; ++k) {
                float sx = px + kSubX[k] * S;
                float sy = py + kSubY[k] * S;

                int val = 0;
                if (stepType == 0) {
                    val = info.pixels[row][col].subsamples[k].originalLutBit;
                } else if (stepType == 1) {
                    val = info.pixels[row][col].subsamples[k].truncatedLutBit;
                } else if (stepType == 2) {
                    val = info.pixels[row][col].subsamples[k].bumped;
                } else if (stepType == 3) {
                    val = info.pixels[row][col].subsamples[k].lutMinusBumped;
                } else if (stepType == 4) {
                    val = info.pixels[row][col].subsamples[k].canonicalY;
                } else if (stepType == 5) {
                    val = info.pixels[row][col].subsamples[k].fillLeft;
                } else if (stepType == 6) {
                    val = info.pixels[row][col].subsamples[k].crossTop;
                }

                SkString str;
                SkPaint* paint = &grayPaint;
                if (val > 0) {
                    str.printf("+%d", val);
                    paint = &greenPaint;
                } else if (val < 0) {
                    str.printf("%d", val);
                    paint = &redPaint;
                } else {
                    str.printf("0");
                    paint = &grayPaint;
                }

                SkRect textBounds;
                font.measureText(str.c_str(), str.size(), SkTextEncoding::kUTF8, &textBounds);
                canvas->drawString(str, sx - textBounds.centerX(), sy - textBounds.centerY(), font, *paint);
            }
        }
    }

    if (drawLine) {
        drawLines(canvas, x, y, S, line, { {0,0},{0,0} }, true);
    }
}

inline void drawResolvedGrid(SkCanvas* canvas, float x, float y, const float S, const TileInfo& combinedInfo, const Line& l1, const Line& l2, bool projectInfinite = false) {
    SkPaint bgPaint;
    bgPaint.setColor(SkColorSetRGB(38, 38, 42));
    bgPaint.setStyle(SkPaint::kFill_Style);
    canvas->drawRect(SkRect::MakeXYWH(x, y, 4 * S, 4 * S), bgPaint);

    for (int row = 0; row < 4; ++row) {
        for (int col = 0; col < 4; ++col) {
            int activeSamples = 0;
            for (int k = 0; k < 8; ++k) {
                int sum = combinedInfo.pixels[row][col].subsamples[k].canonicalY;
                if (sum != 0) {
                    activeSamples++;
                }
            }
            if (activeSamples > 0) {
                SkPaint fillPaint;
                fillPaint.setColor(SkColorSetARGB((activeSamples * 255) / 8, 76, 175, 80));
                fillPaint.setStyle(SkPaint::kFill_Style);
                canvas->drawRect(SkRect::MakeXYWH(x + col * S, y + row * S, S, S), fillPaint);
            }
        }
    }

    SkPaint gridPaint;
    gridPaint.setColor(SkColorSetRGB(65, 65, 70));
    gridPaint.setStyle(SkPaint::kStroke_Style);
    gridPaint.setStrokeWidth(1.0f);
    for (int i = 0; i <= 4; ++i) {
        canvas->drawLine(x + i * S, y, x + i * S, y + 4 * S, gridPaint);
        canvas->drawLine(x, y + i * S, x + 4 * S, y + i * S, gridPaint);
    }

    // Intersected cell outlines (color-coded)
    SkPaint outlinePaint;
    outlinePaint.setStyle(SkPaint::kStroke_Style);
    outlinePaint.setStrokeWidth(1.5f);
    outlinePaint.setAntiAlias(true);
    for (int row = 0; row < 4; ++row) {
        for (int col = 0; col < 4; ++col) {
            int highlight = combinedInfo.pixels[row][col].highlightType;
            if (highlight > 0) {
                if (highlight == 1) {
                    outlinePaint.setColor(SkColorSetRGB(255, 235, 59)); // Yellow
                } else if (highlight == 2) {
                    outlinePaint.setColor(SkColorSetRGB(255, 152, 0)); // Start (Orange)
                } else if (highlight == 3) {
                    outlinePaint.setColor(SkColorSetRGB(233, 30, 99)); // End (Magenta)
                }
                canvas->drawRect(SkRect::MakeXYWH(x + col * S, y + row * S, S, S), outlinePaint);
            }
        }
    }

    drawLines(canvas, x, y, S, l1, l2, true, projectInfinite);
}

struct VariationDef {
    const char* name;
    SkPath path;
};

class SparseStripsCornerGMBase : public skiagm::GM {
public:
    SparseStripsCornerGMBase(const char* gmName, const char* caseName, std::vector<VariationDef> variations, bool forceLeftOnSecond = true)
        : fGmName(gmName)
        , fCaseName(caseName)
        , fVariations(variations)
        , fForceLeftOnSecond(forceLeftOnSecond) {}

protected:
    SkString getName() const override {
        return SkString(fGmName);
    }

    SkISize getISize() override {
        return SkISize::Make(1600, static_cast<int>(fVariations.size() * 370 + 80));
    }

    void onDraw(SkCanvas* canvas) override {
        canvas->clear(SkColorSetRGB(20, 20, 24));

        const SkTDArray<uint8_t> lut = GenerateMSAALUT<uint8_t>();

        SkFont titleFont(ToolUtils::DefaultTypeface(), 14.0f);
        SkFont subTitleFont(ToolUtils::DefaultTypeface(), 12.0f);
        SkFont labelFont(ToolUtils::DefaultTypeface(), 11.0f);
        SkFont headerFont(ToolUtils::DefaultTypeface(), 10.0f);
        SkPaint textPaint;
        textPaint.setColor(SkColorSetRGB(230, 230, 235));

        // Draw Case Title
        canvas->drawString(fCaseName, 36.0f, 30.0f, titleFont, textPaint);

        float currentY = 60.0f;
        const float S = 36.0f;

        const char* headers[9] = {
            "Original LUT",
            "Truncated LUT",
            "Bumping",
            "LUT + Bumped",
            "Canonical Y",
            "Fill Left",
            "Cross Top",
            "Combined Sum",
            "Resolved Alpha"
        };

        for (size_t vIdx = 0; vIdx < fVariations.size(); ++vIdx) {
            const auto& var = fVariations[vIdx];

            // Draw Variation Subtitle
            canvas->drawString(var.name, 36.0f, currentY, subTitleFont, textPaint);

            // Draw Column Headers
            for (int col = 0; col < 9; ++col) {
                float hx = 100.0f + col * 164.0f;
                canvas->drawString(headers[col], hx, currentY + 16.0f, headerFont, textPaint);
            }

            MockedTilePair pair = setupCase(var.path, fForceLeftOnSecond);
            TileInfo tInfo1 = computeLineTileInfo(pair.line1, pair.tile1, lut);
            TileInfo tInfo2 = computeLineTileInfo(pair.line2, pair.tile2, lut);

            TileInfo combinedUntruncated = combineTileInfos(tInfo1, tInfo2, false);
            TileInfo combinedCorrect = combineTileInfos(tInfo1, tInfo2, true);

            // Row 1: Line 1 (Top Row)
            float row1Y = currentY + 30.0f;
            canvas->drawString("Line 1", 36.0f, row1Y + 72.0f, labelFont, textPaint);
            for (int step = 0; step < 7; ++step) {
                float gx = 100.0f + step * 164.0f;
                drawGrid(canvas, gx, row1Y, S, tInfo1, step, pair.line1, true);
            }

            // Row 1 Col 7: Combined Sum (Untruncated)
            float cx = 100.0f + 7 * 164.0f;
            drawGrid(canvas, cx, row1Y, S, combinedUntruncated, 4, { {0,0},{0,0} }, false);
            drawLines(canvas, cx, row1Y, S, pair.line1, pair.line2, true, true);
            canvas->drawString("Untruncated", cx, row1Y + 4 * S + 12.0f, labelFont, textPaint);

            // Row 1 Col 8: Resolved Alpha (Untruncated)
            float ax = 100.0f + 8 * 164.0f;
            drawResolvedGrid(canvas, ax, row1Y, S, combinedUntruncated, pair.line1, pair.line2, true);
            canvas->drawString("Untruncated", ax, row1Y + 4 * S + 12.0f, labelFont, textPaint);

            // Row 2: Line 2 (Bottom Row)
            float row2Y = currentY + 194.0f;
            canvas->drawString("Line 2", 36.0f, row2Y + 72.0f, labelFont, textPaint);
            for (int step = 0; step < 7; ++step) {
                float gx = 100.0f + step * 164.0f;
                drawGrid(canvas, gx, row2Y, S, tInfo2, step, pair.line2, true);
            }

            // Row 2 Col 7: Combined Sum (Correct)
            drawGrid(canvas, cx, row2Y, S, combinedCorrect, 4, { {0,0},{0,0} }, false);
            drawLines(canvas, cx, row2Y, S, pair.line1, pair.line2, true, false);
            canvas->drawString("Correct", cx, row2Y + 4 * S + 12.0f, labelFont, textPaint);

            // Row 2 Col 8: Resolved Alpha (Correct)
            drawResolvedGrid(canvas, ax, row2Y, S, combinedCorrect, pair.line1, pair.line2, false);
            canvas->drawString("Correct", ax, row2Y + 4 * S + 12.0f, labelFont, textPaint);

            currentY += 370.0f;
        }
    }

private:
    const char* fGmName;
    const char* fCaseName;
    std::vector<VariationDef> fVariations;
    bool fForceLeftOnSecond;
};

} // namespace skgpu::graphite

#endif // sparse_strips_corners_h
