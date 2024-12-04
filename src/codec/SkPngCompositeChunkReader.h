/*
 * Copyright 2024 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkPngCompositeChunkReader_DEFINED
#define SkPngCompositeChunkReader_DEFINED

#include "include/codec/SkPngChunkReader.h"
#include "include/core/SkStream.h"

/**
 * Reader for PNG chunks that handles the composite of:
 * 1. Chunks currently unknown to the PNG spec, but are necessary to correctly
 *    decode certain images. For example, images that contain gainmap chunks.
 * 2. Client-provided callbacks that may listen to other unknown chunks.
 *
 * The PNG api can only register one callback for unknown chunks at a time,
 * hence this wrapper class.
 */
class SkPngCompositeChunkReader : public SkPngChunkReader {
public:
    SkPngCompositeChunkReader(SkPngChunkReader* chunkReader)
            : fChunkReader(SkSafeRef(chunkReader)) {}

    bool readChunk(const char tag[], const void* data, size_t length) override {
        if (fChunkReader && !fChunkReader->readChunk(tag, data, length)) {
            return false;
        }

        if (strcmp("gmAP", tag) == 0) {
            SkMemoryStream stream(data, length);
            sk_sp<SkData> streamData = stream.getData();
            SkGainmapInfo info;
            if (SkGainmapInfo::Parse(streamData.get(), info)) {
                fGainmapInfo.emplace(std::move(info));
            }
        } else if (strcmp("gdAT", tag) == 0) {
            fGainmapStream = SkMemoryStream::MakeCopy(data, length);
        }

        return true;
    }

    std::unique_ptr<SkStream> takeGaimapStream() { return std::move(fGainmapStream); }

    std::optional<SkGainmapInfo> getGainmapInfo() const { return fGainmapInfo; }

private:
    sk_sp<SkPngChunkReader> fChunkReader = nullptr;
    std::optional<SkGainmapInfo> fGainmapInfo;
    std::unique_ptr<SkStream> fGainmapStream = nullptr;
};

#endif  // SkPngCompositeChunkReader_DEFINED
