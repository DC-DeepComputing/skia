/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/gpu/graphite/Recording.h"

#include "src/gpu/graphite/CommandBuffer.h"
#include "src/gpu/graphite/RecordingPriv.h"
#include "src/gpu/graphite/Resource.h"
#include "src/gpu/graphite/ResourceProvider.h"
#include "src/gpu/graphite/TaskGraph.h"
#include "src/gpu/graphite/Texture.h"
#include "src/gpu/graphite/TextureProxy.h"

namespace skgpu::graphite {

Recording::Recording(std::unique_ptr<TaskGraph> graph,
                     std::unordered_set<TextureProxy*>&& volatileProxies)
        : fGraph(std::move(graph))
        , fVolatileProxies(std::move(volatileProxies)) {

    for (auto iter = fVolatileProxies.begin(); iter != fVolatileProxies.end(); ++iter) {
        if ((*iter)->texture()) {
            (*iter)->texture()->printRefs();
        }
        SkDebugf("\n");
    }
}

Recording::~Recording() {

    for (auto iter = fVolatileProxies.begin(); iter != fVolatileProxies.end(); ++iter) {
        if ((*iter)->texture()) {
            (*iter)->texture()->printRefs();
        }
        SkDebugf("\n");
    }
}

////////////////////////////////////////////////////////////////////////////////

void RecordingPriv::instantiateVolatileProxies(ResourceProvider* resourceProvider) {

    for (TextureProxy* proxy : fRecording->fVolatileProxies) {
        proxy->volatileInstantiate(resourceProvider);
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
