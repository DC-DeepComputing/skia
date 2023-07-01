/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/AtlasProvider.h"

#include "include/gpu/graphite/Recorder.h"
#include "src/gpu/graphite/PathAtlas.h"
#include "src/gpu/graphite/RecorderPriv.h"
#include "src/gpu/graphite/text/TextAtlasManager.h"

namespace skgpu::graphite {

AtlasProvider::AtlasProvider(Recorder* recorder)
        : fTextAtlasManager(std::make_unique<TextAtlasManager>(recorder)) {}

std::unique_ptr<ComputePathAtlas> AtlasProvider::createComputePathAtlas(Recorder* recorder) const {
#ifdef SK_ENABLE_VELLO_SHADERS
    if (recorder->priv().caps()->computeSupport()) {
        return std::make_unique<VelloComputePathAtlas>();
    }
#endif  // SK_ENABLE_VELLO_SHADERS
    return nullptr;
}

}  // namespace skgpu::graphite
