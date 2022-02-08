/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/graphite/src/ContextUtils.h"

#include <string>
#include "experimental/graphite/src/ContextPriv.h"
#include "experimental/graphite/src/DrawTypes.h"
#include "experimental/graphite/src/PaintParams.h"
#include "include/core/SkPaint.h"
#include "include/private/SkShaderCodeDictionary.h"
#include "include/private/SkUniquePaintParamsID.h"
#include "src/core/SkBlenderBase.h"
#include "src/core/SkKeyHelpers.h"
#include "src/core/SkUniform.h"
#include "src/core/SkUniformData.h"

namespace skgpu {

#if 0

std::tuple<SkUniquePaintParamsID, std::unique_ptr<SkUniformBlock>> GetUniqueIDAndUniformData(
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

std::tuple<std::string, std::vector<SkSpan<const SkUniform>>> GetSkSLAndUniformDescription(
        SkShaderCodeDictionary* dict,
        SkUniquePaintParamsID uniqueID) {
    std::vector<SkSpan<const SkUniform>> uniforms;

    auto entry = dict->lookup(uniqueID);

    // TODO: cache this
    std::string skslCode = entry->paintParamsKey().toSkSL();

    return { skslCode, uniforms };
}

std::tuple<SkUniquePaintParamsID, std::unique_ptr<SkUniformBlock>> ExtractPaintData(
        SkShaderCodeDictionary* dictionary,
        const PaintParams& p) {

    SkPaintParamsKey key;
    std::unique_ptr<SkUniformBlock> block = std::make_unique<SkUniformBlock>();

    p.toKey(dictionary, SkBackend::kGraphite, &key, block.get());

    auto entry = dictionary->findOrCreate(key);

    return { entry->uniqueID(), std::move(block) };
}
#endif

} // namespace skgpu
