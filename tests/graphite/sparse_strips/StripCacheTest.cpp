/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "include/core/SkPathBuilder.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkSpan.h"
#include "include/core/SkSurface.h"
#include "include/gpu/graphite/Context.h"
#include "include/gpu/graphite/ContextOptions.h"
#include "include/gpu/graphite/Recorder.h"
#include "include/gpu/graphite/Recording.h"
#include "include/gpu/graphite/Surface.h"
#include "include/private/SkFloatingPoint.h"
#include "include/private/SkTArray.h"
#include "src/gpu/graphite/AtlasProvider.h"
#include "src/gpu/graphite/ContextOptionsPriv.h"
#include "src/gpu/graphite/ContextPriv.h"
#include "src/gpu/graphite/RecorderPriv.h"
#include "src/gpu/graphite/RendererProvider.h"
#include "src/gpu/graphite/geom/EndCaps.h"
#include "src/gpu/graphite/geom/Shape.h"
#include "src/gpu/graphite/geom/WideTiles.h"
#include "src/gpu/graphite/sparse_strips/AlphaAtlasManager.h"
#include "src/gpu/graphite/sparse_strips/MSAA_LUT.h"
#include "src/gpu/graphite/sparse_strips/SparseStripsConfig.h"
#include "src/gpu/graphite/sparse_strips/StripCache.h"
#include "src/gpu/graphite/sparse_strips/StripGenerator.h"
#include "tests/CtsEnforcement.h"
#include "tests/Test.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <iterator>
#include <memory>
#include <optional>
#include <vector>

namespace skgpu::graphite {

namespace {

constexpr float kStarRadius = 60.f;

// A star with integer vertices, centered on the origin, that spans [-kStarRadius, kStarRadius].
// With more than 10 verbs, its Shape key is based on its generation ID rather than its contents.
SkPath make_star() {
    constexpr int kPoints = 12;
    constexpr float kInnerRadius = 30.f;
    SkPathBuilder builder;
    for (int i = 0; i < 2 * kPoints; ++i) {
        const float radius = (i % 2 == 0) ? kStarRadius : kInnerRadius;
        const float theta = i * SK_ScalarPI / kPoints;
        const SkPoint p = {std::round(radius * std::cos(theta)),
                           std::round(radius * std::sin(theta))};
        if (i == 0) {
            builder.moveTo(p);
        } else {
            builder.lineTo(p);
        }
    }
    builder.close();
    return builder.detach();
}

// A small path, whose Shape key is based on its contents.
SkPath make_triangle() {
    return SkPathBuilder().moveTo(0, 0).lineTo(30, 5).lineTo(10, 25).close().detach();
}

SkMatrix scale_translate(float scale, float tx, float ty) {
    return SkMatrix::MakeAll(scale, 0, tx, 0, scale, ty, 0, 0, 1);
}

std::optional<StripCache::Key> key_for(const SkPath& path, const SkMatrix& ctm, SkIPoint* origin) {
    return StripCache::Key::Make(Shape(path), ctm, origin);
}

std::optional<StripCache::Key> key_for(const SkPath& path, const SkMatrix& ctm) {
    SkIPoint origin;
    return key_for(path, ctm, &origin);
}

std::optional<StripCache::Key> clipped_key_for(const SkPath& path,
                                               const SkMatrix& ctm,
                                               const SkIRect& tileClip) {
    return StripCache::Key::MakeClipped(Shape(path), ctm, tileClip);
}

bool keys_equal(const std::optional<StripCache::Key>& a, const std::optional<StripCache::Key>& b) {
    return a.has_value() && b.has_value() && *a == *b;
}

StripCache::Key rect_key(int i) {
    SkIPoint origin;
    return *StripCache::Key::Make(Shape(SkRect::MakeXYWH(static_cast<float>(i), 0.f, 10.f, 10.f)),
                                  SkMatrix::I(),
                                  &origin);
}

StripCache::Key clipped_rect_key(int i) {
    return *StripCache::Key::MakeClipped(
            Shape(SkRect::MakeXYWH(static_cast<float>(i), 0.f, 10.f, 10.f)),
            SkMatrix::I(),
            SkIRect::MakeWH(512, 512));
}

// Checks that the generator's current strips are `caps` and `wides` translated by (dx, dy), and
// reference the same alphas.
void check_translated(skiatest::Reporter* reporter,
                      const StripGenerator& gen,
                      const skia_private::TArray<EndCaps::EndCap>& caps,
                      const skia_private::TArray<WideTiles::WideTile>& wides,
                      int dx,
                      int dy) {
    const skia_private::TArray<EndCaps::EndCap>& gotCaps = gen.ends().caps();
    const skia_private::TArray<WideTiles::WideTile>& gotWides = gen.wides().tiles();
    REPORTER_ASSERT(reporter,
                    gotCaps.size() == caps.size() && gotWides.size() == wides.size(),
                    "Got %d caps and %d wides, want %d and %d",
                    gotCaps.size(), gotWides.size(), caps.size(), wides.size());
    if (gotCaps.size() != caps.size() || gotWides.size() != wides.size()) {
        return;
    }
    for (int i = 0; i < caps.size(); ++i) {
        const EndCaps::EndCap& got = gotCaps[i];
        const EndCaps::EndCap& want = caps[i];
        REPORTER_ASSERT(reporter,
                        got.fX == want.fX + dx && got.fY == want.fY + dy &&
                                got.fWidth == want.fWidth && got.fAlphaIndex == want.fAlphaIndex &&
                                got.fTexPage == want.fTexPage,
                        "Cap %d: got (%u, %u, w=%u, idx=%d, page=%u), want (%d, %d, w=%u, idx=%d, "
                        "page=%u)",
                        i, got.fX, got.fY, got.fWidth, got.fAlphaIndex, got.fTexPage,
                        want.fX + dx, want.fY + dy, want.fWidth, want.fAlphaIndex, want.fTexPage);
    }
    for (int i = 0; i < wides.size(); ++i) {
        const WideTiles::WideTile& got = gotWides[i];
        const WideTiles::WideTile& want = wides[i];
        REPORTER_ASSERT(reporter,
                        got.fX == want.fX + dx && got.fY == want.fY + dy &&
                                got.fWidth == want.fWidth,
                        "Wide %d: got (%u, %u, w=%u), want (%d, %d, w=%u)",
                        i, got.fX, got.fY, got.fWidth, want.fX + dx, want.fY + dy, want.fWidth);
    }
    for (const sk_sp<TextureProxy>& proxy : gen.ends().proxies()) {
        REPORTER_ASSERT(reporter, proxy != nullptr);
    }
}

// Fills both regular pages of an empty atlas, so that the alphas of the next draw overflow.
void fill_regular_pages(skiatest::Reporter* reporter, AlphaAtlasManager* atlas) {
    for (int32_t rows : {1, 2}) {
        const int32_t numBytes = rows * SparseStripConfig::kAtlasWidthBytes;
        REPORTER_ASSERT(reporter, atlas->requestAlphaSpace(numBytes) != nullptr);
        REPORTER_ASSERT(reporter, atlas->finalizeRun().has_value());
    }
    REPORTER_ASSERT(reporter, atlas->numPages() == 2);
}

void use_sparse_strips(ContextOptions* options) {
    if (options->fOptionsPriv) {
        options->fOptionsPriv->fPathRendererStrategy = PathRendererStrategy::kCPUSparseStripsMSAA8;
    }
}

constexpr int kRenderSize = 512;
// The fractional part of the translation of every rendered copy.
constexpr float kBaseTx = 0.25f;
constexpr float kBaseTy = 0.5f;
// The pixels covered by a copy, relative to its integer offset, plus a margin.
constexpr SkIRect kCopyBounds = SkIRect::MakeLTRB(-64, -64, 66, 66);

void clear_and_draw(SkCanvas* canvas, const SkPath& path, SkIPoint offset) {
    SkPaint paint;
    paint.setAntiAlias(true);
    // Translucent, so that any coverage difference changes the blended result.
    paint.setColor(SkColorSetARGB(0xC0, 0x20, 0x60, 0xA0));
    canvas->setMatrix(SkMatrix::Translate(offset.fX + kBaseTx, offset.fY + kBaseTy));
    canvas->drawPath(path, paint);
}

bool read_back(skiatest::Reporter* reporter, SkSurface* surface, SkBitmap* result) {
    result->allocPixels(SkImageInfo::Make(
            kRenderSize, kRenderSize, kRGBA_8888_SkColorType, kPremul_SkAlphaType));
    if (!surface->readPixels(*result, 0, 0)) {
        ERRORF(reporter, "readPixels failed");
        return false;
    }
    return true;
}

bool snap_and_insert(skiatest::Reporter* reporter, Context* context, Recorder* recorder) {
    std::unique_ptr<Recording> recording = recorder->snap();
    if (!recording || !context->insertRecording({recording.get()})) {
        ERRORF(reporter, "Failed to snap and insert the recording");
        return false;
    }
    return true;
}

// Renders `path` at each of `offsets` within a single Recording of a new Recorder, and returns the
// number of draws that reused cached strips.
std::optional<int> render_copies(skiatest::Reporter* reporter,
                                 Context* context,
                                 const SkPath& path,
                                 SkSpan<const SkIPoint> offsets,
                                 SkBitmap* result) {
    std::unique_ptr<Recorder> recorder = context->makeRecorder();
    sk_sp<SkSurface> surface = SkSurfaces::RenderTarget(
            recorder.get(),
            SkImageInfo::Make(kRenderSize, kRenderSize, kRGBA_8888_SkColorType,
                              kPremul_SkAlphaType));
    if (!surface) {
        ERRORF(reporter, "Could not create surface");
        return std::nullopt;
    }
    surface->getCanvas()->clear(SK_ColorWHITE);
    for (SkIPoint offset : offsets) {
        clear_and_draw(surface->getCanvas(), path, offset);
    }
    if (!snap_and_insert(reporter, context, recorder.get()) ||
        !read_back(reporter, surface.get(), result)) {
        return std::nullopt;
    }
    return recorder->priv().getOrCreateAtlasProvider()->alphaAtlasManager()->stripCacheHits();
}

// Returns the number of pixels within kCopyBounds that differ between the copy at `offsetA` in `a`
// and the copy at `offsetB` in `b`.
int count_copy_mismatches(const SkBitmap& a, SkIPoint offsetA, const SkBitmap& b, SkIPoint offsetB) {
    int mismatches = 0;
    for (int y = kCopyBounds.fTop; y < kCopyBounds.fBottom; ++y) {
        for (int x = kCopyBounds.fLeft; x < kCopyBounds.fRight; ++x) {
            if (a.getColor(offsetA.fX + x, offsetA.fY + y) !=
                b.getColor(offsetB.fX + x, offsetB.fY + y)) {
                mismatches++;
            }
        }
    }
    return mismatches;
}

bool is_sparse_strips_context(skiatest::Reporter* reporter, Context* context) {
    if (context->priv().rendererProvider()->pathRendererStrategy() !=
        PathRendererStrategy::kCPUSparseStripsMSAA8) {
        ERRORF(reporter, "Context is not using the sparse strips path renderer strategy");
        return false;
    }
    return true;
}

}  // anonymous namespace

DEF_TEST(SparseStrips_StripCacheKey, reporter) {
    const SkPath triangle = make_triangle();
    SkIPoint originA;
    SkIPoint originB;

    // Integer translations share a key, and are returned as the origin.
    auto keyA = key_for(triangle, SkMatrix::Translate(10, 20), &originA);
    auto keyB = key_for(triangle, SkMatrix::Translate(-35, 300), &originB);
    REPORTER_ASSERT(reporter, keys_equal(keyA, keyB));
    REPORTER_ASSERT(reporter, keyA && keyB && keyA->hash() == keyB->hash());
    REPORTER_ASSERT(reporter, originA == SkIPoint::Make(10, 20));
    REPORTER_ASSERT(reporter, originB == SkIPoint::Make(-35, 300));

    // So do translations with the same fractional part. The origin is the floor of the translation.
    keyA = key_for(triangle, SkMatrix::Translate(10.25f, 20.5f), &originA);
    keyB = key_for(triangle, SkMatrix::Translate(11.25f, -3.5f), &originB);
    REPORTER_ASSERT(reporter, keys_equal(keyA, keyB));
    REPORTER_ASSERT(reporter, originA == SkIPoint::Make(10, 20));
    REPORTER_ASSERT(reporter, originB == SkIPoint::Make(11, -4));
    REPORTER_ASSERT(reporter, keys_equal(key_for(triangle, scale_translate(1.5f, 10.5f, 3.f)),
                                         key_for(triangle, scale_translate(1.5f, 99.5f, 7.f))));

    // A different fractional translation, 2x2 matrix, or fill type changes the key.
    REPORTER_ASSERT(reporter,
                    !keys_equal(keyA, key_for(triangle, SkMatrix::Translate(10.5f, 20.5f))));
    REPORTER_ASSERT(reporter, !keys_equal(key_for(triangle, SkMatrix::I()),
                                          key_for(triangle, SkMatrix::Scale(2, 2))));
    REPORTER_ASSERT(reporter, !keys_equal(key_for(triangle, SkMatrix::I()),
                                          key_for(triangle, SkMatrix::RotateDeg(90))));
    REPORTER_ASSERT(reporter,
                    !keys_equal(key_for(triangle, SkMatrix::I()),
                                key_for(triangle.makeFillType(SkPathFillType::kEvenOdd),
                                        SkMatrix::I())));

    // Small paths are keyed by their contents, larger paths by their generation ID.
    REPORTER_ASSERT(reporter, keys_equal(key_for(triangle, SkMatrix::I()),
                                         key_for(make_triangle(), SkMatrix::I())));
    const SkPath star = make_star();
    const SkPath starCopy = star;
    REPORTER_ASSERT(reporter,
                    keys_equal(key_for(star, SkMatrix::I()), key_for(starCopy, SkMatrix::I())));
    REPORTER_ASSERT(reporter,
                    !keys_equal(key_for(star, SkMatrix::I()), key_for(triangle, SkMatrix::I())));

    // Shapes whose strips can't be reused at other translations have no translation invariant key.
    REPORTER_ASSERT(reporter,
                    !key_for(triangle.makeFillType(SkPathFillType::kInverseWinding),
                             SkMatrix::I()));
    SkPath volatileTriangle = triangle;
    volatileTriangle.setIsVolatile(true);
    REPORTER_ASSERT(reporter, !key_for(volatileTriangle, SkMatrix::I()));
    SkMatrix perspective = SkMatrix::I();
    perspective.setPerspX(0.001f);
    REPORTER_ASSERT(reporter, !key_for(triangle, perspective));
    REPORTER_ASSERT(reporter, !key_for(triangle, SkMatrix::Translate(1e7f, 0)));
    REPORTER_ASSERT(reporter, !key_for(triangle, SkMatrix::Translate(SK_FloatNaN, 0)));
    REPORTER_ASSERT(reporter, !StripCache::Key::Make(Shape(), SkMatrix::I(), &originA));
}

DEF_TEST(SparseStrips_StripCacheClippedKey, reporter) {
    const SkPath triangle = make_triangle();
    const SkMatrix ctm = SkMatrix::Translate(10.25f, 20.5f);
    const SkIRect tileClip = SkIRect::MakeLTRB(8, 4, 30, 40);
    const auto key = clipped_key_for(triangle, ctm, tileClip);
    REPORTER_ASSERT(reporter, key.has_value());

    // Clipped keys only match draws of the same shape with the exact same transform and tile clip.
    REPORTER_ASSERT(reporter, keys_equal(key, clipped_key_for(make_triangle(), ctm, tileClip)));
    REPORTER_ASSERT(reporter,
                    !keys_equal(key, clipped_key_for(triangle,
                                                     SkMatrix::Translate(11.25f, 20.5f),
                                                     tileClip)));
    REPORTER_ASSERT(reporter,
                    !keys_equal(key, clipped_key_for(triangle,
                                                     scale_translate(2.f, 10.25f, 20.5f),
                                                     tileClip)));
    REPORTER_ASSERT(reporter,
                    !keys_equal(key, clipped_key_for(triangle, ctm, tileClip.makeOffset(8, 4))));
    REPORTER_ASSERT(reporter,
                    !keys_equal(key, clipped_key_for(triangle,
                                                     ctm,
                                                     SkIRect::MakeLTRB(8, 4, 31, 40))));

    // They never match translation invariant keys.
    REPORTER_ASSERT(reporter, !keys_equal(key, key_for(triangle, ctm)));

    // Inverse fills, whose strips always depend on the clip, have clipped keys of their own.
    const SkPath inverse = triangle.makeFillType(SkPathFillType::kInverseWinding);
    REPORTER_ASSERT(reporter, clipped_key_for(inverse, ctm, tileClip).has_value());
    REPORTER_ASSERT(reporter, !keys_equal(key, clipped_key_for(inverse, ctm, tileClip)));

    // Any finite translation can be keyed, since the strips are never translated.
    REPORTER_ASSERT(reporter,
                    clipped_key_for(triangle, SkMatrix::Translate(1e7f, 0), tileClip).has_value());

    // Shapes whose strips can't be reused have no clipped key.
    SkPath volatileTriangle = triangle;
    volatileTriangle.setIsVolatile(true);
    REPORTER_ASSERT(reporter, !clipped_key_for(volatileTriangle, ctm, tileClip));
    SkMatrix perspective = SkMatrix::I();
    perspective.setPerspX(0.001f);
    REPORTER_ASSERT(reporter, !clipped_key_for(triangle, perspective, tileClip));
    REPORTER_ASSERT(reporter,
                    !clipped_key_for(triangle, SkMatrix::Translate(SK_FloatNaN, 0), tileClip));
    REPORTER_ASSERT(reporter, !StripCache::Key::MakeClipped(Shape(), ctm, tileClip));
}

DEF_TEST(SparseStrips_StripCacheLRU, reporter) {
    const EndCaps::EndCap cap(/*x=*/16, /*y=*/24, /*width=*/24, /*alphaIndex=*/128, /*texPage=*/1);
    const WideTiles::WideTile wide(/*x=*/40, /*y=*/24, /*width=*/16);
    const SkSpan<const EndCaps::EndCap> oneCap(&cap, 1);
    const SkSpan<const WideTiles::WideTile> oneWide(&wide, 1);
    const StripCache::PageIDs pageIDs = {AlphaAtlasManager::kInvalidPageID, 7};
    // Each use is in a later flush than the previous use was recent for, so that entries are only
    // kept for being recently used by the LRU order.
    uint64_t flushToken = 0;
    auto nextUse = [&]() { return flushToken += SparseStripConfig::kStripCacheRecentFlushes; };

    StripCache cache;

    // Entries hold a copy of their strips, page IDs and origin, and the bounds of their strips.
    REPORTER_ASSERT(reporter, cache.add(rect_key(0), nextUse(), {3, 4}, pageIDs, oneCap, oneWide));
    const StripCache::Entry* entry = cache.find(rect_key(0), nextUse());
    REPORTER_ASSERT(reporter, entry != nullptr);
    if (entry) {
        REPORTER_ASSERT(reporter, entry->fCaps.size() == 1 && entry->fCaps[0].fAlphaIndex == 128);
        REPORTER_ASSERT(reporter, entry->fWides.size() == 1 && entry->fWides[0].fX == 40);
        REPORTER_ASSERT(reporter, entry->fPageIDs == pageIDs);
        REPORTER_ASSERT(reporter, entry->fOrigin == SkIPoint::Make(3, 4));
        REPORTER_ASSERT(reporter, entry->fBounds == SkIRect::MakeLTRB(16, 24, 56, 32));
    }
    REPORTER_ASSERT(reporter, cache.count() == 1 && cache.bytes() > 0);
    REPORTER_ASSERT(reporter, cache.find(rect_key(1), nextUse()) == nullptr);

    // Adding an entry for an existing key replaces it.
    REPORTER_ASSERT(reporter, cache.add(rect_key(0), nextUse(), {5, 6}, pageIDs, oneCap, {}));
    entry = cache.find(rect_key(0), nextUse());
    REPORTER_ASSERT(reporter, cache.count() == 1);
    REPORTER_ASSERT(reporter,
                    entry && entry->fOrigin == SkIPoint::Make(5, 6) && entry->fWides.empty());

    // Entries larger than kStripCacheMaxEntryBytes are rejected.
    const std::vector<EndCaps::EndCap> tooManyCaps(
            SparseStripConfig::kStripCacheMaxEntryBytes / sizeof(EndCaps::EndCap), cap);
    REPORTER_ASSERT(reporter, !cache.add(rect_key(1), nextUse(), {0, 0}, pageIDs, tooManyCaps, {}));
    REPORTER_ASSERT(reporter, cache.count() == 1 && cache.find(rect_key(1), nextUse()) == nullptr);

    // Once there are too many entries, the least recently used one is evicted.
    cache.reset();
    REPORTER_ASSERT(reporter, cache.count() == 0 && cache.bytes() == 0);
    constexpr int kMaxEntries = SparseStripConfig::kStripCacheMaxEntries;
    for (int i = 0; i < kMaxEntries; ++i) {
        REPORTER_ASSERT(reporter, cache.add(rect_key(i), nextUse(), {0, 0}, pageIDs, oneCap, {}));
    }
    REPORTER_ASSERT(reporter, cache.count() == kMaxEntries);
    // Using entry 0 makes entry 1 the least recently used one.
    REPORTER_ASSERT(reporter, cache.find(rect_key(0), nextUse()) != nullptr);
    REPORTER_ASSERT(reporter,
                    cache.add(rect_key(kMaxEntries), nextUse(), {0, 0}, pageIDs, oneCap, {}));
    REPORTER_ASSERT(reporter, cache.count() == kMaxEntries);
    REPORTER_ASSERT(reporter, cache.find(rect_key(0), nextUse()) != nullptr);
    REPORTER_ASSERT(reporter, cache.find(rect_key(1), nextUse()) == nullptr);
    REPORTER_ASSERT(reporter, cache.find(rect_key(2), nextUse()) != nullptr);

    // The least recently used entries are also evicted to stay within kStripCacheMaxBytes.
    cache.reset();
    const std::vector<EndCaps::EndCap> manyCaps(
            SparseStripConfig::kStripCacheMaxEntryBytes / (2 * sizeof(EndCaps::EndCap)), cap);
    const size_t manyCapsBytes = manyCaps.size() * sizeof(EndCaps::EndCap);
    const int numLargeEntries = static_cast<int>(2 * SparseStripConfig::kStripCacheMaxBytes /
                                                 manyCapsBytes);
    for (int i = 0; i < numLargeEntries; ++i) {
        REPORTER_ASSERT(reporter, cache.add(rect_key(i), nextUse(), {0, 0}, pageIDs, manyCaps, {}));
        REPORTER_ASSERT(reporter, cache.bytes() <= SparseStripConfig::kStripCacheMaxBytes);
    }
    REPORTER_ASSERT(reporter, cache.count() < numLargeEntries);
    REPORTER_ASSERT(reporter,
                    cache.bytes() + manyCapsBytes > SparseStripConfig::kStripCacheMaxBytes);
    REPORTER_ASSERT(reporter, cache.find(rect_key(numLargeEntries - 1), nextUse()) != nullptr);
    REPORTER_ASSERT(reporter, cache.find(rect_key(0), nextUse()) == nullptr);

    cache.remove(rect_key(numLargeEntries - 1));
    REPORTER_ASSERT(reporter, cache.find(rect_key(numLargeEntries - 1), nextUse()) == nullptr);
    cache.reset();
    REPORTER_ASSERT(reporter, cache.count() == 0 && cache.bytes() == 0);
}

// Entries used within the latest flushes aren't evicted to make room for others, except that
// clipped ones make room for translation invariant ones.
DEF_TEST(SparseStrips_StripCacheEvictionPolicy, reporter) {
    const EndCaps::EndCap cap(/*x=*/16, /*y=*/24, /*width=*/24, /*alphaIndex=*/128, /*texPage=*/1);
    const SkSpan<const EndCaps::EndCap> oneCap(&cap, 1);
    const StripCache::PageIDs pageIDs = {AlphaAtlasManager::kInvalidPageID, 7};
    constexpr int kMaxEntries = SparseStripConfig::kStripCacheMaxEntries;
    constexpr uint64_t kToken = 100;
    // The last flush in which entries used in kToken are still recently used, and the first in
    // which they aren't.
    constexpr uint64_t kRecentToken = kToken + SparseStripConfig::kStripCacheRecentFlushes - 1;
    constexpr uint64_t kIdleToken = kRecentToken + 1;
    auto add = [&](StripCache* cache, const StripCache::Key& key, uint64_t flushToken) {
        return cache->add(key, flushToken, {0, 0}, pageIDs, oneCap, {});
    };

    StripCache cache;
    for (int i = 0; i < kMaxEntries; ++i) {
        REPORTER_ASSERT(reporter, add(&cache, rect_key(i), kToken));
    }

    // While every entry was used recently, new entries are rejected.
    REPORTER_ASSERT(reporter, !add(&cache, rect_key(kMaxEntries), kRecentToken));
    REPORTER_ASSERT(reporter, !add(&cache, clipped_rect_key(0), kRecentToken));
    REPORTER_ASSERT(reporter, cache.count() == kMaxEntries);
    REPORTER_ASSERT(reporter, cache.find(rect_key(0), kRecentToken) != nullptr);

    // Entries that weren't used recently make room for entries of either kind, least recently used
    // first.
    REPORTER_ASSERT(reporter, add(&cache, clipped_rect_key(0), kIdleToken));
    REPORTER_ASSERT(reporter, add(&cache, rect_key(kMaxEntries), kIdleToken));
    REPORTER_ASSERT(reporter, cache.count() == kMaxEntries);
    REPORTER_ASSERT(reporter, cache.find(rect_key(1), kIdleToken) == nullptr);
    REPORTER_ASSERT(reporter, cache.find(rect_key(2), kIdleToken) == nullptr);
    REPORTER_ASSERT(reporter, cache.find(rect_key(0), kIdleToken) != nullptr);

    // Recently used clipped entries still make room for translation invariant entries, but not for
    // other clipped entries.
    cache.reset();
    for (int i = 0; i < kMaxEntries - 1; ++i) {
        REPORTER_ASSERT(reporter, add(&cache, rect_key(i), kToken));
    }
    REPORTER_ASSERT(reporter, add(&cache, clipped_rect_key(0), kToken));
    REPORTER_ASSERT(reporter, !add(&cache, clipped_rect_key(1), kToken));
    REPORTER_ASSERT(reporter, cache.find(clipped_rect_key(0), kToken) != nullptr);
    REPORTER_ASSERT(reporter, add(&cache, rect_key(kMaxEntries), kToken));
    REPORTER_ASSERT(reporter, cache.count() == kMaxEntries);
    REPORTER_ASSERT(reporter, cache.find(clipped_rect_key(0), kToken) == nullptr);
    for (int i = 0; i < kMaxEntries - 1; ++i) {
        REPORTER_ASSERT(reporter, cache.find(rect_key(i), kToken) != nullptr);
    }
    REPORTER_ASSERT(reporter, cache.find(rect_key(kMaxEntries), kToken) != nullptr);
}

// Frames tend to draw the same shapes in the same order, which makes an LRU that can't hold all of
// them miss on every lookup. Instead, the entries that fit keep being reused.
DEF_TEST(SparseStrips_StripCacheCyclicAccess, reporter) {
    const EndCaps::EndCap cap(/*x=*/16, /*y=*/24, /*width=*/24, /*alphaIndex=*/128, /*texPage=*/1);
    const SkSpan<const EndCaps::EndCap> oneCap(&cap, 1);
    const StripCache::PageIDs pageIDs = {AlphaAtlasManager::kInvalidPageID, 7};
    constexpr int kMaxEntries = SparseStripConfig::kStripCacheMaxEntries;
    constexpr int kNumFrames = 3;

    struct Misses {
        int clipped = 0;
        int invariant = 0;
    };
    // Draws `numClipped` clipped shapes and then `numInvariant` translation invariant ones in each
    // frame, with one flush per frame, and returns the misses of each kind after the first frame.
    auto drawFrames = [&](int numClipped, int numInvariant) {
        StripCache cache;
        Misses misses;
        for (uint64_t frame = 1; frame <= kNumFrames; ++frame) {
            for (int i = 0; i < numClipped + numInvariant; ++i) {
                const bool clipped = i < numClipped;
                const StripCache::Key key = clipped ? clipped_rect_key(i) : rect_key(i);
                if (!cache.find(key, frame)) {
                    if (frame > 1) {
                        (clipped ? misses.clipped : misses.invariant)++;
                    }
                    cache.add(key, frame, {0, 0}, pageIDs, oneCap, {});
                }
            }
        }
        return misses;
    };

    // Translation invariant shapes that fit keep being reused, even if clipped ones don't fit too.
    Misses misses = drawFrames(/*numClipped=*/32, /*numInvariant=*/kMaxEntries - 8);
    REPORTER_ASSERT(reporter, misses.invariant == 0, "%d misses", misses.invariant);
    REPORTER_ASSERT(reporter, misses.clipped == (kNumFrames - 1) * 24, "%d misses", misses.clipped);

    // When there are too many shapes of a kind to fit, only the excess misses.
    misses = drawFrames(/*numClipped=*/0, /*numInvariant=*/kMaxEntries + 8);
    REPORTER_ASSERT(
            reporter, misses.invariant == (kNumFrames - 1) * 8, "%d misses", misses.invariant);
}

// Exercises reuse through StripGenerator::processGeometry(), which is what Device uses.
DEF_GRAPHITE_TEST_FOR_RENDERING_CONTEXTS(SparseStrips_StripCacheReuse,
                                         reporter,
                                         context,
                                         CtsEnforcement::kNever) {
    std::unique_ptr<Recorder> recorder = context->makeRecorder();
    const SparseStripConfig::LUTArray lut = GenerateMSAALUT();
    constexpr int kDeviceSize = 512;
    const SkIRect deviceClip = SkIRect::MakeWH(kDeviceSize, kDeviceSize);
    StripGenerator gen(kDeviceSize, kDeviceSize, lut, recorder.get());
    AlphaAtlasManager* atlas = recorder->priv().getOrCreateAtlasProvider()->alphaAtlasManager();
    if (!atlas) {
        ERRORF(reporter, "No alpha atlas");
        return;
    }
    auto hits = [&]() { return atlas->stripCacheHits(); };
    auto entries = [&]() { return atlas->stripCache().count(); };
    auto draw = [&](const Shape& shape, const SkMatrix& ctm, const SkIRect& clip) {
        REPORTER_ASSERT(reporter, gen.processGeometry(shape, ctm, clip));
    };

    const SkPath star = make_star();
    const Shape shape(star);
    const SkMatrix ctm = SkMatrix::Translate(100.25f, 120.5f);

    // The first draw generates the strips and caches them.
    draw(shape, ctm, deviceClip);
    REPORTER_ASSERT(reporter, hits() == 0 && entries() == 1);
    REPORTER_ASSERT(reporter, !gen.ends().empty() && !gen.wides().empty());
    REPORTER_ASSERT(reporter, !gen.ends().hasOverflowCaps());
    const skia_private::TArray<EndCaps::EndCap> caps = gen.ends().caps();
    const skia_private::TArray<WideTiles::WideTile> wides = gen.wides().tiles();

    // Translating by integer amounts, which need not be multiples of the tile size, reuses them.
    draw(shape, SkMatrix::Translate(100.25f + 37, 120.5f - 11), deviceClip);
    REPORTER_ASSERT(reporter, hits() == 1 && entries() == 1);
    check_translated(reporter, gen, caps, wides, 37, -11);

    draw(shape, ctm, deviceClip);
    REPORTER_ASSERT(reporter, hits() == 2);
    check_translated(reporter, gen, caps, wides, 0, 0);

    // The SkPath overloads always generate new strips.
    REPORTER_ASSERT(reporter, gen.processGeometry(star, ctm, deviceClip));
    REPORTER_ASSERT(reporter, hits() == 2 && entries() == 1);

    // A different fractional translation needs new strips.
    draw(shape, SkMatrix::Translate(100.75f, 120.5f), deviceClip);
    REPORTER_ASSERT(reporter, hits() == 2 && entries() == 2);

    // Clipped strips depend on the clip, so they are only reused by draws with the same transform
    // and clip.
    const SkMatrix ctm2 = SkMatrix::Translate(300.5f, 300.5f);
    const SkIRect clip2 = SkIRect::MakeLTRB(0, 0, 320, kDeviceSize);
    draw(shape, ctm2, clip2);
    REPORTER_ASSERT(reporter, hits() == 2 && entries() == 3);
    const skia_private::TArray<EndCaps::EndCap> clippedCaps = gen.ends().caps();
    const skia_private::TArray<WideTiles::WideTile> clippedWides = gen.wides().tiles();
    draw(shape, ctm2, clip2);
    REPORTER_ASSERT(reporter, hits() == 3 && entries() == 3);
    check_translated(reporter, gen, clippedCaps, clippedWides, 0, 0);
    draw(shape, ctm2, SkIRect::MakeLTRB(0, 0, 330, kDeviceSize));
    REPORTER_ASSERT(reporter, hits() == 3 && entries() == 4);
    draw(shape, SkMatrix::Translate(301.5f, 300.5f), clip2);
    REPORTER_ASSERT(reporter, hits() == 3 && entries() == 5);
    // Unclipped, the same draw's strips don't depend on the clip...
    draw(shape, ctm2, deviceClip);
    REPORTER_ASSERT(reporter, hits() == 3 && entries() == 6);
    const skia_private::TArray<EndCaps::EndCap> completeCaps = gen.ends().caps();
    const skia_private::TArray<WideTiles::WideTile> completeWides = gen.wides().tiles();
    // ...so clipped draws of the shape can reuse them at other integer translations, since the clip
    // still applies when they are drawn.
    draw(shape, SkMatrix::Translate(200.5f, 200.5f), SkIRect::MakeLTRB(0, 0, 220, kDeviceSize));
    REPORTER_ASSERT(reporter, hits() == 4 && entries() == 6);
    check_translated(reporter, gen, completeCaps, completeWides, -100, -100);

    // Strips can't be translated to negative coordinates, so new strips are generated. Those cross
    // the edge of the device, so they are cached as clipped strips, and the existing entry remains
    // usable.
    draw(shape, SkMatrix::Translate(20.5f, 20.5f), deviceClip);
    REPORTER_ASSERT(reporter, hits() == 4 && entries() == 7);
    draw(shape, SkMatrix::Translate(150.5f, 160.5f), deviceClip);
    REPORTER_ASSERT(reporter, hits() == 5 && entries() == 7);

    // Inverse fills fill the clip, so their strips are cached like clipped ones. Volatile paths are
    // never cached.
    const Shape inverse(star.makeFillType(SkPathFillType::kInverseWinding));
    SkPath volatileStar = star;
    volatileStar.setIsVolatile(true);
    const Shape volatileShape(volatileStar);
    for (int i = 0; i < 2; ++i) {
        draw(inverse, ctm, deviceClip);
        draw(volatileShape, ctm, deviceClip);
    }
    REPORTER_ASSERT(reporter, hits() == 6 && entries() == 8);

    // Cached strips are dropped once the page holding their alphas is retired.
    atlas->freeGpuResources();
    REPORTER_ASSERT(reporter, entries() == 0);
    draw(shape, ctm, deviceClip);
    REPORTER_ASSERT(reporter, entries() == 1 && !gen.ends().empty());
    REPORTER_ASSERT(reporter, gen.ends().caps()[0].fTexPage == 0);
    // Spill a run into slot 1, so that the next flush retires slot 0.
    REPORTER_ASSERT(reporter,
                    atlas->requestAlphaSpace(SparseStripConfig::kAtlasWidthBytes) != nullptr);
    auto spilled = atlas->finalizeRun();
    REPORTER_ASSERT(reporter, spilled.has_value() && spilled->second == 1);
    atlas->recordUploads(/*dc=*/nullptr);
    REPORTER_ASSERT(reporter, atlas->pageID(0) == AlphaAtlasManager::kInvalidPageID);
    draw(shape, ctm, deviceClip);
    REPORTER_ASSERT(reporter, hits() == 6 && entries() == 1);
    REPORTER_ASSERT(reporter, !gen.ends().empty() && gen.ends().caps()[0].fTexPage == 1);
    draw(shape, ctm, deviceClip);
    REPORTER_ASSERT(reporter, hits() == 7);

    // Or when the atlas is evicted.
    atlas->evictAtlases();
    REPORTER_ASSERT(reporter, entries() == 0);
    draw(shape, ctm, deviceClip);
    REPORTER_ASSERT(reporter, hits() == 7 && entries() == 1);

    // Strips with alphas in the overflow page aren't cached.
    atlas->freeGpuResources();
    fill_regular_pages(reporter, atlas);
    draw(shape, ctm, deviceClip);
    REPORTER_ASSERT(reporter, gen.ends().hasOverflowCaps());
    REPORTER_ASSERT(reporter, hits() == 7 && entries() == 0);
}

// Draws a path at several integer offsets within one Recording, so that every draw after the first
// reuses the first draw's strips.
DEF_CONDITIONAL_GRAPHITE_TEST_FOR_CONTEXTS(SparseStrips_StripCacheRendering,
                                           skgpu::IsRenderingContext,
                                           reporter,
                                           context,
                                           testContext,
                                           /* anonymous options */,
                                           use_sparse_strips,
                                           /* condition= */ true,
                                           CtsEnforcement::kNever) {
    if (!is_sparse_strips_context(reporter, context)) {
        return;
    }

    // Offsets relative to the first copy include ones that aren't multiples of the tile size.
    constexpr SkIPoint kOffsets[] = {{80, 80},   {237, 83},  {400, 72},
                                     {77, 245},  {243, 243}, {406, 251},
                                     {85, 405},  {251, 397}, {403, 411}};
    constexpr int kNumCopies = static_cast<int>(std::size(kOffsets));

    const SkPath star = make_star();
    SkPath volatileStar = star;
    volatileStar.setIsVolatile(true);

    SkBitmap reused;
    SkBitmap generated;
    std::optional<int> reusedHits = render_copies(reporter, context, star, kOffsets, &reused);
    std::optional<int> generatedHits =
            render_copies(reporter, context, volatileStar, kOffsets, &generated);
    if (!reusedHits || !generatedHits) {
        return;
    }
    REPORTER_ASSERT(reporter, *reusedHits == kNumCopies - 1, "%d hits", *reusedHits);
    REPORTER_ASSERT(reporter, *generatedHits == 0, "%d hits", *generatedHits);

    // Every copy reuses the alphas of the first one, so they must render identically to it.
    for (int i = 1; i < kNumCopies; ++i) {
        const int mismatches = count_copy_mismatches(reused, kOffsets[0], reused, kOffsets[i]);
        REPORTER_ASSERT(reporter,
                        mismatches == 0,
                        "Copy %d differs from copy 0 in %d pixels",
                        i, mismatches);
    }

    // Generating every copy doesn't render them exactly alike either: each copy's position relative
    // to the tile grid, and the float precision of its device coordinates, differ, which flips a few
    // sub-samples along its edges. Reused strips must stay within that noise, whereas a misplaced
    // alpha or translation would change most edge pixels.
    const SkColor interior = reused.getColor(kOffsets[0].fX, kOffsets[0].fY);
    int edgePixels = 0;
    int mismatches = 0;
    int maxDiff = 0;
    for (int y = 0; y < kRenderSize; ++y) {
        for (int x = 0; x < kRenderSize; ++x) {
            const SkColor a = reused.getColor(x, y);
            const SkColor b = generated.getColor(x, y);
            if (a != SK_ColorWHITE && a != interior) {
                edgePixels++;
            }
            if (a != b) {
                mismatches++;
                maxDiff = std::max({maxDiff,
                                    std::abs(int(SkColorGetA(a)) - int(SkColorGetA(b))),
                                    std::abs(int(SkColorGetR(a)) - int(SkColorGetR(b))),
                                    std::abs(int(SkColorGetG(a)) - int(SkColorGetG(b))),
                                    std::abs(int(SkColorGetB(a)) - int(SkColorGetB(b)))});
            }
        }
    }
    // A single sub-sample changes a channel of the blended paint by at most 22.
    constexpr int kMaxSubSampleFlips = 3;
    REPORTER_ASSERT(reporter,
                    maxDiff <= kMaxSubSampleFlips * 22 && mismatches <= edgePixels / 32,
                    "Reused strips differ from generated ones in %d of %d edge pixels, by up to %d",
                    mismatches, edgePixels, maxDiff);
    // The first copy is generated in both cases.
    REPORTER_ASSERT(reporter,
                    count_copy_mismatches(reused, kOffsets[0], generated, kOffsets[0]) == 0);
}

// Strips cached by one Device's draw can be reused by another Device of the same Recorder.
DEF_CONDITIONAL_GRAPHITE_TEST_FOR_CONTEXTS(SparseStrips_StripCacheAcrossDevices,
                                           skgpu::IsRenderingContext,
                                           reporter,
                                           context,
                                           testContext,
                                           /* anonymous options */,
                                           use_sparse_strips,
                                           /* condition= */ true,
                                           CtsEnforcement::kNever) {
    if (!is_sparse_strips_context(reporter, context)) {
        return;
    }

    std::unique_ptr<Recorder> recorder = context->makeRecorder();
    const SkImageInfo ii =
            SkImageInfo::Make(kRenderSize, kRenderSize, kRGBA_8888_SkColorType, kPremul_SkAlphaType);
    sk_sp<SkSurface> surfaceA = SkSurfaces::RenderTarget(recorder.get(), ii);
    sk_sp<SkSurface> surfaceB = SkSurfaces::RenderTarget(recorder.get(), ii);
    if (!surfaceA || !surfaceB) {
        ERRORF(reporter, "Could not create surfaces");
        return;
    }

    const SkPath star = make_star();
    constexpr SkIPoint kOffsetA = {100, 100};
    constexpr SkIPoint kOffsetB = {301, 203};
    surfaceA->getCanvas()->clear(SK_ColorWHITE);
    surfaceB->getCanvas()->clear(SK_ColorWHITE);
    clear_and_draw(surfaceA->getCanvas(), star, kOffsetA);
    clear_and_draw(surfaceB->getCanvas(), star, kOffsetB);

    if (!snap_and_insert(reporter, context, recorder.get())) {
        return;
    }
    const int hits =
            recorder->priv().getOrCreateAtlasProvider()->alphaAtlasManager()->stripCacheHits();
    REPORTER_ASSERT(reporter, hits == 1, "%d hits", hits);

    SkBitmap bitmapA;
    SkBitmap bitmapB;
    if (!read_back(reporter, surfaceA.get(), &bitmapA) ||
        !read_back(reporter, surfaceB.get(), &bitmapB)) {
        return;
    }
    const int mismatches = count_copy_mismatches(bitmapA, kOffsetA, bitmapB, kOffsetB);
    REPORTER_ASSERT(reporter, mismatches == 0, "%d mismatched pixels", mismatches);
}

// Clipped and inverse filled draws only reuse the strips of draws with the same transform and clip,
// so they must render exactly as if their strips had been generated.
DEF_CONDITIONAL_GRAPHITE_TEST_FOR_CONTEXTS(SparseStrips_StripCacheClippedRendering,
                                           skgpu::IsRenderingContext,
                                           reporter,
                                           context,
                                           testContext,
                                           /* anonymous options */,
                                           use_sparse_strips,
                                           /* condition= */ true,
                                           CtsEnforcement::kNever) {
    if (!is_sparse_strips_context(reporter, context)) {
        return;
    }

    // Every draw after the first of each kind reuses the first one's strips.
    constexpr int kRepeats = 3;
    // The clip cuts through the star.
    constexpr SkIPoint kClippedOffset = {120, 120};
    constexpr SkRect kClip = SkRect::MakeLTRB(90, 30, 150, 200);
    // The inverse fill covers the clip around the star.
    constexpr SkIPoint kInverseOffset = {360, 360};
    constexpr SkRect kInverseClip = SkRect::MakeLTRB(280, 280, 450, 450);

    auto render = [&](const SkPath& path, SkBitmap* result) -> std::optional<int> {
        std::unique_ptr<Recorder> recorder = context->makeRecorder();
        sk_sp<SkSurface> surface = SkSurfaces::RenderTarget(
                recorder.get(),
                SkImageInfo::Make(kRenderSize, kRenderSize, kRGBA_8888_SkColorType,
                                  kPremul_SkAlphaType));
        if (!surface) {
            ERRORF(reporter, "Could not create surface");
            return std::nullopt;
        }
        SkCanvas* canvas = surface->getCanvas();
        canvas->clear(SK_ColorWHITE);
        const SkPath inverse = path.makeFillType(SkPathFillType::kInverseWinding);
        for (int i = 0; i < kRepeats; ++i) {
            canvas->save();
            canvas->clipRect(kClip);
            clear_and_draw(canvas, path, kClippedOffset);
            canvas->restore();

            canvas->save();
            canvas->clipRect(kInverseClip);
            clear_and_draw(canvas, inverse, kInverseOffset);
            canvas->restore();
        }
        if (!snap_and_insert(reporter, context, recorder.get()) ||
            !read_back(reporter, surface.get(), result)) {
            return std::nullopt;
        }
        return recorder->priv().getOrCreateAtlasProvider()->alphaAtlasManager()->stripCacheHits();
    };

    const SkPath star = make_star();
    SkPath volatileStar = star;
    volatileStar.setIsVolatile(true);

    SkBitmap reused;
    SkBitmap generated;
    std::optional<int> reusedHits = render(star, &reused);
    std::optional<int> generatedHits = render(volatileStar, &generated);
    if (!reusedHits || !generatedHits) {
        return;
    }
    REPORTER_ASSERT(reporter, *reusedHits == 2 * (kRepeats - 1), "%d hits", *reusedHits);
    REPORTER_ASSERT(reporter, *generatedHits == 0, "%d hits", *generatedHits);

    int mismatches = 0;
    for (int y = 0; y < kRenderSize; ++y) {
        for (int x = 0; x < kRenderSize; ++x) {
            if (reused.getColor(x, y) != generated.getColor(x, y)) {
                mismatches++;
            }
        }
    }
    REPORTER_ASSERT(reporter, mismatches == 0, "%d mismatched pixels", mismatches);
}

}  // namespace skgpu::graphite
