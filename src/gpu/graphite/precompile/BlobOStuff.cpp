/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/precompile/BlobOStuff.h"

#if 0
#include "include/core/SkFourByteTag.h"
#include "include/core/SkStream.h"
#include "include/gpu/graphite/Context.h"
#include "src/core/SkStreamPriv.h"
#include "src/gpu/graphite/ContextPriv.h"

#include "src/base/SkAutoMalloc.h"

#include "src/gpu/graphite/RenderPassDesc.h"
#include "src/gpu/graphite/TextureInfoPriv.h"

#include "src/gpu/graphite/precompile/SerializationUtils.h"

namespace {

} // anonymous namespace


namespace skgpu::graphite {

std::unique_ptr<BlobOStuff> BlobOStuff::MakeFromStream(const Caps* caps,
                                                       ShaderCodeDictionary* shaderCodeDictionary,
                                                       SkStream* stream) {
    if (!stream) {
        return nullptr;
    }

    std::unique_ptr<BlobOStuff> foo = std::make_unique<BlobOStuff>();

    if (!DeserializePipelineDesc(caps, shaderCodeDictionary, stream,
                                 &foo->fPipelineDesc,
                                 &foo->fRenderPassDesc)) {
        return nullptr;
    }

    return foo;
}

std::unique_ptr<BlobOStuff> BlobOStuff::MakeFromData(const Caps* caps,
                                                     ShaderCodeDictionary* shaderCodeDictionary,
                                                     const SkData* data) {
    if (!data) {
        return nullptr;
    }
    SkMemoryStream stream(data->data(), data->size());
    return MakeFromStream(caps, shaderCodeDictionary, &stream);
}

std::unique_ptr<BlobOStuff> BlobOStuff::MakeFromData(const Caps* caps,
                                                     ShaderCodeDictionary* shaderCodeDictionary,
                                                     const void* data, size_t size) {
    if (!data) {
        return nullptr;
    }
    SkMemoryStream stream(data, size);
    return MakeFromStream(caps, shaderCodeDictionary, &stream);
}

sk_sp<SkData> BlobOStuff::snap1(SharedContext* sharedContext) const {
    SkDynamicMemoryWStream stream;
    if (!SerializePipelineDesc(sharedContext->shaderCodeDictionary(),
                               &stream,
                               fPipelineDesc, fRenderPassDesc)) {
        return nullptr;
    }

    return stream.detachAsData();
}

bool BlobOStuff::compare(const BlobOStuff* other) const {
    if (fPipelineDesc != other->fPipelineDesc) {
        return false;
    }

    if (fRenderPassDesc != other->fRenderPassDesc) {
        return false;
    }

    return true;
}

void BlobOStuff::dump(const char* label, ShaderCodeDictionary* shaderCodeDictionary) const {
    SkString str1 = fPipelineDesc.toPipelineLabel(shaderCodeDictionary);
    SkString str2 = fRenderPassDesc.toPipelineLabel();
    SkDebugf("%s: %s - %s\n", label, str1.c_str(), str2.c_str());
}

} // namespace skgpu::graphite
#endif
