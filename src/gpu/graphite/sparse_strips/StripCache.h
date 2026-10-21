/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef skgpu_graphite_sparse_strips_StripCache_DEFINED
#define skgpu_graphite_sparse_strips_StripCache_DEFINED

#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkSpan.h"
#include "include/private/SkTArray.h"
#include "src/core/SkTHash.h"
#include "src/core/SkTInternalLList.h"
#include "src/gpu/graphite/geom/EndCaps.h"
#include "src/gpu/graphite/geom/WideTiles.h"
#include "src/gpu/graphite/sparse_strips/SparseStripsConfig.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <utility>

class SkMatrix;

namespace skgpu::graphite {

class Shape;

// An LRU cache of the strips (EndCaps and WideTiles) generated for shapes. A shape drawn again with
// the same transform, up to an integer translation, can reuse its strips and the alphas they
// reference in the alpha atlas, eliding strip generation entirely. Strips that depend on the clip
// can only be reused by draws with the same transform and clip.
//
// Frames tend to draw the same shapes in the same order, and an LRU cycled through more entries
// than it holds misses on every lookup. So entries used within the latest kStripCacheRecentFlushes
// flushes aren't evicted, and new entries are rejected instead, except that recently used clipped
// entries still make room for translation invariant ones.
//
// The cache only records where an entry's alphas were written. Whether they are still resident is
// decided by the AlphaAtlasManager, which owns the cache (see AlphaAtlasManager::findCachedStrips).
class StripCache {
public:
    using PageIDs = std::array<uint32_t, SparseStripConfig::kMaxTexturePages>;

    // Identifies the strips of a shape drawn with a transform. Keys come in two kinds, which never
    // match each other:
    //  - Translation invariant keys identify strips that don't depend on the clip: the shape's key,
    //    the transform's 2x2 matrix, and the fractional part of its translation. Translating by an
    //    integer amount translates the strips by exactly that amount, so the integer part of the
    //    translation is returned separately as the strips' origin.
    //  - Clipped keys identify strips that depend on the clip, because the shape was clipped or is
    //    inverse filled: the shape's key, the exact transform, and the tile clip that the strips
    //    were generated within. Their origin is always (0, 0).
    class Key {
    public:
        // Returns a translation invariant key, or nullopt if the shape's strips can't be reused at
        // other translations: inverse fills (which fill the clip), volatile paths, and perspective
        // or very large translations. Otherwise, `origin` receives the integer part of the
        // transform's translation.
        static std::optional<Key> Make(const Shape& shape, const SkMatrix& ctm, SkIPoint* origin);

        // Returns a clipped key for strips generated within `tileClip`, or nullopt if the shape's
        // strips can't be reused: volatile paths, and perspective or non-finite transforms.
        static std::optional<Key> MakeClipped(const Shape& shape,
                                              const SkMatrix& ctm,
                                              const SkIRect& tileClip);

        bool operator==(const Key& that) const {
            return fHash == that.fHash && fWords == that.fWords;
        }
        bool operator!=(const Key& that) const { return !(*this == that); }

        uint32_t hash() const { return fHash; }
        size_t bytes() const { return fWords.size_bytes(); }
        bool isClipped() const;

    private:
        Key() = default;

        skia_private::STArray<16, uint32_t> fWords;
        uint32_t fHash = 0;
    };

    struct Entry {
        skia_private::TArray<EndCaps::EndCap> fCaps;
        skia_private::TArray<WideTiles::WideTile> fWides;
        // For each regular atlas page slot, the ID of the page that the caps' alphas were written
        // to, or AlphaAtlasManager::kInvalidPageID if no cap references the slot.
        PageIDs fPageIDs;
        // The integer part of the translation the strips were generated with.
        SkIPoint fOrigin;
        // The device space bounds of the caps and wides.
        SkIRect fBounds;
    };

    StripCache() = default;
    ~StripCache();

    StripCache(const StripCache&) = delete;
    StripCache& operator=(const StripCache&) = delete;

    // Returns the entry for `key` and marks it as the most recently used one, used in the flush
    // `flushToken`, or null if there is none. The entry is valid until the cache is next modified.
    const Entry* find(const Key& key, uint64_t flushToken);

    // Adds an entry for `key`, used in the flush `flushToken`, replacing any existing one. The
    // least recently used entries are evicted as needed to stay within budget, except for entries
    // used within the latest kStripCacheRecentFlushes flushes, which are only evicted if they're
    // clipped, to make room for a translation invariant entry. Returns false, adding nothing, if
    // there is no room for the entry, or the entry alone would exceed kStripCacheMaxEntryBytes.
    bool add(const Key& key,
             uint64_t flushToken,
             SkIPoint origin,
             const PageIDs& pageIDs,
             SkSpan<const EndCaps::EndCap> caps,
             SkSpan<const WideTiles::WideTile> wides);

    void remove(const Key& key);
    void reset();

    int count() const { return fMap.count(); }
    // The approximate memory used by the entries.
    size_t bytes() const { return fBytes; }

private:
    struct Node {
        Node(const Key& key, Entry&& entry, size_t bytes, uint64_t flushToken)
                : fKey(key)
                , fEntry(std::move(entry))
                , fBytes(bytes)
                , fLastUse(flushToken)
                , fClipped(key.isClipped()) {}

        const Key fKey;
        Entry fEntry;
        size_t fBytes;
        // The flush token of the entry's most recent use.
        uint64_t fLastUse;
        const bool fClipped;

        SK_DECLARE_INTERNAL_LLIST_INTERFACE(Node);
    };

    struct Traits {
        static const Key& GetKey(const Node* node) { return node->fKey; }
        static uint32_t Hash(const Key& key) { return key.hash(); }
    };

    SkTInternalLList<Node>& lruFor(const Node* node) {
        return node->fClipped ? fClippedLRU : fInvariantLRU;
    }
    // Returns the entry to evict to make room for a new entry, or null if there is none.
    Node* findVictim(bool forClipped, uint64_t flushToken);
    void removeNode(Node* node);

    skia_private::THashTable<Node*, Key, Traits> fMap;
    // The translation invariant and clipped entries, each ordered from most to least recently used.
    SkTInternalLList<Node> fInvariantLRU;
    SkTInternalLList<Node> fClippedLRU;
    size_t fBytes = 0;
};

}  // namespace skgpu::graphite

#endif  // skgpu_graphite_sparse_strips_StripCache_DEFINED
