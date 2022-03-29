/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/graphite/src/ContextUtils.h"

#include <string>
#include "experimental/graphite/src/PaintParams.h"
#include "experimental/graphite/src/RecorderPriv.h"
#include "include/private/SkUniquePaintParamsID.h"
#include "src/core/SkBlenderBase.h"
#include "src/core/SkKeyContext.h"
#include "src/core/SkPipelineData.h"
#include "src/core/SkShaderCodeDictionary.h"

namespace skgpu {

std::tuple<SkUniquePaintParamsID, UniformDataCache::Index, TextureDataCache::Index> ExtractPaintData(
        Recorder* recorder,
        SkPipelineDataGatherer* gatherer,
        SkPaintParamsKeyBuilder* builder,
        const PaintParams& p) {

    SkDEBUGCODE(gatherer->checkReset());
    SkDEBUGCODE(builder->checkReset());

    SkKeyContext keyContext(recorder);

    p.toKey(keyContext, builder, gatherer);

    SkPaintParamsKey key = builder->lockAsKey();

    auto dict = keyContext.dict();
    UniformDataCache* uniformDataCache = recorder->priv().uniformDataCache();
    TextureDataCache* textureDataCache = recorder->priv().textureDataCache();

    auto entry = dict->findOrCreate(key, gatherer->blendInfo());

    UniformDataCache::Index shadingUniformIndex = uniformDataCache->insert(gatherer->uniformDataBlock());
    TextureDataCache::Index textureBindingIndex = textureDataCache->insert(gatherer->textureDataBlock());

    return { entry->uniqueID(), shadingUniformIndex, textureBindingIndex };
}

} // namespace skgpu
