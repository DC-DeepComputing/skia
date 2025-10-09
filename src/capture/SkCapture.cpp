/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/capture/SkCapture.h"

#include "include/core/SkData.h"
#include "include/core/SkPicture.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSerialProcs.h"
#include "include/core/SkStream.h"
#include "src/capture/SkCaptureManager.h"

const char SkCapture::kMagic[] = { 's', 'k', 'i', 'a', 'c', 'a', 'p', 't' };

sk_sp<SkCapture> SkCapture::MakeFromData(sk_sp<SkData> data) {
    if (!data) {
        return nullptr;
    }

    // 1. Setup Stream
    SkMemoryStream stream(data->data(), data->size());

    // 2. Read and Validate Magic Number
    char magic[sizeof(kMagic)];
    if (stream.read(magic, sizeof(kMagic)) != sizeof(kMagic) ||
        memcmp(magic, kMagic, sizeof(kMagic)) != 0) {
        SkDebugf("Invalid magic number for SkCapture.\n");
        return nullptr;
    }

    // 3. Read and Validate Version
    uint32_t version;
    if (!stream.readU32(&version) || version != kVersion) {
        SkDebugf("Unsupported SkCapture version: %u.\n", version);
        return nullptr;
    }

    // 4. Read Picture Count
    uint32_t pictureCount;
    if (!stream.readU32(&pictureCount)) {
        SkDebugf("Failed to read picture count.\n");
        return nullptr;
    }

    auto capture = sk_make_sp<SkCapture>();
    capture->fMetadata = {version, pictureCount};

    // 5. Loop and Deserialize Each Picture
    for (uint32_t i = 0; i < pictureCount; ++i) {
        // Read size (uint64_t)
        uint64_t pictureDataSize64;
        if (!stream.readU64(&pictureDataSize64)) {
            SkDebugf("Failed to read picture data size for picture %u.\n", i);
            return nullptr;
        }

        // TODO: is this cast safe to assume
        size_t pictureDataSize = static_cast<size_t>(pictureDataSize64);

        // Read the picture data into an SkData object
        sk_sp<SkData> pictureData = SkData::MakeUninitialized(pictureDataSize);

        // Check if allocation failed *or* if the stream read failed.
        if (!pictureData || stream.read(pictureData->writable_data(), pictureDataSize) != pictureDataSize) {
            SkDebugf("Failed to read picture data for picture %u or allocation failed.\n", i);
            return nullptr;
        }

        // Deserialize the SkPicture from its raw data
        SkDeserialProcs procs;
        procs.fImageDataProc = SkCaptureManager::deserializeImageProc;
        sk_sp<SkPicture> picture = SkPicture::MakeFromData(pictureData.get(), &procs);
        if (!picture) {
            SkDebugf("Failed to deserialize SkPicture for picture %u.\n", i);
            return nullptr;
        }

        // Add the deserialized picture to the SkCapture object
        capture->fPictures.emplace_back(std::move(picture));
    }

    SkDebugf("Successfully read %zu pictures into SkCapture.\n", capture->fPictures.size());
    return capture;
}
