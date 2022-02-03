/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_CopyTask_DEFINED
#define skgpu_CopyTask_DEFINED

#include "experimental/graphite/src/Task.h"

#include "include/core/SkRect.h"

namespace skgpu {

class Buffer;
class CommandBuffer;
class Texture;

class CopyTextureToBufferTask final : public Task {
public:
    static sk_sp<CopyTextureToBufferTask> Make(sk_sp<Texture>,
                                               SkIRect srcRect,
                                               sk_sp<Buffer>,
                                               size_t bufferOffset,
                                               size_t bufferRowBytes);

    ~CopyTextureToBufferTask() override;

    void addCommands(ResourceProvider*, CommandBuffer*) override;

private:
    CopyTextureToBufferTask(sk_sp<Texture>,
                            SkIRect srcRect,
                            sk_sp<Buffer>,
                            size_t bufferOffset,
                            size_t bufferRowBytes);

    sk_sp<Texture> fTexture;
    SkIRect fSrcRect;
    sk_sp<Buffer> fBuffer;
    size_t fBufferOffset;
    size_t fBufferRowBytes;
};

//// Batches a set of uploads from a single buffer
//class UploadTask final : public Task {
//public:
//    static sk_sp<UploadTask> Make(const Uploads*,
//                                  size_t* bufferOffsets,
//                                  int count,
//                                  sk_sp<Buffer>,
//                                  size_t bufferRowBytes);
//
//    ~UploadTask() override;
//
//    void addCommands(ResourceProvider*, CommandBuffer*) override;
//
//private:
//    UploadTask(const Uploads*,
//               size_t* bufferOffsets,
//               int count,
//               sk_sp<Buffer>,
//               size_t bufferRowBytes);
//
//    std::vector<Upload> fUploads; // TODO: combine these two
//    std::vector<size_t> fOffsets;
//    sk_sp<Buffer> fBuffer;
//    size_t fBufferRowBytes;
//};

} // namespace skgpu

#endif // skgpu_CopyTask_DEFINED
