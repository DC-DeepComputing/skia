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
#include "src/gpu/graphite/RenderPassDesc.h"

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

std::unique_ptr<BlobOStuff> BlobOStuff::MakeFromStream(SkStream* stream,
                                                       ShaderCodeDictionary* scd,
                                                       const Caps* caps) {
    if (!stream) {
        return nullptr;
    }
    return BlobOStuff::Deserialize(stream, scd, caps);
}

std::unique_ptr<BlobOStuff> BlobOStuff::MakeFromData(const SkData* data,
                                                     ShaderCodeDictionary* scd,
                                                     const Caps* caps) {
    if (!data) {
        return nullptr;
    }
    SkMemoryStream stream(data->data(), data->size());
    return MakeFromStream(&stream, scd, caps);
}

std::unique_ptr<BlobOStuff> BlobOStuff::MakeFromData(const void* data, size_t size,
                                                     ShaderCodeDictionary* scd,
                                                     const Caps* caps) {
    if (!data) {
        return nullptr;
    }
    SkMemoryStream stream(data, size);
    return MakeFromStream(&stream, scd, caps);
}

sk_sp<SkData> BlobOStuff::snap1(SharedContext* sharedContext) const {
    SkDynamicMemoryWStream stream;
    if (!this->serialize(sharedContext, &stream)) {
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

static const uint32_t kLoadOpShift = 0;
static const uint32_t kLoadOpMask = 0x3;  // 2 bits for the loadOp

static const uint32_t kStoreOpShift = 2;
static const uint32_t kStoreOpMask = 0x1; // 1 bit for the storeOp

static const uint32_t kDepthStencilFlagsShift = 3;
static const uint32_t kDepthStencilFlagsMask = 0x3; // 2 bits for the depth/stencil flags

#define SK_BLOB_RENDERSTEP_ID_TAG   SkSetFourByteTag('r', 's', 't', 'p')
#define SK_BLOB_PAINTPARAMS_KEY_TAG SkSetFourByteTag('p', 'p', 'k', 't')
#define SK_BLOB_END_TAG SkSetFourByteTag('e', 'n', 'd', ' ')

bool BlobOStuff::serialize(SharedContext* sharedContext, SkWStream* stream) const {
    ShaderCodeDictionary* shaderCodeDictionary = sharedContext->shaderCodeDictionary();

    stream->write(kMagic, sizeof(kMagic));
    stream->write32(skgpu::graphite::BlobOStuff::kCurrent_Version);

    {
        stream->write32(SK_BLOB_RENDERSTEP_ID_TAG);
        stream->write32(static_cast<uint32_t>(fPipelineDesc.renderStepID()));
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

    stream->write32(SkSetFourByteTag(fRenderPassDesc.fWriteSwizzle[0],
                                     fRenderPassDesc.fWriteSwizzle[1],
                                     fRenderPassDesc.fWriteSwizzle[2],
                                     fRenderPassDesc.fWriteSwizzle[3]));

    for (int i = 0; i < 4; ++i) {
        stream->writeScalar(fRenderPassDesc.fClearColor[i]);
    }

    if (fRenderPassDesc.fDepthStencilAttachment.fTextureInfo.isValid()) {

    }

    bool requiresMSAA = fRenderPassDesc.fSampleCount > 1;

    LoadOp loadOp;
    StoreOp storeOp;
    if (fRenderPassDesc.fColorResolveAttachment.fTextureInfo.isValid()) {
        storeOp = fRenderPassDesc.fColorResolveAttachment.fStoreOp;
    } else {
        storeOp = fRenderPassDesc.fColorAttachment.fStoreOp;
        loadOp = fRenderPassDesc.fColorAttachment.fLoadOp;
    }
    SkASSERT(storeOp == StoreOp::kStore);



    stream->write32(SK_BLOB_END_TAG);
    return true;
}

static GraphicsPipelineDesc deserialize_graphics_pipeline_desc(SkStream* stream,
                                                               ShaderCodeDictionary* scd) {
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

    UniquePaintParamsID paintParamsID = UniquePaintParamsID::InvalidID();

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

        PaintParamsKey ppk(SkSpan<uint32_t>((uint32_t*) storage.get(), size));

        paintParamsID = scd->findOrCreate1(&ppk);
    }

    return GraphicsPipelineDesc(renderStepID, paintParamsID);
}

static RenderPassDesc deserialize_render_pass_desc(SkStream* stream, const Caps* caps) {


    TextureInfo targetInfo;
    LoadOp loadOp = LoadOp::kLoad;
    StoreOp storeOp = StoreOp::kStore;
    SkEnumBitMask<DepthStencilFlags> depthStencilFlags(DepthStencilFlags::kNone);
    std::array<float, 4> clearColor;
    bool requiresMSAA = false;

    //--------------------
    uint32_t tag;
    if (!stream->readU32(&tag)) {
        return {};
    }

    char tmp[4] = {
            (char) (0xF & (tag >> 24)),
            (char) (0xF & (tag >> 16)),
            (char) (0xF & (tag >> 8)),
            (char) (0xF & (tag)),
    };

    Swizzle writeSwizzle(tmp);

    //--------------------
    for (int i = 0; i < 4; ++i) {
        if (!stream->readScalar(&clearColor[i])) { return {}; }
    }

    //--------------------
    if (!stream->readU32(&tag)) {
        return {};
    }

    // check loadOp & storeOp validity
    loadOp = static_cast<LoadOp>((tag >> kLoadOpShift) & kLoadOpMask);
    storeOp = static_cast<StoreOp>((tag >> kStoreOpShift) & kStoreOpMask);
    uint32_t bbb = ((tag >> kDepthStencilFlagsShift) & kDepthStencilFlagsMask);

    //--------------------
    return RenderPassDesc::Make(caps,
                                targetInfo,
                                loadOp,
                                storeOp,
                                depthStencilFlags,
                                clearColor,         //*
                                requiresMSAA,
                                writeSwizzle);      //*
}

std::unique_ptr<BlobOStuff> BlobOStuff::Deserialize(SkStream* stream,
                                                    ShaderCodeDictionary* shaderCodeDictionary,
                                                    const Caps* caps) {
    SkASSERT(stream);

    if (!stream_is_blob(stream)) {
        return nullptr;
    }

    GraphicsPipelineDesc pipelineDesc = deserialize_graphics_pipeline_desc(stream,
                                                                           shaderCodeDictionary);
    if (!pipelineDesc.isValid()) {
        return nullptr;
    }

    RenderPassDesc renderPassDesc = deserialize_render_pass_desc(stream, caps);

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
