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

#include <unordered_set>

constexpr SkFourByteTag kMagic1  = SkSetFourByteTag('s','k','i','a');
constexpr SkFourByteTag kMagic2  = SkSetFourByteTag('c','a','p','t');

// TODO (b/412351769): Add comprehensive input validation for deserializing .capt files
// (e.g., validate pictureDataSize, assetCount, and taskCount against remaining stream bytes,
// and handle unrecognized asset types).
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

        SkCaptureAsset::Type assetType = static_cast<SkCaptureAsset::Type>(typeVal);
        auto asset = sk_make_sp<SkCaptureAsset>();
        asset->fPixelStorageID = pixelStorageID;
        asset->fContentID = contentID;
        asset->fType = assetType;
        uint64_t key = MakeAssetKey(pixelStorageID, contentID);
        if (capture->fAssets.find(key) == capture->fAssets.end()) {
            capture->fAssetKeys.push_back(key);
        }
        capture->fAssets[key] = asset;

        if (assetType == SkCaptureAsset::Type::kPicture) {
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

            asset->fPicture = std::move(picture);
        } else if (assetType == SkCaptureAsset::Type::kDerivedStorage) {
            uint32_t parentPixelStorageID;
            uint32_t parentContentID;
            if (!stream.readU32(&parentPixelStorageID) || !stream.readU32(&parentContentID)) {
                SKIA_LOG_E("Failed to read origin parent IDs for asset %u.", i);
                return nullptr;
            }
            asset->fProvenance = SkPixelStorage::Provenance::Copy(parentPixelStorageID,
                                                                  parentContentID);
        } else {
            SKIA_LOG_W("Unsupported asset type %d for asset %u.", (int)assetType, i);
            return nullptr;
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
            uint32_t pixelStorageID;
            uint32_t contentID;
            if (!stream.readU32(&pixelStorageID) || !stream.readU32(&contentID)) {
                SKIA_LOG_E("Failed to read task %u for recording capture %u.", j, i);
                return nullptr;
            }
            rec.fDrawTasks.push_back({pixelStorageID, contentID});
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

    // Assets where fPixelStorageID == 0 do not have a tracked GPU SkPixelStorage. This should not
    // happen. But if it does, to preserve the capture, we assign a unique synthetic content ID so
    // that the asset gets a unique key in fAssets (0, syntheticContentID) so we don't collide with
    // existing {0, 0} keys.
    if (asset->fPixelStorageID == 0 && asset->fContentID == 0) {
        SKIA_LOG_W("Asset has uninitialized SkPixelStorage.");
        asset->fContentID = fNextSyntheticContentID++;
    }

    if (asset->fPicture || asset->fType == SkCaptureAsset::Type::kDerivedStorage) {
        uint64_t key = MakeAssetKey(asset->fPixelStorageID, asset->fContentID);
        if (fAssets.find(key) == fAssets.end()) {
            fAssetKeys.push_back(key);
        }
        fAssets[key] = std::move(asset);
        fMetadata.numAssets = fAssets.size();
    }
}

void SkCapture::addRecordingCapture(RecordingCapture rec) {
    fTimeline.push_back(std::move(rec));
    fMetadata.numRecordingCaptures = fTimeline.size();
}

const SkCaptureAsset* SkCapture::getAsset(uint32_t storageID, uint32_t contentID) const {
    uint64_t key = MakeAssetKey(storageID, contentID);
    auto it = fAssets.find(key);
    if (it != fAssets.end()) {
        return it->second.get();
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

    // 1. Assets Section: SkCaptureAssets serialized in insertion order
    for (uint64_t key : fAssetKeys) {
        auto it = fAssets.find(key);
        if (it == fAssets.end() || !it->second) continue;
        const auto& asset = it->second;
        stream.write32(asset->fPixelStorageID);
        stream.write32(asset->fContentID);
        stream.write32(static_cast<uint32_t>(asset->fType));

        if (asset->fType == SkCaptureAsset::Type::kPicture) {
            SkDynamicMemoryWStream pictureStream;
            SkSerialProcs procs;
            procs.fImageProc = SkCapture::serializeImageProc;
            asset->fPicture->serialize(&pictureStream, &procs);
            sk_sp<SkData> pictureData = pictureStream.detachAsData();

            stream.write32(pictureData->size());
            stream.write(pictureData->data(), pictureData->size());
        } else if (asset->fType == SkCaptureAsset::Type::kDerivedStorage) {
            stream.write32(asset->fProvenance.fParentKey.fStorageID);
            stream.write32(asset->fProvenance.fParentKey.fContentID);
        } else {
            SKIA_LOG_W("Unsupported asset type %d for asset key %llu.", (int)asset->fType,
                       (unsigned long long)key);
        }
    }

    // 2. Timeline Section: RecordingCaptures
    for (const auto& rec : fTimeline) {
        // Write DrawTasks count
        stream.write32(rec.fDrawTasks.size());
        for (const auto& task : rec.fDrawTasks) {
            stream.write32(task.fPixelStorageID);
            stream.write32(task.fContentID);
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
    uint32_t contentID = 0;
    auto storages = as_IB(img)->getPixelStorages();

    // Note: Multi-planar (YUV) images contain multiple pixel storages.
    // We track the image asset based on the primary pixel storage (storages[0]).
    if (!storages.empty() && storages[0]) {
        const auto& s = storages[0];
        if (s->provenance().hasParent()) {
            storageID = s->provenance().fParentKey.fStorageID;
            contentID = s->provenance().fParentKey.fContentID;
        } else {
            storageID = s->getPixelStorageId();
            contentID = s->getContentId();
        }
    }

    ImageAssetRef assetRef{storageID, contentID};
    return SkData::MakeWithCopy(&assetRef, sizeof(ImageAssetRef));
}

const SkCaptureAsset* SkCapture::resolveRootAsset(uint32_t storageID, uint32_t contentID) const {
    uint64_t currKey = MakeAssetKey(storageID, contentID);
    std::unordered_set<uint64_t> visited;

    while (currKey != 0) {
        if (visited.count(currKey)) {
            SKIA_LOG_E("Cycle detected in SkCaptureAsset derived storage chain for key %llu",
                       (unsigned long long)currKey);
            return nullptr;
        }
        visited.insert(currKey);

        auto it = fAssets.find(currKey);
        if (it == fAssets.end() || !it->second) {
            return nullptr;
        }

        const SkCaptureAsset* asset = it->second.get();
        if (asset->fType == SkCaptureAsset::Type::kDerivedStorage
            && asset->fProvenance.hasParent()) {
            currKey = MakeAssetKey(asset->fProvenance.fParentKey.fStorageID,
                                   asset->fProvenance.fParentKey.fContentID);
            continue;
        }

        return asset;
    }
    return nullptr;
}

sk_sp<SkImage> SkCapture::deserializeImageProc(sk_sp<SkData> data,
                                               std::optional<SkAlphaType>,
                                               void* ctx) {
    if (!data || data->size() < sizeof(ImageAssetRef)) {
        SKIA_LOG_E("deserializeImageProc: invalid data size.");
        return nullptr;
    }

    ImageAssetRef assetRef;
    memcpy(&assetRef, data->data(), sizeof(ImageAssetRef));

    auto* capture = static_cast<SkCapture*>(ctx);
    if (capture) {
        const SkCaptureAsset* targetAsset = capture->resolveRootAsset(assetRef.fPixelStorageID,
            assetRef.fContentID);
        if (targetAsset && targetAsset->fPicture) {
            SkISize dims = targetAsset->fPicture->cullRect().roundOut().size();
            if (dims.isEmpty()) {
                dims = {1, 1};
            }
            SKIA_LOG_I("deserializeImageProc: FOUND matching asset for storageID=%u, contentID=%u",
                       assetRef.fPixelStorageID, assetRef.fContentID);
            return SkImages::DeferredFromPicture(targetAsset->fPicture,
                                                 dims,
                                                 nullptr,
                                                 nullptr,
                                                 // TODO: we need to try and preserve the
                                                 // original alpha type and color space.
                                                 SkImages::BitDepth::kU8,
                                                 SkColorSpace::MakeSRGB());
        }
    }

    SKIA_LOG_W("deserializeImageProc: matching asset not found for storageID=%u, contentID=%u",
               assetRef.fPixelStorageID, assetRef.fContentID);

    SkBitmap b;
    b.allocN32Pixels(5, 5);
    SkCanvas canvas(b);
    canvas.drawColor(SK_ColorMAGENTA);
    return b.asImage();
}
