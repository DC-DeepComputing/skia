/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef skgpu_graphite_ClipAtlasManager_DEFINED
#define skgpu_graphite_ClipAtlasManager_DEFINED

#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/private/base/SkTDArray.h"
#include "src/base/SkTInternalLList.h"
#include "src/core/SkTHash.h"
#include "src/gpu/ResourceKey.h"
#include "src/gpu/graphite/ClipStack.h"
#include "src/gpu/graphite/DrawAtlas.h"

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string_view>

struct SkIPoint;

namespace skgpu::graphite {

class Caps;
class DrawContext;
class Recorder;
class TextureProxy;

//////////////////////////////////////////////////////////////////////////////////////////////////
/** The ClipAtlasManager manages the lifetime of and access to rasterized clip masks.
 */
class ClipAtlasManager {
public:
    ClipAtlasManager(Recorder* recorder);
    ~ClipAtlasManager() = default;

    using Result = std::tuple<sk_sp<TextureProxy>, SkIPoint>;
    Result findOrCreateEntry(uint32_t stackRecordID,
                             const ClipStack::ElementList*,
                             SkIRect maskDeviceBounds);

    bool recordUploads(DrawContext* dc);
    void compact();
    void freeGpuResources();

    void evictAtlases();

private:
    // Wrapper class to manage DrawAtlas and associated caching operations
    class DrawAtlasMgr : public DrawAtlas::EvictionCallback {
    public:
        DrawAtlasMgr(size_t width, size_t height,
                     size_t plotWidth, size_t plotHeight,
                     DrawAtlas::UseStorageTextures useStorageTextures,
                     std::string_view label, const Caps*);

        Result findOrCreateEntry(Recorder* recorder,
                                 const skgpu::UniqueKey&,
                                 const ClipStack::ElementList*,
                                 SkIRect maskDeviceBounds,
                                 SkIRect keyBounds);
        // Adds to DrawAtlas but not the cache
        std::optional<DrawAtlas::AddResult> addToAtlas(Recorder* recorder,
                                                       const ClipStack::ElementList*,
                                                       SkIRect maskDeviceBounds);
        bool recordUploads(DrawContext*, Recorder*);
        void evict(const DrawAtlas::RecordVisitor&) override;
        void compact(Recorder*);
        void freeGpuResources(Recorder*);

        void evictAll();

    private:
        std::unique_ptr<DrawAtlas> fDrawAtlas;

        // Tracks whether a combined clip mask is already in the DrawAtlas and its location
        struct MaskHashEntry {
            MaskHashEntry* fNext = nullptr;
            DrawAtlas::Record fRecord;
            DrawAtlas::Rect16 fMaskRelativeBounds;
            SkIPoint16 fAtlasPos{0, 0};
            uint8_t fPageIdx = 0;
        };
        struct UniqueKeyHash {
            uint32_t operator()(const skgpu::UniqueKey& key) const { return key.hash(); }
        };
        using MaskCache = skia_private::THashMap<skgpu::UniqueKey, MaskHashEntry, UniqueKeyHash>;
        MaskCache fMaskCache;

        // Reverse map used to remove MaskHashEntry from fMaskCache upon eviction. Note that each
        // key in fMaskCache maps to a list. We can use the DrawAtlas::Record to find the entry
        // that was evicted.
        skia_private::THashMap<DrawAtlas::Record, UniqueKey> fAtlasRecordMap;
    };

    Recorder* fRecorder;
    // We have two atlas managers, one for clips that can be keyed via the path keys,
    // and a smaller one for those that can only be keyed by the SaveRecord ID. We keep
    // them separate because the SaveRecord keyed clips will be far more transient, i.e.,
    // once the SaveRecord is popped they'll never be used again.
    DrawAtlasMgr fPathKeyAtlasMgr;
    DrawAtlasMgr fSaveRecordKeyAtlasMgr;
};

}  // namespace skgpu::graphite

#endif  // skgpu_graphite_ClipAtlasManager_DEFINED
