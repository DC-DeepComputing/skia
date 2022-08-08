/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/piet/PietRenderTask.h"

#include "src/gpu/graphite/Log.h"
#include "src/gpu/graphite/ResourceProvider.h"
#include "src/gpu/graphite/Texture.h"
#include "src/gpu/graphite/TextureProxy.h"
#include "src/gpu/graphite/piet/PietRenderer.h"

namespace skgpu::graphite {

bool PietRenderInstance::prepareResources(ResourceProvider* resourceProvider) {
    if (!fTextureProxy) {
        SKGPU_LOG_E("No texture proxy specified for UploadTask");
        return false;
    }
    if (!fTextureProxy->instantiate(resourceProvider)) {
        SKGPU_LOG_E("Could not instantiate texture proxy for UploadTask!");
        return false;
    }
    return true;
}

bool PietRenderInstance::addCommands(ResourceProvider* resourceProvider,
                                     CommandBuffer* commandBuffer) {
    SkASSERT(fTextureProxy && fTextureProxy->isInstantiated());
    sk_sp<Texture> target = fTextureProxy->refTexture();
    resourceProvider->pietRenderer()->encodeSceneRender(*fScene, target.get(), commandBuffer);
    commandBuffer->trackResource(std::move(target));
    return true;
}

bool PietRenderTask::prepareResources(ResourceProvider* resourceProvider) {
    for (PietRenderInstance& instance : fInstances) {
        if (!instance.prepareResources(resourceProvider)) {
            return false;
        }
    }
    return true;
}

bool PietRenderTask::addCommands(ResourceProvider* resourceProvider, CommandBuffer* commandBuffer) {
    for (PietRenderInstance& instance : fInstances) {
        instance.addCommands(resourceProvider, commandBuffer);
    }
    return true;
}

}  // namespace skgpu::graphite
