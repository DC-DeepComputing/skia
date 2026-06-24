/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef skgpu_graphite_sparse_strips_StripProcessorSimd_DEFINED
#define skgpu_graphite_sparse_strips_StripProcessorSimd_DEFINED

#include "include/private/SkTDArray.h"
#include "src/core/SkVx.h"
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
    // Type aliases for convenient handling of the simd registers. In addition to SIMD, we use SWAR
    // to pack subsample winding 4 to 1 u32, each subsample being alloted 1 byte of memory, e.g.
    // -128 / 127 of winding. So a quality of 8 subsamples per pixel (MSAAx8) requires u32x2
    // (SwarPixel) or u8x8 (PixelBytes). Note, we currently hard code to these values, so higher
    // quality (e.g. MSAAx16) will require more handling.
    using SwarPixel = skvx::Vec<2, uint32_t>;
    using PixelBytes = skvx::Vec<8, uint8_t>;
    static_assert(Strip::kNumSubSamples == 8);

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
        clearWinding(kInitialWinding);
    }

    SK_ALWAYS_INLINE void clearWinding(uint32_t val) {
        SwarPixel* flatSubsampleWinding = &fSubsampleWinding[0][0];
        constexpr int totalPixels = kTileWidth * kTileHeight;

        if constexpr (kTileWidth % 8 == 0) {
            skvx::Vec<16, uint32_t> vec(val);
            for (int i = 0; i < totalPixels; i += 8) {
                vec.store(flatSubsampleWinding + i);
            }
        } else if constexpr (kTileWidth % 4 == 0) {
            skvx::Vec<8, uint32_t> vec(val);
            for (int i = 0; i < totalPixels; i += 4) {
                vec.store(flatSubsampleWinding + i);
            }
        } else {
            SwarPixel p = {val, val};
            for (int i = 0; i < totalPixels; ++i) {
                flatSubsampleWinding[i] = p;
            }
        }
    }

    SK_ALWAYS_INLINE void clearWithCoarseWinding() {
        uint8_t wByte;
        if constexpr (kIsWinding) {
            // Cast to int8_t to sign extend
            wByte = 0x80 + static_cast<uint8_t>(static_cast<int8_t>(fCoarseWinding));
        } else {
            wByte = (fCoarseWinding & 1) ? 1 : 0;
        }
        this->clearWinding(wByte * 0x01010101u);
    }

    SK_ALWAYS_INLINE void clearWindingForNewRow() { this->clearWinding(kInitialWinding); }

    SK_ALWAYS_INLINE static bool ShouldFill(int32_t w) {
        if constexpr (kIsWinding) {
            return w != 0;
        } else {
            return (w & 1) != 0;
        }
    }

    SK_ALWAYS_INLINE int32_t coarseWinding() const { return fCoarseWinding; }
    SK_ALWAYS_INLINE void setCoarseWinding(int32_t val) { fCoarseWinding = val; }
    SK_ALWAYS_INLINE int32_t localAlphaIdx() const { return fLocalAlphaIdx; }

    // Convert the winding to alpha in row sized chunks. Technically, processChunk could be renamed
    // to processRow, but it is intended to be flexible, so that if the tile width were to exceed
    // the simd width, the row could be proccessed in serial chunks.
    SK_ALWAYS_INLINE void resolveWindingToAlpha() {
        uint8_t* tileAlphaBase = reserveAlphaBuffer();

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

    SK_ALWAYS_INLINE void rasterizeLineToTile(const Tile& tile, std::array<SkPoint, 2> tileBounds) {
        Line line = fPolyline.getLine(tile.lineIdx());
        bool canonicalXDir = line.p1.fX >= line.p0.fX;
        bool canonicalYDir = line.p1.fY >= line.p0.fY;

        uint32_t windingBit = tile.coarseWinding() ? 1 : 0;
        if constexpr (kIsWinding) {
            fCoarseWinding += (canonicalYDir ? 1 : -1) * static_cast<int32_t>(windingBit);
        } else {
            fCoarseWinding ^= static_cast<int32_t>(windingBit);
        }

        // TODO (thomsmit): remove once culling lands
        // Cull lines that exist entirely to the left of the tile.
        float rightEdge = canonicalXDir ? line.p1.fX : line.p0.fX;
        if (rightEdge < 0.0f) {
            return;
        }

        float dx = line.p1.fX - line.p0.fX;
        float dy = line.p1.fY - line.p0.fY;
        float idx = (std::abs(dx) <= Strip::kStripEpsilon) ? 0.0f : 1.0f / dx;
        float idy = (std::abs(dy) <= Strip::kStripEpsilon) ? 0.0f : 1.0f / dy;
        float dxdy = dx * idy;
        std::array<float, 4> derivs = {dx, dy, idx, idy};

        auto [clippedLine, topIsOnLeftEdge, botIsOnLeftEdge] =
                Tile::ClipToTile<kTileWidth, kTileHeight>(line,
                                                          tileBounds,
                                                          derivs,
                                                          tile.intersectionMask(),
                                                          canonicalXDir,
                                                          canonicalYDir);
        SkPoint pTop = clippedLine.p0;
        SkPoint pBot = clippedLine.p1;

        if (tile.hasLeftIntersection()) {
            float yEdge = (pTop.fX < pBot.fX) ? pTop.fY : pBot.fY;
            fillLeft(yEdge, canonicalXDir);
        }

        if (std::abs(dy) < Strip::kStripEpsilon && pTop.fY == std::floor(pTop.fY)) {
            return;
        }

        int32_t startY = static_cast<int32_t>(std::floor(pTop.fY));
        int32_t endY = static_cast<int32_t>(std::ceil(pBot.fY));
        auto edgeX = FindRowIntersections(pTop, pBot, dxdy, startY, endY);

        LineStepParams stepParams = computeLineStepParams(pTop, pBot, dx, dy);

        if (startY < endY) {
            bool isOnlyRow = (startY == endY - 1);
            float pTopY = pTop.fY;
            float pBotY = isOnlyRow ? pBot.fY : static_cast<float>(startY + 1);

            uint8_t startMaskVal = 0xff;
            bool startBumped = topIsOnLeftEdge && stepParams.firstRowTopTouch;
            if (!topIsOnLeftEdge) {
                uint32_t shift = static_cast<uint32_t>(
                        std::round(8.0f * (pTopY - static_cast<float>(startY))));
                startMaskVal = static_cast<uint8_t>(0xff << shift);
            }

            uint8_t endMaskVal = 0xff;
            if (isOnlyRow && !botIsOnLeftEdge) {
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
                           rightBump, crossed, edgeX, stepParams, canonicalYDir);

            for (int32_t row = startY + 1; row < endY - 1; ++row) {
                processRowSpan(row, 0xff, 0xff, false, false, false, true, edgeX, stepParams,
                               canonicalYDir);
            }

            if (!isOnlyRow) {
                int32_t lastY = endY - 1;
                float pBotYLast = pBot.fY;

                uint8_t endMaskLast = 0xff;
                if (!botIsOnLeftEdge) {
                    uint32_t shift = static_cast<uint32_t>(
                            std::round(8.0f * (pBotYLast - static_cast<float>(lastY))));
                    endMaskLast = static_cast<uint8_t>(~(0xff << shift));
                }

                uint8_t lMask = stepParams.xDir ? 0xff : endMaskLast;
                uint8_t rMask = stepParams.xDir ? endMaskLast : 0xff;

                processRowSpan(lastY, lMask, rMask, false, false, false, true, edgeX, stepParams,
                               canonicalYDir);
            }
        }
    }

private:
    static constexpr int32_t kTilePixelCount = kTileWidth * kTileHeight;
    static constexpr uint32_t kInitialWinding = kIsWinding ? 0x80808080u : 0u;

    struct LineStepParams {
        const uint8_t* maskRowLut;
        int32_t stepXFixed;
        int32_t stepYFixed;
        int32_t tBaseFixed;
        bool xDir;
        bool firstRowTopTouch;
        bool firstRowBumped;
    };

    template <int N>
    SK_ALWAYS_INLINE static void ApplyWinding(SwarPixel* target, uint32_t fillVal) {
        auto vSubsampleWinding = skvx::Vec<N, uint32_t>::Load(target);
        skvx::Vec<N, uint32_t> vFill(fillVal);
        if constexpr (kIsWinding) {
            auto wB = sk_bit_cast<skvx::Vec<N * 4, uint8_t>>(vSubsampleWinding);
            auto fB = sk_bit_cast<skvx::Vec<N * 4, uint8_t>>(vFill);
            vSubsampleWinding = sk_bit_cast<skvx::Vec<N, uint32_t>>(wB + fB);
        } else {
            vSubsampleWinding ^= vFill;
        }
        vSubsampleWinding.store(target);
    }

    // Evaluate the subsamples against the winding rule to determine which are "active" (covered).
    // For Even-Odd no processing is required. For the Non-Zero case, we compare subsample winding
    // against the empty SWAR mask. In SkVX, the result of !=, true, is ~0, so we mask away any
    // extraneous bits.
    template <typename VecT> static SK_ALWAYS_INLINE VecT GetActivesWide(VecT v) {
        using Vec8 = skvx::Vec<sizeof(VecT), uint8_t>;
        if constexpr (kIsWinding) {
            const Vec8 bias_bytes(0x80);
            Vec8 v_bytes = sk_bit_cast<skvx::Vec<sizeof(VecT), uint8_t>>(v);
            Vec8 cmp = (v_bytes != bias_bytes);
            return sk_bit_cast<VecT>(cmp) & VecT(0x01010101u);
        } else {
            SkASSERT(all(sk_bit_cast<Vec8>(v) <= 1));
            return v;
        }
    }

    SK_ALWAYS_INLINE uint8_t* reserveAlphaBuffer() {
        if (fAlphaBuf.size() + kTilePixelCount > fAlphaBuf.capacity()) {
            constexpr size_t kChunkSize = 4 * kTilePixelCount;
            fAlphaBuf.reserve(fAlphaBuf.capacity() + kChunkSize);
        }
        return fAlphaBuf.append(kTilePixelCount);
    }

    [[maybe_unused]]
    SK_ALWAYS_INLINE void observeChunk(int32_t row, int32_t column, int32_t chunkSize) {
        for (int32_t x = column; x < column + chunkSize; ++x) {
            SwarPixel v = fSubsampleWinding[row][x];
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
                if (ShouldFill(sLo)) exactMask |= (1 << s);
                if (ShouldFill(sHi)) exactMask |= (1 << (s + 4));
                lo >>= 8;
                hi >>= 8;
            }
            if (fIsInverse) {
                exactMask = ~exactMask & ((1 << Strip::kNumSubSamples) - 1);
            }
            fObserver(exactMask);
        }
    }

    // Resolves the subsample windings of a chunk of pixels to their equivalent 8-bit alpha values:
    //
    //      1) GetActivesWide(): Evaluates the winding rule, returning a skvx::Vec of the same size
    //         where each byte lane contains 1 (active) or 0 (inactive).
    //      2) activeLo / activeHi: Splits the active masks into two separate 32-bit words, each
    //         containing 4 subsamples (one per byte lane).
    //      3) combinedBytes: Sums the two halves together. Each byte lane now holds a value of 0,
    //         1, or 2.
    //      4) SWAR Horizontal Sum: Multiplying by 0x01010101u (which is 2^24 + 2^16 + 2^8 + 1)
    //         accumulates the sum of all 4 byte lanes into the highest byte. Given a 32-bit word
    //         [ A | B | C | D ], the multiplication expands across the lanes:
    //              Shifted by 0:  [ A | B | C | D ]
    //            + Shifted by 8:  [ B | C | D | 0 ]
    //            + Shifted by 16: [ C | D | 0 | 0 ]
    //            + Shifted by 24: [ D | 0 | 0 | 0 ]
    //         Summed together, the highest byte holds A + B + C + D. This can never overflow into
    //         neighboring lanes, since the maximum value per byte lane is 2.
    //      5) Shifting >> 24: Right-shifting the result isolates the final sum. This discards the
    //         lower accumulation bytes and moves the total subsample count into the lowest byte.
    //      6) Alpha Conversion: The total is multiplied by the maximum alpha value (255), summed by
    //         subsampleCount / 2 for rounding, and right-shifted by log2(subsampleCount). This
    //         keeps the entire conversion in integer math and avoids division.
    template <int ChunkSize>
    SK_ALWAYS_INLINE void processChunk(int32_t row,
                                       int32_t column,
                                       uint8_t* tileAlphaBase,
                                       int& localWriteIdx) {
        auto vSubsampleWinding =
                skvx::Vec<ChunkSize * 2, uint32_t>::Load(&fSubsampleWinding[row][column]);
        auto actives = GetActivesWide(vSubsampleWinding);

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

    SK_ALWAYS_INLINE static std::array<float, kTileHeight + 1> FindRowIntersections(
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

    SK_ALWAYS_INLINE LineStepParams computeLineStepParams(SkPoint pTop,
                                                          SkPoint pBot,
                                                          float dx,
                                                          float dy) const {
        float normalX = dy;
        float normalY = -dx;
        if (normalX < 0.0f) {
            normalX = -normalX;
            normalY = -normalY;
        }
        float D = normalX + std::abs(normalY);
        float invD = (D < Strip::kStripEpsilon) ? 0.0f : 1.0f / D;

        bool hasPositiveSlope = normalY <= 0.0f;
        float C = normalX * pTop.fX + normalY * pTop.fY;
        float s = std::abs(normalY) * invD;
        int lutRowOffset = std::clamp(
                static_cast<int>(std::floor(s * (Strip::kLutMaskHeight / 2))),
                0,
                (Strip::kLutMaskHeight / 2) - 1);
        int lutRow = hasPositiveSlope ? (lutRowOffset + Strip::kLutMaskHeight / 2) : lutRowOffset;

        // Unlike the scalar version, we simply return the raw pointer to the row in the LUT
        const uint8_t* maskRowLut = fMaskLut.data() + (lutRow * Strip::kLutMaskWidth);

        float stepX = normalX * invD;
        float stepY = normalY * invD;
        float tBase = ((hasPositiveSlope ? normalX : D) - C) * invD;

        // We use 16.16 fixed-point arithmetic to avoid floating-point math and conversions inside
        // the inner loops. A 16.16 fixed-point number uses a 32-bit integer where:
        //   - The upper 16 bits represent the integer part.
        //   - The lower 16 bits represent the fractional part.
        //
        // Encoding: value_fixed = round(x * 65536)
        // Decoding (floor): integer_part = value_fixed >> 16
        //
        // In addition to the 16.16 representation (scaled by 65536), we scale by the LUT's width
        // (64.0) so that extracting the LUT column index `u = floor(t * 64)` can be done using a
        // simple bit-shift: `u = clamp(tFixed >> 16, 0, 63)`.
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
                    ApplyWinding<16>(&fSubsampleWinding[row][column], fill32);
                }
            } else if constexpr (kTileWidth % 4 == 0) {
                for (int32_t column = 0; column < kTileWidth; column += 4) {
                    ApplyWinding<8>(&fSubsampleWinding[row][column], fill32);
                }
            }
        }
    }

    template <bool kIsEdgePixel>
    SK_ALWAYS_INLINE void processPixel(SwarPixel& pixel,
                                       uint8_t edgeMask,
                                       const PixelBytes& pBump,
                                       int32_t tFixed,
                                       const uint8_t* maskRowLut,
                                       bool canonicalYDir) {
        // Shift right by 16 to extract the integer LUT column index `u = floor(t * 64)`.
        int u = std::clamp(tFixed >> 16, 0, Strip::kLutMaskWidthExcl);
        uint8_t maskVal = maskRowLut[u];
        if constexpr (kIsEdgePixel) {
            maskVal &= edgeMask;
        }

    /*
     * Convert 1 byte of packed coverage bits (maskVal) into 8 separate SIMD byte lanes.
     * E.g., maskVal = 0b10100011:
     *
     * 1) Splat maskVal into all 8 lanes:
     *    [   L0    |    L1    |    L2    |    L3    |    L4    |    L5    |    L6    |   L7    ]
     *    [10100011 | 10100011 | 10100011 | 10100011 | 10100011 | 10100011 | 10100011 | 10100011]
     *
     * 2) Bitwise AND with vBit (2^0, 2^1, ..., 2^7) to isolate bit k in lane k:
     *  & [00000001 | 00000010 | 00000100 | 00001000 | 00010000 | 00100000 | 01000000 | 10000000]
     *    ---------------------------------------------------------------------------------------
     *    [00000001 | 00000010 | 00000000 | 00000000 | 00000000 | 00100000 | 00000000 | 10000000]
     *
     * 3) In SkVX, Vector comparison of != 0 yields ~0 for true (SkVx.h:L316-L318,L357-359); this is
     *    then negated to produce the correct winding across the lanes:
     * != 0:   [  0xFF   |  0xFF   |  0x00   |  0x00   |  0x00   |  0xFF   |  0x00   |  0xFF   ]
     * Unary -:[  0x01   |  0x01   |  0x00   |  0x00   |  0x00   |  0x01   |  0x00   |  0x01   ]
     *         (+1 for covered lanes, 0x00 for uncovered)
     *
     * WARNING: relies on pcmpeqb behavior for 3! If this is not true, this will fail.
     */
        const PixelBytes vBit{1, 2, 4, 8, 16, 32, 64, 128};
        PixelBytes pRes = -((PixelBytes(maskVal) & vBit) != 0);
        PixelBytes pSubsampleWinding = sk_bit_cast<PixelBytes>(pixel);

        if constexpr (kIsWinding) {
            pRes -= pBump;
            if (canonicalYDir) {
                pSubsampleWinding += pRes;
            } else {
                pSubsampleWinding -= pRes;
            }
        } else {
            pRes ^= pBump;
            pSubsampleWinding ^= pRes;
        }

        pixel = sk_bit_cast<SwarPixel>(pSubsampleWinding);
    }

    // The bumping masks could maybe be moved into templating, but for now simply expose them
    // as function arguments and rely on the compiler's DCE to optimize them.
    SK_ALWAYS_INLINE void processRowSpan(int32_t row,
                                         uint8_t leftMask, uint8_t rightMask,
                                         bool leftBump, bool midBump, bool rightBump,
                                         bool crossedTop,
                                         const std::array<float, kTileHeight + 1>& edgeX,
                                         const LineStepParams& params,
                                         bool canonicalYDir) {
        float pTopX = edgeX[row];
        float pBotX = edgeX[row + 1];

        float xMin = std::fmin(pTopX, pBotX);
        float xMax = std::fmax(pTopX, pBotX);
        int32_t xStart = std::clamp(static_cast<int32_t>(std::floor(xMin)), 0, kTileWidth - 1);
        int32_t xEnd = std::clamp(static_cast<int32_t>(std::floor(xMax)), 0, kTileWidth - 1);

        // Compute the initial translation parameter `tFixed` in 16.16 fixed-point format
        // (pre-scaled by 64 * 65536) at the starting pixel (xStart, row) of the span using:
        // tFixed = tBaseFixed + stepYFixed * row + stepXFixed * xStart
        int32_t tFixed =
                params.tBaseFixed + (params.stepYFixed * row) + (params.stepXFixed * xStart);
        SwarPixel* rowSubsampleWindings = fSubsampleWinding[row];

        PixelBytes pMidBump(midBump ? 1 : 0);

        if (xStart == xEnd) {
            uint8_t combinedMask = leftMask & rightMask;
            processPixel</*kIsEdgePixel=*/true>(rowSubsampleWindings[xStart], combinedMask,
                                                PixelBytes(leftBump ? 1 : 0), tFixed,
                                                params.maskRowLut, canonicalYDir);
            tFixed += params.stepXFixed;
        } else {
            processPixel</*kIsEdgePixel=*/true>(rowSubsampleWindings[xStart], leftMask,
                                                PixelBytes(leftBump ? 1 : 0), tFixed,
                                                params.maskRowLut, canonicalYDir);
            tFixed += params.stepXFixed;

            for (int32_t column = xStart + 1; column < xEnd; ++column) {
                processPixel</*kIsEdgePixel=*/false>(rowSubsampleWindings[column], 0, pMidBump,
                                                     tFixed, params.maskRowLut, canonicalYDir);
                tFixed += params.stepXFixed;
            }

            processPixel</*kIsEdgePixel=*/true>(rowSubsampleWindings[xEnd], rightMask,
                                                PixelBytes(rightBump ? 1 : 0), tFixed,
                                                params.maskRowLut, canonicalYDir);
            tFixed += params.stepXFixed;
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
                    ApplyWinding<16>(&fSubsampleWinding[row][column], fill32);
                    column += 8;
                }
            } else if constexpr (kTileWidth >= 4) {
                while (column + 4 <= kTileWidth) {
                    ApplyWinding<8>(&fSubsampleWinding[row][column], fill32);
                    column += 4;
                }
            }

            const PixelBytes pFill(fillByte);
            for (; column < kTileWidth; ++column) {
                PixelBytes pSubsampleWinding =
                        sk_bit_cast<PixelBytes>(fSubsampleWinding[row][column]);
                if constexpr (kIsWinding) {
                    pSubsampleWinding += pFill;
                } else {
                    pSubsampleWinding ^= pFill;
                }
                fSubsampleWinding[row][column] = sk_bit_cast<SwarPixel>(pSubsampleWinding);
            }
        }
    }

    SwarPixel fSubsampleWinding[kTileHeight][kTileWidth];
    int32_t fCoarseWinding = 0;
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
