/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/task/CopyTask.h"

#include "src/gpu/graphite/Buffer.h"
#include "src/gpu/graphite/CommandBuffer.h"
#include "src/gpu/graphite/Log.h"
#include "src/gpu/graphite/Texture.h"
#include "src/gpu/graphite/TextureProxy.h"

namespace skgpu::graphite {

sk_sp<CopyBufferToBufferTask> CopyBufferToBufferTask::Make(const Buffer* srcBuffer,
                                                           size_t srcOffset,
                                                           sk_sp<Buffer> dstBuffer,
                                                           size_t dstOffset,
                                                           size_t size) {
    SkASSERT(srcBuffer);
    SkASSERT(size <= srcBuffer->size() - srcOffset);
    SkASSERT(dstBuffer);
    SkASSERT(size <= dstBuffer->size() - dstOffset);
    return sk_sp<CopyBufferToBufferTask>(new CopyBufferToBufferTask(srcBuffer,
                                                                    srcOffset,
                                                                    std::move(dstBuffer),
                                                                    dstOffset,
                                                                    size));
}

CopyBufferToBufferTask::CopyBufferToBufferTask(const Buffer* srcBuffer,
                                               size_t srcOffset,
                                               sk_sp<Buffer> dstBuffer,
                                               size_t dstOffset,
                                               size_t size)
        : fSrcBuffer(srcBuffer)
        , fSrcOffset(srcOffset)
        , fDstBuffer(std::move(dstBuffer))
        , fDstOffset(dstOffset)
        , fSize(size) {}

CopyBufferToBufferTask::~CopyBufferToBufferTask() = default;

Task::Status CopyBufferToBufferTask::prepareResources(ResourceProvider*,
                                                      ScratchResourceManager*,
                                                      const RuntimeEffectDictionary*) {
    return Status::kSuccess;
}

Task::Status CopyBufferToBufferTask::addCommands(Context*,
                                                 CommandBuffer* commandBuffer,
                                                 ReplayTargetData) {
    if (commandBuffer->copyBufferToBuffer(fSrcBuffer, fSrcOffset, fDstBuffer, fDstOffset, fSize)) {
        return Status::kSuccess;
    } else {
        return Status::kFail;
    }
}

sk_sp<CopyTextureToBufferTask> CopyTextureToBufferTask::Make(sk_sp<TextureProxy> textureProxy,
                                                             SkIRect srcRect,
                                                             sk_sp<Buffer> buffer,
                                                             size_t bufferOffset,
                                                             size_t bufferRowBytes) {
    return sk_sp<CopyTextureToBufferTask>(new CopyTextureToBufferTask(std::move(textureProxy),
                                                                      srcRect,
                                                                      std::move(buffer),
                                                                      bufferOffset,
                                                                      bufferRowBytes));
}

CopyTextureToBufferTask::CopyTextureToBufferTask(sk_sp<TextureProxy> textureProxy,
                                                 SkIRect srcRect,
                                                 sk_sp<Buffer> buffer,
                                                 size_t bufferOffset,
                                                 size_t bufferRowBytes)
        : fTextureProxy(std::move(textureProxy))
        , fSrcRect(srcRect)
        , fBuffer(std::move(buffer))
        , fBufferOffset(bufferOffset)
        , fBufferRowBytes(bufferRowBytes) {
}

CopyTextureToBufferTask::~CopyTextureToBufferTask() {}

Task::Status CopyTextureToBufferTask::prepareResources(ResourceProvider* resourceProvider,
                                                       ScratchResourceManager*,
                                                       const RuntimeEffectDictionary*) {
    // TODO: Move this check to Make()
    if (!fTextureProxy) {
        SKGPU_LOG_E("No texture proxy specified for CopyTextureToBufferTask");
        return Status::kFail;
    }
    // TODO: Assert that the texture has already been instantiated; if it hasn't been then this
    // marks the first task that uses the proxy and its contents are undefined.
    if (!TextureProxy::InstantiateIfNotLazy(resourceProvider, fTextureProxy.get())) {
        SKGPU_LOG_E("Could not instantiate texture proxy for CopyTextureToBufferTask!");
        return Status::kFail;
    }
    // TODO:The copy is also a consumer of the source, so it should notify the manager.
    return Status::kSuccess;
}

Task::Status CopyTextureToBufferTask::addCommands(Context*,
                                                  CommandBuffer* commandBuffer,
                                                  ReplayTargetData) {
    if (commandBuffer->copyTextureToBuffer(fTextureProxy->refTexture(),
                                           fSrcRect,
                                           std::move(fBuffer),
                                           fBufferOffset,
                                           fBufferRowBytes)) {
        // TODO(b/332681367): CopyTextureToBuffer is currently only used for readback operations,
        // which are a one-time event. Should this just default to returning kDiscard?
        return Status::kSuccess;
    } else {
        return Status::kFail;
    }
}

//--------------------------------------------------------------------------------------------------

sk_sp<CopyTextureToTextureTask> CopyTextureToTextureTask::Make(sk_sp<TextureProxy> srcProxy,
                                                               SkIRect srcRect,
                                                               sk_sp<TextureProxy> dstProxy,
                                                               SkIPoint dstPoint,
                                                               int dstLevel) {
    return sk_sp<CopyTextureToTextureTask>(new CopyTextureToTextureTask(std::move(srcProxy),
                                                                        srcRect,
                                                                        std::move(dstProxy),
                                                                        dstPoint,
                                                                        dstLevel));
}

CopyTextureToTextureTask::CopyTextureToTextureTask(sk_sp<TextureProxy> srcProxy,
                                                   SkIRect srcRect,
                                                   sk_sp<TextureProxy> dstProxy,
                                                   SkIPoint dstPoint,
                                                   int dstLevel)
        : fSrcProxy(std::move(srcProxy))
        , fSrcRect(srcRect)
        , fDstProxy(std::move(dstProxy))
        , fDstPoint(dstPoint)
        , fDstLevel(dstLevel) {}

CopyTextureToTextureTask::~CopyTextureToTextureTask() {}

Task::Status CopyTextureToTextureTask::prepareResources(ResourceProvider* resourceProvider,
                                                        ScratchResourceManager*,
                                                        const RuntimeEffectDictionary*) {
    // TODO: Move this check to Make()
    if (!fSrcProxy) {
        SKGPU_LOG_E("No src texture proxy specified for CopyTextureToTextureTask");
        return Status::kFail;
    }
    // TODO: Assert that the source proxy has already been instantiated; otherwise this is the first
    // use of the proxy and its contents are undefined so copying from it makes no sense.
    if (!TextureProxy::InstantiateIfNotLazy(resourceProvider, fSrcProxy.get())) {
        SKGPU_LOG_E("Could not instantiate src texture proxy for CopyTextureToTextureTask!");
        return Status::kFail;
    }
    // TODO: Move this check to Make()
    if (!fDstProxy) {
        SKGPU_LOG_E("No dst texture proxy specified for CopyTextureToTextureTask");
        return Status::kFail;
    }
    // TODO: Use the scratch resource manager to instantiate the dst proxy. The copy is also a
    // consumer of the source, so it should notify after its instantiated. It's also a writer to
    // the scratch dst so it should add a pending listener (maybe? are we expecting this to be
    // consumed in that manner -> should it always just assume 1 use?)
    //
    // -> does that cause challenges with interleaving dst readback copies with render pass tasks?
    //    the copy tex to tex has to come first but we don't want it to notify and return textures
    //    that are part of the scope that the renderpass is in.
    //    -> we could do a tiny 2 task scope for this?
    //    -> the other plans for moving drawpass creation into prepareResources() could also mean
    //       that this copy is directly inside the RenderPassTask and just a CB operation
    // but what about other copies? could they cause a consumption too soon?
    if (!TextureProxy::InstantiateIfNotLazy(resourceProvider, fDstProxy.get())) {
        SKGPU_LOG_E("Could not instantiate dst texture proxy for CopyTextureToTextureTask!");
        return Status::kFail;
    }
    return Status::kSuccess;
}

Task::Status CopyTextureToTextureTask::addCommands(Context*,
                                                   CommandBuffer* commandBuffer,
                                                   ReplayTargetData) {

    if (commandBuffer->copyTextureToTexture(fSrcProxy->refTexture(),
                                            fSrcRect,
                                            fDstProxy->refTexture(),
                                            fDstPoint,
                                            fDstLevel)) {
        // TODO(b/332681367): The calling context should be able to specify whether or not this copy
        // is a repeatable operation (e.g. dst readback copy for blending) or one time (e.g. client
        // asked for a copy of an image or surface).
        return Status::kSuccess;
    } else {
        return Status::kFail;
    }
}

} // namespace skgpu::graphite
