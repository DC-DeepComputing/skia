/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkCaptureAsset_DEFINED
#define SkCaptureAsset_DEFINED

#include "include/core/SkPicture.h"
#include "include/private/SkPixelStorage.h"

#include <atomic>
#include <cstdint>

class SkCaptureAsset {
public:
    enum class AssetType : uint32_t {
        kPicture = 0,
        kDerivedStorage = 1,
    };
    using Type = AssetType;

    SkCaptureAsset() : fRefCnt(1) {}
    virtual ~SkCaptureAsset() = default;

    void ref() const {
        fRefCnt.fetch_add(1, std::memory_order_relaxed);
    }

    void unref() const {
        if (fRefCnt.fetch_sub(1, std::memory_order_acq_rel) == 1) {
            delete this;
        }
    }

    uint32_t fPixelStorageID = 0;
    uint32_t fContentID = 0;
    AssetType fType = AssetType::kPicture;
    sk_sp<SkPicture> fPicture;
    SkPixelStorage::PixelStorageOrigin fOrigin;

private:
    mutable std::atomic<int32_t> fRefCnt{1};
};

#endif // SkCaptureAsset_DEFINED
