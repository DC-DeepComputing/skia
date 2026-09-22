/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/task/CopyTask.h"

#include "include/private/SkAssert.h"
#include "include/private/SkLog.h"
#include "src/gpu/graphite/Buffer.h"
#include "src/gpu/graphite/CommandBuffer.h"
#include "src/gpu/graphite/Texture.h"  // IWYU pragma: keep
#include "src/gpu/graphite/TextureInfoPriv.h"
#include "src/gpu/graphite/TextureProxy.h"

#include <utility>

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
        , fSize(size) {
    SkASSERT(fSrcBuffer->isProtected() == Protected::kNo ||
             fDstBuffer->isProtected() == Protected::kYes);
}

CopyBufferToBufferTask::~CopyBufferToBufferTask() = default;

Task::Status CopyBufferToBufferTask::prepareResources(ResourceProvider*,
                                                      ScratchResourceManager*,
                                                      sk_sp<const RuntimeEffectDictionary>) {
    return Status::kSuccess;
}

Task::Status CopyBufferToBufferTask::addCommands(Context*,
                                                 CommandBuffer* commandBuffer,
                                                 ReplayTargetData) {
    if (fDstBuffer->isProtected() == Protected::kNo && fSrcBuffer->isProtected() == Protected::kYes) {
        SKIA_LOG_E("CopyBufferToBufferTask dstBuffer %s is unprotected but srcBuffer %s is "
                   "protected.",
                   fDstBuffer->getLabel().c_str(),
                   fSrcBuffer->getLabel().c_str());
        return Status::kFail;
    }
    if (commandBuffer->copyBufferToBuffer(fSrcBuffer, fSrcOffset, fDstBuffer, fDstOffset, fSize)) {
        return Status::kSuccess;
    } else {
        return Status::kFail;
    }
}

Protected CopyBufferToBufferTask::isProtected() const {
    if (fDstBuffer && fDstBuffer->isProtected() == Protected::kYes) {
        return Protected::kYes;
    }
    if (fSrcBuffer && fSrcBuffer->isProtected() == Protected::kYes) {
        return Protected::kYes;
    }
    return Protected::kNo;
}

sk_sp<CopyTextureToBufferTask> CopyTextureToBufferTask::Make(sk_sp<TextureProxy> textureProxy,
                                                             SkIRect srcRect,
                                                             sk_sp<Buffer> buffer,
                                                             size_t bufferOffset,
                                                             size_t bufferRowBytes) {
    if (!textureProxy) {
        return nullptr;
    }
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
    SkASSERT(fTextureProxy->isProtected() == Protected::kNo ||
             fBuffer->isProtected() == Protected::kYes);
}

CopyTextureToBufferTask::~CopyTextureToBufferTask() {}

Task::Status CopyTextureToBufferTask::prepareResources(ResourceProvider* resourceProvider,
                                                       ScratchResourceManager*,
                                                       sk_sp<const RuntimeEffectDictionary>) {
    // If the source texture hasn't been instantiated yet, it means there was no prior task that
    // could have initialized its contents so a readback to a buffer does not make sense.
    SkASSERT(fTextureProxy->isInstantiated() || fTextureProxy->isLazy());
    // TODO: The copy is also a consumer of the source, so it should participate in returning
    // scratch resources like RenderPassTask does. For now, though, all copy tasks side step reuse
    // entirely and they cannot participate until they've been moved into scoping tasks like
    // DrawTask first.
    return Status::kSuccess;
}

Task::Status CopyTextureToBufferTask::addCommands(Context*,
                                                  CommandBuffer* commandBuffer,
                                                  ReplayTargetData) {
    if (fBuffer && fBuffer->isProtected() == Protected::kNo
             && fTextureProxy->isProtected() == Protected::kYes) {
        SKIA_LOG_E("CopyTextureToBufferTask buffer %s is unprotected but texture proxy %s is "
                   "protected.",
                   fBuffer->getLabel().c_str(),
                   fTextureProxy->label());
        return Status::kFail;
    }
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

Protected CopyTextureToBufferTask::isProtected() const {
    if (fBuffer && fBuffer->isProtected() == Protected::kYes) {
        return Protected::kYes;
    }
    if (fTextureProxy && fTextureProxy->isProtected() == Protected::kYes) {
        return Protected::kYes;
    }
    return Protected::kNo;
}

//--------------------------------------------------------------------------------------------------

sk_sp<CopyTextureToTextureTask> CopyTextureToTextureTask::Make(sk_sp<TextureProxy> srcProxy,
                                                               SkIRect srcRect,
                                                               sk_sp<TextureProxy> dstProxy,
                                                               SkIPoint dstPoint,
                                                               int dstLevel) {
    if (!srcProxy || !dstProxy) {
        return nullptr;
    }
    // Texture-to-texture copies do not do format conversions
    TextureFormat srcFormat = srcProxy->format();
    TextureFormat dstFormat = dstProxy->format();
    if (srcFormat != dstFormat) {
        SKIA_LOG_E("Unable to copy between textures of different formats, src = %s, dst = %s",
                    TextureFormatName(srcFormat), TextureFormatName(dstFormat));
        return nullptr;
    }
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
        , fDstLevel(dstLevel) {
    SkASSERT(fSrcProxy->isProtected() == Protected::kNo ||
             fDstProxy->isProtected() == Protected::kYes);
}

CopyTextureToTextureTask::~CopyTextureToTextureTask() {}

Task::Status CopyTextureToTextureTask::prepareResources(ResourceProvider* resourceProvider,
                                                        ScratchResourceManager*,
                                                        sk_sp<const RuntimeEffectDictionary>) {
    // Do not instantiate the src proxy. If the source texture hasn't been instantiated yet, it
    // means there was no prior task that could have initialized its contents so propagating the
    // undefined contents to the dst does not make sense.
    // TODO(b/333729316): Assert that fSrcProxy is instantiated or lazy; right now it may not be
    // instantatiated if this is a dst readback copy for a scratch Device. In that case, a
    // RenderPassTask will immediately follow this copy task and instantiate the source proxy so
    // that addCommands() has a texture to operate on. That said, the texture's contents will be
    // undefined when the copy is executed ideally it just shouldn't happen.

    // TODO: The copy is also a consumer of the source, so it should participate in returning
    // scratch resources like RenderPassTask does. For now, though, all copy tasks side step reuse
    // entirely and they cannot participate until they've been moved into scoping tasks like
    // DrawTask first. In particular, for texture-to-texture copies, they should be scoped to not
    // invoke pending listeners for a subsequent RenderPassTask.

    // TODO: Use the scratch resource manager to instantiate fDstProxy, although the details of when
    // that texture can be returned need to be worked out. While brittle, all current use cases
    // of scratch texture-to-texture copies have the dst used immediately by the next task, so it
    // could just add a pending listener that returns the texture w/o any read counting.
    if (!TextureProxy::InstantiateIfNotLazy(resourceProvider, fDstProxy.get())) {
        SKIA_LOG_E("Could not instantiate dst texture proxy for CopyTextureToTextureTask!");
        return Status::kFail;
    }
    return Status::kSuccess;
}

Task::Status CopyTextureToTextureTask::addCommands(Context*,
                                                   CommandBuffer* commandBuffer,
                                                   ReplayTargetData) {
    if (fDstProxy->isProtected() == Protected::kNo && fSrcProxy->isProtected() == Protected::kYes) {
        SKIA_LOG_E("CopyTextureToTextureTask dstProxy %s is unprotected but srcProxy %s is "
                   "protected.",
                   fDstProxy->label(), fSrcProxy->label());
        return Status::kFail;
    }
    // prepareResources() doesn't instantiate the source assuming that a prior task will have do so
    // as part of initializing the texture contents.
    SkASSERT(fSrcProxy->isInstantiated());
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

Protected CopyTextureToTextureTask::isProtected() const {
    if (fDstProxy && fDstProxy->isProtected() == Protected::kYes) {
        return Protected::kYes;
    }
    if (fSrcProxy && fSrcProxy->isProtected() == Protected::kYes) {
        return Protected::kYes;
    }
    return Protected::kNo;
}

} // namespace skgpu::graphite
