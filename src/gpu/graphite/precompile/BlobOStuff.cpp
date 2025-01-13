/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/gpu/graphite/precompile/BlobOStuff.h"

#include "include/core/SkStream.h"
#include "src/core/SkStreamPriv.h"

namespace {


bool stream_is_blob(SkStream* stream) {
    static const char kMagic[] = { 's', 'k', 'i', 'a', 'b', 'l', 'o', 'b' };

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

    if (version != BlobOStuff::kCurrent_Version) {
        return false;
    }

    return true;
}

} // anonymous namespace


namespace skgpu::graphite {


BlobInfo BlobOStuff::createHeader() const {
    BlobInfo info;
    // Copy magic bytes at the beginning of the header
    static_assert(sizeof(kMagic) == 8, "");
    static_assert(sizeof(kMagic) == sizeof(info.fMagic), "");
    memcpy(info.fMagic, kMagic, sizeof(kMagic));

    // Set picture info after magic bytes in the header
    info.setVersion(SkPicturePriv::kCurrent_Version);
    return info;
}

BlobOStuff* BlobOStuff::MakeFromStream(SkStream* stream) {

    if (!stream_is_blob(stream)) {
        return nullptr;
    }

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

} // namespace skgpu::graphite
