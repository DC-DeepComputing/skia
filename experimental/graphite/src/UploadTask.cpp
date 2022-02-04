/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/graphite/src/UploadTask.h"

#include "experimental/graphite/src/Buffer.h"
#include "experimental/graphite/src/CommandBuffer.h"
#include "experimental/graphite/src/Log.h"
#include "experimental/graphite/src/Texture.h"
#include "experimental/graphite/src/TextureProxy.h"

namespace skgpu {
std::vector<UploadCommand> UploadCommand::MakeCommands(Recorder* recorder,
                                                       const std::unique_ptr<UploadList>& list) {
    // TODO:
    // At this point we process the contents of the upload list.
    // For each Upload in the list we do:
    // * request a staging buffer
    // * from the recorder's ResourceProvider, copy the data from the list into the buffer
    //   (format conversion and flips would happen here as well)
    // * set up the BufferTextureCopyData for this transfer
    // * push onto vector

    return {};
}

UploadCommand::UploadCommand(sk_sp<Buffer> buffer,
                             sk_sp<TextureProxy> textureProxy,
                             std::vector<BufferTextureCopyData> copyData)
    : fBuffer(std::move(buffer))
    , fTextureProxy(std::move(textureProxy))
    , fCopyData(std::move(copyData)) {}

void UploadCommand::addCommand(ResourceProvider* resourceProvider,
                               CommandBuffer* commandBuffer) const {
    if (!fTextureProxy) {
        SKGPU_LOG_E("No texture proxy specified for UploadTask");
        return;
    }
    if (!fTextureProxy->instantiate(resourceProvider)) {
        SKGPU_LOG_E("Given invalid texture proxy for UploadTask!");
        return;
    }

    commandBuffer->copyBufferToTexture(std::move(fBuffer),
                                       fTextureProxy->refTexture(),
                                       fCopyData.data(),
                                       fCopyData.size());
}

//---------------------------------------------------------------------------

sk_sp<UploadTask> UploadTask::Make(std::vector<UploadCommand> commands) {
    return sk_sp<UploadTask>(new UploadTask(std::move(commands)));
}

UploadTask::UploadTask(std::vector<UploadCommand> commands) : fCommands(std::move(commands)) {}

UploadTask::~UploadTask() {}

void UploadTask::addCommands(ResourceProvider* resourceProvider,
                             CommandBuffer* commandBuffer) {
    for (unsigned int i = 0; i < fCommands.size(); ++i) {
        fCommands[i].addCommand(resourceProvider, commandBuffer);
    }
}

} // namespace skgpu
