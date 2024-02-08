/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_SmallPathAtlas_DEFINED
#define skgpu_graphite_SmallPathAtlas_DEFINED

#include "src/base/SkTInternalLList.h"
#include "src/core/SkTHash.h"
#include "src/gpu/ResourceKey.h"
#include "src/gpu/graphite/DrawAtlas.h"
#include "src/gpu/graphite/PathAtlas.h"

namespace skgpu::graphite {

/**
 * PathAtlas class that rasterizes coverage masks on the CPU.
 *
 * When a new shape gets added, its path is rasterized in preparation for upload. These
 * uploads are recorded by `recordUploads()` and subsequently added to an UploadTask.
 *
 * After a successful call to `recordUploads()`, the client is free to call `reset()` and start
 * adding new shapes for a future atlas render.
 * TODO: We should cache Shapes for future frames to avoid the cost of raster pipeline rendering.
 */
class SmallPathAtlas : public PathAtlas {
public:
    SmallPathAtlas();
    ~SmallPathAtlas() override {}

protected:
    const TextureProxy* onAddShape(Recorder* recorder,
                                   const Shape&,
                                   const Transform& transform,
                                   const SkStrokeRec&,
                                   skvx::half2 maskSize,
                                   skvx::half2* outPos) override;

private:
    // TODO: select atlas size dynamically? Take ContextOptions::fMaxTextureAtlasSize into account?
    static constexpr int kDefaultAtlasDim = 2048;

    std::unique_ptr<DrawAtlas> fDrawAtlas;
};

}  // namespace skgpu::graphite

#endif  // skgpu_graphite_SmallPathAtlas_DEFINED
