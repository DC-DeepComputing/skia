/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/gpu/graphite/precompile/BlobOStuff.h"

#include "include/core/SkFourByteTag.h"
#include "include/core/SkStream.h"
#include "include/gpu/graphite/Context.h"
#include "src/core/SkStreamPriv.h"
#include "src/gpu/graphite/ContextPriv.h"

#include "src/base/SkAutoMalloc.h"
#include "src/gpu/graphite/PaintParamsKey.h"
#include "src/gpu/graphite/RenderPassDesc.h"
#include "src/gpu/graphite/TextureInfoPriv.h"

//#include "src/gpu/graphite/mtl/MtlGraphiteUtilsPriv.h"

#include "include/gpu/graphite/mtl/MtlGraphiteTypesUtils.h"

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

bool MtlTextureInfoData_Deserialize(SkStream* stream, void* out);

#define SK_BLOB_END_TAG SkSetFourByteTag('e', 'n', 'd', ' ')

// TextureInfo holds a TextureInfoData
//   TextureInfoData is a pure abstract class w/ a serialize method

static bool serialize_texture_info(SkWStream* stream, const TextureInfo& textureInfo) {
    if (!stream->write32(
            SkSetFourByteTag(static_cast<uint8_t>(textureInfo.backend()),
                             static_cast<uint8_t>(textureInfo.isValid()),
                             static_cast<uint8_t>(textureInfo.mipmapped()),
                             static_cast<uint8_t>(textureInfo.isProtected())))) {
        return false;
    }

    if (!stream->write32(textureInfo.numSamples())) { return false; }

    if (!textureInfo.isValid()) {
        // We don't bother serializing the textureInfoData is it isn't valid
        return true;
    }

    if (!textureInfo.fTextureInfoData.get()->serialize(stream)) { return false; }

    return true;
}

static bool deserialize_texture_info(const Caps* caps,
                                     SkStream* stream,
                                     TextureInfo* textureInfo) {
    uint32_t tag;
    if (!stream->readU32(&tag)) {
        return false;
    }

    BackendApi backendAPI = static_cast<BackendApi>(0xF & (tag >> 24));
    bool isValid = SkToBool(0xF & (tag >> 16));
    Mipmapped mipmapped = static_cast<Mipmapped>(0xF & (tag >> 8));
    Protected isProtected = static_cast<Protected>(0xF & tag);

    uint32_t sampleCount;
    if (!stream->readU32(&sampleCount)) { return false; }

    if (!isValid) { return false; }

    if (!caps->deserialize(stream, backendAPI, mipmapped, isProtected,
                           sampleCount, textureInfo)) { return false; }

    return true;
}

static bool serialize_attachment_desc(SkWStream* stream, const AttachmentDesc& attachmentDesc) {
    if (!serialize_texture_info(stream, attachmentDesc.fTextureInfo)) { return false; }

    if (!stream->write32(SkSetFourByteTag(static_cast<uint8_t>(attachmentDesc.fStoreOp),
                                          static_cast<uint8_t>(attachmentDesc.fLoadOp),
                                          0, 0))) {
        return false;
    }

    return true;
}

static bool deserialize_attachment_desc(const Caps* caps,
                                        SkStream* stream,
                                        AttachmentDesc* attachmentDesc) {
    if (!deserialize_texture_info(caps, stream, &attachmentDesc->fTextureInfo)) { return false; }

    uint32_t tag;
    if (!stream->readU32(&tag)) {
        return false;
    }

    attachmentDesc->fStoreOp = static_cast<StoreOp>(0xF & (tag >> 24));
    attachmentDesc->fLoadOp  = static_cast<LoadOp> (0xF & (tag >> 16));
    return true;
}

static bool serialize_render_pass_desc(SkWStream* stream, const RenderPassDesc& renderPassDesc) {
    if (!serialize_attachment_desc(stream, renderPassDesc.fColorAttachment)) { return false; }

    for (int i = 0; i < 4; ++i) {
        if (!stream->writeScalar(renderPassDesc.fClearColor[i])) { return false; }
    }

    if (!serialize_attachment_desc(stream, renderPassDesc.fColorResolveAttachment)) { return false; }
    if (!serialize_attachment_desc(stream, renderPassDesc.fDepthStencilAttachment)) { return false; }

    if (!stream->writeScalar(renderPassDesc.fClearDepth)) { return false; }
    if (!stream->write32(renderPassDesc.fClearStencil)) { return false; }

    if (!stream->write32(SkSetFourByteTag(renderPassDesc.fWriteSwizzle[0],
                                          renderPassDesc.fWriteSwizzle[1],
                                          renderPassDesc.fWriteSwizzle[2],
                                          renderPassDesc.fWriteSwizzle[3]))) {
        return false;
    }

    if (!stream->write32(renderPassDesc.fSampleCount)) { return false; }

    return true;
}

static bool deserialize_render_pass_desc(const Caps* caps,
                                         SkStream* stream,
                                         RenderPassDesc* renderPassDesc) {
    if (!deserialize_attachment_desc(caps, stream, &renderPassDesc->fColorAttachment)) { return false; }

    for (int i = 0; i < 4; ++i) {
        if (!stream->readScalar(&renderPassDesc->fClearColor[i])) { return false; }
    }

    if (!deserialize_attachment_desc(caps, stream, &renderPassDesc->fColorResolveAttachment)) {
        return false;
    }
    if (!deserialize_attachment_desc(caps, stream, &renderPassDesc->fDepthStencilAttachment)) {
        return false;
    }

    if (!stream->readScalar(&renderPassDesc->fClearDepth)) { return false; }
    if (!stream->readU32(&renderPassDesc->fClearStencil)) { return false; }

    uint32_t tag;
    if (!stream->readU32(&tag)) { return false; }

    char tmpSwizzle[4] = {
            (char) (0xF & (tag >> 24)),
            (char) (0xF & (tag >> 16)),
            (char) (0xF & (tag >> 8)),
            (char) (0xF & (tag)),
    };

    renderPassDesc->fWriteSwizzle = Swizzle(tmpSwizzle);

    if (!stream->readU32(&renderPassDesc->fSampleCount)) { return false; }

    return true;
}

static bool serialize_graphics_pipeline_desc(ShaderCodeDictionary* shaderCodeDictionary,
                                             SkWStream* stream,
                                             const GraphicsPipelineDesc& graphicsPipelineDesc) {
    PaintParamsKey key = shaderCodeDictionary->lookup(graphicsPipelineDesc.paintParamsID());
    SkASSERT(key.isValid());
    if (!key.isValid()) {
        return false;
    }

    if (!stream->write32(static_cast<uint32_t>(graphicsPipelineDesc.renderStepID()))) { return false; }
    if (!stream->write32(SkToU32(key.fData.size()))) { return false; }
    if (!stream->write(key.fData.data(), key.fData.size())) { return false; }
    return true;
}

static bool deserialize_graphics_pipeline_desc(ShaderCodeDictionary* shaderCodeDictionary,
                                               SkStream* stream,
                                               GraphicsPipelineDesc* graphicsPipelineDesc) {
    uint32_t tmp;
    if (!stream->readU32(&tmp)) { return false; }

    if (tmp >= RenderStep::kNumRenderSteps) { return false; }
    RenderStep::RenderStepID renderStepID = static_cast<RenderStep::RenderStepID>(tmp);

    if (!stream->readU32(&tmp)) { return false; }

    SkAutoMalloc storage(tmp);
    if (stream->read(storage.get(), tmp) != tmp) { return false; }

    PaintParamsKey ppk(SkSpan<uint32_t>((uint32_t*) storage.get(), tmp));

    UniquePaintParamsID paintParamsID = shaderCodeDictionary->findOrCreate(&ppk);

    *graphicsPipelineDesc = GraphicsPipelineDesc(renderStepID, paintParamsID);
    return true;
}

std::unique_ptr<BlobOStuff> BlobOStuff::MakeFromStream(const Caps* caps,
                                                       ShaderCodeDictionary* shaderCodeDictionary,
                                                       SkStream* stream) {
    if (!stream) {
        return nullptr;
    }
    return BlobOStuff::Deserialize(caps, shaderCodeDictionary, stream);
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
    if (!this->serialize(sharedContext->shaderCodeDictionary(), &stream)) {
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

bool BlobOStuff::serialize(ShaderCodeDictionary* shaderCodeDictionary,
                           SkWStream* stream) const {

    stream->write(kMagic, sizeof(kMagic));
    stream->write32(skgpu::graphite::BlobOStuff::kCurrent_Version);

    if (!serialize_graphics_pipeline_desc(shaderCodeDictionary, stream, fPipelineDesc)) { return false; }

    if (!serialize_render_pass_desc(stream, fRenderPassDesc)) { return false; }

    stream->write32(SK_BLOB_END_TAG);
    return true;
}

std::unique_ptr<BlobOStuff> BlobOStuff::Deserialize(const Caps* caps,
                                                    ShaderCodeDictionary* shaderCodeDictionary,
                                                    SkStream* stream) {
    SkASSERT(stream);

    if (!stream_is_blob(stream)) {
        return nullptr;
    }

    GraphicsPipelineDesc pipelineDesc;
    if (!deserialize_graphics_pipeline_desc(shaderCodeDictionary, stream, &pipelineDesc)) { return nullptr; }
    SkASSERT(pipelineDesc.isValid());

    RenderPassDesc renderPassDesc;
    if (deserialize_render_pass_desc(caps, stream, &renderPassDesc)) { return nullptr; }

    uint32_t tag;
    if (!stream->readU32(&tag)) {
        return nullptr;
    }

    if (tag != SK_BLOB_END_TAG) {
        return nullptr;
    }

    return std::make_unique<BlobOStuff>(pipelineDesc, renderPassDesc);
}

} // namespace skgpu::graphite
