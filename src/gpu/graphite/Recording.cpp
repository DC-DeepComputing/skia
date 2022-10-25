/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/gpu/graphite/Recording.h"

#include "src/gpu/RefCntedCallback.h"
#include "src/gpu/graphite/CommandBuffer.h"
#include "src/gpu/graphite/RecordingPriv.h"
#include "src/gpu/graphite/Resource.h"
#include "src/gpu/graphite/ResourceProvider.h"
#include "src/gpu/graphite/TaskGraph.h"
#include "src/gpu/graphite/TextureProxy.h"
#include "src/gpu/graphite/VolatileProxyContext.h"

#include <unordered_set>

namespace skgpu::graphite {

Recording::Recording(std::unique_ptr<TaskGraph> graph,
                     sk_sp<VolatileProxyContext> volatileProxyContext)
        : fGraph(std::move(graph))
        , fVolatileProxyContext(std::move(volatileProxyContext)) {
}

Recording::~Recording() {}

////////////////////////////////////////////////////////////////////////////////
bool RecordingPriv::hasVolatileProxies() const {
    return fRecording->fVolatileProxyContext && !fRecording->fVolatileProxyContext->empty();
}

std::tuple<bool, sk_sp<RefCntedCallback>> RecordingPriv::instantiateVolatileProxies(
        ResourceProvider* resourceProvider) {
    SkASSERT(this->hasVolatileProxies());

#if 0
    fRecording->fVolatileProxyContext->ref(); // the release callback gets an implicit ref
    sk_sp<RefCntedCallback> callback =
            RefCntedCallback::Make(VolatileProxyContext::Release,
                                   fRecording->fVolatileProxyContext.get());
#else
    sk_sp<RefCntedCallback> callback;
#endif

    for (auto proxy : fRecording->fVolatileProxyContext->fVolatileProxies) {
        if (!proxy->volatileInstantiate(resourceProvider)) {
            return { false, std::move(callback) };
        }
    }

    return { true, std::move(callback) };
}

void RecordingPriv::deinstantiateVolatileProxies() {
    SkASSERT(this->hasVolatileProxies());

    for (auto proxy : fRecording->fVolatileProxyContext->fVolatileProxies) {
        SkASSERT(proxy->isVolatile());
        proxy->deinstantiate();
    }
}

bool RecordingPriv::addCommands(ResourceProvider* resourceProvider, CommandBuffer* commandBuffer) {
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
