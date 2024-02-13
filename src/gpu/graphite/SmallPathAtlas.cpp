/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/SmallPathAtlas.h"

/*
 #include "include/core/SkColorSpace.h"
#include "include/gpu/graphite/Recorder.h"
#include "src/core/SkBlitter_A8.h"
#include "src/core/SkDrawBase.h"
#include "src/core/SkIPoint16.h"
#include "src/core/SkRasterClip.h"
#include "src/gpu/graphite/AtlasProvider.h"
#include "src/gpu/graphite/DrawContext.h"
#include "src/gpu/graphite/Log.h"
#include "src/gpu/graphite/RecorderPriv.h"
*/
#include "src/gpu/graphite/DrawAtlas.h"
#include "src/gpu/graphite/TextureProxy.h"

namespace skgpu::graphite {

SmallPathAtlas::SmallPathAtlas(Recorder* recorder)
        : PathAtlas(recorder, kDefaultAtlasDim, kDefaultAtlasDim) {
}

bool SmallPathAtlas::initAtlas() {
    if (fDrawAtlas) {
        return true;
    }

    static constexpr size_t kPlotWidth = 512;
    static constexpr size_t kPlotHeight = 256;

    SkColorType colorType = kAlpha_8_SkColorType;
    // TODO: Set atlas size based on maxTextureSize()
    fDrawAtlas = DrawAtlas::Make(colorType,
                                 SkColorTypeBytesPerPixel(colorType),
                                 kDefaultAtlasDim, kDefaultAtlasDim,
                                 kPlotWidth, kPlotHeight,
                                 this,
                                 DrawAtlas::AllowMultitexturing::kYes,
                                 nullptr,
                                 /*label=*/"SmallPathAtlas");

    return SkToBool (fDrawAtlas);
}

const TextureProxy* SmallPathAtlas::onAddShape(const Shape& shape,
                                               const Transform& transform,
                                               const SkStrokeRec& strokeRec,
                                               skvx::half2 maskSize,
                                               skvx::half2* outPos) {
    return nullptr;
}

}  // namespace skgpu::graphite
