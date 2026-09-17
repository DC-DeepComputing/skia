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
#include "include/private/SkLog.h"
#include "include/private/SkTArray.h"
#include "src/capture/SkCaptureManager.h"
#include "src/image/SkImage_Base.h"

constexpr SkFourByteTag kMagic1  = SkSetFourByteTag('s','k','i','a');
constexpr SkFourByteTag kMagic2  = SkSetFourByteTag('c','a','p','t');

sk_sp<SkCapture> SkCapture::MakeFromData(sk_sp<const SkData> data) {
    if (!data) {
        return nullptr;
    }

    // 1. Setup Stream
    SkMemoryStream stream(data->data(), data->size());

    // 2. Read and Validate Magic Number
    uint32_t magic1;
    uint32_t magic2;
    if (!stream.readU32(&magic1) || !stream.readU32(&magic2) ||
        magic1 != kMagic1 || magic2 != kMagic2) {
        SKIA_LOG_E("Invalid magic number for SkCapture.");
        return nullptr;
    }

    // 3. Read and Validate Version
    uint32_t version;
    if (!stream.readU32(&version) || version != kVersion) {
        SKIA_LOG_E("Unsupported SkCapture version: %u.", version);
        return nullptr;
    }

    // 4. Read Asset and RecordingCapture Counts
    uint32_t assetCount;
    uint32_t recordingCaptureCount;
    if (!stream.readU32(&assetCount) || !stream.readU32(&recordingCaptureCount)) {
        SKIA_LOG_E("Failed to read asset or recording capture counts.");
        return nullptr;
    }

    auto capture = sk_make_sp<SkCapture>();
    capture->fMetadata = {version, assetCount, recordingCaptureCount};

    // 5. Loop and Deserialize Each Asset
    for (uint32_t i = 0; i < assetCount; ++i) {
        uint32_t pixelStorageID;
        uint32_t contentID;
        uint32_t typeVal;
        if (!stream.readU32(&pixelStorageID) || !stream.readU32(&contentID) || !stream.readU32(&typeVal)) {
            SKIA_LOG_E("Failed to read asset header for asset %u.", i);
            return nullptr;
        }

        SkCaptureAsset::AssetType assetType = static_cast<SkCaptureAsset::AssetType>(typeVal);
        if (assetType == SkCaptureAsset::AssetType::kPicture) {
            uint32_t pictureDataSize;
            if (!stream.readU32(&pictureDataSize)) {
                SKIA_LOG_E("Failed to read picture data size for asset %u.", i);
                return nullptr;
            }

            sk_sp<SkData> pictureData = SkData::MakeUninitialized(pictureDataSize);
            if (!pictureData || stream.read(pictureData->writable_data(),
                                            pictureDataSize) != pictureDataSize) {
                SKIA_LOG_E("Failed to read picture data for asset %u or allocation failed.", i);
                return nullptr;
            }

            SkDeserialProcs procs;
            procs.fImageDataProc = SkCapture::deserializeImageProc;
            procs.fImageCtx = capture.get();
            sk_sp<SkPicture> picture = SkPicture::MakeFromData(pictureData.get(), &procs);
            if (!picture) {
                SKIA_LOG_E("Failed to deserialize SkPicture for asset %u.", i);
                return nullptr;
            }

            auto asset = sk_make_sp<SkCaptureAsset>();
            asset->fPixelStorageID = pixelStorageID;
            asset->fContentID = contentID;
            asset->fType = assetType;
            asset->fPicture = std::move(picture);
            capture->fAssets.push_back(std::move(asset));
        } else if (assetType == SkCaptureAsset::AssetType::kDerivedStorage) {
            uint32_t parentPixelStorageID;
            uint32_t parentContentID;
            if (!stream.readU32(&parentPixelStorageID) || !stream.readU32(&parentContentID)) {
                SKIA_LOG_E("Failed to read origin parent IDs for asset %u.", i);
                return nullptr;
            }
            auto asset = sk_make_sp<SkCaptureAsset>();
            asset->fPixelStorageID = pixelStorageID;
            asset->fContentID = contentID;
            asset->fType = assetType;
            asset->fOrigin = SkPixelStorage::PixelStorageOrigin::Copy(parentPixelStorageID,
                                                                       parentContentID);
            capture->fAssets.push_back(std::move(asset));
        }
    }

    // 6. Loop and Deserialize Each RecordingCapture
    for (uint32_t i = 0; i < recordingCaptureCount; ++i) {
        RecordingCapture rec;
        uint32_t taskCount;
        if (!stream.readU32(&taskCount)) {
            SKIA_LOG_E("Failed to read task count for recording capture %u.", i);
            return nullptr;
        }
        for (uint32_t j = 0; j < taskCount; ++j) {
            uint32_t assetIdx;
            if (!stream.readU32(&assetIdx)) {
                SKIA_LOG_E("Failed to read task %u for recording capture %u.", j, i);
                return nullptr;
            }
            if (assetIdx >= static_cast<uint32_t>(capture->fAssets.size())) {
                SKIA_LOG_E("Out-of-bounds asset index %u parsed for task %u inside recording %u.",
                           assetIdx, j, i);
                return nullptr;
            }
            rec.fDrawTasks.push_back({assetIdx});
        }
        capture->fTimeline.push_back(std::move(rec));
    }

    SKIA_LOG_I("Successfully read %d assets and %d recording captures into SkCapture.",
               (int)capture->fAssets.size(), (int)capture->fTimeline.size());
    return capture;
}

sk_sp<SkCapture> SkCapture::MakeEmpty() {
    auto capture = sk_make_sp<SkCapture>();
    capture->fMetadata = {SkCapture::kVersion, 0, 0};
    return capture;
}

void SkCapture::addAsset(sk_sp<SkCaptureAsset> asset) {
    if (!asset) return;
    if (asset->fPicture || asset->fType == SkCaptureAsset::AssetType::kDerivedStorage) {
        fAssets.push_back(std::move(asset));
        fMetadata.numAssets = fAssets.size();
    }
}

void SkCapture::addAsset(sk_sp<SkPicture> picture, uint32_t pixelStorageID, uint32_t contentID) {
    if (picture) {
        auto asset = sk_make_sp<SkCaptureAsset>();
        asset->fPixelStorageID = pixelStorageID;
        asset->fContentID = contentID;
        asset->fType = SkCaptureAsset::AssetType::kPicture;
        asset->fPicture = std::move(picture);
        fAssets.push_back(std::move(asset));
        fMetadata.numAssets = fAssets.size();
    }
}

void SkCapture::addRecordingCapture(RecordingCapture rec) {
    fTimeline.push_back(std::move(rec));
    fMetadata.numRecordingCaptures = fTimeline.size();
}

sk_sp<SkPicture> SkCapture::getAsset(int i) const {
    if (i >= 0 && i < fAssets.size()) {
        return fAssets[i] ? fAssets[i]->fPicture : nullptr;
    }
    return nullptr;
}

const SkCaptureAsset* SkCapture::getAssetRecord(int i) const {
    if (i >= 0 && i < fAssets.size()) {
        return fAssets[i].get();
    }
    return nullptr;
}

const SkCapture::RecordingCapture* SkCapture::getRecordingCapture(int i) const {
    if (i >= 0 && i < fTimeline.size()) {
        return &fTimeline[i];
    }
    return nullptr;
}

SkCapture::Metadata SkCapture::getMetadata() const {
    return fMetadata;
}

sk_sp<SkData> SkCapture::serializeCapture() {
    SkDynamicMemoryWStream stream;

    stream.write32(kMagic1);
    stream.write32(kMagic2);
    stream.write32(SkCapture::kVersion);

    // Number of assets and recording captures
    stream.write32(fAssets.size());
    stream.write32(fTimeline.size());

    // 1. Assets Section: Serialized Pictures wrapped with header
    for (const auto& asset : fAssets) {
        if (!asset) continue;
        stream.write32(asset->fPixelStorageID);
        stream.write32(asset->fContentID);
        stream.write32(static_cast<uint32_t>(asset->fType));

        if (asset->fType == SkCaptureAsset::AssetType::kPicture) {
            SkDynamicMemoryWStream pictureStream;
            SkSerialProcs procs;
            procs.fImageProc = SkCapture::serializeImageProc;
            asset->fPicture->serialize(&pictureStream, &procs);
            sk_sp<SkData> pictureData = pictureStream.detachAsData();

            stream.write32(pictureData->size());
            stream.write(pictureData->data(), pictureData->size());
        } else if (asset->fType == SkCaptureAsset::AssetType::kDerivedStorage) {
            stream.write32(asset->fOrigin.fParentPixelStorageID);
            stream.write32(asset->fOrigin.fParentContentID);
        }
    }

    // 2. Timeline Section: RecordingCaptures
    for (const auto& rec : fTimeline) {
        // Write DrawTasks count
        stream.write32(rec.fDrawTasks.size());
        for (const auto& task : rec.fDrawTasks) {
            stream.write32(task.fAssetIndex);
        }
    }

    auto data = stream.detachAsData();
    SKIA_LOG_I("Wrote %d assets and %d recording captures to SkData block.",
               (int)fAssets.size(), (int)fTimeline.size());
    return data;
}

SkSerialReturnType SkCapture::serializeImageProc(SkImage* img, void* ctx) {
    if (!img) {
        return nullptr;
    }
    uint32_t storageID = 0;
    uint32_t contentID = as_IB(img)->getContentId();
    auto storages = as_IB(img)->getPixelStorages();
    for (const auto& s : storages) {
        if (s && (s->getPixelStorageId() != 0 || s->origin().hasParent())) {
            if (s->origin().hasParent()) {
                storageID = s->origin().fParentPixelStorageID;
                contentID = s->origin().fParentContentID;
            } else {
                storageID = s->getPixelStorageId();
            }
            break;
        }
    }

    ImageRefHeader header{storageID, contentID};
    return SkData::MakeWithCopy(&header, sizeof(ImageRefHeader));
}

sk_sp<SkImage> SkCapture::deserializeImageProc(sk_sp<SkData> data,
                                               std::optional<SkAlphaType>,
                                               void* ctx) {
    if (!data || data->size() < sizeof(ImageRefHeader)) {
        SKIA_LOG_E("deserializeImageProc: invalid data size.");
        return nullptr;
    }

    ImageRefHeader header;
    memcpy(&header, data->data(), sizeof(ImageRefHeader));

    auto* capture = static_cast<SkCapture*>(ctx);
    if (capture) {
        uint32_t targetStorageID = header.fPixelStorageID;
        uint32_t targetContentID = header.fContentID;
        // Hop N times to find an asset with the content ID we're looking for. Safeguard to avoid
        // infinite loops.
        for (int hop = 0; hop < 4; ++hop) {
            const SkCaptureAsset* targetAsset = nullptr;
            for (uint32_t i = 0; i < capture->getMetadata().numAssets; ++i) {
                const auto* asset = capture->getAssetRecord(i);
                if (asset && asset->fPixelStorageID == targetStorageID
                          && asset->fContentID == targetContentID) {
                    targetAsset = asset;
                    break;
                }
            }

            if (!targetAsset) {
                break;
            }

            if (targetAsset->fType == SkCaptureAsset::AssetType::kDerivedStorage
                && targetAsset->fOrigin.hasParent()) {
                targetStorageID = targetAsset->fOrigin.fParentPixelStorageID;
                targetContentID = targetAsset->fOrigin.fParentContentID;
                continue;
            }

            if (targetAsset->fPicture) {
                SkISize dims = targetAsset->fPicture->cullRect().roundOut().size();
                if (dims.isEmpty()) {
                    dims = {1, 1};
                }
                SKIA_LOG_I("deserializeImageProc: FOUND matching asset for storageID=%u, contentID=%u",
                           header.fPixelStorageID, header.fContentID);
                return SkImages::DeferredFromPicture(targetAsset->fPicture,
                                                     dims,
                                                     nullptr,
                                                     nullptr,
                                                     // TODO: we should try and preserve the
                                                     // original alpha type and color space.
                                                     SkImages::BitDepth::kU8,
                                                     SkColorSpace::MakeSRGB());
            }
        }
    }

    SKIA_LOG_W("deserializeImageProc: matching asset not found for storageID=%u, contentID=%u",
               header.fPixelStorageID, header.fContentID);

    SkBitmap b;
    b.allocN32Pixels(5, 5);
    SkCanvas canvas(b);
    canvas.drawColor(SK_ColorMAGENTA);
    return b.asImage();
}
