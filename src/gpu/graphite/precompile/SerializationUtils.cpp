/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/precompile/SerializationUtils.h"

#include "include/core/SkFourByteTag.h"
#include "include/core/SkStream.h"
#include "src/base/SkAutoMalloc.h"
#include "src/gpu/graphite/GraphicsPipelineDesc.h"
#include "src/gpu/graphite/PaintParamsKey.h"
#include "src/gpu/graphite/RenderPassDesc.h"
#include "src/gpu/graphite/Renderer.h"
#include "src/gpu/graphite/ShaderCodeDictionary.h"
#include "src/gpu/graphite/TextureInfoPriv.h"

namespace skgpu::graphite {

// This is the main control to version the serialized Pipelines (c.f. stream_is_blob)
static const int kCurrent_Version = 1;

namespace {

static const char kMagic[] = { 's', 'k', 'i', 'a', 'p', 'i', 'p', 'e' };

[[nodiscard]] bool stream_is_pipeline(SkStream* stream) {
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

    if (version != kCurrent_Version) {
        return false;
    }

    return true;
}

[[nodiscard]] bool serialize_graphics_pipeline_desc(ShaderCodeDictionary* shaderCodeDictionary,
                                                    SkWStream* stream,
                                                    const GraphicsPipelineDesc& pipelineDesc) {
    PaintParamsKey key = shaderCodeDictionary->lookup(pipelineDesc.paintParamsID());

    if (!stream->write32(static_cast<uint32_t>(pipelineDesc.renderStepID()))) {
        return false;
    }

    if (!key.isValid()) {
        if (!stream->write32(0)) {
            return false;
        }
        // Not all GraphicsPipeline have a valid PaintParamsKey
        return true;
    }

    const SkSpan<const uint32_t> keySpan = key.data();

    if (!key.isSerializable(shaderCodeDictionary)) {
        return false;
    }

    if (!stream->write32(SkToU32(keySpan.size()))) {
        return false;
    }
    if (!stream->write(keySpan.data(), 4 * keySpan.size())) {
        return false;
    }
    return true;
}

[[nodiscard]] bool deserialize_graphics_pipeline_desc(ShaderCodeDictionary* shaderCodeDictionary,
                                                      SkStream* stream,
                                                      GraphicsPipelineDesc* pipelineDesc) {
    uint32_t tmp;
    if (!stream->readU32(&tmp)) {
        return false;
    }

    if (tmp >= RenderStep::kNumRenderSteps) {
        return false;
    }
    RenderStep::RenderStepID renderStepID = static_cast<RenderStep::RenderStepID>(tmp);

    if (!stream->readU32(&tmp)) {
        return false;
    }

    UniquePaintParamsID paintParamsID = UniquePaintParamsID::InvalidID();
    if (tmp) {
        SkAutoMalloc storage(4 * tmp);
        if (stream->read(storage.get(), 4 * tmp) != 4 * tmp) {
            return false;
        }

        PaintParamsKey ppk = PaintParamsKey(SkSpan<uint32_t>((uint32_t*) storage.get(), tmp));

        if (!ppk.isSerializable(shaderCodeDictionary)) {
            return false;
        }

        paintParamsID = shaderCodeDictionary->findOrCreate(ppk);
    }

    *pipelineDesc = GraphicsPipelineDesc(renderStepID, paintParamsID);
    return true;
}

[[nodiscard]] bool serialize_attachment_desc(SkWStream* stream,
                                             const AttachmentDesc& attachmentDesc) {

    stream->write8(128);
    stream->write8(0);
    stream->write8(0);
    stream->write8(128);

    if (!TextureInfoPriv::Serialize(stream, attachmentDesc.fTextureInfo)) {
        return false;
    }

    stream->write8(127);
    stream->write8(0);
    stream->write8(0);
    stream->write8(127);

    if (attachmentDesc.fTextureInfo.isValid()) {
        stream->write8(static_cast<uint8_t>(attachmentDesc.fStoreOp));
        stream->write8(static_cast<uint8_t>(attachmentDesc.fLoadOp));
        stream->write8(77);
        stream->write8(77);
//        if (!stream->write32(SkSetFourByteTag(static_cast<uint8_t>(attachmentDesc.fStoreOp),
//                                              static_cast<uint8_t>(attachmentDesc.fLoadOp),
//                                              0, 0))) {
//            return false;
//        }
    }

    stream->write8(126);
    stream->write8(0);
    stream->write8(0);
    stream->write8(126);

    return true;
}

[[nodiscard]] bool deserialize_attachment_desc(const Caps* caps,
                                               SkStream* stream,
                                               AttachmentDesc* attachmentDesc) {

    uint8_t tmp;
    stream->readU8(&tmp);
    stream->readU8(&tmp);
    stream->readU8(&tmp);
    stream->readU8(&tmp);

    if (!TextureInfoPriv::Deserialize(caps, stream, &attachmentDesc->fTextureInfo)) {
        return false;
    }

    stream->readU8(&tmp);
    stream->readU8(&tmp);
    stream->readU8(&tmp);
    stream->readU8(&tmp);

    if (attachmentDesc->fTextureInfo.isValid()) {
//        uint32_t tag;
//        if (!stream->readU32(&tag)) {
//            return false;
//        }

        stream->readU8(&tmp);
        attachmentDesc->fStoreOp = static_cast<StoreOp>(tmp);
        stream->readU8(&tmp);
        attachmentDesc->fLoadOp  = static_cast<LoadOp> (tmp);
        stream->readU8(&tmp);
        stream->readU8(&tmp);
    }

    stream->readU8(&tmp);
    stream->readU8(&tmp);
    stream->readU8(&tmp);
    stream->readU8(&tmp);

    return true;
}

[[nodiscard]] bool serialize_render_pass_desc(SkWStream* stream,
                                              const RenderPassDesc& renderPassDesc) {
    if (!serialize_attachment_desc(stream, renderPassDesc.fColorAttachment)) {
        return false;
    }

    stream->write8(254);
    stream->write8(0);
    stream->write8(0);
    stream->write8(254);

    for (int i = 0; i < 4; ++i) {
        if (!stream->writeScalar(renderPassDesc.fClearColor[i])) {
            return false;
        }
    }

    if (!serialize_attachment_desc(stream, renderPassDesc.fColorResolveAttachment)) {
        return false;
    }

    stream->write8(253);
    stream->write8(0);
    stream->write8(0);
    stream->write8(253);

    if (!serialize_attachment_desc(stream, renderPassDesc.fDepthStencilAttachment)) {
        return false;
    }

    if (!stream->writeScalar(renderPassDesc.fClearDepth)) {
        return false;
    }
    if (!stream->write32(renderPassDesc.fClearStencil)) {
        return false;
    }

    stream->write8(252);
    stream->write8(0);
    stream->write8(0);
    stream->write8(252);

    if (!stream->write32(SkSetFourByteTag(renderPassDesc.fWriteSwizzle[0],
                                          renderPassDesc.fWriteSwizzle[1],
                                          renderPassDesc.fWriteSwizzle[2],
                                          renderPassDesc.fWriteSwizzle[3]))) {
        return false;
    }

    if (!stream->write32(renderPassDesc.fSampleCount)) {
        return false;
    }

    if (!stream->write8(static_cast<uint8_t>(renderPassDesc.fDstReadStrategyIfRequired))) {
        return false;
    }

    return true;
}

[[nodiscard]] bool deserialize_render_pass_desc(const Caps* caps,
                                                SkStream* stream,
                                                RenderPassDesc* renderPassDesc) {
    if (!deserialize_attachment_desc(caps, stream, &renderPassDesc->fColorAttachment)) {
        return false;
    }

    uint8_t tmp;
    stream->readU8(&tmp);
    stream->readU8(&tmp);
    stream->readU8(&tmp);
    stream->readU8(&tmp);

    for (int i = 0; i < 4; ++i) {
        if (!stream->readScalar(&renderPassDesc->fClearColor[i])) {
            return false;
        }
    }

    if (!deserialize_attachment_desc(caps, stream, &renderPassDesc->fColorResolveAttachment)) {
        return false;
    }

    stream->readU8(&tmp);
    stream->readU8(&tmp);
    stream->readU8(&tmp);
    stream->readU8(&tmp);

    if (!deserialize_attachment_desc(caps, stream, &renderPassDesc->fDepthStencilAttachment)) {
        return false;
    }

    if (!stream->readScalar(&renderPassDesc->fClearDepth)) {
        return false;
    }
    if (!stream->readU32(&renderPassDesc->fClearStencil)) {
        return false;
    }

    stream->readU8(&tmp);
    stream->readU8(&tmp);
    stream->readU8(&tmp);
    stream->readU8(&tmp);

    uint32_t tag;
    if (!stream->readU32(&tag)) {
        return false;
    }

    char tmpSwizzle[4] = {
            (char) (0xFF & (tag >> 24)),
            (char) (0xFF & (tag >> 16)),
            (char) (0xFF & (tag >> 8)),
            (char) (0xFF & (tag)),
    };

    renderPassDesc->fWriteSwizzle = Swizzle(tmpSwizzle);

    if (!stream->readU32(&renderPassDesc->fSampleCount)) {
        return false;
    }

    uint8_t tmp8;
    if (!stream->readU8(&tmp8)) {
        return false;
    }

    renderPassDesc->fDstReadStrategyIfRequired = static_cast<DstReadStrategy>(tmp8);

    return true;
}

#define SK_BLOB_END_TAG SkSetFourByteTag('e', 'n', 'd', ' ')

bool SerializePipelineDesc(ShaderCodeDictionary* shaderCodeDictionary,
                           SkWStream* stream,
                           const GraphicsPipelineDesc& pipelineDesc,
                           const RenderPassDesc& renderPassDesc) {

    stream->write(kMagic, sizeof(kMagic));
    stream->write32(kCurrent_Version);

    if (!serialize_graphics_pipeline_desc(shaderCodeDictionary, stream, pipelineDesc)) {
        return false;
    }

    stream->write8(255);
    stream->write8(0);
    stream->write8(0);
    stream->write8(255);

    if (!serialize_render_pass_desc(stream, renderPassDesc)) {
        return false;
    }

    stream->write32(SK_BLOB_END_TAG);
    return true;
}

bool DeserializePipelineDesc(const Caps* caps,
                             ShaderCodeDictionary* shaderCodeDictionary,
                             SkStream* stream,
                             GraphicsPipelineDesc* pipelineDesc,
                             RenderPassDesc* renderPassDesc) {
    SkASSERT(stream);

    if (!stream_is_pipeline(stream)) {
        return false;
    }

    if (!deserialize_graphics_pipeline_desc(shaderCodeDictionary, stream, pipelineDesc)) {
        return false;
    }

    uint8_t tmp;
    stream->readU8(&tmp);
    stream->readU8(&tmp);
    stream->readU8(&tmp);
    stream->readU8(&tmp);

    if (!deserialize_render_pass_desc(caps, stream, renderPassDesc)) {
        return false;
    }

    uint32_t tag;
    if (!stream->readU32(&tag)) {
        return false;
    }

    if (tag != SK_BLOB_END_TAG) {
        return false;
    }

    return true;
}

} // anonymous namespace

sk_sp<SkData> PipelineDescToData(ShaderCodeDictionary* shaderCodeDictionary,
                                 const GraphicsPipelineDesc& pipelineDesc,
                                 const RenderPassDesc& renderPassDesc) {
    SkDynamicMemoryWStream stream;

    if (!SerializePipelineDesc(shaderCodeDictionary,
                               &stream,
                               pipelineDesc, renderPassDesc)) {
        return nullptr;
    }

    return stream.detachAsData();
}

bool DataToPipelineDesc(const Caps* caps,
                        ShaderCodeDictionary* shaderCodeDictionary,
                        const SkData* data,
                        GraphicsPipelineDesc* pipelineDesc,
                        RenderPassDesc* renderPassDesc) {
    if (!data) {
        return false;
    }
    SkMemoryStream stream(data->data(), data->size());

    if (!DeserializePipelineDesc(caps, shaderCodeDictionary, &stream,
                                 pipelineDesc,
                                 renderPassDesc)) {
        return false;
    }

    return true;
}

#if defined(GPU_TEST_UTILS)
void DumpPipelineDesc(const char* label, ShaderCodeDictionary* shaderCodeDictionary,
                      const GraphicsPipelineDesc& pipelineDesc,
                      const RenderPassDesc& renderPassDesc) {
    SkString pipelineStr = pipelineDesc.toString(shaderCodeDictionary);
    SkString renderPassStr = renderPassDesc.toPipelineLabel();
    SkDebugf("%s: %s - %s\n", label, pipelineStr.c_str(), renderPassStr.c_str());
}

bool ComparePipelineDescs(const GraphicsPipelineDesc& a1, const RenderPassDesc& b1,
                          const GraphicsPipelineDesc& a2, const RenderPassDesc& b2) {
    return (a1 == a2) && (b1 == b2);
}
#endif

} // namespace skgpu::graphite
