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
    struct DrawTask {
        uint32_t fAssetIndex;
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
    void addAsset(sk_sp<SkPicture> picture, uint32_t pixelStorageID = 0, uint32_t contentID = 0);
    void addRecordingCapture(RecordingCapture);

    sk_sp<SkData> serializeCapture();

    sk_sp<SkPicture> getAsset(int i) const;
    const SkCaptureAsset* getAssetRecord(int i) const;
    const RecordingCapture* getRecordingCapture(int i) const;
    Metadata getMetadata() const;

private:
    struct ImageRefHeader {
        uint32_t fPixelStorageID;
        uint32_t fContentID;
    };

    static SkSerialReturnType serializeImageProc(SkImage* img, void* ctx);
    static sk_sp<SkImage> deserializeImageProc(sk_sp<SkData>,
                                               std::optional<SkAlphaType>, void* ctx);

    Metadata fMetadata;
    skia_private::TArray<sk_sp<SkCaptureAsset>> fAssets;
    skia_private::TArray<RecordingCapture> fTimeline;

    static const uint32_t kVersion = 0;
};

#endif //SkCapture_DEFINED
