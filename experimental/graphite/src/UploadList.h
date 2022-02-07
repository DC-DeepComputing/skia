/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_UploadList_DEFINED
#define skgpu_UploadList_DEFINED

#include <vector>

#include "include/core/SkData.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/private/SkTemplates.h"
#include "src/core/SkTBlockList.h"

namespace skgpu {

class TextureProxy;

struct MipLevel {
    const void* fPixels = nullptr;
    size_t fRowBytes = 0;
    // This may be used to keep fPixels from being freed while a MipLevel exists.
    sk_sp<SkData> fOptionalStorage;
};

/**
 * An UploadList represents a collection of uploads in a form that closely mirrors what can be
 * rendered efficiently and directly by the GPU backend.
 *
 * An upload command combines:
 *   - the target texture proxy
 *   - a set of miplevel data in level order
 *   - top level destination rectangle in the texture
 *
 * Currently commands are accumulated in order and processed in the same order. Dependency
 * management is expected to be handled by the TaskGraph.
 */
class UploadList {
public:
    void appendUpload(sk_sp<TextureProxy> targetProxy,
                      std::vector<MipLevel> levels,
                      const SkIRect& dstRect);

    int uploadCount() const { return fUploads.count(); }

private:
    struct Upload {
        sk_sp<TextureProxy> fTextureProxy;
        std::vector<MipLevel> fLevels;
        SkIRect fDstRect;
        // TODO: src & dst colorSpaces?

        Upload(sk_sp<TextureProxy> targetProxy,
               std::vector<MipLevel> levels,
               const SkIRect& dstRect)
            : fTextureProxy(std::move(targetProxy))
            , fLevels(std::move(levels))
            , fDstRect(dstRect) {}
    };

    SkTBlockList<Upload, 16>    fUploads;
};

} // namespace skgpu

#endif // skgpu_UploadList_DEFINED
