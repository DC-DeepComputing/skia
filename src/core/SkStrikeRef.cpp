/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkStrikeRef.h"

#include "include/core/SkRect.h"
#include "include/private/base/SkTArray.h"
#include "include/private/base/SkTemplates.h"
#include "include/private/base/SkTo.h"
#include "src/base/SkZip.h"
#include "src/core/SkGlyph.h"
#include "src/core/SkStrike.h"
#include "src/core/SkTHash.h"

#include <algorithm>
#include <cmath>
#include <limits>

using namespace skia_private;

namespace {

static inline SkRect scale_rect(SkRect r, SkScalar s) {
    SkASSERT(s >= 0);
    return {r.fLeft * s, r.fTop * s, r.fRight * s, r.fBottom * s};
}

}  // namespace

// Per-glyph advance cache attached to an SkStrikeRef. Single-owner: callers
// that share an SkStrikeRef across threads must synchronize externally.
//
// Storage layout mirrors Blink's GlyphMetricsMap: a flat 256-entry "primary
// page" covers the common Latin-1 / first-page case (where almost all CSS
// text glyphs land), plus a hash map of additional 256-entry pages for higher
// glyph IDs (e.g. CJK). This avoids the hash-map overhead on the hot path.
class SkStrikeRef::AdvanceCache {
public:
    AdvanceCache() {
        std::fill_n(fPrimary, kPageSize, kUnknown);
    }

    bool tryGet(SkGlyphID glyph, SkScalar* out) const {
        const SkScalar* page = this->findPage(glyph / kPageSize);
        if (!page) {
            return false;
        }
        SkScalar v = page[glyph % kPageSize];
        if (std::isnan(v)) {  // kUnknown sentinel
            return false;
        }
        *out = v;
        return true;
    }

    void set(SkGlyphID glyph, SkScalar value) {
        SkScalar* page = this->getOrCreatePage(glyph / kPageSize);
        page[glyph % kPageSize] = value;
    }

private:
    static constexpr size_t kPageSize = 256;
    // NaN is used as the "unknown" sentinel because valid glyph advances are
    // always non-NaN finite values.
    static const SkScalar kUnknown;

    const SkScalar* findPage(unsigned page_index) const {
        if (page_index == 0) {
            return fPrimary;
        }
        const std::unique_ptr<SkScalar[]>* page = fPages.find(page_index);
        return page ? page->get() : nullptr;
    }

    SkScalar* getOrCreatePage(unsigned page_index) {
        if (page_index == 0) {
            return fPrimary;
        }
        std::unique_ptr<SkScalar[]>* existing = fPages.find(page_index);
        if (existing) {
            return existing->get();
        }
        std::unique_ptr<SkScalar[]> page(new SkScalar[kPageSize]);
        std::fill_n(page.get(), kPageSize, kUnknown);
        SkScalar* raw = page.get();
        fPages.set(page_index, std::move(page));
        return raw;
    }

    SkScalar fPrimary[kPageSize];
    THashMap<unsigned, std::unique_ptr<SkScalar[]>> fPages;
};

const SkScalar SkStrikeRef::AdvanceCache::kUnknown =
        std::numeric_limits<SkScalar>::quiet_NaN();

SkStrikeRef::SkStrikeRef() = default;
SkStrikeRef::~SkStrikeRef() = default;
SkStrikeRef::SkStrikeRef(const SkStrikeRef&) = default;
SkStrikeRef& SkStrikeRef::operator=(const SkStrikeRef&) = default;
SkStrikeRef::SkStrikeRef(SkStrikeRef&&) = default;
SkStrikeRef& SkStrikeRef::operator=(SkStrikeRef&&) = default;

SkStrikeRef::SkStrikeRef(sk_sp<SkStrike> strike,
                         SkScalar strikeToSourceScale,
                         bool isSubpixel)
    : fStrike(std::move(strike))
    , fStrikeToSourceScale(strikeToSourceScale)
    , fIsSubpixel(isSubpixel) {}

SkStrikeRef::AdvanceCache& SkStrikeRef::getOrCreateCache() const {
    if (!fAdvanceCache) {
        fAdvanceCache = std::make_shared<AdvanceCache>();
    }
    return *fAdvanceCache;
}

void SkStrikeRef::getWidths(SkSpan<const SkGlyphID> glyphs, SkSpan<SkScalar> widths) const {
    this->getWidthsBounds(glyphs, widths, {});
}

SkScalar SkStrikeRef::getWidth(SkGlyphID glyph) const {
    SkScalar width;
    this->getWidths({&glyph, 1}, {&width, 1});
    return width;
}

SkScalar SkStrikeRef::getCachedWidth(SkGlyphID glyph) const {
    AdvanceCache& cache = this->getOrCreateCache();
    SkScalar width;
    if (cache.tryGet(glyph, &width)) {
        return width;
    }
    width = this->getWidth(glyph);
    cache.set(glyph, width);
    return width;
}

void SkStrikeRef::getCachedWidths(SkSpan<const SkGlyphID> glyphs,
                                  SkSpan<SkScalar> widths) const {
    AdvanceCache& cache = this->getOrCreateCache();
    const size_t n = std::min(glyphs.size(), widths.size());

    // First pass: try to satisfy from the cache and collect the misses.
    STArray<128, SkGlyphID> miss_glyphs;
    STArray<128, size_t> miss_indices;
    for (size_t i = 0; i < n; ++i) {
        if (!cache.tryGet(glyphs[i], &widths[i])) {
            miss_glyphs.push_back(glyphs[i]);
            miss_indices.push_back(i);
        }
    }

    if (miss_glyphs.empty()) {
        return;  // All-cached fast path: no SkStrike lookup needed.
    }

    // Second pass: bulk-fetch the misses, scatter into output, populate cache.
    STArray<128, SkScalar> miss_widths;
    miss_widths.resize(miss_glyphs.size());
    this->getWidths(miss_glyphs, miss_widths);
    for (int i = 0; i < miss_glyphs.size(); ++i) {
        widths[miss_indices[i]] = miss_widths[i];
        cache.set(miss_glyphs[i], miss_widths[i]);
    }
}

void SkStrikeRef::getWidthsBounds(SkSpan<const SkGlyphID> glyphs,
                                  SkSpan<SkScalar> widths,
                                  SkSpan<SkRect> bounds) const {
    SkASSERT(fStrike);

    AutoSTArray<64, const SkGlyph*> glyphPtrs(SkTo<int>(glyphs.size()));
    SkSpan<const SkGlyph*> results = fStrike->metrics(glyphs, glyphPtrs.get());

    if (bounds.size()) {
        const auto n = std::min(bounds.size(), results.size());
        for (auto [bound, glyph] : SkMakeZip(bounds.first(n), results.first(n))) {
            bound = scale_rect(glyph->rect(), fStrikeToSourceScale);
        }
    }

    if (widths.size()) {
        const auto n = std::min(widths.size(), results.size());
        for (auto [width, glyph] : SkMakeZip(widths.first(n), results.first(n))) {
            width = glyph->advanceX() * fStrikeToSourceScale;
        }
    }
}
