/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/graphite/src/CopyTask.h"

#include "experimental/graphite/src/Buffer.h"
#include "experimental/graphite/src/CommandBuffer.h"
#include "experimental/graphite/src/Texture.h"

namespace skgpu {

sk_sp<CopyTextureToBufferTask> CopyTextureToBufferTask::Make(sk_sp<Texture> texture,
                                                             SkIRect srcRect,
                                                             sk_sp<Buffer> buffer,
                                                             size_t bufferOffset,
                                                             size_t bufferRowBytes) {
    return sk_sp<CopyTextureToBufferTask>(new CopyTextureToBufferTask(std::move(texture),
                                                                      srcRect,
                                                                      std::move(buffer),
                                                                      bufferOffset,
                                                                      bufferRowBytes));
}

CopyTextureToBufferTask::CopyTextureToBufferTask(sk_sp<Texture> texture,
                                                 SkIRect srcRect,
                                                 sk_sp<Buffer> buffer,
                                                 size_t bufferOffset,
                                                 size_t bufferRowBytes)
        : fTexture(std::move(texture))
        , fSrcRect(srcRect)
        , fBuffer(std::move(buffer))
        , fBufferOffset(bufferOffset)
        , fBufferRowBytes(bufferRowBytes) {
}

CopyTextureToBufferTask::~CopyTextureToBufferTask() {}

void CopyTextureToBufferTask::addCommands(ResourceProvider*, CommandBuffer* commandBuffer) {
    commandBuffer->copyTextureToBuffer(std::move(fTexture),
                                       fSrcRect,
                                       std::move(fBuffer),
                                       fBufferOffset,
                                       fBufferRowBytes);
}

//sk_sp<UploadTask> UploadTask::Make(const Uploads* uploads,
//                                   size_t* bufferOffsets,
//                                   int count,
//                                   sk_sp<Buffer> buffer,
//                                   size_t bufferRowBytes) {
//    return sk_sp<UploadTask>(new UploadTask(uploads,
//                                            bufferOffsets,
//                                            count,
//                                            std::move(buffer),
//                                            bufferRowBytes));
//}
//
//UploadTask::UploadTask(const Uploads* uploads,
//                       size_t* bufferOffsets,
//                       int count,
//                       sk_sp<Buffer> buffer,
//                       size_t bufferRowBytes) {
//    for (int i = 0; i < count; ++i) {
//        fUploads[i] = uploads[i];
//        fOffsets[i] = offsets[i];
//    }
//}
//
//void UploadTask::addCommands(ResourceProvider*, CommandBuffer*) {
//    // do something here
//
//    // need to add a CopyFromBufferToTexture command to command buffer
//    // can we batch them with some APIs?
//}

} // namespace skgpu
