/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkBitmap.h"
#include "include/core/SkClipOp.h"
#include "include/core/SkColorType.h"
#include "include/core/SkM44.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkSize.h"
#include "include/core/SkStrokeRec.h"
#include "src/base/SkVx.h"
#include "src/gpu/graphite/ClipStack.h"
#include "src/gpu/graphite/RasterPathUtils.h"
#include "src/gpu/graphite/geom/Shape.h"
#include "src/gpu/graphite/geom/Transform.h"
#include "tests/Test.h"

#include <cstdint>

using skgpu::graphite::ClipStack;
using skgpu::graphite::GenerateClipMaskKey;
using skgpu::graphite::GeneratePathMaskKey;
using skgpu::graphite::RasterMaskHelper;
using skgpu::graphite::Shape;
using skgpu::graphite::Transform;

namespace {

skvx::half2 origin(int x, int y) { return skvx::half2{(uint16_t)x, (uint16_t)y}; }

}  // namespace

// Two GeneratePathMaskKey calls with byte-identical inputs must produce equal
// keys.  This is the cache-hit guarantee atlas lookups depend on.
DEF_GRAPHITE_TEST(RasterPathUtils_PathMaskKey_SameInputsEqual,
                  reporter,
                  CtsEnforcement::kNextRelease) {
    Shape rectShape{SkRect::MakeWH(8, 8)};
    Transform identity{SkM44()};
    SkStrokeRec fill{SkStrokeRec::kFill_InitStyle};

    auto a = GeneratePathMaskKey(rectShape, identity, fill, origin(0, 0), origin(16, 16));
    auto b = GeneratePathMaskKey(rectShape, identity, fill, origin(0, 0), origin(16, 16));
    REPORTER_ASSERT(reporter, a == b);
}

// maskOrigin lives in the first key word (per add_transform_key wiring); a
// different origin must yield a different key so that two cache entries
// covering different sub-rects of the same path don't collide.
DEF_GRAPHITE_TEST(RasterPathUtils_PathMaskKey_OriginAffectsKey,
                  reporter,
                  CtsEnforcement::kNextRelease) {
    Shape rectShape{SkRect::MakeWH(8, 8)};
    Transform identity{SkM44()};
    SkStrokeRec fill{SkStrokeRec::kFill_InitStyle};

    auto a = GeneratePathMaskKey(rectShape, identity, fill, origin(0, 0),  origin(16, 16));
    auto b = GeneratePathMaskKey(rectShape, identity, fill, origin(8, 8),  origin(16, 16));
    REPORTER_ASSERT(reporter, a != b);
}

// maskSize is packed alongside the origin; a different size must change the
// key for the same reason as above.
DEF_GRAPHITE_TEST(RasterPathUtils_PathMaskKey_SizeAffectsKey,
                  reporter,
                  CtsEnforcement::kNextRelease) {
    Shape rectShape{SkRect::MakeWH(8, 8)};
    Transform identity{SkM44()};
    SkStrokeRec fill{SkStrokeRec::kFill_InitStyle};

    auto a = GeneratePathMaskKey(rectShape, identity, fill, origin(0, 0), origin(16, 16));
    auto b = GeneratePathMaskKey(rectShape, identity, fill, origin(0, 0), origin(32, 32));
    REPORTER_ASSERT(reporter, a != b);
}

// The transform's upper-left 2x2 (scale + skew) is bit-encoded into the key.
// Changing the scale must produce a distinct key.
DEF_GRAPHITE_TEST(RasterPathUtils_PathMaskKey_TransformAffectsKey,
                  reporter,
                  CtsEnforcement::kNextRelease) {
    Shape rectShape{SkRect::MakeWH(8, 8)};
    Transform identity{SkM44()};
    Transform scaled{SkM44::Scale(2.0f, 2.0f)};
    SkStrokeRec fill{SkStrokeRec::kFill_InitStyle};

    auto a = GeneratePathMaskKey(rectShape, identity, fill, origin(0, 0), origin(16, 16));
    auto b = GeneratePathMaskKey(rectShape, scaled,   fill, origin(0, 0), origin(16, 16));
    REPORTER_ASSERT(reporter, a != b);
}

// Stroke style is encoded in styleBits; fill, hairline, and full-stroke must
// all produce distinct keys.  Full-stroke also widens the key (extra width +
// miter words) per the styleKeySize branch in the implementation.
DEF_GRAPHITE_TEST(RasterPathUtils_PathMaskKey_StrokeStyleAffectsKey,
                  reporter,
                  CtsEnforcement::kNextRelease) {
    Shape rectShape{SkRect::MakeWH(8, 8)};
    Transform identity{SkM44()};

    SkStrokeRec fill{SkStrokeRec::kFill_InitStyle};
    SkStrokeRec hairline{SkStrokeRec::kHairline_InitStyle};
    SkStrokeRec stroke{SkStrokeRec::kFill_InitStyle};
    stroke.setStrokeStyle(/*width=*/1.5f, /*strokeAndFill=*/false);

    auto kFill = GeneratePathMaskKey(rectShape, identity, fill,     origin(0, 0), origin(16, 16));
    auto kHair = GeneratePathMaskKey(rectShape, identity, hairline, origin(0, 0), origin(16, 16));
    auto kStrk = GeneratePathMaskKey(rectShape, identity, stroke,   origin(0, 0), origin(16, 16));

    REPORTER_ASSERT(reporter, kFill != kHair);
    REPORTER_ASSERT(reporter, kFill != kStrk);
    REPORTER_ASSERT(reporter, kHair != kStrk);
}

// With element count <= kMaxShapeCountForKey (= 2), GenerateClipMaskKey takes
// the per-element path: usesPathKey is true and identical inputs hash equal.
DEF_GRAPHITE_TEST(RasterPathUtils_ClipMaskKey_PerElementPath_SameInputsEqual,
                  reporter,
                  CtsEnforcement::kNextRelease) {
    ClipStack::Element e{Shape{SkRect::MakeWH(10, 10)}, Transform{SkM44()}, SkClipOp::kIntersect};
    ClipStack::ElementList listA;
    listA.push_back(&e);
    ClipStack::ElementList listB;
    listB.push_back(&e);

    SkIRect deviceBounds = SkIRect::MakeWH(10, 10);
    SkIRect keyBoundsA{}, keyBoundsB{};
    bool usesPathKeyA = false, usesPathKeyB = false;

    auto kA = GenerateClipMaskKey(/*stackRecordID=*/42, &listA, deviceBounds,
                                  /*includeBounds=*/false, &keyBoundsA, &usesPathKeyA);
    auto kB = GenerateClipMaskKey(/*stackRecordID=*/42, &listB, deviceBounds,
                                  /*includeBounds=*/false, &keyBoundsB, &usesPathKeyB);

    REPORTER_ASSERT(reporter, usesPathKeyA);
    REPORTER_ASSERT(reporter, usesPathKeyB);
    REPORTER_ASSERT(reporter, kA == kB);
}

// With more than kMaxShapeCountForKey (=2) elements, the function falls back
// to a single-word key keyed only on stackRecordID, sets usesPathKey=false,
// and zeroes keyBounds.  The keys for two different stackRecordIDs must
// differ even if the elements are identical.
DEF_GRAPHITE_TEST(RasterPathUtils_ClipMaskKey_TooManyElementsFallsBackToStackRecord,
                  reporter,
                  CtsEnforcement::kNextRelease) {
    ClipStack::Element e0{Shape{SkRect::MakeWH(10, 10)}, Transform{SkM44()}, SkClipOp::kIntersect};
    ClipStack::Element e1{Shape{SkRect::MakeWH(20, 20)}, Transform{SkM44()}, SkClipOp::kIntersect};
    ClipStack::Element e2{Shape{SkRect::MakeWH(30, 30)}, Transform{SkM44()}, SkClipOp::kIntersect};
    ClipStack::ElementList list;
    list.push_back(&e0);
    list.push_back(&e1);
    list.push_back(&e2);

    SkIRect deviceBounds = SkIRect::MakeWH(30, 30);
    SkIRect keyBounds = SkIRect::MakeXYWH(7, 7, 7, 7);  // pre-fill to detect zeroing
    bool usesPathKey = true;

    auto k1 = GenerateClipMaskKey(/*stackRecordID=*/1, &list, deviceBounds,
                                  /*includeBounds=*/false, &keyBounds, &usesPathKey);
    REPORTER_ASSERT(reporter, !usesPathKey);
    REPORTER_ASSERT(reporter, keyBounds.isEmpty());

    auto k2 = GenerateClipMaskKey(/*stackRecordID=*/2, &list, deviceBounds,
                                  /*includeBounds=*/false, &keyBounds, &usesPathKey);
    REPORTER_ASSERT(reporter, k1 != k2);
}

// includeBounds=true appends two extra words (top-left and bottom-right) to
// the key.  Two calls with the same elements but different includeBounds must
// produce different keys.
DEF_GRAPHITE_TEST(RasterPathUtils_ClipMaskKey_IncludeBoundsAffectsKey,
                  reporter,
                  CtsEnforcement::kNextRelease) {
    ClipStack::Element e{Shape{SkRect::MakeWH(10, 10)}, Transform{SkM44()}, SkClipOp::kIntersect};
    ClipStack::ElementList list;
    list.push_back(&e);

    SkIRect deviceBounds = SkIRect::MakeWH(10, 10);
    SkIRect keyBounds{};
    bool usesPathKey = false;

    auto kNo  = GenerateClipMaskKey(/*stackRecordID=*/1, &list, deviceBounds,
                                    /*includeBounds=*/false, &keyBounds, &usesPathKey);
    auto kYes = GenerateClipMaskKey(/*stackRecordID=*/1, &list, deviceBounds,
                                    /*includeBounds=*/true,  &keyBounds, &usesPathKey);
    REPORTER_ASSERT(reporter, kNo != kYes);
}

// keyBounds must be returned as maskDeviceBounds offset by the negative of
// the unclipped bounds' top-left.  With identity transform, unclipped bounds
// equal the shape's bounds, so for a shape of [0,0,10,10] and device bounds
// at (3,4), the resulting keyBounds top-left should be (3,4).
DEF_GRAPHITE_TEST(RasterPathUtils_ClipMaskKey_KeyBoundsRelativeToUnclipped,
                  reporter,
                  CtsEnforcement::kNextRelease) {
    ClipStack::Element e{Shape{SkRect::MakeWH(10, 10)}, Transform{SkM44()}, SkClipOp::kIntersect};
    ClipStack::ElementList list;
    list.push_back(&e);

    SkIRect deviceBounds = SkIRect::MakeXYWH(3, 4, 5, 5);
    SkIRect keyBounds{};
    bool usesPathKey = false;
    (void)GenerateClipMaskKey(/*stackRecordID=*/1, &list, deviceBounds,
                              /*includeBounds=*/true, &keyBounds, &usesPathKey);

    REPORTER_ASSERT(reporter, keyBounds.left() == 3);
    REPORTER_ASSERT(reporter, keyBounds.top()  == 4);
    REPORTER_ASSERT(reporter, keyBounds.width()  == 5);
    REPORTER_ASSERT(reporter, keyBounds.height() == 5);
}

// Allocate must produce an A8 bitmap of size + 2*padding on each side, and
// every byte (including the padded ring) must equal initialAlpha.
DEF_GRAPHITE_TEST(RasterPathUtils_RasterMaskHelper_AllocatePadsAndPrefills,
                  reporter,
                  CtsEnforcement::kNextRelease) {
    constexpr SkAlpha kInit = 0x33;
    auto [bm, helper] = RasterMaskHelper::Allocate(SkISize::Make(4, 4),
                                                   /*translation=*/{0, 0},
                                                   /*padding=*/2,
                                                   /*initialAlpha=*/kInit);

    REPORTER_ASSERT(reporter, bm.colorType() == kAlpha_8_SkColorType);
    REPORTER_ASSERT(reporter, bm.width()  == 8);  // 4 + 2*2
    REPORTER_ASSERT(reporter, bm.height() == 8);

    bool allInit = true;
    for (int y = 0; y < bm.height() && allInit; ++y) {
        for (int x = 0; x < bm.width(); ++x) {
            if (*bm.getAddr8(x, y) != kInit) { allInit = false; break; }
        }
    }
    REPORTER_ASSERT(reporter, allInit);
}

// drawShape with a fill SkStrokeRec on an axis-aligned rect under identity
// transform should write 0xFF inside the rect and leave pixels outside the
// rect (still inside the inner pixmap) at the initialAlpha.
DEF_GRAPHITE_TEST(RasterPathUtils_RasterMaskHelper_DrawShapeWritesAlpha,
                  reporter,
                  CtsEnforcement::kNextRelease) {
    auto [bm, helper] = RasterMaskHelper::Allocate(SkISize::Make(8, 8),
                                                   /*translation=*/{0, 0},
                                                   /*padding=*/0,
                                                   /*initialAlpha=*/0);

    Shape rectShape{SkRect::MakeLTRB(2, 2, 6, 6)};
    Transform identity{SkM44()};
    SkStrokeRec fill{SkStrokeRec::kFill_InitStyle};

    helper.drawShape(rectShape, identity, fill);

    // Inside the rect: opaque coverage.
    REPORTER_ASSERT(reporter, *bm.getAddr8(3, 3) == 0xFF);
    REPORTER_ASSERT(reporter, *bm.getAddr8(5, 5) == 0xFF);
    // Outside the rect: still zero.
    REPORTER_ASSERT(reporter, *bm.getAddr8(0, 0) == 0x00);
    REPORTER_ASSERT(reporter, *bm.getAddr8(7, 7) == 0x00);
}

// drawClip with alpha != 0xFF takes the drawPath branch (not drawPathCoverage)
// and the resulting interior pixels should equal the requested alpha exactly.
DEF_GRAPHITE_TEST(RasterPathUtils_RasterMaskHelper_DrawClipAlpha,
                  reporter,
                  CtsEnforcement::kNextRelease) {
    auto [bm, helper] = RasterMaskHelper::Allocate(SkISize::Make(8, 8),
                                                   /*translation=*/{0, 0},
                                                   /*padding=*/0,
                                                   /*initialAlpha=*/0);

    Shape rectShape{SkRect::MakeLTRB(2, 2, 6, 6)};
    Transform identity{SkM44()};

    helper.drawClip(rectShape, identity, /*alpha=*/0x80);

    // Interior pixel should be exactly the requested alpha (kSrc blend mode +
    // unpremul color builds A8 = alpha).
    REPORTER_ASSERT(reporter, *bm.getAddr8(3, 3) == 0x80);
    REPORTER_ASSERT(reporter, *bm.getAddr8(5, 5) == 0x80);
    // Outside the rect: still zero.
    REPORTER_ASSERT(reporter, *bm.getAddr8(0, 0) == 0x00);
}

// Allocate's translation argument is applied via postTranslate inside drawShape,
// so a rect drawn at the origin in shape-space lands offset by (translation +
// padding) in the bitmap.
DEF_GRAPHITE_TEST(RasterPathUtils_RasterMaskHelper_TranslationApplied,
                  reporter,
                  CtsEnforcement::kNextRelease) {
    constexpr int kPad = 1;
    auto [bm, helper] = RasterMaskHelper::Allocate(SkISize::Make(8, 8),
                                                   /*translation=*/{2, 3},
                                                   /*padding=*/kPad,
                                                   /*initialAlpha=*/0);

    Shape rectShape{SkRect::MakeLTRB(0, 0, 2, 2)};
    Transform identity{SkM44()};
    SkStrokeRec fill{SkStrokeRec::kFill_InitStyle};

    helper.drawShape(rectShape, identity, fill);

    // The rect should land at translation + padding = (3, 4) in bitmap space,
    // covering pixels [3..5) x [4..6).
    REPORTER_ASSERT(reporter, *bm.getAddr8(3, 4) == 0xFF);
    REPORTER_ASSERT(reporter, *bm.getAddr8(4, 5) == 0xFF);
    // Original origin (post-pad) should be untouched.
    REPORTER_ASSERT(reporter, *bm.getAddr8(kPad, kPad) == 0x00);
}
