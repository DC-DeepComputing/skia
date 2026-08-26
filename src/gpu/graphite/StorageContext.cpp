/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/StorageContext.h"

#include "include/private/SkAlign.h"
#include "src/gpu/graphite/BufferManager.h"
#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/DrawPass.h"
#include "src/gpu/graphite/RecorderPriv.h"
#include "src/gpu/graphite/TextureProxy.h"
#include "src/gpu/graphite/TextureProxyView.h"
#include "src/gpu/graphite/task/UploadTask.h"
#include "src/shaders/gradients/SkGradientBaseShader.h"

namespace skgpu::graphite {

StorageContext::StorageContext() = default;

StorageContext::~StorageContext() {
    this->resetCache();
}

void StorageContext::GradientCache::reset() {
    fLocalGradientOffsetCache.foreach([](const SkGradientBaseShader* shader, int*) {
        shader->unref();
    });
    fLocalGradientOffsetCache.reset();
    fGradientData.clear();
    fGradientDataSize = 0;
}

void StorageContext::resetCache() {
    fGradientCache.reset();
    fVertexData.clear();
    fRunningLCM = 1;
    SkDEBUGCODE(fGradientsFinalized = false;)
}

std::pair<float*, int> StorageContext::allocateGradientData(int numStops,
                                                            const SkGradientBaseShader* shader) {
    SkASSERT(!fGradientsFinalized);
    if (numStops > GradientCache::kMaxGradientStops) {
        return {nullptr, -1};
    }

    int* existingLocalOffset = fGradientCache.fLocalGradientOffsetCache.find(shader);
    if (existingLocalOffset) {
        return {nullptr, *existingLocalOffset};
    }

    int floatOffset = fGradientCache.fGradientData.size();
    int floatCount = numStops * 5;
    if (GradientCache::kMaxStorageFloats - floatCount < floatOffset) {
        return {nullptr, -1};
    }

    fGradientCache.fGradientData.resize(floatOffset + floatCount);
    float* dstData = fGradientCache.fGradientData.data() + floatOffset;

    shader->ref();
    fGradientCache.fLocalGradientOffsetCache.set(shader, floatOffset);
    fGradientCache.fGradientDataSize = fGradientCache.fGradientData.size_bytes();

    return {dstData, floatOffset};
}

void StorageContext::recordAlignment(size_t stride, size_t align) {
    SkASSERT(stride > 0 && align > 0);
    SkASSERT(!fGradientsFinalized);
    uint32_t align32 = BufferAligner::LcmAlignment(SkTo<uint32_t>(align), SkTo<uint32_t>(stride));
    fRunningLCM = BufferAligner::LcmAlignment(fRunningLCM, align32);
}

uint32_t StorageContext::appendVertices(const void* data,
                                        size_t count,
                                        size_t stride,
                                        size_t align) {
    SkASSERT(data && count > 0 && stride > 0 && align > 0);

    uint32_t align32 = BufferAligner::LcmAlignment(SkTo<uint32_t>(align), SkTo<uint32_t>(stride));
    SkASSERT(fRunningLCM % align32 == 0);

    uint32_t requiredBytes =
            BufferAligner::ValidateCountAndStride(count, stride, /*headroom=*/0, align32);
    if (requiredBytes == 0) {
        return 0;
    }

    uint32_t alignedVertOffset = SkAlignNonPow2(static_cast<uint32_t>(fVertexData.size()), align32);
    if (alignedVertOffset > static_cast<uint32_t>(fVertexData.size())) {
        int padBytes = alignedVertOffset - fVertexData.size();
        memset(fVertexData.append(padBytes), 0, padBytes);
    }

    char* dst = fVertexData.append(requiredBytes);
    memcpy(dst, data, requiredBytes);

    uint32_t totalOffset = SkTo<uint32_t>(fGradientCache.fGradientDataSize) + alignedVertOffset;
    return totalOffset;
}

void StorageContext::finalizePrecachedStorageData() {
    fGradientCache.fGradientDataSize =
            SkAlignNonPow2<size_t>(fGradientCache.fGradientDataSize, fRunningLCM);
    SkDEBUGCODE(fGradientsFinalized = true;)
}

BindBufferInfo StorageContext::finalize(DrawBufferManager* bufferMgr,
                                        Recorder* recorder,
                                        DrawPass* drawPass) {
    if (this->isEmpty()) {
        SkDEBUGCODE(fGradientsFinalized = false;)
        return {};
    }

    if (recorder && !recorder->priv().caps()->storageBufferSupport()) {
        if (drawPass) {
            TextureProxyView proxyView = this->finalizeTexture(recorder, drawPass);
            if (proxyView) {
                drawPass->fStorageTextureProxy = proxyView.refProxy();
            }
        }
        SkDEBUGCODE(fGradientsFinalized = false;)
        return {};
    }

    return this->finalizeSSBO(bufferMgr);
}

BindBufferInfo StorageContext::finalizeSSBO(DrawBufferManager* bufferMgr) {
    SkASSERT(fGradientsFinalized);
    size_t totalBytes = fGradientCache.fGradientDataSize + fVertexData.size_bytes();

    BindBufferInfo result;
    if (totalBytes > 0 && bufferMgr) {
        auto [writer, bufferInfo, _] = bufferMgr->getMappedStorageBuffer(totalBytes, /*stride=*/1);
        if (writer) {
            if (!fGradientCache.isEmpty()) {
                writer.write(fGradientCache.fGradientData.data(),
                             fGradientCache.fGradientData.size_bytes());
                if (fGradientCache.fGradientDataSize > fGradientCache.fGradientData.size_bytes()) {
                    writer.zeroBytes(fGradientCache.fGradientDataSize -
                                     fGradientCache.fGradientData.size_bytes());
                }
            }
            if (!fVertexData.empty()) {
                writer.write(fVertexData.data(), fVertexData.size_bytes());
            }

            result = bufferInfo;
        }
    }

    SkDEBUGCODE(fGradientsFinalized = false;)
    return result;
}

sk_sp<TextureProxy> StorageContext::createTextureProxy(Recorder* recorder, int width, int height) {
    const Caps* caps = recorder->priv().caps();
    TextureInfo info = caps->getDefaultReadableTextureInfo(
            kRGBA_F32_SkColorType,
            recorder->priv().isProtected());

    sk_sp<TextureProxy> proxy = TextureProxy::Make(caps,
                                                   recorder->priv().resourceProvider(),
                                                   SkISize::Make(width, height),
                                                   info,
                                                   "StorageContextFallbackTexture",
                                                   Budgeted::kYes);
    if (proxy) {
        proxy->instantiate(recorder->priv().resourceProvider());
    }
    return proxy;
}

TextureProxyView StorageContext::finalizeTexture(Recorder* recorder, DrawPass* drawPass) {
    if (this->isEmpty() || !recorder) {
        return {};
    }

    size_t gradSize = fGradientCache.fGradientDataSize;
    size_t vertSize = SkAlignTo<size_t>(fVertexData.size_bytes(), StorageContext::kStructAlignment);
    size_t totalBytes = gradSize + vertSize;

    if (totalBytes == 0) {
        return {};
    }

    if (drawPass && gradSize > 0) {
        // Polyfill: gradients write to StorageContext for fragment shader evaluation.
        drawPass->fStorageBufferStages |= PipelineStageFlags::kFragmentShader;
    }

    SkTDArray<char> uploadBuffer;
    uploadBuffer.resize(totalBytes);

    if (gradSize > 0) {
        memcpy(uploadBuffer.data(), fGradientCache.fGradientData.data(), fGradientCache.fGradientData.size_bytes());
        if (gradSize > fGradientCache.fGradientData.size_bytes()) {
            memset(uploadBuffer.data() + fGradientCache.fGradientData.size_bytes(), 0,
                   gradSize - fGradientCache.fGradientData.size_bytes());
        }
    }

    if (!fVertexData.empty()) {
        memcpy(uploadBuffer.data() + gradSize, fVertexData.data(), fVertexData.size_bytes());
        size_t paddingBytes = vertSize - fVertexData.size_bytes();
        if (paddingBytes > 0) {
            memset(uploadBuffer.data() + gradSize + fVertexData.size_bytes(), 0, paddingBytes);
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

    UploadInstance uploadInstance = UploadInstance::Make(recorder, source, nullptr);
    if (!uploadInstance.isValid()) {
        return {};
    }

    sk_sp<Task> uploadTask = UploadTask::Make(std::move(uploadInstance));
    if (!uploadTask) {
        return {};
    }

    if (drawPass) {
        drawPass->fTextureFallbackUploadTask = std::move(uploadTask);
    }

    SkDEBUGCODE(fGradientsFinalized = false;)
    return proxyView;
}

}  // namespace skgpu::graphite
