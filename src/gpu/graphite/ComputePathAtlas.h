/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_ComputePathAtlas_DEFINED
#define skgpu_graphite_ComputePathAtlas_DEFINED

#include "src/gpu/graphite/PathAtlas.h"

#include "src/base/SkTInternalLList.h"
#include "src/core/SkTHash.h"
#include "src/gpu/AtlasTypes.h"
#include "src/gpu/RectanizerSkyline.h"
#include "src/gpu/ResourceKey.h"
#include "src/gpu/graphite/DrawAtlas.h"

#ifdef SK_ENABLE_VELLO_SHADERS
#include "src/gpu/graphite/compute/VelloRenderer.h"
#endif

#include <memory>

namespace skgpu::graphite {

class DispatchGroup;

/**
 * Base class for PathAtlas implementations that rasterize coverage masks on the GPU using compute
 * shaders.
 *
 * When a new shape gets added, it gets tracked as input to a series of GPU compute passes. This
 * data is recorded by `recordDispatches()` into a DispatchGroup which can be added to a
 * ComputeTask.
 *
 * After a successful call to `recordDispatches()`, the client is free to call `reset()` and start
 * adding new shapes for a future atlas render.
 */
class ComputePathAtlas : public PathAtlas {
public:
    // Returns the currently preferred ComputePathAtlas implementation.
    static std::unique_ptr<ComputePathAtlas> CreateDefault(Recorder*);

    virtual std::unique_ptr<DispatchGroup> recordDispatches(Recorder*) const = 0;

    // Clear all scheduled atlas draws and free up atlas allocations, if necessary. After this call
    // the atlas can be considered cleared and available for new shape insertions. However this
    // method does not have any bearing on the contents of any atlas textures themselves, which may
    // be in use by GPU commands that are in-flight or yet to be submitted.
    void reset();

protected:
    explicit ComputePathAtlas(Recorder*);

    const TextureProxy* texture() const { return fTexture.get(); }
    const TextureProxy* addRect(skvx::half2 maskSize,
                                SkIPoint16* outPos);
    bool isSuitableForAtlasing(const Rect& transformedShapeBounds) const override;

    virtual void onReset() = 0;

private:
    bool initializeTextureIfNeeded();

    // Wrapper class to manage DrawAtlas and associated caching operations
    class DrawAtlasMgr : public AtlasGenerationCounter, public PlotEvictionCallback {
    public:
        DrawAtlasMgr(size_t width, size_t height,
                     size_t plotWidth, size_t plotHeight,
                     const Caps*);

        const TextureProxy* findOrCreateEntry(Recorder* recorder,
                                              const Shape& shape,
                                              const Transform& transform,
                                              const SkStrokeRec& strokeRec,
                                              skvx::half2 maskSize,
                                              skvx::half2* outPos);
        // Adds to DrawAtlas but not the cache
        const TextureProxy* addToAtlas(Recorder* recorder,
                                       const Shape& shape,
                                       const Transform& transform,
                                       const SkStrokeRec& strokeRec,
                                       skvx::half2 maskSize,
                                       skvx::half2* outPos,
                                       AtlasLocator* locator);
        bool recordUploads(DrawContext*, Recorder*);
        void evict(PlotLocator) override;
        void postFlush(Recorder*);

    private:
        std::unique_ptr<DrawAtlas> fDrawAtlas;

        // Tracks whether a shape is already in the DrawAtlas, and its location in the atlas
        struct UniqueKeyHash {
            uint32_t operator()(const skgpu::UniqueKey& key) const { return key.hash(); }
        };
        using ShapeCache = skia_private::THashMap<skgpu::UniqueKey, AtlasLocator, UniqueKeyHash>;
        ShapeCache fShapeCache;

        // List of stored keys per Plot, used to invalidate cache entries.
        // When a Plot is invalidated via evict(), we'll get its index and Page index from the
        // PlotLocator, index into the fKeyLists array to get the ShapeKeyList for that Plot,
        // then iterate through the list and remove entries matching those keys from the ShapeCache.
        struct ShapeKeyEntry {
            skgpu::UniqueKey fKey;
            SK_DECLARE_INTERNAL_LLIST_INTERFACE(ShapeKeyEntry);
        };
        using ShapeKeyList = SkTInternalLList<ShapeKeyEntry>;
        SkTDArray<ShapeKeyList> fKeyLists;
    };

    DrawAtlasMgr fCachedAtlasMgr;

    //////////////////
    // Uncached data
    skgpu::RectanizerSkyline fRectanizer;

    // ComputePathAtlas lazily requests a texture from the AtlasProvider when the first shape gets
    // added to it and references the same texture for the duration of its lifetime. A reference to
    // this texture is stored here, which is used by AtlasShapeRenderStep when encoding the render
    // pass.
    sk_sp<TextureProxy> fTexture;
};

}  // namespace skgpu::graphite

#endif  // skgpu_graphite_ComputePathAtlas_DEFINED
