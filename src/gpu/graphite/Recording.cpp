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

#include <unordered_set>

namespace skgpu::graphite {

Recording::Recording(std::unique_ptr<TaskGraph> graph,
                     std::unordered_set<sk_sp<TextureProxy>, ProxyHash>&& nonVolatileLazyProxies,
                     std::unordered_set<sk_sp<TextureProxy>, ProxyHash>&& volatileLazyProxies,
                     sk_sp<TextureProxy> targetProxy,
                     std::unique_ptr<LazyProxyData> targetProxyData)
        : fGraph(std::move(graph))
        , fNonVolatileLazyProxies(std::move(nonVolatileLazyProxies))
        , fVolatileLazyProxies(std::move(volatileLazyProxies))
        , fTargetProxy(std::move(targetProxy))
        , fTargetProxyData(std::move(targetProxyData)) {
    SkASSERT(SkToBool(fTargetProxy) == SkToBool(fTargetProxyData));
}

Recording::~Recording() {}

#if GRAPHITE_TEST_UTILS
bool Recording::isTargetProxyInstantiated() const { return fTargetProxy->isInstantiated(); }
#endif

////////////////////////////////////////////////////////////////////////////////
bool RecordingPriv::hasNonVolatileLazyProxies() const {
    return !fRecording->fNonVolatileLazyProxies.empty();
}

bool RecordingPriv::instantiateNonVolatileLazyProxies(ResourceProvider* resourceProvider) {
    SkASSERT(this->hasNonVolatileLazyProxies());

    for (auto proxy : fRecording->fNonVolatileLazyProxies) {
        if (!proxy->lazyInstantiate(resourceProvider)) {
            return false;
        }
    }

    // Note: once all the lazy proxies have been instantiated, that's it - there are no more
    // chances to instantiate.
    fRecording->fNonVolatileLazyProxies.clear();
    return true;
}

bool RecordingPriv::hasVolatileLazyProxies() const {
    return !fRecording->fVolatileLazyProxies.empty();
}

bool RecordingPriv::instantiateVolatileLazyProxies(ResourceProvider* resourceProvider) {
    SkASSERT(this->hasVolatileLazyProxies());

    for (auto proxy : fRecording->fVolatileLazyProxies) {
        if (!proxy->lazyInstantiate(resourceProvider)) {
            return false;
        }
    }

    return true;
}

void RecordingPriv::deinstantiateVolatileLazyProxies() {
    if (!this->hasVolatileLazyProxies()) {
        return;
    }

    for (auto proxy : fRecording->fVolatileLazyProxies) {
        SkASSERT(proxy->isVolatile());
        proxy->deinstantiate();
    }
}

#if GR_TEST_UTILS
int RecordingPriv::numVolatilePromiseImages() const {
    return fRecording->fVolatileLazyProxies.size();
}

int RecordingPriv::numNonVolatilePromiseImages() const {
    return fRecording->fNonVolatileLazyProxies.size();
}
#endif

bool RecordingPriv::addCommands(ResourceProvider* resourceProvider,
                                CommandBuffer* commandBuffer,
                                Surface* targetSurface) {
    AutoDeinstantiateTextureProxy autoDeinstantiateTargetProxy(fRecording->fTargetProxy.get());

    SkASSERT(SkToBool(fRecording->fTargetProxy) == SkToBool(targetSurface));
    if (fRecording->fTargetProxy && fRecording->fTargetProxyData) {
        if (!targetSurface) {
            SKGPU_LOG_E("No surface provided to instantiate target texture proxy.");
            return false;
        }
        TextureProxy* surfaceTexture = targetSurface->backingTextureProxy();
        if (!surfaceTexture->instantiate(resourceProvider)) {
            SKGPU_LOG_E("Could not instantiate target texture proxy.");
            return false;
        }
        // Used by TextureProxy in lazyInstantiate.
        fRecording->fTargetProxyData->fReplayDest = surfaceTexture->refTexture();
        fRecording->fTargetProxy->lazyInstantiate(resourceProvider);
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
