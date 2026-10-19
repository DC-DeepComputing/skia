/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "src/gpu/graphite/sparse_strips/SparseStripsCoverageStep.h"

#include "include/core/SkSize.h"
#include "include/private/SkAssert.h"
#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/sparse_strips/SparseStripsConfig.h"

#include <algorithm>

namespace skgpu::graphite {

namespace {

using ResourceType = ComputeStep::ResourceType;
using DataFlow = ComputeStep::DataFlow;
using ResourcePolicy = ComputeStep::ResourcePolicy;
using Slot = SparseStripsCoverageStep::Slot;

static_assert(SparseStripsCoverageStep::kLanesPerTile == SparseStripConfig::kTileHeight);
static_assert(SparseStripConfig::kTileWidth == 8 && SparseStripConfig::kNumSubSamples == 8);
static_assert(SparseStripConfig::kAtlasWidth == 8192);
static_assert(SparseStripConfig::kColorType == kRGBA_8888_SkColorType);

// Both TileWork and PreMergeTile are read as uint4s: a TileWork is one, and a PreMergeTile is two,
// {fAlphaIndex, fPackedLineIdxIntersectionMask, fPackedScannedWinding, fPackedXY} and fLine.
constexpr ComputeStep::ResourceDesc kResources[] = {
        {ResourceType::kReadOnlyStorageBuffer, DataFlow::kShared, ResourcePolicy::kNone,
         Slot::kTilesSlot, "TileBlock { uint4 tiles[]; }"},
        {ResourceType::kReadOnlyStorageBuffer, DataFlow::kShared, ResourcePolicy::kNone,
         Slot::kPmtsSlot, "PmtBlock { uint4 pmts[]; }"},
        {ResourceType::kReadOnlyStorageBuffer, DataFlow::kShared, ResourcePolicy::kNone,
         Slot::kMaskLUTSlot, "MaskLUTBlock { uint maskLUT[]; }"},
        {ResourceType::kWriteOnlyStorageTexture, DataFlow::kShared, ResourcePolicy::kNone,
         Slot::kAtlasSlot, "atlas"},
};

// The coverage kernel. The per-pixel rules are a straight port of StripProcessorSimd (and of
// Tile::ClipToTile), with the same floating point operations, so that the GPU and CPU paths only
// differ where the hardware rounds differently (e.g. FMA contraction).
//
// Each PreMergeTile changes the winding of sample s of pixel (r, c) by
//     backdrop                                (first PreMergeTile of the tile only)
//   + fillSgn * [r >= fillStart]                                            fillLeft()
//   + sgn     * [crossedTop(r) && c > xEnd(r)]                              processRowSpan() tail
//   + sgn     * (bit_s(mask(r, c)) - inv(r, c))   for c in [xStart(r), xEnd(r)]   processPixel()
// with sgn = canonicalYDir ? 1 : -1. All but the mask bit are the same for every sample of a pixel,
// so they are accumulated per pixel ("coarse", one byte per pixel), and only the masks per sample
// ("fine", one byte per sample). Like the CPU's SWAR subsample winding, each byte wraps on its own.
//
// Each invocation owns a row of the tile (its lane within the 8 invocations of the tile) and
// computes that row's coarse winding. The masks are only needed where the line crosses a pixel, at
// most 2 pixels per row if the line is steep (|dx| <= |dy|), and at most 2 pixels per column if it
// is shallow, in which case each invocation computes the masks of its column instead, and they are
// transposed afterwards. The orientation only selects which pixels an invocation takes; the rest
// is the same code, so the 8 invocations of the other tiles in a subgroup don't diverge.
constexpr char kCoverageSkSL[] = R"SKSL(
const float kEps = 0.00001;          // SparseStripConfig::kStripEpsilon
const float kFixedMult = 4194304.0;  // computeLineStepParams()'s kFixedMult, 64 * 65536
const uint kMaskT = 1u;
const uint kMaskB = 2u;
const uint kMaskL = 4u;
const uint kMaskR = 8u;
// A set of "along" indices (see pixel_bytes()) holds index k in bit 4k + 3.
const uint kNibbleMsbs = 0x88888888u;

// ---- Per-byte SWAR on the 8 bytes of a uint2, which wrap like skvx::Vec<8, uint8_t> ----

uint2 add8(uint2 a, uint2 b) {
    const uint2 kLow7 = uint2(0x7F7F7F7Fu);
    return ((a & kLow7) + (b & kLow7)) ^ ((a ^ b) & uint2(0x80808080u));
}

uint2 splat8(int v) {
    return uint2((uint(v) & 0xFFu) * 0x01010101u);
}

// Bytes of 0 or 1 to bytes of 0 or s, for s = +/-1.
uint2 signed8(uint2 v01, int s) {
    return s < 0 ? v01 * 0xFFu : v01;
}

// Bit k of m to byte k.
uint2 expand8(uint m) {
    return uint2(((m & 0xFu) * 0x00204081u) & 0x01010101u,
                 ((m >> 4u) * 0x00204081u) & 0x01010101u);
}

// 1 in the bytes with an index of at least k.
uint2 ge8(int k) {
    uint kk = uint(clamp(k, 0, 8));
    uint lo = kk < 4u ? (0x01010101u << (8u * kk)) : 0u;
    uint hi = kk <= 4u ? 0x01010101u : (kk < 8u ? (0x01010101u << (8u * (kk - 4u))) : 0u);
    return uint2(lo, hi);
}

// 1 in the bytes with an index in [lo, hi].
uint2 ones8(int lo, int hi) {
    return lo <= hi ? (ge8(lo) ^ ge8(hi + 1)) : uint2(0u);
}

uint byte_at(uint2 v, uint i) {
    return ((i >= 4u ? v.y : v.x) >> ((i & 3u) * 8u)) & 0xFFu;
}

// The most significant bit of each nonzero byte.
uint nonzero8(uint x) {
    return (((x & 0x7F7F7F7Fu) + 0x7F7F7F7Fu) | x) & 0x80808080u;
}

// ---- The geometry of one PreMergeTile: rasterizeLineToTileImpl()'s setup ----

struct Geo {
    float2 pTop;      // Tile-local and sorted top to bottom, like Tile::ClipToTile().
    float2 pBot;
    int sgn;          // canonicalYDir ? 1 : -1
    int fillStart;    // fillLeft(): rows >= fillStart (8 = none) get fillSgn.
    int fillSgn;
    int startY;       // The rows [startY, endY) have masks and crossed-top fills.
    int endY;
    float dxdy;
    bool crossedTop0;
    uint lm0;         // The truncation masks of the left and right pixel of the first and last row.
    uint rm0;
    uint lmL;
    uint rmL;
    bool linv0;       // The inversions of the left, middle and right pixels of the first row.
    bool minv0;
    bool rinv0;
    uint lutRow;      // computeLineStepParams()
    int stepX;
    int stepY;
    int tBase;
    bool rotate;      // Whether the invocations take columns instead of rows.
};

// Tile::ClipToTile()'s clipPt, including its association p0 + ((bound - start) * invD) * d. (SkSL
// compute functions can't have out parameters on Metal, so this returns a struct.)
struct ClipPt {
    float2 p;
    bool onLeft;
};

ClipPt clip_pt(float2 p, float2 p0, float2 d, float2 invD, uint mask, uint hits, uint maskH,
               float boundH, float boundV, float2 tileMin) {
    if (hits == 0u) {
        return ClipPt(clamp(p - tileMin, float2(0.0), float2(8.0)), false);
    }
    bool useH = (mask & maskH) != 0u;
    float bound = useH ? boundH : boundV;
    float t = (bound - (useH ? p0.y : p0.x)) * (useH ? invD.y : invD.x);
    float2 q = p0 + t * d;
    if (useH) {
        q.y = bound;
        q -= tileMin;
        q.x = clamp(q.x, 0.0, 8.0);
        return ClipPt(q, false);
    }
    q.x = bound;
    q -= tileMin;
    q.y = clamp(q.y, 0.0, 8.0);
    return ClipPt(q, bound == tileMin.x);
}

// GetTruncationMask()'s std::round(8 * (p - row)), which rounds half away from zero. The clamp only
// matters for rows that aren't rasterized, and keeps the shifts of the masks in range.
uint trunc_shift(float p, int row) {
    float x = 8.0 * (p - float(row));
    float f = floor(x);
    return uint(clamp(x - f >= 0.5 ? f + 1.0 : f, 0.0, 8.0));
}

Geo setup(float4 seg, uint mask, float2 tileMin) {
    Geo g;
    float2 p0 = seg.xy;
    float2 p1 = seg.zw;
    float2 d = p1 - p0;
    float2 invD = float2(abs(d.x) <= kEps ? 0.0 : 1.0 / d.x,
                         abs(d.y) <= kEps ? 0.0 : 1.0 / d.y);
    bool cx = p1.x >= p0.x;
    bool cy = p1.y >= p0.y;
    g.sgn = cy ? 1 : -1;
    g.rotate = abs(d.x) > abs(d.y);

    float2 tileMax = tileMin + float2(8.0);
    uint entryH = cy ? kMaskT : kMaskB;
    uint exitH = cy ? kMaskB : kMaskT;
    uint entryV = cx ? kMaskL : kMaskR;
    uint exitV = cx ? kMaskR : kMaskL;
    ClipPt entry = clip_pt(p0, p0, d, invD, mask, mask & (entryV | entryH), entryH,
                           cy ? tileMin.y : tileMax.y, cx ? tileMin.x : tileMax.x, tileMin);
    ClipPt exit = clip_pt(p1, p0, d, invD, mask, mask & (exitV | exitH), exitH,
                          cy ? tileMax.y : tileMin.y, cx ? tileMax.x : tileMin.x, tileMin);
    float2 pTop = cy ? entry.p : exit.p;
    float2 pBot = cy ? exit.p : entry.p;
    bool topOnLeft = cy ? entry.onLeft : exit.onLeft;
    bool botOnLeft = cy ? exit.onLeft : entry.onLeft;
    g.pTop = pTop;
    g.pBot = pBot;

    // fillLeft() happens before the horizontal early out.
    g.fillSgn = cx ? -1 : 1;
    g.fillStart = 8;
    if ((mask & kMaskL) != 0u) {
        g.fillStart = int(ceil(pTop.x < pBot.x ? pTop.y : pBot.y));
    }

    g.startY = int(floor(pTop.y));
    g.endY = int(ceil(pBot.y));
    if (abs(d.y) < kEps && pTop.y == floor(pTop.y)) {
        g.endY = g.startY;  // The horizontal early out: no masks or crossed-top fills.
    }
    g.dxdy = d.x * invD.y;

    // The first and last row rules of the processRowSpan() calls.
    bool onlyRow = g.startY == g.endY - 1;
    bool sorted = pTop.x <= pBot.x;
    g.crossedTop0 = pTop.y == floor(pTop.y);
    bool defaultInv = !g.crossedTop0 && sorted;
    bool startInv = topOnLeft && !g.crossedTop0;
    uint startMask = topOnLeft ? 0xFFu : ((0xFFu << trunc_shift(pTop.y, g.startY)) & 0xFFu);
    uint endMask = (onlyRow && !botOnLeft)
                           ? (~(0xFFu << trunc_shift(pBot.y, g.startY)) & 0xFFu)
                           : 0xFFu;
    g.lm0 = sorted ? startMask : endMask;
    g.rm0 = sorted ? endMask : startMask;
    g.linv0 = sorted ? startInv : defaultInv;
    g.rinv0 = sorted ? defaultInv : startInv;
    g.minv0 = defaultInv;
    uint endMaskLast = botOnLeft ? 0xFFu : (~(0xFFu << trunc_shift(pBot.y, g.endY - 1)) & 0xFFu);
    g.lmL = sorted ? 0xFFu : endMaskLast;
    g.rmL = sorted ? endMaskLast : 0xFFu;

    // computeLineStepParams(), in 16.16 fixed point pre-scaled by the LUT width.
    float2 n = float2(d.y, -d.x);
    if (n.x < 0.0) {
        n = -n;
    }
    float dn = n.x + abs(n.y);
    float invDn = dn < kEps ? 0.0 : 1.0 / dn;
    bool posSlope = n.y <= 0.0;
    float c = n.x * pTop.x + n.y * pTop.y;
    int rowOffset = clamp(int(floor(abs(n.y) * invDn * 32.0)), 0, 31);
    g.lutRow = uint(posSlope ? rowOffset + 32 : rowOffset) * 64u;
    g.stepX = int(n.x * invDn * kFixedMult);
    g.stepY = int(n.y * invDn * kFixedMult);
    g.tBase = int(((posSlope ? n.x : dn) - c) * invDn * kFixedMult);
    return g;
}

// FindRowIntersections(), including its endpoint overrides.
float row_x(Geo g, int k) {
    if (k == g.startY) {
        return g.pTop.x;
    }
    if (k == g.endY) {
        return g.pBot.x;
    }
    return g.pTop.x + (float(k) - g.pTop.y) * g.dxdy;
}

// ---- The span table: every row's [xStart, xEnd] in one register ----
// With f_k = clamp(floor(rowInt[k]), 0, 7), processRowSpan()'s xStart(r) is min(f_r, f_r+1) and
// xEnd(r) is max(f_r, f_r+1), exactly, since floor and clamp are monotonic. Lane k computes f_k,
// and the tile's 8 lanes gather them into nibble k of `t`. f_8 is only read when endY is 8, where
// rowInt[8] is pBot.x, so every lane computes it. Since the coarse winding (by row) and the masks
// (by row or column) are all derived from this one table, they always agree on which pixels are
// crossed.

uint span_table(Geo g, uint lane, uint sid) {
    uint t = uint(clamp(floor(row_x(g, int(lane))), 0.0, 7.0)) << (4u * lane);
    t |= subgroupShuffle(t, sid ^ 1u);
    t |= subgroupShuffle(t, sid ^ 2u);
    t |= subgroupShuffle(t, sid ^ 4u);
    return t;
}

uint2 row_span(uint t, uint f8, uint r) {
    uint a = (t >> (4u * r)) & 7u;
    uint b = r < 7u ? ((t >> (4u * r + 4u)) & 7u) : f8;
    return uint2(min(a, b), max(a, b));
}

// The along indices [lo, hi], for lo <= hi <= 7.
uint nibble_range(uint lo, uint hi) {
    return (kNibbleMsbs << (4u * lo)) & (kNibbleMsbs >> (4u * (7u - hi)));
}

// The indices of the nibbles of x that are at most c, for nibbles and c in [0, 7]. Every nibble of
// (8 + c) - x is in [1, 15], so nothing borrows across nibbles.
uint nibbles_le(uint x, uint c) {
    return ((kNibbleMsbs | (c * 0x11111111u)) - x) & kNibbleMsbs;
}

// The rows whose span contains column c: those whose span ends aren't both right of c, and aren't
// both left of it.
uint column_rows(uint t, uint f8, uint c) {
    uint a = t;                          // nibble r: f_r
    uint b = (t >> 4u) | (f8 << 28u);    // nibble r: f_r+1
    uint right = ~nibbles_le(a, c) & ~nibbles_le(b, c);
    uint left = c > 0u ? (nibbles_le(a, c - 1u) & nibbles_le(b, c - 1u)) : 0u;
    return ~(right | left) & kNibbleMsbs;
}

// processPixel()'s mask of the crossed pixel (r, c), with sp = [xStart(r), xEnd(r)]. Its inversion
// is part of the coarse winding instead.
uint pixel_mask(Geo g, uint r, uint c, uint2 sp) {
    bool firstRow = int(r) == g.startY;
    bool lastRow = int(r) == g.endY - 1;
    uint lm = firstRow ? g.lm0 : (lastRow ? g.lmL : 0xFFu);
    uint rm = firstRow ? g.rm0 : (lastRow ? g.rmL : 0xFFu);
    // A single pixel span uses leftMask & rightMask, otherwise only the ends of the span truncate.
    uint truncMask = sp.x == sp.y ? (lm & rm) : (c == sp.x ? lm : (c == sp.y ? rm : 0xFFu));
    int tFixed = g.tBase + g.stepY * int(r) + g.stepX * int(c);
    uint i = g.lutRow + uint(clamp(tFixed >> 16, 0, 63));
    return ((maskLUT[i >> 2u] >> ((i & 3u) * 8u)) & 0xFFu) & truncMask;
}

// The coarse winding of row r: fillLeft(), the crossed-top fill right of the span, and the
// inversions of the first row's crossed pixels.
uint2 coarse_row(Geo g, int r, int2 sp) {
    uint2 v = r >= g.fillStart ? splat8(g.fillSgn) : uint2(0u);
    if (r >= g.startY && r < g.endY) {
        if (r > g.startY || g.crossedTop0) {
            v = add8(v, signed8(ones8(sp.y + 1, 7), g.sgn));
        }
        if (r == g.startY) {
            uint2 inv = g.linv0 ? ones8(sp.x, sp.x) : uint2(0u);
            if (sp.x != sp.y) {
                inv |= (g.minv0 ? ones8(sp.x + 1, sp.y - 1) : uint2(0u)) |
                       (g.rinv0 ? ones8(sp.y, sp.y) : uint2(0u));
            }
            v = add8(v, signed8(inv, -g.sgn));
        }
    }
    return v;
}

// Returns the mask of the next crossed pixel of this lane's row, or of its column if the tile is
// rotated, in byte `a` of the result, where `a`, the lowest index in `along`, is the pixel's
// column, or row if rotated. The caller then removes `a` from `along`.
uint2 pixel_bytes(Geo g, uint t, uint f8, uint lane, uint along) {
    bool ok = along != 0u;
    uint a = ok ? (uint(findLSB(along)) >> 2u) : 0u;
    uint r = g.rotate ? a : lane;
    uint c = g.rotate ? lane : a;
    uint v = (ok ? pixel_mask(g, r, c, row_span(t, f8, r)) : 0u) << ((a & 3u) * 8u);
    return a >= 4u ? uint2(0u, v) : uint2(v, 0u);
}

// Transposes the 8x8 bytes of a tile's lanes: byte j of lane i trades places with byte i of lane j.
// Each step swaps the blocks whose lane and byte indices differ in one bit.
uint2 transpose8x8(uint2 v, uint lane, uint sid) {
    bool up = (lane & 4u) != 0u;
    uint r = subgroupShuffle(up ? v.x : v.y, sid ^ 4u);
    v = up ? uint2(r, v.y) : uint2(v.x, r);

    up = (lane & 2u) != 0u;
    r = subgroupShuffle(up ? ((v.x & 0xFFFFu) | (v.y << 16u))
                           : ((v.x >> 16u) | (v.y & 0xFFFF0000u)), sid ^ 2u);
    v = up ? uint2((v.x & 0xFFFF0000u) | (r & 0xFFFFu), (v.y & 0xFFFF0000u) | (r >> 16u))
           : uint2((v.x & 0xFFFFu) | (r << 16u), (v.y & 0xFFFFu) | (r & 0xFFFF0000u));

    up = (lane & 1u) != 0u;
    r = subgroupShuffle(up ? ((v.x & 0x00FF00FFu) | ((v.y & 0x00FF00FFu) << 8u))
                           : (((v.x >> 8u) & 0x00FF00FFu) | (v.y & 0xFF00FF00u)), sid ^ 1u);
    v = up ? uint2((v.x & 0xFF00FF00u) | (r & 0x00FF00FFu),
                   (v.y & 0xFF00FF00u) | ((r >> 8u) & 0x00FF00FFu))
           : uint2((v.x & 0x00FF00FFu) | ((r & 0x00FF00FFu) << 8u),
                   (v.y & 0x00FF00FFu) | (r & 0xFF00FF00u));
    return v;
}

void main() {
    // A tile's 8 lanes are an aligned group of invocations of one subgroup. The subgroups of the
    // workgroup are full, so this numbers the workgroup's tiles 0 to 7.
    uint sid = sk_SubgroupInvocationID;
    uint lane = sid & 7u;
    uint tileInWorkgroup = sk_SubgroupID * (sk_SubgroupSize >> 3u) + (sid >> 3u);
    uint workgroupIndex = sk_WorkgroupID.y * sk_NumWorkgroups.x + sk_WorkgroupID.x;

    // TileWork: {PreMergeTile start, count | backdrop << 24, alpha index | flags, packed x, y}.
    uint4 tile = tiles[workgroupIndex * 8u + tileInWorkgroup];
    uint count = tile.y & 0xFFFFFFu;
    float2 tileMin = float2(float(tile.w & 0xFFFFu), float(tile.w >> 16u)) * 8.0;

    uint2 fine[8];      // Per pixel of the lane's row: the masks' winding of each sample.
    for (int c = 0; c < 8; ++c) {
        fine[c] = uint2(0u);
    }
    uint2 coarse = splat8(int(tile.y) >> 24);   // Per pixel of the lane's row: the rest.

    // Loop as long as the subgroup's longest tile, so the shuffles stay in uniform control flow.
    uint iterations = subgroupMax(count);
    for (uint i = 0u; i < iterations; ++i) {
        bool isActive = i < count;
        uint index = tile.x + min(i, max(count, 1u) - 1u);   // In bounds, even if inactive.
        uint4 header = pmts[2u * index];
        Geo g = setup(uintBitsToFloat(pmts[2u * index + 1u]), header.y & 0xFu, tileMin);
        g.rotate = g.rotate && isActive;

        uint t = span_table(g, lane, sid);
        uint f8 = uint(clamp(floor(g.pBot.x), 0.0, 7.0));
        uint2 rowSpan = row_span(t, f8, lane);
        uint rows = g.startY < g.endY ? nibble_range(uint(g.startY), uint(g.endY - 1)) : 0u;

        // The crossed pixels of the lane's row, or of its column if rotated, as along indices.
        uint ownRow = (rows & (8u << (4u * lane))) != 0u ? nibble_range(rowSpan.x, rowSpan.y)
                                                         : 0u;
        uint along = !isActive ? 0u : (g.rotate ? (column_rows(t, f8, lane) & rows) : ownRow);

        uint2 m = uint2(0u);
        m |= pixel_bytes(g, t, f8, lane, along);   // At most 2 crossed pixels, ...
        along &= along - 1u;
        m |= pixel_bytes(g, t, f8, lane, along);
        along &= along - 1u;
        while (subgroupAny(along != 0u)) {          // ... unless rounding makes it 3.
            m |= pixel_bytes(g, t, f8, lane, along);
            along &= along - 1u;
        }
        if (subgroupAny(g.rotate)) {
            uint2 transposed = transpose8x8(m, lane, sid);
            m = g.rotate ? transposed : m;
        }

        if (isActive) {
            coarse = add8(coarse, coarse_row(g, int(lane), int2(rowSpan)));
        }
        for (int c = 0; c < 8; ++c) {
            fine[c] = add8(fine[c], signed8(expand8(byte_at(m, uint(c))), g.sgn));
        }
    }
    if (count == 0u) {
        return;  // Padding
    }

    // Resolve to alpha, exactly like StripProcessorSimd::processChunk().
    bool evenOdd = (tile.z & 1u) != 0u;
    bool isInverse = (tile.z & 2u) != 0u;
    uint alphas[8];
    for (int c = 0; c < 8; ++c) {
        uint2 s = add8(fine[c], splat8(int(byte_at(coarse, uint(c)))));
        int covered = evenOdd ? bitCount(s.x & 0x01010101u) + bitCount(s.y & 0x01010101u)
                              : bitCount(nonzero8(s.x)) + bitCount(nonzero8(s.y));
        uint n = uint(covered);
        alphas[c] = isInverse ? 255u - ((n * 255u + 3u) >> 3u) : ((n * 255u + 4u) >> 3u);
    }

    // The tile's 64 alphas are 16 consecutive texels of an 8192 texel wide row, row-major, with
    // the leftmost pixel in R (see endcap_coverage_fn).
    uint t0 = ((tile.z & 0xFFFFFFC0u) >> 2u) + 2u * lane;
    uint t1 = t0 + 1u;
    textureWrite(atlas, uint2(t0 & 8191u, t0 >> 13u),
                 half4(float4(alphas[0], alphas[1], alphas[2], alphas[3]) / 255.0));
    textureWrite(atlas, uint2(t1 & 8191u, t1 >> 13u),
                 half4(float4(alphas[4], alphas[5], alphas[6], alphas[7]) / 255.0));
}
)SKSL";

}  // namespace

SparseStripsCoverageStep::SparseStripsCoverageStep()
        : ComputeStep(/*name=*/"SparseStripsCoverage",
                      /*localDispatchSize=*/{kWorkgroupSize, 1, 1},
                      kResources) {}

SparseStripsCoverageStep::~SparseStripsCoverageStep() = default;

WorkgroupSize SparseStripsCoverageStep::DispatchSize(uint32_t tileCount) {
    const uint32_t workgroups = (tileCount + kTilesPerWorkgroup - 1) / kTilesPerWorkgroup;
    const uint32_t width = std::min(workgroups, kMaxWorkgroupsPerDimension);
    const uint32_t height = width ? (workgroups + width - 1) / width : 0;
    SkASSERT(height <= kMaxWorkgroupsPerDimension);
    return WorkgroupSize(width, height, 1);
}

std::string SparseStripsCoverageStep::computeSkSL() const { return kCoverageSkSL; }

std::tuple<SkISize, SkColorType> SparseStripsCoverageStep::calculateTextureParameters(
        int resourceIndex, const ResourceDesc&) const {
    // The atlas page is always assigned by the dispatch; only its format is needed.
    SkASSERT(resourceIndex == Slot::kAtlasSlot);
    return {SkISize::MakeEmpty(), SparseStripConfig::kColorType};
}

std::optional<uint32_t> SparseStripsCoverageStep::preferredSubgroupSize(const Caps& caps) const {
    // A tile's lanes must be in one subgroup, and keep the tile's whole state in registers. So keep
    // subgroups between 8 and 64 invocations, e.g. on Adreno, which can run 128, where possible.
    // TODO (thomsmit): Without subgroup size control, require that subgroups have at least 8
    // invocations some other way.
    if (!caps.subgroupSizeControlSupport()) {
        return std::nullopt;
    }
    if (caps.maxSubgroupSize() > 64) {
        return 64;
    }
    if (caps.minSubgroupSize() < kLanesPerTile) {
        return kLanesPerTile;
    }
    return std::nullopt;
}

}  // namespace skgpu::graphite
