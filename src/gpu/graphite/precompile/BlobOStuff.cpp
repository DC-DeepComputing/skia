/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/precompile/BlobOStuff.h"

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

    if (!SerializationUtils::Deserialize(caps, shaderCodeDictionary, stream,
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
    if (!SerializationUtils::Serialize(sharedContext->shaderCodeDictionary(),
                                       fPipelineDesc, fRenderPassDesc,
                                       &stream)) {
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


} // namespace skgpu::graphite
