/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_RasterPathAtlas_DEFINED
#define skgpu_graphite_RasterPathAtlas_DEFINED

#include "src/gpu/graphite/PathAtlas.h"

namespace skgpu::graphite {

/**
 * PathAtlas class that rasterizes coverage masks on the CPU.
 *
 * When a new shape gets added, its path is rasterized in preparation for upload. These
 * uploads are recorded by `recordUploads()` and subsequently added to an UploadTask.
 *
 * Shapes are cached for future frames to avoid the cost of raster pipeline rendering. Multiple
 * textures (or Pages) are used to cache masks, so if the atlas is full we can reset a Page and
 * start adding new shapes for a future atlas render.
 */
class RasterPathAtlas : public PathAtlas {
public:
    explicit RasterPathAtlas(Recorder* recorder);
    ~RasterPathAtlas() override {}

    void recordUploads(DrawContext* ctx) {
        fCachedAtlasMgr.recordUploads(ctx, fRecorder);
        fSmallPathAtlasMgr.recordUploads(ctx, fRecorder);
        fUncachedAtlasMgr.recordUploads(ctx, fRecorder);
    }

    void compact() {
        fCachedAtlasMgr.compact(fRecorder);
        fSmallPathAtlasMgr.compact(fRecorder);
        fUncachedAtlasMgr.compact(fRecorder);
    }

    void freeGpuResources() {
        fCachedAtlasMgr.freeGpuResources(fRecorder);
        fSmallPathAtlasMgr.freeGpuResources(fRecorder);
        fUncachedAtlasMgr.freeGpuResources(fRecorder);
    }

    void evictAtlases() {
        fCachedAtlasMgr.evictAll();
        fSmallPathAtlasMgr.evictAll();
        fUncachedAtlasMgr.evictAll();
    }

private:
    struct AtlasConfig {
        uint32_t fWidth, fHeight;
        uint32_t fPlotWidth, fPlotHeight;
        DrawAtlas::AllowMultitexturing fAllowMultitexturing;
    };
    static AtlasConfig GetDefaultAtlasConfig(uint32_t size, const Caps* caps);
    static AtlasConfig GetSmallPathAtlasConfig(const Caps* caps);

    class RasterAtlasMgr : public PathAtlas::DrawAtlasMgr {
    public:
        RasterAtlasMgr(AtlasConfig config,
                       uint32_t maxWidth, uint32_t maxHeight,
                       std::string_view label)
            : PathAtlas::DrawAtlasMgr(std::min(config.fWidth, maxWidth),
                                      std::min(config.fHeight, maxHeight),
                                      config.fPlotWidth, config.fPlotHeight,
                                      DrawAtlas::UseStorageTextures::kNo,
                                      config.fAllowMultitexturing,
                                      label) {}

    protected:
        bool onAddToAtlas(const Shape&,
                          const Transform& localToDevice,
                          const SkStrokeRec&,
                          SkIRect shapeBounds,
                          SkIVector transformedMaskOffset,
                          const AtlasLocator&) override;
    };

    sk_sp<TextureProxy> onAddShape(const Shape&,
                                   const Transform& localToDevice,
                                   const SkStrokeRec&,
                                   skvx::half2 maskOrigin,
                                   skvx::half2 maskSize,
                                   SkIVector transformedMaskOffset,
                                   skvx::half2* outPos) override;

    RasterAtlasMgr fCachedAtlasMgr;
    RasterAtlasMgr fSmallPathAtlasMgr;
    RasterAtlasMgr fUncachedAtlasMgr;

    // Behavior policy for putting paths in the small path atlas and whether or
    // not to allow falling back to MSAA.
    float fMaxSmallPathSize;
    bool fUseProxyCacheFallback;
};

}  // namespace skgpu::graphite

#endif  // skgpu_graphite_RasterPathAtlas_DEFINED
