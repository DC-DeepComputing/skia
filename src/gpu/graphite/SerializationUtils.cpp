/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/SerializationUtils.h"

#include "include/core/SkFourByteTag.h"
#include "include/core/SkStream.h"
#include "src/base/SkAutoMalloc.h"
#include "src/gpu/SwizzlePriv.h"
#include "src/gpu/graphite/Caps.h"
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

constexpr bool is_valid_samplecount(uint32_t sampleCount) {
    return SkIsPow2(sampleCount) && sampleCount >= 1 && sampleCount <= 16;
}

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

    const SkSpan<const int32_t> keySpan = key.data();

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

    UniquePaintParamsID paintParamsID = UniquePaintParamsID::Invalid();
    if (tmp) {
        SkAutoMalloc storage(4 * tmp);
        if (stream->read(storage.get(), 4 * tmp) != 4 * tmp) {
            return false;
        }

        PaintParamsKey ppk = PaintParamsKey(SkSpan((int32_t*) storage.get(), tmp));

        if (!ppk.isSerializable(shaderCodeDictionary)) {
            return false;
        }

        paintParamsID = shaderCodeDictionary->findOrCreate(ppk);
    }

    *pipelineDesc = GraphicsPipelineDesc(renderStepID, paintParamsID);
    return true;
}

// check a single block and, recursively, all its children
[[nodiscard]] bool block_contains_ext_format(const ShaderCodeDictionary* dict,
                                             SkStream* stream) {
    uint32_t codeSnippetID;
    if (!stream->readU32(&codeSnippetID)) {
        return false;
    }

    auto entry = dict->getEntry(codeSnippetID);
    if (!entry) {
        return false;
    }

    if (entry->storesSamplerDescData()) {
        int32_t dataLengthEncoded;
        if (!stream->readS32(&dataLengthEncoded)) {
            return false;
        }

        int32_t dataLength = PaintParamsKey::EncodeDataSize(dataLengthEncoded);
        SkASSERT(dataLength >= 0 && dataLength <= PaintParamsKey::kEmbeddedDataSizeLimit);

        if (dataLength == 3) {
            return true;
        }

        if (!stream->skip(dataLength)) {
            return false;
        }
    }

    for (int i = 0; i < entry->fNumChildren; ++i) {
        if (block_contains_ext_format(dict, stream)) {
            return true;
        }
    }

    return false;
}

[[nodiscard]] bool graphics_pipeline_desc_contains_ext_format(
        const ShaderCodeDictionary* shaderCodeDictionary,
        SkMemoryStream* stream) {
    uint32_t renderStepID;
    if (!stream->readU32(&renderStepID)) {
        return false;
    }

    uint32_t keySize;
    if (!stream->readU32(&keySize)) {
        return false;
    }

    if (keySize) {
        uint32_t endOfKey = static_cast<uint32_t>(stream->getPosition()) + 4 * keySize;
        if (endOfKey > stream->getLength()) {
            return false;
        }

        while (stream->getPosition() < endOfKey) {
            if (block_contains_ext_format(shaderCodeDictionary, stream)) {
                return true;
            }
        }
    }

    return false;
}

[[nodiscard]] bool serialize_attachment_desc(SkWStream* stream,
                                             const AttachmentDesc& attachmentDesc) {
    uint32_t tag = attachmentDesc.fFormat == TextureFormat::kUnsupported
            ? SkSetFourByteTag(static_cast<uint8_t>(TextureFormat::kUnsupported), 0, 0, 1)
            : SkSetFourByteTag(static_cast<uint8_t>(attachmentDesc.fFormat),
                               static_cast<uint8_t>(attachmentDesc.fLoadOp),
                               static_cast<uint8_t>(attachmentDesc.fStoreOp),
                               static_cast<uint8_t>(attachmentDesc.fSampleCount));
    return stream->write32(tag);
}

[[nodiscard]] bool deserialize_attachment_desc(SkStream* stream,
                                               AttachmentDesc* attachmentDesc) {
    uint32_t tag;
    if (!stream->readU32(&tag)) {
        return false;
    }

    uint8_t format      = static_cast<uint8_t>((tag >> 24) & 0xFF);
    uint8_t loadOp      = static_cast<uint8_t>((tag >> 16) & 0xFF);
    uint8_t storeOp     = static_cast<uint8_t>((tag >>  8) & 0xFF);
    uint8_t sampleCount = static_cast<uint8_t>((tag >>  0) & 0xFF);

    if (format >= kTextureFormatCount) {
        return false;
    }
    if (loadOp >= kLoadOpCount) {
        return false;
    }
    if (storeOp >= kStoreOpCount) {
        return false;
    }
    if (!is_valid_samplecount(sampleCount)) {
        return false;
    }

    *attachmentDesc = {static_cast<TextureFormat>(format),
                       static_cast<LoadOp>(loadOp),
                       static_cast<StoreOp>(storeOp),
                       static_cast<SampleCount>(sampleCount)};

    return true;
}

[[nodiscard]] bool serialize_render_pass_desc(SkWStream* stream,
                                              const RenderPassDesc& renderPassDesc) {
    if (!serialize_attachment_desc(stream, renderPassDesc.fColorAttachment)) {
        return false;
    }
    if (!serialize_attachment_desc(stream, renderPassDesc.fColorResolveAttachment)) {
        return false;
    }
    if (!serialize_attachment_desc(stream, renderPassDesc.fDepthStencilAttachment)) {
        return false;
    }

    if (!stream->write16(renderPassDesc.fWriteSwizzle.asKey())) {
        return false;
    }
    if (!stream->write8(static_cast<uint8_t>(renderPassDesc.fSampleCount))) {
        return false;
    }

    // Omit clear values for the various attachments as they do not effect structure.
    // Omit fDstReadStrategy from the serialization because it is not a part of RenderPassDesc
    // keys and does not impact pipeline creation. When deserializing, the strategy can be
    // obtained via caps->getDstReadStrategy().

    return true;
}

[[nodiscard]] bool deserialize_render_pass_desc(const Caps* caps,
                                                SkStream* stream,
                                                RenderPassDesc* renderPassDesc) {
    if (!deserialize_attachment_desc(stream, &renderPassDesc->fColorAttachment)) {
        return false;
    }
    if (!deserialize_attachment_desc(stream, &renderPassDesc->fColorResolveAttachment)) {
        return false;
    }
    if (!deserialize_attachment_desc(stream, &renderPassDesc->fDepthStencilAttachment)) {
        return false;
    }

    uint16_t swizzle;
    if (!stream->readU16(&swizzle)) {
        return false;
    }
    renderPassDesc->fWriteSwizzle = SwizzleCtorAccessor::Make(swizzle);

    uint8_t sampleCount;
    if (!stream->readU8(&sampleCount) || !is_valid_samplecount(sampleCount)) {
        return false;
    }
    renderPassDesc->fSampleCount = static_cast<SampleCount>(sampleCount);

    // RenderPassDesc dst read strategy is not serialized as it is not something we key on and does
    // not impact pipeline creation. When deserializing, simply query Caps again for a
    // DstReadStrategy. Leave clear color/depth/stencil as their default values.
    renderPassDesc->fDstReadStrategy = caps->getDstReadStrategy();

    return true;
}

[[nodiscard]] bool render_pass_contains_ext_format(SkMemoryStream* stream) {
    AttachmentDesc tmp;
    if (!deserialize_attachment_desc(stream, &tmp)) {
        return false;
    }
    if (tmp.fFormat == TextureFormat::kExternal) {
        return true;
    }

    if (!deserialize_attachment_desc(stream, &tmp)) {
        return false;
    }
    if (tmp.fFormat == TextureFormat::kExternal) {
        return true;
    }

    if (!deserialize_attachment_desc(stream, &tmp)) {
        return false;
    }
    if (tmp.fFormat == TextureFormat::kExternal) {
        return true;
    }

    uint16_t swizzle;
    if (!stream->readU16(&swizzle)) {
        return false;
    }

    uint8_t sampleCount;
    if (!stream->readU8(&sampleCount) || !is_valid_samplecount(sampleCount)) {
        return false;
    }

    return false;
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

bool serialized_key_contains_ext_format(const ShaderCodeDictionary* shaderCodeDictionary,
                                        SkMemoryStream* stream) {
    SkASSERT(stream);

    if (!stream_is_pipeline(stream)) {
        return false;
    }

    if (graphics_pipeline_desc_contains_ext_format(shaderCodeDictionary, stream)) {
        return true;
    }

    // It seems implausible that the RenderPass would contain an external format.
    // Just return 'false' from here?
    if (render_pass_contains_ext_format(stream)) {
        return true;
    }

    return false;
}

} // anonymous namespace

sk_sp<SkData> PipelineDescToData(const Caps* caps,
                                 ShaderCodeDictionary* shaderCodeDictionary,
                                 const GraphicsPipelineDesc& pipelineDesc,
                                 const RenderPassDesc& renderPassDesc) {
    SkDynamicMemoryWStream stream;

    if (!SerializePipelineDesc(shaderCodeDictionary,
                               &stream,
                               pipelineDesc, renderPassDesc)) {
        return nullptr;
    }

    sk_sp<SkData> data = stream.detachAsData();

#if 0  // Enable this to thoroughly test Pipeline serialization
    {
        // Check that the PipelineDesc round trips through serialization
        GraphicsPipelineDesc readBackPipelineDesc;
        RenderPassDesc readBackRenderPassDesc;

        SkAssertResult(DataToPipelineDesc(caps,
                                          shaderCodeDictionary,
                                          data.get(),
                                          &readBackPipelineDesc,
                                          &readBackRenderPassDesc));

        DumpPipelineDesc("invokeCallback - original", shaderCodeDictionary,
                         pipelineDesc, renderPassDesc);

        DumpPipelineDesc("invokeCallback - readback", shaderCodeDictionary,
              readBackPipelineDesc, readBackRenderPassDesc);

        SkASSERT(ComparePipelineDescs(pipelineDesc, renderPassDesc,
                                      readBackPipelineDesc, readBackRenderPassDesc));
    }
#endif

    return data;
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

    if (!DeserializePipelineDesc(caps,
                                 shaderCodeDictionary,
                                 &stream,
                                 pipelineDesc,
                                 renderPassDesc)) {
        return false;
    }

    return true;
}

bool DataContainsExternalFormat(const ShaderCodeDictionary* shaderCodeDictionary,
                                const SkData* data) {
    if (!data) {
        return false;
    }
    SkMemoryStream stream(data->data(), data->size());

    return serialized_key_contains_ext_format(shaderCodeDictionary, &stream);
}

#if defined(GPU_TEST_UTILS)
void DumpPipelineDesc(const Caps* caps,
                      const char* label,
                      ShaderCodeDictionary* shaderCodeDictionary,
                      const GraphicsPipelineDesc& pipelineDesc,
                      const RenderPassDesc& renderPassDesc) {
    SkString pipelineStr = pipelineDesc.toString(caps, shaderCodeDictionary);
    SkString renderPassStr = renderPassDesc.toPipelineLabel();
    SkDebugf("%s: %s - %s\n", label, pipelineStr.c_str(), renderPassStr.c_str());
}

bool ComparePipelineDescs(const GraphicsPipelineDesc& a1, const RenderPassDesc& b1,
                          const GraphicsPipelineDesc& a2, const RenderPassDesc& b2) {
    return (a1 == a2) && (b1 == b2);
}
#endif

} // namespace skgpu::graphite
