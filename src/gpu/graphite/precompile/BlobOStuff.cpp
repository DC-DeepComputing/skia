/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/gpu/graphite/precompile/BlobOStuff.h"

#include "include/gpu/graphite/Context.h"
#include "include/core/SkFourByteTag.h"
#include "include/core/SkStream.h"
#include "src/core/SkStreamPriv.h"
#include "src/gpu/graphite/ContextPriv.h"

#include "src/base/SkAutoMalloc.h"
#include "src/gpu/graphite/PaintParamsKey.h"

namespace {

static const char kMagic[] = { 's', 'k', 'i', 'a', 'b', 'l', 'o', 'b' };

bool stream_is_blob(SkStream* stream) {
    char magic[8];
    static_assert(sizeof(kMagic) == sizeof(magic), "");

    if (stream->read(magic, sizeof(kMagic)) != sizeof(kMagic)) {
        return false;
    }

    if (0 != memcmp(magic, kMagic, sizeof(kMagic))) {
        return false;
    }

    uint32_t version;
    if (!stream->readU32(&version)) {
        return false;
    }

    if (version != skgpu::graphite::BlobOStuff::kCurrent_Version) {
        return false;
    }

    return true;
}

} // anonymous namespace


namespace skgpu::graphite {

BlobOStuff* BlobOStuff::MakeFromStream(SkStream* stream) {
    if (!stream) {
        return nullptr;
    }
    return BlobOStuff::Deserialize(stream);
}

BlobOStuff* BlobOStuff::MakeFromData(const SkData* data) {
    if (!data) {
        return nullptr;
    }
    SkMemoryStream stream(data->data(), data->size());
    return MakeFromStream(&stream);
}

BlobOStuff* BlobOStuff::MakeFromData(const void* data, size_t size) {
    if (!data) {
        return nullptr;
    }
    SkMemoryStream stream(data, size);
    return MakeFromStream(&stream);
}

sk_sp<SkData> BlobOStuff::snap(SharedContext* sharedContext) const {
    SkDynamicMemoryWStream stream;
    if (!this->serialize(sharedContext, &stream)) {
        return nullptr;
    }

    return stream.detachAsData();
}

#define SK_BLOB_RENDERSTEP_ID_TAG   SkSetFourByteTag('r', 's', 't', 'p')
#define SK_BLOB_PAINTPARAMS_KEY_TAG SkSetFourByteTag('p', 'p', 'k', 't')
#define SK_BLOB_END_TAG SkSetFourByteTag('e', 'n', 'd', ' ')

bool BlobOStuff::serialize(SharedContext* sharedContext, SkWStream* stream) const {
    ShaderCodeDictionary* shaderCodeDictionary = sharedContext->shaderCodeDictionary9();

    stream->write(kMagic, sizeof(kMagic));
    stream->write32(skgpu::graphite::BlobOStuff::kCurrent_Version);

    {
        stream->write32(SK_BLOB_RENDERSTEP_ID_TAG);
        stream->write32(static_cast<uint32_t>(fPipelineDesc.renderStepID1()));
    }

    {
        PaintParamsKey key = shaderCodeDictionary->lookup(fPipelineDesc.paintParamsID());
        SkASSERT(key.isValid());
        if (!key.isValid()) {
            return false;
        }

        stream->write32(SK_BLOB_PAINTPARAMS_KEY_TAG);
        stream->write32(SkToU32(key.fData.size()));
        stream->write(key.fData.data(), key.fData.size());
    }

    stream->write32(SK_BLOB_END_TAG);
    return true;
}

static GraphicsPipelineDesc deserialize_graphics_pipeline_desc(SkStream* stream) {
    uint32_t tag;
    uint32_t size;

    RenderStep::RenderStepID renderStepID = RenderStep::RenderStepID::kInvalid;

    {
        if (!stream->readU32(&tag)) {
            return {};
        }

        if (tag != SK_BLOB_RENDERSTEP_ID_TAG) {
            return {};
        }

        uint32_t tmp;
        if (!stream->readU32(&tmp)) {
            return {};
        }

        // TODO: check validity of tmp!!
        renderStepID = static_cast<RenderStep::RenderStepID>(tmp);
    }

    {
        if (!stream->readU32(&tag)) {
            return {};
        }

        if (tag != SK_BLOB_PAINTPARAMS_KEY_TAG) {
            return {};
        }

        if (!stream->readU32(&size)) {
            return {};
        }

        SkAutoMalloc storage(size);
        if (stream->read(storage.get(), size) != size) {
            return {};
        }
    }

    return GraphicsPipelineDesc(renderStepID, UniquePaintParamsID::InvalidID());
}

BlobOStuff* BlobOStuff::Deserialize(SkStream* stream) {
    SkASSERT(stream);

    if (!stream_is_blob(stream)) {
        return nullptr;
    }

    GraphicsPipelineDesc pipelineDesc = deserialize_graphics_pipeline_desc(stream);
    if (!pipelineDesc.isValid()) {
        return nullptr;
    }

    uint32_t tag;
    if (!stream->readU32(&tag)) {
        return nullptr;
    }

    if (tag != SK_BLOB_END_TAG) {
        return nullptr;
    }

    RenderPassDesc renderPassDesc;
#if 0
    = RenderPassDesc::Make(const Caps* caps,
                                                         const TextureInfo& targetInfo,
                                                         LoadOp loadOp,
                                                         StoreOp storeOp,
                                                         SkEnumBitMask<DepthStencilFlags> depthStencilFlags,
                                                         const std::array<float, 4>& clearColor,
                                                         bool requiresMSAA,
                                                         Swizzle writeSwizzle);
#endif

    return new BlobOStuff(pipelineDesc, renderPassDesc);
}

} // namespace skgpu::graphite
