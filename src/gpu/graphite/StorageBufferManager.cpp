/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/private/SkDebug.h"
#include "src/gpu/graphite/StorageBufferManager.h"

#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/DrawContext.h"
#include "src/gpu/graphite/DrawPass.h"
#include "src/gpu/graphite/RecorderPriv.h"
#include "src/gpu/graphite/TextureFormat.h"
#include "src/gpu/graphite/TextureProxy.h"
#include "src/gpu/graphite/TextureProxyView.h"
#include "src/gpu/graphite/task/UploadTask.h"
#include "src/shaders/gradients/SkGradientBaseShader.h"

namespace skgpu::graphite {

StorageBufferManager::StorageBufferManager(bool storageBufferSupport)
        : fStorageBufferSupport(storageBufferSupport) {}

StorageBufferManager::~StorageBufferManager() { this->reset(); }

void StorageBufferManager::GradientCache::reset() {
    for (auto [k, _] : fGradientHash) {
        k->unref();
    }
    fGradientHash.reset();
    fGradientData.clear();
}

void StorageBufferManager::reset() {
    fGradientCache.reset();
}

std::pair<float*, int> StorageBufferManager::allocateGradientData(
        StorageContext* ctx,
        int numStops,
        const SkGradientBaseShader* shader) {
    if (!ctx) {
        return {nullptr, -1};
    }
    SkASSERT(!ctx->fGradientsFinalized);
    if (numStops > kMaxGradientStops) {
        return {nullptr, -1};
    }

    int* existingLocalOffset = ctx->fLocalGradientOffsetCache.find(shader);
    if (existingLocalOffset) {
        return {nullptr, *existingLocalOffset};
    }

    float* dstData = nullptr;
    GradientCache::GradientInfo* globalInfo = fGradientCache.fGradientHash.find(shader);
    if (!globalInfo) {
        int globalOffset = fGradientCache.fGradientData.size();
        int floatCount = numStops * 5;
        if (kMaxStorageFloats - floatCount < globalOffset) {
            return {nullptr, -1};
        }

        fGradientCache.fGradientData.resize(globalOffset + floatCount);
        dstData = fGradientCache.fGradientData.data() + globalOffset;

        shader->ref();
        fGradientCache.fGradientHash.set(shader, {globalOffset, floatCount});
        globalInfo = fGradientCache.fGradientHash.find(shader);
    }

    int localFloatOffset = ctx->fGradientDataSize / sizeof(float);
    ctx->fLocalGradientOffsetCache.set(shader, localFloatOffset);
    ctx->fContextGradientsList.push_back(shader);
    ctx->fGradientDataSize += globalInfo->fFloatCount * sizeof(float);

    return {dstData, localFloatOffset};
}

BindBufferInfo StorageBufferManager::finalize(DrawBufferManager* bufferMgr,
                                                StorageContext* ctx,
                                                Recorder* recorder,
                                                DrawPass* drawPass) {
    if (!ctx) {
        return {};
    }

    SkDebugf("[StorageBufferManager::finalize] fStorageBufferSupport=%d, ctxSize=%d, drawPass=%p\n",
             fStorageBufferSupport, (int)ctx->size(), drawPass);

    if (fStorageBufferSupport) {
        return this->finalizeSSBO(bufferMgr, ctx);
    }

    if (ctx->size() > 0 && recorder && drawPass) {
        TextureProxyView proxyView = this->finalizeTexture(recorder, ctx, drawPass);
        if (proxyView) {
            drawPass->fStorageTextureProxy = proxyView.refProxy();
            drawPass->fSampledTextures.push_back(proxyView.refProxy());
        }
    }

    return {};
}

BindBufferInfo StorageBufferManager::finalizeSSBO(DrawBufferManager* bufferMgr, StorageContext* ctx) {
    if (!ctx) {
        return BindBufferInfo{};
    }

    ctx->finalizeGradientStorage();

    size_t gradSize = ctx->fGradientDataSize;
    size_t appSize = SkAlignTo<size_t>(ctx->fAppendedData.size(), StorageContext::kStructAlignment);
    size_t totalBytes = gradSize + appSize;

    if (totalBytes > 0) {
        auto [writer, bufferInfo, _] =
                bufferMgr->getMappedStorageBuffer(totalBytes, /*stride=*/1);
        if (writer) {
            // 1. Pack gradient floats for this context in local run order
            if (gradSize > 0) {
                size_t writtenGradientBytes = 0;
                for (const SkGradientBaseShader* shader : ctx->fContextGradientsList) {
                    GradientCache::GradientInfo* info =
                            fGradientCache.fGradientHash.find(shader);
                    if (info) {
                        size_t bytesToCopy = info->fFloatCount * sizeof(float);
                        writer.write(fGradientCache.fGradientData.data() + info->fGlobalFloatOffset,
                                     bytesToCopy);
                        writtenGradientBytes += bytesToCopy;
                    }
                }
                if (gradSize > writtenGradientBytes) {
                    writer.zeroBytes(gradSize - writtenGradientBytes);
                }
            }

            // 2. Write appended vertex/instance storage
            if (!ctx->fAppendedData.empty()) {
                writer.write(ctx->fAppendedData.data(), ctx->fAppendedData.size());
                size_t paddingBytes = appSize - ctx->fAppendedData.size();
                if (paddingBytes > 0) {
                    writer.zeroBytes(paddingBytes);
                }
            }

            ctx->fBufferInfo = bufferInfo;
            return bufferInfo;
        }
    }
    return BindBufferInfo{};
}

sk_sp<TextureProxy> StorageBufferManager::createTextureProxy(Recorder* recorder, int width, int height) {
    const Caps* caps = recorder->priv().caps();
    TextureInfo info = caps->getDefaultSampledTextureInfo(
            kRGBA_F32_SkColorType, Mipmapped::kNo, recorder->priv().isProtected(), Renderable::kNo);

    if (!info.isValid()) {
        SkDebugf("[StorageBufferManager::createTextureProxy] getDefaultSampledTextureInfo for kRGBA_F32_SkColorType is INVALID!\n");
        return nullptr;
    }

    sk_sp<TextureProxy> proxy = TextureProxy::Make(caps,
                                                   recorder->priv().resourceProvider(),
                                                   SkISize::Make(width, height),
                                                   info,
                                                   "StorageBufferFallbackTexture",
                                                   Budgeted::kYes);
    if (!proxy) {
        SkDebugf("[StorageBufferManager::createTextureProxy] TextureProxy::Make returned nullptr!\n");
        return nullptr;
    }

    if (!proxy->instantiate(recorder->priv().resourceProvider())) {
        SkDebugf("[StorageBufferManager::createTextureProxy] proxy->instantiate failed!\n");
    }
    return proxy;
}

TextureProxyView StorageBufferManager::finalizeTexture(Recorder* recorder,
                                                        StorageContext* ctx,
                                                        DrawPass* drawPass) {
    if (!ctx || ctx->size() == 0 || !recorder) {
        return {};
    }

    ctx->finalizeGradientStorage();

    size_t gradSize = ctx->fGradientDataSize;
    size_t appSize = SkAlignTo<size_t>(ctx->fAppendedData.size(), StorageContext::kStructAlignment);
    size_t totalBytes = gradSize + appSize;

    if (totalBytes == 0) {
        return {};
    }

    SkTDArray<char> uploadBuffer;
    uploadBuffer.resize(totalBytes);

    if (gradSize > 0) {
        size_t writtenGradientBytes = 0;
        for (const SkGradientBaseShader* shader : ctx->fContextGradientsList) {
            GradientCache::GradientInfo* info = fGradientCache.fGradientHash.find(shader);
            if (info) {
                size_t bytesToCopy = info->fFloatCount * sizeof(float);
                memcpy(uploadBuffer.data() + writtenGradientBytes,
                       fGradientCache.fGradientData.data() + info->fGlobalFloatOffset,
                       bytesToCopy);
                writtenGradientBytes += bytesToCopy;
            }
        }
        if (gradSize > writtenGradientBytes) {
            memset(uploadBuffer.data() + writtenGradientBytes, 0, gradSize - writtenGradientBytes);
        }
    }

    if (!ctx->fAppendedData.empty()) {
        memcpy(uploadBuffer.data() + gradSize, ctx->fAppendedData.data(), ctx->fAppendedData.size());
        size_t paddingBytes = appSize - ctx->fAppendedData.size();
        if (paddingBytes > 0) {
            memset(uploadBuffer.data() + gradSize + ctx->fAppendedData.size(), 0, paddingBytes);
        }
    }

    constexpr int kTexelBytes = 16;
    constexpr int kMaxAtlasWidth = 8192;
    int maxAtlasWidth = std::min(recorder->priv().caps()->maxTextureSize(), kMaxAtlasWidth);

    int totalTexels = (totalBytes + kTexelBytes - 1) / kTexelBytes;
    int width = std::min(totalTexels, maxAtlasWidth);
    int height = (totalTexels + maxAtlasWidth - 1) / maxAtlasWidth;
    int atlasRowBytes = width * kTexelBytes;
    int paddedBytes = height * atlasRowBytes;

    if (uploadBuffer.size() < paddedBytes) {
        uploadBuffer.resize(paddedBytes);
    }

    sk_sp<TextureProxy> proxy = this->createTextureProxy(recorder, width, height);
    if (!proxy) {
        SkDebugf("[StorageBufferManager::finalizeTexture] createTextureProxy(%d, %d) returned nullptr!\n", width, height);
        return {};
    }

    MipLevel level;
    level.fPixels = uploadBuffer.data();
    level.fRowBytes = atlasRowBytes;
    SkIRect dstRect = SkIRect::MakeWH(width, height);

    Swizzle readSwizzle = ReadSwizzleForColorType(kRGBA_F32_SkColorType, proxy->format());
    TextureProxyView proxyView(std::move(proxy), readSwizzle);

    SkColorInfo colorInfo(kRGBA_F32_SkColorType, kPremul_SkAlphaType, nullptr);
    UploadSource source = UploadSource::Make(recorder->priv().caps(),
                                             proxyView,
                                             colorInfo,
                                             colorInfo,
                                             SkSpan<const MipLevel>(&level, 1),
                                             dstRect);

    // TODO necessary
    // if (source.attemptUploadOnhost()) {
    //     return proxyView;
    // }

    UploadInstance uploadInstance = UploadInstance::Make(recorder, source, nullptr);
    if (!uploadInstance.isValid()) {
        SkDebugf("[StorageBufferManager::finalizeTexture] UploadInstance::Make failed!\n");
        return {};
    }

    sk_sp<Task> uploadTask = UploadTask::Make(std::move(uploadInstance));
    if (!uploadTask) {
        SkDebugf("[StorageBufferManager::finalizeTexture] UploadTask::Make failed!\n");
        return {};
    }

    SkDebugf("[StorageBufferManager::finalizeTexture] Created fallback uploadTask=%p, drawPass=%p\n",
             uploadTask.get(), drawPass);

    if (drawPass) {
        drawPass->fTextureFallbackUploadTask = std::move(uploadTask);
    }

    return proxyView;
}

} // namespace skgpu::graphite
