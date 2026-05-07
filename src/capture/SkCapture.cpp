/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/capture/SkCapture.h"

#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkData.h"
#include "include/core/SkFourByteTag.h"
#include "include/core/SkImage.h"
#include "include/core/SkPicture.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSerialProcs.h"
#include "include/core/SkStream.h"
#include "include/private/base/SkTArray.h"
#include "src/capture/SkCaptureManager.h"
#include "src/image/SkImage_Base.h"

constexpr SkFourByteTag kMagic1 = SkSetFourByteTag('s', 'k', 'i', 'a');
constexpr SkFourByteTag kMagic2 = SkSetFourByteTag('c', 'a', 'p', 't');

sk_sp<SkCapture> SkCapture::MakeFromData(sk_sp<const SkData> data) {
    if (!data) {
        return nullptr;
    }

    // 1. Setup Stream
    SkMemoryStream stream(data->data(), data->size());

    // 2. Read and Validate Magic Number
    uint32_t magic1;
    uint32_t magic2;
    if (!stream.readU32(&magic1) || !stream.readU32(&magic2) || magic1 != kMagic1 ||
        magic2 != kMagic2) {
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

    // 5. Loop and Read Each Picture's raw data
    for (uint32_t i = 0; i < pictureCount; ++i) {
        uint32_t pictureDataSize;
        if (!stream.readU32(&pictureDataSize)) {
            SkDebugf("Failed to read picture data size for picture %u.\n", i);
            return nullptr;
        }

        // Read the picture data into an SkData object
        sk_sp<SkData> pictureData = SkData::MakeUninitialized(pictureDataSize);

        // Check if allocation failed *or* if the stream read failed.
        if (!pictureData ||
            stream.read(pictureData->writable_data(), pictureDataSize) != pictureDataSize) {
            SkDebugf("Failed to read picture data for picture %u or allocation failed.\n", i);
            return nullptr;
        }

        capture->fPictureDatas.emplace_back(std::move(pictureData));
        capture->fPictures.emplace_back(nullptr);
    }

    SkDebugf("Successfully read %d picture datas into SkCapture.\n", capture->fPictureDatas.size());
    return capture;
}

sk_sp<SkCapture> SkCapture::MakeFromPictures(skia_private::TArray<sk_sp<SkPicture>> pictures) {
    auto capture = sk_make_sp<SkCapture>();

    capture->fMetadata = {SkCapture::kVersion, static_cast<uint32_t>(pictures.size())};
    capture->fPictures = pictures;
    capture->fPictureDatas.resize(pictures.size());
    return capture;
}

sk_sp<SkPicture> SkCapture::getPicture(int i) {
    if (i >= 0 && i < fPictures.size()) {
        if (fPictures[i]) {
            return fPictures[i];
        }
        if (i < fPictureDatas.size() && fPictureDatas[i]) {
            // Deserialize the SkPicture from its raw data on demand
            SkDeserialProcs procs;
            procs.fImageDataProc = SkCapture::deserializeImageProc;
            procs.fImageCtx = this;
            fPictures[i] = SkPicture::MakeFromData(fPictureDatas[i].get(), &procs);
            return fPictures[i];
        }
    }
    return nullptr;
}

SkCapture::Metadata SkCapture::getMetadata() const { return fMetadata; }

sk_sp<SkData> SkCapture::serializeCapture() {
    SkDynamicMemoryWStream stream;

    stream.write32(kMagic1);
    stream.write32(kMagic2);
    stream.write32(SkCapture::kVersion);

    // Number of pictures
    stream.write32(fPictures.size());

    // TODO (b/412351769): Write metadata on each picture and assosiated canvas. This will be needed
    // when we have multiple SkPictures per canvas and we want to track which ones are drawn into
    // each other.

    for (int i = 0; i < fPictures.size(); ++i) {
        sk_sp<SkPicture> picture = this->getPicture(i);
        if (!picture) {
            // Should we write an empty block or fail?
            stream.write32(0);
            continue;
        }
        SkDynamicMemoryWStream pictureStream;
        SkSerialProcs procs;
        procs.fImageProc = SkCapture::serializeImageProc;
        procs.fImageCtx = this;
        picture->serialize(&pictureStream, &procs);
        sk_sp<SkData> pictureData = pictureStream.detachAsData();

        // Write size and then data
        stream.write32(pictureData->size());
        stream.write(pictureData->data(), pictureData->size());
    }

    auto data = stream.detachAsData();
    SkDebugf("Wrote %d pictures to SkData block.\n", fPictures.size());
    return data;
}

// TODO(b/412351769): When serializing our SkPictures, the images that are drawn through drawImage
// and similar functions will also need to be serialized. Instead of naively encoding all images as
// PNGs, we want the images that refer to content created from an SkSurface to point to its
// corresponding SkPicture. This means that we need to create a context for the proc to track these
// contentIDs.
SkSerialReturnType SkCapture::serializeImageProc(SkImage* img, void* ctx) {
    ImageMetadata imgMD = {
            .contentID = as_IB(img)->getContentID(),
            .derivedSurfaceID = as_IB(img)->getDerivedSurfaceID(),
    };

    return SkData::MakeWithCopy(&imgMD, sizeof(ImageMetadata));
}

sk_sp<SkImage> SkCapture::deserializeImageProc(sk_sp<SkData> data,
                                               std::optional<SkAlphaType> type,
                                               void* ctx) {

    if (!ctx || data->size() != sizeof(ImageMetadata)) return nullptr;

    SkCapture* capture = static_cast<SkCapture*>(ctx);
    const ImageMetadata* imgMD = reinterpret_cast<const ImageMetadata*>(data->data());

    int pictureIndex = static_cast<int>(imgMD->contentID);

    sk_sp<SkPicture> picture = capture->getPicture(pictureIndex);
    SkISize dimensions = {(int)picture->cullRect().width(), (int)picture->cullRect().height()};

    if (picture) {
        sk_sp<SkImage> image =
                SkImages::DeferredFromPicture(std::move(picture),
                                              dimensions,
                                              nullptr,
                                              nullptr,
                                              SkImages::BitDepth::kU8,
                                              SkColorSpace::MakeSRGB());

        as_IB(image.get())->setContentID(SkContentID(imgMD->contentID));
        as_IB(image.get())->setDerivedSurfaceID(imgMD->derivedSurfaceID);

        return image;
    }

    return nullptr;
}
