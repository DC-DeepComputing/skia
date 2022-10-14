/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/gpu/graphite/Recording.h"

#include "src/gpu/graphite/CommandBuffer.h"
#include "src/gpu/graphite/Log.h"
#include "src/gpu/graphite/RecordingPriv.h"
#include "src/gpu/graphite/Resource.h"
#include "src/gpu/graphite/ResourceProvider.h"
#include "src/gpu/graphite/Surface_Graphite.h"
#include "src/gpu/graphite/TaskGraph.h"
#include "src/gpu/graphite/Texture.h"
#include "src/gpu/graphite/TextureProxy.h"

namespace skgpu::graphite {

Recording::Recording(std::unique_ptr<TaskGraph> graph, sk_sp<TextureProxy> targetProxy)
        : fGraph(std::move(graph)), fTargetProxy(targetProxy) {}

Recording::~Recording() {}

#if GRAPHITE_TEST_UTILS
bool Recording::isTargetProxyInstantiated() const { return fTargetProxy->isInstantiated(); }
#endif

////////////////////////////////////////////////////////////////////////////////

bool RecordingPriv::addCommands(ResourceProvider* resourceProvider,
                                CommandBuffer* commandBuffer,
                                Surface* surface) {
    AutoUninstantiateTextureProxy targetProxy(fRecording->fTargetProxy.get());
    SkASSERT(SkToBool(fRecording->fTargetProxy) == SkToBool(surface));
    if (fRecording->fTargetProxy) {
        if (!surface) {
            SKGPU_LOG_E("No surface provided to instantiate Recording's texture proxy.");
            return false;
        }
        TextureProxy* backingProxy = surface->backingTextureProxy();
        if (!backingProxy->instantiate(resourceProvider)) {
            SKGPU_LOG_E("Could not instantiate target texture proxy.");
            return false;
        }
        targetProxy.instantiate(backingProxy->refTexture());
    }

    if (!fRecording->fGraph->addCommands(resourceProvider, commandBuffer)) {
        return false;
    }
    for (size_t i = 0; i < fRecording->fExtraResourceRefs.size(); ++i) {
        commandBuffer->trackResource(fRecording->fExtraResourceRefs[i]);
    }
    return true;
}

void RecordingPriv::addResourceRef(sk_sp<Resource> resource) {
    fRecording->fExtraResourceRefs.push_back(std::move(resource));
}

} // namespace skgpu::graphite
