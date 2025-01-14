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

#include "src/gpu/graphite/PaintParamsKey.h"

namespace {

static const char kMagic[] = { 's', 'k', 'i', 'a', 'b', 'l', 'o', 'b' };

bool stream_is_blob(SkStream* stream) {

    if (!stream) {
        return false;
    }

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

    if (!stream_is_blob(stream)) {
        return nullptr;
    }

    return nullptr;
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

#define SK_BLOB_FOO_TAG SkSetFourByteTag('f', 'o', 'o', ' ')
#define SK_BLOB_BAR_TAG SkSetFourByteTag('b', 'a', 'r', ' ')
#define SK_BLOB_END_TAG SkSetFourByteTag('e', 'n', 'd', ' ')

void BlobOStuff::serialize(Context* context, SkWStream* stream) {
    ShaderCodeDictionary* shaderCodeDictionary = context->priv().shaderCodeDictionary9();

    stream->write(kMagic, sizeof(kMagic));
    stream->write32(skgpu::graphite::BlobOStuff::kCurrent_Version);

    stream->write32(SK_BLOB_FOO_TAG);

    stream->write32(SK_BLOB_BAR_TAG);
    PaintParamsKey key = shaderCodeDictionary->lookup(fPipelineDesc.paintParamsID());

    stream->write32(SK_BLOB_END_TAG);
}

} // namespace skgpu::graphite
