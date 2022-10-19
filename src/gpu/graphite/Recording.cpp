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

namespace skgpu::graphite {

Recording::Recording(std::unique_ptr<TaskGraph> graph,
                     std::unordered_set<TextureProxy*>&& volatileProxies)
        : fGraph(std::move(graph))
        , fVolatileProxies(std::move(volatileProxies)) {
}

Recording::~Recording() {}

////////////////////////////////////////////////////////////////////////////////

bool RecordingPriv::hasVolatileProxies() const {
    return !fRecording->fVolatileProxies.empty();
}

struct VolatileContext {
    std::vector<TextureProxy*> fVolatileProxies;
};

static void release_volatile_proxies(void* context) {
    VolatileContext* volatileContext = reinterpret_cast<VolatileContext*>(context);

    for (auto proxy : volatileContext->fVolatileProxies) {
        proxy->deinstantiate();
    }

    delete volatileContext;
}

std::tuple<bool, sk_sp<RefCntedCallback>> RecordingPriv::instantiateVolatileProxies(
        ResourceProvider* resourceProvider) {

    if (fRecording->fVolatileProxies.empty()) {
        return { true, nullptr };
    }

    VolatileContext* volatileContext = new VolatileContext;
    sk_sp<RefCntedCallback> callback = RefCntedCallback::Make(release_volatile_proxies,
                                                              volatileContext);

    for (TextureProxy* proxy : fRecording->fVolatileProxies) {
        if (!proxy->volatileInstantiate(resourceProvider)) {
            return { false, std::move(callback) };
        }

        volatileContext->fVolatileProxies.push_back(proxy);
    }

    return { true, std::move(callback) };
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
