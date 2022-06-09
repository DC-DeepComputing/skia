/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/RecordingPriv.h"

#include "src/gpu/graphite/CommandBuffer.h"
#include "src/gpu/graphite/Resource.h"
#include "src/gpu/graphite/ResourceProvider.h"
#include "src/gpu/graphite/TaskGraph.h"

namespace skgpu::graphite {


bool RecordingPriv::addCommands(CommandBuffer* commandBuffer, ResourceProvider* resourceProvider) {
    return fRecording->fGraph->addCommands(resourceProvider, commandBuffer);
}

void RecordingPriv::addResourceRef(sk_sp<Resource> resource) {
    fRecording->fExtraResourceRefs.push_back(std::move(resource));
}


} // namespace skgpu::graphite


