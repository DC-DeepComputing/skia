/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/graphite/src/ContextUtils.h"

#include "experimental/graphite/src/PaintParams.h"
#include "include/private/SkShaderCodeDictionary.h"
#include "include/private/SkUniquePaintParamsID.h"
#include "src/core/SkUniformData.h"

namespace skgpu {

std::tuple<SkUniquePaintParamsID, std::unique_ptr<SkUniformBlock>> ExtractPaintData(
        SkShaderCodeDictionary* dict,
        const PaintParams& p,
        SkBackend backend) {
    std::unique_ptr<SkUniformBlock> block = std::make_unique<SkUniformBlock>();

    SkPaintParamsKey key;
    p.toKey(dict, backend, &key, block.get());

//    key.dump();

    auto entry = dict->findOrCreate(key);

    return { entry->uniqueID(), std::move(block) };
}

} // namespace skgpu
