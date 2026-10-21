/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "src/gpu/graphite/sparse_strips/StripCache.h"

#include "include/core/SkMatrix.h"
#include "src/core/SkChecksum.h"
#include "src/core/SkFloatBits.h"
#include "src/gpu/graphite/geom/Shape.h"

#include <algorithm>
#include <cmath>

namespace skgpu::graphite {

namespace {

// The first word of every key is its kind, so that keys of different kinds never match.
enum class KeyKind : uint32_t {
    kTranslationInvariant = 0,
    kClipped = 1,
};

// The number of words preceding the shape's key in a translation invariant key: its kind, the
// transform's 2x2 matrix, and the fractional part of its translation.
constexpr int kInvariantHeaderWords = 7;
// The number of words preceding the shape's key in a clipped key: its kind, the transform's 2x2
// matrix and translation, and the tile clip.
constexpr int kClippedHeaderWords = 11;

// Larger translations are not keyed. Within this range the fractional part of a translation is
// exact, and anything further away couldn't be translated into the uint16 coordinate range of the
// strips anyway.
constexpr float kMaxTranslate = 1 << 20;

static_assert(SparseStripConfig::kStripCacheMaxEntryBytes <= SparseStripConfig::kStripCacheMaxBytes);

bool within_budget(int count, size_t bytes) {
    return count <= SparseStripConfig::kStripCacheMaxEntries &&
           bytes <= SparseStripConfig::kStripCacheMaxBytes;
}

}  // anonymous namespace

std::optional<StripCache::Key> StripCache::Key::Make(const Shape& shape,
                                                     const SkMatrix& ctm,
                                                     SkIPoint* origin) {
    SkASSERT(origin);
    if (ctm.hasPerspective() || shape.isEmpty() || shape.inverted() || shape.isVolatilePath()) {
        return std::nullopt;
    }
    const float tx = ctm.getTranslateX();
    const float ty = ctm.getTranslateY();
    // Also rejects NaNs.
    if (!(std::fabs(tx) < kMaxTranslate && std::fabs(ty) < kMaxTranslate)) {
        return std::nullopt;
    }
    // t - floor(t) is exact, so two translations whose fractional parts have identical bits differ
    // by exactly the difference of their integer parts.
    const float ix = std::floor(tx);
    const float iy = std::floor(ty);

    Key key;
    uint32_t* words = key.fWords.push_back_n(kInvariantHeaderWords + shape.keySize());
    words[0] = static_cast<uint32_t>(KeyKind::kTranslationInvariant);
    words[1] = SkFloat2Bits(ctm.getScaleX());
    words[2] = SkFloat2Bits(ctm.getSkewX());
    words[3] = SkFloat2Bits(ctm.getSkewY());
    words[4] = SkFloat2Bits(ctm.getScaleY());
    words[5] = SkFloat2Bits(tx - ix);
    words[6] = SkFloat2Bits(ty - iy);
    shape.writeKey(words + kInvariantHeaderWords, /*includeInverted=*/true);
    key.fHash = SkChecksum::Hash32(key.fWords.data(), key.fWords.size_bytes());

    *origin = {static_cast<int32_t>(ix), static_cast<int32_t>(iy)};
    return key;
}

std::optional<StripCache::Key> StripCache::Key::MakeClipped(const Shape& shape,
                                                            const SkMatrix& ctm,
                                                            const SkIRect& tileClip) {
    if (ctm.hasPerspective() || !ctm.isFinite() || shape.isEmpty() || shape.isVolatilePath()) {
        return std::nullopt;
    }

    Key key;
    uint32_t* words = key.fWords.push_back_n(kClippedHeaderWords + shape.keySize());
    words[0] = static_cast<uint32_t>(KeyKind::kClipped);
    words[1] = SkFloat2Bits(ctm.getScaleX());
    words[2] = SkFloat2Bits(ctm.getSkewX());
    words[3] = SkFloat2Bits(ctm.getSkewY());
    words[4] = SkFloat2Bits(ctm.getScaleY());
    words[5] = SkFloat2Bits(ctm.getTranslateX());
    words[6] = SkFloat2Bits(ctm.getTranslateY());
    words[7] = static_cast<uint32_t>(tileClip.fLeft);
    words[8] = static_cast<uint32_t>(tileClip.fTop);
    words[9] = static_cast<uint32_t>(tileClip.fRight);
    words[10] = static_cast<uint32_t>(tileClip.fBottom);
    shape.writeKey(words + kClippedHeaderWords, /*includeInverted=*/true);
    key.fHash = SkChecksum::Hash32(key.fWords.data(), key.fWords.size_bytes());
    return key;
}

StripCache::~StripCache() { this->reset(); }

bool StripCache::Key::isClipped() const {
    SkASSERT(!fWords.empty());
    return fWords[0] == static_cast<uint32_t>(KeyKind::kClipped);
}

const StripCache::Entry* StripCache::find(const Key& key, uint64_t flushToken) {
    Node** found = fMap.find(key);
    if (!found) {
        return nullptr;
    }
    Node* node = *found;
    node->fLastUse = std::max(node->fLastUse, flushToken);
    SkTInternalLList<Node>& lru = this->lruFor(node);
    if (node != lru.head()) {
        lru.remove(node);
        lru.addToHead(node);
    }
    return &node->fEntry;
}

StripCache::Node* StripCache::findVictim(bool forClipped, uint64_t flushToken) {
    Node* invariant = fInvariantLRU.tail();
    Node* clipped = fClippedLRU.tail();
    // The least recently used entry can be evicted if it hasn't been used recently.
    Node* oldest = !invariant ? clipped
                   : !clipped ? invariant
                              : (clipped->fLastUse < invariant->fLastUse ? clipped : invariant);
    if (oldest && oldest->fLastUse + SparseStripConfig::kStripCacheRecentFlushes <= flushToken) {
        return oldest;
    }
    // Otherwise all entries were used recently, and only clipped ones make room, and only for
    // translation invariant entries.
    return forClipped ? nullptr : clipped;
}

bool StripCache::add(const Key& key,
                     uint64_t flushToken,
                     SkIPoint origin,
                     const PageIDs& pageIDs,
                     SkSpan<const EndCaps::EndCap> caps,
                     SkSpan<const WideTiles::WideTile> wides) {
    this->remove(key);

    const size_t bytes = sizeof(Node) + key.bytes() + caps.size_bytes() + wides.size_bytes();
    if (bytes > SparseStripConfig::kStripCacheMaxEntryBytes) {
        return false;
    }
    while (!within_budget(fMap.count() + 1, fBytes + bytes)) {
        Node* victim = this->findVictim(key.isClipped(), flushToken);
        if (!victim) {
            return false;
        }
        this->removeNode(victim);
    }

    Entry entry;
    entry.fCaps = skia_private::TArray<EndCaps::EndCap>(caps);
    entry.fWides = skia_private::TArray<WideTiles::WideTile>(wides);
    entry.fPageIDs = pageIDs;
    entry.fOrigin = origin;
    entry.fBounds = SkIRect::MakeEmpty();
    for (const EndCaps::EndCap& cap : caps) {
        entry.fBounds.join(
                SkIRect::MakeXYWH(cap.fX, cap.fY, cap.fWidth, SparseStripConfig::kTileHeight));
    }
    for (const WideTiles::WideTile& wide : wides) {
        entry.fBounds.join(
                SkIRect::MakeXYWH(wide.fX, wide.fY, wide.fWidth, SparseStripConfig::kTileHeight));
    }

    Node* node = new Node(key, std::move(entry), bytes, flushToken);
    fMap.set(node);
    this->lruFor(node).addToHead(node);
    fBytes += bytes;
    return true;
}

void StripCache::remove(const Key& key) {
    if (Node** found = fMap.find(key)) {
        this->removeNode(*found);
    }
}

void StripCache::removeNode(Node* node) {
    SkASSERT(node && fBytes >= node->fBytes);
    fBytes -= node->fBytes;
    fMap.remove(node->fKey);
    this->lruFor(node).remove(node);
    delete node;
}

void StripCache::reset() {
    fMap.reset();
    for (SkTInternalLList<Node>* lru : {&fInvariantLRU, &fClippedLRU}) {
        while (Node* node = lru->head()) {
            lru->remove(node);
            delete node;
        }
    }
    fBytes = 0;
}

}  // namespace skgpu::graphite
