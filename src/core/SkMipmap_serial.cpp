/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkMipmap.h"

#include "include/core/SkImageGenerator.h"
#include "include/core/SkStream.h"
#include "include/encode/SkPngEncoder.h"
#include "src/core/SkMipmapBuilder.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkWriteBuffer.h"

static sk_sp<SkData> encode_to_data(const SkPixmap& pm) {
    SkDynamicMemoryWStream stream;
    // TODO(kjlubick, scroggo) remove dependency on PNG encoding.
    if (SkPngEncoder::Encode(&stream, pm, SkPngEncoder::Options())) {
        return stream.detachAsData();
    }
    return nullptr;
}

/*  Format
        count_levels:32
        for each level, starting with the biggest (index 0 in our iterator)
            encoded_size:32
            encoded_data (padded)
 */
sk_sp<SkData> SkMipmap::serialize() const {
    const int count = this->countLevels();

    SkBinaryWriteBuffer buffer;
    buffer.write32(count);
    for (int i = 0; i < count; ++i) {
        Level level;
        if (this->getLevel(i, &level)) {
            buffer.writeDataAsByteArray(encode_to_data(level.fPixmap).get());
        } else {
            return nullptr;
        }
    }
    return buffer.snapshotAsData();
}

bool SkMipmap::Deserialize(SkMipmapBuilder* builder, const void* data, size_t length) {
    SkReadBuffer buffer(data, length);

    int count = buffer.read32();
    if (builder->countLevels() != count) {
        return false;
    }
    for (int i = 0; i < count; ++i) {
        size_t size = buffer.read32();
        const void* ptr = buffer.skip(size);
        if (!ptr) {
            return false;
        }
        auto gen = SkImageGenerator::MakeFromEncoded(
                             SkData::MakeWithProc(ptr, size, nullptr, nullptr));
        if (!gen) {
            return false;
        }

        SkPixmap pm = builder->level(i);
        if (gen->getInfo().dimensions() != pm.dimensions()) {
            return false;
        }
        if (!gen->getPixels(pm)) {
            return false;
        }
    }
    return buffer.isValid();
}
