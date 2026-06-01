/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef skgpu_graphite_sparse_strips_StripProcessorSimd_DEFINED
#define skgpu_graphite_sparse_strips_StripProcessorSimd_DEFINED

#include "include/private/base/SkTDArray.h"
#include "src/base/SkVx.h"
#include "src/gpu/graphite/sparse_strips/Polyline.h"
#include "src/gpu/graphite/sparse_strips/SparseStripsTypes.h"
#include "src/gpu/graphite/sparse_strips/Strip.h"
#include "src/gpu/graphite/sparse_strips/Tiler.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <limits>

namespace skgpu::graphite {

template <uint16_t kTileWidth, uint16_t kTileHeight, bool kIsWinding>
class StripProcessorSimd {
public:
    using swar_pixel = skvx::Vec<2, uint32_t>;
    using pixel_bytes = skvx::Vec<8, uint8_t>;

    StripProcessorSimd(SkTDArray<Strip>* stripBuf,
                       SkTDArray<uint8_t>* alphaBuf,
                       bool isInverse,
                       const Polyline& polyline,
                       const SkTDArray<uint8_t>& maskLut,
                       int32_t initialAlphaIdx
#if defined(GPU_TEST_UTILS)
                       , MsaaExactMaskObserver observer
#endif
                       )
            : fStripBuf(*stripBuf)
            , fAlphaBuf(*alphaBuf)
            , fIsInverse(isInverse)
            , fPolyline(polyline)
            , fMaskLut(maskLut)
            , fLocalAlphaIdx(initialAlphaIdx)
#if defined(GPU_TEST_UTILS)
            , fObserver(observer)
#endif
    {
        clearMask(initialMaskVal);
    }

    SK_ALWAYS_INLINE void clearMask(uint32_t val) {
        swar_pixel* flatMask = &fMaskBuffer[0][0];
        constexpr int totalPixels = kTileWidth * kTileHeight;

        if constexpr (kTileWidth % 8 == 0) {
            skvx::Vec<16, uint32_t> vec(val);
            for (int i = 0; i < totalPixels; i += 8) {
                vec.store(flatMask + i);
            }
        } else if constexpr (kTileWidth % 4 == 0) {
            skvx::Vec<8, uint32_t> vec(val);
            for (int i = 0; i < totalPixels; i += 4) {
                vec.store(flatMask + i);
            }
        } else {
            swar_pixel p = {val, val};
            for (int i = 0; i < totalPixels; ++i) {
                flatMask[i] = p;
            }
        }
    }

    SK_ALWAYS_INLINE void clearMaskWithWindingDelta() {
        uint8_t wByte;
        if constexpr (kIsWinding) {
            // Cast to int8_t to sign extend
            wByte = 0x80 + static_cast<uint8_t>(static_cast<int8_t>(fWindingDelta));
        } else {
            wByte = (fWindingDelta & 1) ? 1 : 0;
        }
        clearMask(wByte * 0x01010101u);
    }

    SK_ALWAYS_INLINE void clearMaskForNewRow() { clearMask(initialMaskVal); }

    SK_ALWAYS_INLINE bool shouldFill(int32_t w) const {
        if constexpr (kIsWinding) {
            return w != 0;
        } else {
            return (w & 1) != 0;
        }
    }

    SK_ALWAYS_INLINE int32_t windingDelta() const { return fWindingDelta; }
    SK_ALWAYS_INLINE void setWindingDelta(int32_t val) { fWindingDelta = val; }
    SK_ALWAYS_INLINE int32_t localAlphaIdx() const { return fLocalAlphaIdx; }

    SK_ALWAYS_INLINE void resolveTileToAlpha() {
        reserveAlphaBuffer();
        uint8_t* tileAlphaBase = fAlphaBuf.append(kTileAlphaCount);

        int localWriteIdx = 0;
        for (int32_t row = 0; row < kTileHeight; ++row) {
            if constexpr (kTileWidth % 8 == 0) {
                for (int32_t column = 0; column < kTileWidth; column += 8) {
                    processChunk<8>(row, column, tileAlphaBase, localWriteIdx);
                }
            } else if constexpr (kTileWidth % 4 == 0) {
                for (int32_t column = 0; column < kTileWidth; column += 4) {
                    processChunk<4>(row, column, tileAlphaBase, localWriteIdx);
                }
            }
        }
    }

    SK_ALWAYS_INLINE void rasterizeLineToTile(
            const Tile& tile, float tileMinX, float tileMinY, float tileMaxX, float tileMaxY) {
        Line line = fPolyline.getLine(tile.lineIdx());
        bool canonicalXDir = line.p1.fX >= line.p0.fX;
        bool canonicalYDir = line.p1.fY >= line.p0.fY;

        uint32_t windingBit = tile.coarseWinding() ? 1 : 0;
        if constexpr (kIsWinding) {
            fWindingDelta += (canonicalYDir ? 1 : -1) * static_cast<int32_t>(windingBit);
        } else {
            fWindingDelta ^= static_cast<int32_t>(windingBit);
        }

        float rightEdge = canonicalXDir ? line.p1.fX : line.p0.fX;
        if (rightEdge < 0.0f) {
            return;
        }

        float dx = line.p1.fX - line.p0.fX;
        float dy = line.p1.fY - line.p0.fY;

        float idx = (std::abs(dx) <= Strip::kStripEpsilon) ? 0.0f : 1.0f / dx;
        float idy = (std::abs(dy) <= Strip::kStripEpsilon) ? 0.0f : 1.0f / dy;
        float dxdy = dx * idy;

        std::array<float, 4> bounds = {tileMinX, tileMinY, tileMaxX, tileMaxY};
        std::array<float, 4> derivs = {dx, dy, idx, idy};

        auto clipped = Tile::ClipToTile<kTileWidth, kTileHeight>(
                line, bounds, derivs, tile.intersectionMask(), canonicalXDir, canonicalYDir);
        SkPoint pTop = clipped[0];
        SkPoint pBot = clipped[1];

        if (tile.left()) {
            float yEdge = (pTop.fX < pBot.fX) ? pTop.fY : pBot.fY;
            fillLeft(yEdge, canonicalXDir);
        }

        if (std::abs(dy) < Strip::kStripEpsilon && pTop.fY == std::floor(pTop.fY)) {
            return;
        }

        int32_t startY = static_cast<int32_t>(std::floor(pTop.fY));
        int32_t endY = static_cast<int32_t>(std::ceil(pBot.fY));
        auto edgeX = findRowIntersections(pTop, pBot, dxdy, startY, endY);

        LineStepParams stepParams = computeLineStepParams(pTop, pBot, dx, dy);

        if (startY < endY) {
            bool isOnlyRow = (startY == endY - 1);
            float pTopY = pTop.fY;
            float pBotY = isOnlyRow ? pBot.fY : static_cast<float>(startY + 1);

            uint8_t startMaskVal = 0xff;
            bool startBumped = (edgeX[startY] == 0.0f && stepParams.firstRowTopTouch);

            if (!startBumped) {
                uint32_t shift = static_cast<uint32_t>(
                        std::round(8.0f * (pTopY - static_cast<float>(startY))));
                startMaskVal = static_cast<uint8_t>(0xff << shift);
            }

            uint8_t endMaskVal = 0xff;
            if (isOnlyRow && edgeX[startY + 1] != 0.0f) {
                uint32_t shift = static_cast<uint32_t>(
                        std::round(8.0f * (pBotY - static_cast<float>(startY))));
                endMaskVal = static_cast<uint8_t>(~(0xff << shift));
            }

            bool leftBump = stepParams.xDir ? startBumped : stepParams.firstRowBumped;
            bool rightBump = stepParams.xDir ? stepParams.firstRowBumped : startBumped;
            bool crossed = (pTopY == std::floor(pTopY));

            uint8_t leftMask = stepParams.xDir ? startMaskVal : endMaskVal;
            uint8_t rightMask = stepParams.xDir ? endMaskVal : startMaskVal;

            processRowSpan(startY, leftMask, rightMask, leftBump, stepParams.firstRowBumped,
                           rightBump, crossed, edgeX.data(), stepParams, canonicalYDir);

            for (int32_t row = startY + 1; row < endY - 1; ++row) {
                processRowSpan(row, 0xff, 0xff, false, false, false, true, edgeX.data(), stepParams,
                               canonicalYDir);
            }

            if (!isOnlyRow) {
                int32_t lastY = endY - 1;
                float pBotYLast = pBot.fY;

                uint8_t endMaskLast = 0xff;
                if (edgeX[lastY + 1] != 0.0f) {
                    uint32_t shift = static_cast<uint32_t>(
                            std::round(8.0f * (pBotYLast - static_cast<float>(lastY))));
                    endMaskLast = static_cast<uint8_t>(~(0xff << shift));
                }

                uint8_t lMask = stepParams.xDir ? 0xff : endMaskLast;
                uint8_t rMask = stepParams.xDir ? endMaskLast : 0xff;

                processRowSpan(lastY, lMask, rMask, false, false, false, true, edgeX.data(),
                               stepParams, canonicalYDir);
            }
        }
    }

private:
    struct LineStepParams {
        const uint8_t* maskRowLut;
        int32_t stepXFixed;
        int32_t stepYFixed;
        int32_t tBaseFixed;
        bool xDir;
        bool firstRowTopTouch;
        bool firstRowBumped;
    };

    SK_ALWAYS_INLINE std::array<float, kTileHeight + 1> findRowIntersections(
            SkPoint pTop, SkPoint pBot, float dxdy, int32_t startY, int32_t endY) {
        std::array<float, kTileHeight + 1> edgeX;
        skvx::float4 vPTopX(pTop.fX);
        skvx::float4 vPTopY(pTop.fY);
        skvx::float4 vDxDy(dxdy);
        skvx::float4 vKBase(0.0f, 1.0f, 2.0f, 3.0f);

        int32_t height;
        if constexpr (kTileHeight > 4) {
            height = (endY + 3) / 4 * 4;
        } else {
            height = 4;
        }

        for (int32_t k = 0; k < height; k += 4) {
            skvx::float4 vGridY = skvx::float4(static_cast<float>(k)) + vKBase;
            skvx::float4 vGridX = vPTopX + (vGridY - vPTopY) * vDxDy;
            vGridX.store(edgeX.data() + k);
        }

        edgeX[startY] = pTop.fX;
        edgeX[endY] = pBot.fX;
        return edgeX;
    }

    SK_ALWAYS_INLINE LineStepParams computeLineStepParams(
            SkPoint pTop, SkPoint pBot, float dx, float dy) const {
        float nx = dy;
        float ny = -dx;
        if (nx < 0.0f) {
            nx = -nx;
            ny = -ny;
        }
        float D = nx + std::abs(ny);
        float invD = (D < Strip::kStripEpsilon) ? 0.0f : 1.0f / D;

        bool isPos = ny <= 0.0f;
        float cLocal = nx * pTop.fX + ny * pTop.fY;
        float s = std::abs(ny) * invD;
        int vMod = std::clamp(static_cast<int>(std::floor(s * (Strip::kMaskHeight / 2))),
                              0,
                              (Strip::kMaskHeight / 2) - 1);
        int v = isPos ? (vMod + Strip::kMaskHeight / 2) : vMod;

        const uint8_t* maskRowLut = fMaskLut.data() + (v * Strip::kMaskWidth);

        float stepX = nx * invD;
        float stepY = ny * invD;
        float tBase = ((isPos ? nx : D) - cLocal) * invD;

        constexpr float kFixedMult = 64.0f * 65536.0f;
        int32_t stepXFixed = static_cast<int32_t>(stepX * kFixedMult);
        int32_t stepYFixed = static_cast<int32_t>(stepY * kFixedMult);
        int32_t tBaseFixed = static_cast<int32_t>(tBase * kFixedMult);

        bool xDir = pTop.fX <= pBot.fX;
        bool firstRowTopTouch = (pTop.fY != std::floor(pTop.fY));
        bool firstRowBumped = firstRowTopTouch && xDir;

        return {
            maskRowLut,
            stepXFixed,
            stepYFixed,
            tBaseFixed,
            xDir,
            firstRowTopTouch,
            firstRowBumped
        };
    }

    static constexpr int32_t kTileAlphaCount = kTileWidth * kTileHeight;
    static constexpr uint32_t initialMaskVal = kIsWinding ? 0x80808080u : 0u;

    template <int N>
    static SK_ALWAYS_INLINE void ApplyFillToMask(swar_pixel* target, uint32_t fillVal) {
        auto vMask = skvx::Vec<N, uint32_t>::Load(target);
        skvx::Vec<N, uint32_t> vFill(fillVal);
        if constexpr (kIsWinding) {
            auto mB = sk_bit_cast<skvx::Vec<N * 4, uint8_t>>(vMask);
            auto fB = sk_bit_cast<skvx::Vec<N * 4, uint8_t>>(vFill);
            vMask = sk_bit_cast<skvx::Vec<N, uint32_t>>(mB + fB);
        } else {
            vMask ^= vFill;
        }
        vMask.store(target);
    }

    template <typename VecT> static SK_ALWAYS_INLINE VecT GetActivesWide(VecT v) {
        if constexpr (kIsWinding) {
            using Vec8 = skvx::Vec<sizeof(VecT), uint8_t>;
            Vec8 v_bytes = sk_bit_cast<Vec8>(v);
            Vec8 bias_bytes(0x80);
            Vec8 cmp = (v_bytes != bias_bytes);
            return sk_bit_cast<VecT>(cmp) & VecT(0x01010101u);
        } else {
            return v & VecT(0x01010101u);
        }
    }

    SK_ALWAYS_INLINE void reserveAlphaBuffer() {
        if (fAlphaBuf.size() + kTileAlphaCount > fAlphaBuf.capacity()) {
            constexpr size_t kChunkSize = 4 * kTileAlphaCount;
            fAlphaBuf.reserve(fAlphaBuf.capacity() + kChunkSize);
        }
    }

    [[maybe_unused]]
    SK_ALWAYS_INLINE void observeChunk(int32_t row, int32_t column, int32_t chunkSize) {
        for (int32_t x = column; x < column + chunkSize; ++x) {
            swar_pixel v = fMaskBuffer[row][x];
            uint8_t exactMask = 0;
            uint32_t lo = v[0];
            uint32_t hi = v[1];
            for (int s = 0; s < 4; ++s) {
                int8_t sLo = static_cast<int8_t>(lo & 0xFF);
                int8_t sHi = static_cast<int8_t>(hi & 0xFF);
                if constexpr (kIsWinding) {
                    sLo = static_cast<int8_t>(static_cast<uint8_t>(sLo) - 0x80);
                    sHi = static_cast<int8_t>(static_cast<uint8_t>(sHi) - 0x80);
                }
                if (shouldFill(sLo)) exactMask |= (1 << s);
                if (shouldFill(sHi)) exactMask |= (1 << (s + 4));
                lo >>= 8;
                hi >>= 8;
            }
            if (fIsInverse) {
                exactMask = ~exactMask & ((1 << Strip::kNumSubSamples) - 1);
            }
            fObserver(exactMask);
        }
    }

    template <int ChunkSize>
    SK_ALWAYS_INLINE void processChunk(int32_t row,
                                       int32_t column,
                                       uint8_t* tileAlphaBase,
                                       int& localWriteIdx) {
        auto vMask = skvx::Vec<ChunkSize * 2, uint32_t>::Load(&fMaskBuffer[row][column]);
        auto actives = GetActivesWide(vMask);

        skvx::Vec<ChunkSize, uint32_t> activeLo;
        skvx::Vec<ChunkSize, uint32_t> activeHi;
        if constexpr (ChunkSize == 8) {
            activeLo = skvx::shuffle<0, 2, 4, 6, 8, 10, 12, 14>(actives);
            activeHi = skvx::shuffle<1, 3, 5, 7, 9, 11, 13, 15>(actives);
        } else {
            activeLo = skvx::shuffle<0, 2, 4, 6>(actives);
            activeHi = skvx::shuffle<1, 3, 5, 7>(actives);
        }

        auto combinedBytes = activeLo + activeHi;
        skvx::Vec<ChunkSize, uint32_t> activeSamples = (combinedBytes * 0x01010101u) >> 24;
        const uint32_t invMask32 = fIsInverse ? 0xffffffff : 0;
        skvx::Vec<ChunkSize, uint32_t> alpha32 = ((activeSamples * 255 + 4) >> 3) ^ invMask32;
        skvx::cast<uint8_t>(alpha32).store(tileAlphaBase + localWriteIdx);
        localWriteIdx += ChunkSize;
        fLocalAlphaIdx += ChunkSize;
#if defined(GPU_TEST_UTILS)
        if (fObserver) {
            observeChunk(row, column, ChunkSize);
        }
#endif
    }

    SK_ALWAYS_INLINE void fillLeft(float yEdge, bool canonicalXDir) {
        uint8_t fillByte;
        if constexpr (kIsWinding) {
            fillByte = canonicalXDir ? 0xFF : 1;
        } else {
            fillByte = 1;
        }

        uint32_t fill32 = fillByte * 0x01010101u;
        for (int32_t row = static_cast<int32_t>(std::ceil(yEdge)); row < kTileHeight; ++row) {
            if constexpr (kTileWidth % 8 == 0) {
                for (int32_t column = 0; column < kTileWidth; column += 8) {
                    ApplyFillToMask<16>(&fMaskBuffer[row][column], fill32);
                }
            } else if constexpr (kTileWidth % 4 == 0) {
                for (int32_t column = 0; column < kTileWidth; column += 4) {
                    ApplyFillToMask<8>(&fMaskBuffer[row][column], fill32);
                }
            }
        }
    }

    SK_ALWAYS_INLINE void processEdgePixel(int32_t column,
                                           uint8_t edgeMask,
                                           const pixel_bytes& pBump,
                                           int32_t& tFixed,
                                           int32_t stepXFixed,
                                           const uint8_t* maskRowLut,
                                           const pixel_bytes& vBit,
                                           swar_pixel* rowMasks,
                                           bool canonicalYDir) {
        int u = std::clamp(tFixed >> 16, 0, Strip::kMaskWidthExcl);
        uint8_t maskVal = maskRowLut[u] & edgeMask;

        pixel_bytes pRes = -((pixel_bytes(maskVal) & vBit) != 0);
        pixel_bytes pMask = sk_bit_cast<pixel_bytes>(rowMasks[column]);

        if constexpr (kIsWinding) {
            pRes -= pBump;
            if (canonicalYDir) {
                pMask += pRes;
            } else {
                pMask -= pRes;
            }
        } else {
            pRes ^= pBump;
            pMask ^= pRes;
        }

        rowMasks[column] = sk_bit_cast<swar_pixel>(pMask);
        tFixed += stepXFixed;
    }

    SK_ALWAYS_INLINE void processRowSpan(int32_t row,
                                         uint8_t leftMask,
                                         uint8_t rightMask,
                                         bool leftBump,
                                         bool midBump,
                                         bool rightBump,
                                         bool crossedTop,
                                         const float* edgeX,
                                         const LineStepParams& params,
                                         bool canonicalYDir) {
        float pTopX = edgeX[row];
        float pBotX = edgeX[row + 1];

        float xMin = std::fmin(pTopX, pBotX);
        float xMax = std::fmax(pTopX, pBotX);
        int32_t xStart = std::clamp(static_cast<int32_t>(std::floor(xMin)), 0, kTileWidth - 1);
        int32_t xEnd = std::clamp(static_cast<int32_t>(std::floor(xMax)), 0, kTileWidth - 1);

        int32_t tFixed = params.tBaseFixed + (params.stepYFixed * row) + (params.stepXFixed * xStart);
        swar_pixel* rowMasks = fMaskBuffer[row];

        const pixel_bytes vBit{1, 2, 4, 8, 16, 32, 64, 128};
        pixel_bytes pMidBump(midBump ? 1 : 0);

        if (xStart == xEnd) {
            uint8_t combinedMask = leftMask & rightMask;
            processEdgePixel(xStart, combinedMask, pixel_bytes(leftBump ? 1 : 0), tFixed,
                             params.stepXFixed, params.maskRowLut, vBit, rowMasks, canonicalYDir);
        } else {
            processEdgePixel(xStart, leftMask, pixel_bytes(leftBump ? 1 : 0), tFixed,
                             params.stepXFixed, params.maskRowLut, vBit, rowMasks, canonicalYDir);

            for (int32_t column = xStart + 1; column < xEnd; ++column) {
                int u = std::clamp(tFixed >> 16, 0, Strip::kMaskWidthExcl);
                uint8_t maskVal = params.maskRowLut[u];

                pixel_bytes pRes = -((pixel_bytes(maskVal) & vBit) != 0);
                pixel_bytes pMask = sk_bit_cast<pixel_bytes>(rowMasks[column]);

                if constexpr (kIsWinding) {
                    pRes -= pMidBump;
                    if (canonicalYDir) {
                        pMask += pRes;
                    } else {
                        pMask -= pRes;
                    }
                } else {
                    pRes ^= pMidBump;
                    pMask ^= pRes;
                }

                rowMasks[column] = sk_bit_cast<swar_pixel>(pMask);
                tFixed += params.stepXFixed;
            }

            processEdgePixel(xEnd, rightMask, pixel_bytes(rightBump ? 1 : 0), tFixed,
                             params.stepXFixed, params.maskRowLut, vBit, rowMasks, canonicalYDir);
        }

        if (crossedTop) {
            uint8_t fillByte;
            if constexpr (kIsWinding) {
                fillByte = canonicalYDir ? 1 : 0xFF;
            } else {
                fillByte = 1;
            }

            int32_t column = xEnd + 1;
            uint32_t fill32 = fillByte * 0x01010101u;
            if constexpr (kTileWidth >= 8) {
                while (column + 8 <= kTileWidth) {
                    ApplyFillToMask<16>(&fMaskBuffer[row][column], fill32);
                    column += 8;
                }
            } else if constexpr (kTileWidth >= 4) {
                while (column + 4 <= kTileWidth) {
                    ApplyFillToMask<8>(&fMaskBuffer[row][column], fill32);
                    column += 4;
                }
            }

            const pixel_bytes pFill(fillByte);
            for (; column < kTileWidth; ++column) {
                pixel_bytes pMask = sk_bit_cast<pixel_bytes>(fMaskBuffer[row][column]);
                if constexpr (kIsWinding) {
                    pMask += pFill;
                } else {
                    pMask ^= pFill;
                }
                fMaskBuffer[row][column] = sk_bit_cast<swar_pixel>(pMask);
            }
        }
    }

    swar_pixel fMaskBuffer[kTileHeight][kTileWidth];
    int32_t fWindingDelta = 0;
    SkTDArray<Strip>& fStripBuf;
    SkTDArray<uint8_t>& fAlphaBuf;
    bool fIsInverse;
    const Polyline& fPolyline;
    const SkTDArray<uint8_t>& fMaskLut;
    int32_t fLocalAlphaIdx;
#if defined(GPU_TEST_UTILS)
    MsaaExactMaskObserver fObserver;
#endif
};

}  // namespace skgpu::graphite

#endif  // skgpu_graphite_sparse_strips_StripProcessorSimd_DEFINED
