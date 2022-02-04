/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/graphite/src/UploadList.h"

#include "experimental/graphite/src/TextureProxy.h"

namespace skgpu {

void UploadList::appendUpload(sk_sp<TextureProxy> targetProxy,
                              std::vector<MipLevel> levels,
                              const SkIRect& dstRect) {
    fUploads.push_back({std::move(targetProxy), std::move(levels), dstRect});
}

} // namespace skgpu
