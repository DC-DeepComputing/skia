/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkCapture_DEFINED
#define SkCapture_DEFINED

#include "include/core/SkAlphaType.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSerialProcs.h"
#include "include/private/SkPixelStorage.h"
#include "include/private/SkTArray.h"
#include "src/capture/SkCaptureAsset.h"

#include <optional>
#include <unordered_map>

class SkData;
class SkImage;
class SkPicture;

class SkCanvas;
class SkCaptureCanvas;

/**
 * Binary Serialization Layout (.capt):
 *
 * +-----------------------------------------------------------------------------+
 * | Magic Bytes 1 ('skia')                     | 4 Bytes (uint32_t)             |
 * +-----------------------------------------------------------------------------+
 * | Magic Bytes 2 ('capt')                     | 4 Bytes (uint32_t)             |
 * +-----------------------------------------------------------------------------+
 * | Version (kVersion)                         | 4 Bytes (uint32_t)             |
 * +-----------------------------------------------------------------------------+
 * | Asset Count (numAssets)                    | 4 Bytes (uint32_t)             |
 * +-----------------------------------------------------------------------------+
 * | RecordingCapture Count (numRecCaptures)    | 4 Bytes (uint32_t)             |
 * +=============================================================================+
 * |                       ASSETS SECTION (N = numAssets)                        |
 * +-----------------------------------------------------------------------------+
 * | Asset i PixelStorage ID                    | 4 Bytes (uint32_t)             |
 * +-----------------------------------------------------------------------------+
 * | Asset i Content ID                         | 4 Bytes (uint32_t)             |
 * +-----------------------------------------------------------------------------+
 * | Asset i Type (0=kPicture, 1=kDerivedStorage)| 4 Bytes (uint32_t)            |
 * + - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
 * | If Type == 0 (kPicture):                                                    |
 * |   Picture i Data Size                      | 4 Bytes (uint32_t)             |
 * |   Picture i Bytes                          | [Picture i Size] bytes         |
 * |                                                                             |
 * | If Type == 1 (kDerivedStorage):                                             |
 * |   Parent PixelStorage ID                   | 4 Bytes (uint32_t)             |
 * |   Parent Content ID                        | 4 Bytes (uint32_t)             |
 * +-----------------------------------------------------------------------------+
 * | ... (Repeated for N assets)                                                 |
 * +=============================================================================+
 * |                     TIMELINE SECTION (K = numRecCaptures)                   |
 * +-----------------------------------------------------------------------------+
 * | Recording 0 Task Count (taskCount)         | 4 Bytes (uint32_t)             |
 * + - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
 * | Recording 0 Tasks (fAssetIndex)            | taskCount * sizeof(DrawTask)   |
 * +-----------------------------------------------------------------------------+
 * | ... (Repeated for K recordings)                                             |
 * +-----------------------------------------------------------------------------+
 */

class SkCapture : public SkRefCnt {
public:
    // NOTE: For now, all DrawTasks point to an asset identified by (fPixelStorageID, fContentID).
    // Eventually, other draw tasks might not point to an asset.
    struct DrawTask {
        uint32_t fPixelStorageID = 0;
        uint32_t fContentID = 0;
    };

    struct RecordingCapture {
        skia_private::TArray<DrawTask> fDrawTasks;
    };

    struct Metadata {
        uint32_t version;
        uint32_t numAssets;
        uint32_t numRecordingCaptures;
    };

    static sk_sp<SkCapture> MakeFromData(sk_sp<const SkData>);
    static sk_sp<SkCapture> MakeEmpty();

    void addAsset(sk_sp<SkCaptureAsset> asset);
    void addRecordingCapture(RecordingCapture);

    sk_sp<SkData> serializeCapture();

    // Returns the exact asset record stored for (storageID, contentID), or nullptr.
    const SkCaptureAsset* getAsset(uint32_t storageID, uint32_t contentID) const;

    // Resolves storage origin chains (kDerivedStorage) to return the root containing the SkPicture.
    const SkCaptureAsset* resolveRootAsset(uint32_t storageID, uint32_t contentID) const;
    const std::unordered_map<uint64_t, sk_sp<SkCaptureAsset>>& getAssetsMap() const {
        return fAssets;
    }
    const RecordingCapture* getRecordingCapture(int i) const;
    Metadata getMetadata() const;

private:
    // Compact binary payload written into serialized picture streams to point to a captured asset.
    struct ImageAssetRef {
        uint32_t fPixelStorageID;
        uint32_t fContentID;
    };

    static inline uint64_t MakeAssetKey(uint32_t storageID, uint32_t contentID) {
        return (static_cast<uint64_t>(storageID) << 32) | contentID;
    }

    static SkSerialReturnType serializeImageProc(SkImage* img, void* ctx);
    static sk_sp<SkImage> deserializeImageProc(sk_sp<SkData>,
                                               std::optional<SkAlphaType>, void* ctx);

    Metadata fMetadata;
    // fAssetKeys is used to track insertion order of SkCaptureAssets. This helps deserialization
    // by ensuring we don't try to deserialize an asset before any other assets it might reference
    // have been deserialized. This allows us to avoid a two-pass deserialization.
    // Note: A possible better solution is to implement a lazy capture image proxy that delays
    // binding until the image it is depending on gets deserialized.
    skia_private::TArray<uint64_t> fAssetKeys;
    std::unordered_map<uint64_t, sk_sp<SkCaptureAsset>> fAssets;
    skia_private::TArray<RecordingCapture> fTimeline;

    // fNextSyntheticContentID generates unique content IDs for rare cases where SkPixelStorage has
    // not been initialized to avoid map collisions at {0, 0}.
    uint32_t fNextSyntheticContentID = 1;

    static const uint32_t kVersion = 0;
};

#endif //SkCapture_DEFINED
