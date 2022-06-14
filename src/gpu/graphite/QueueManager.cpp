/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/QueueManager.h"

#include "src/gpu/graphite/CommandBuffer.h"

namespace skgpu::graphite {

CommandBuffer* QueueManager::currentCommandBuffer() {
    if (!fCurrentCommandBuffer) {
        fCurrentCommandBuffer = this->getNewCommandBuffer();
    }
    return fCurrentCommandBuffer.getNewCommandBuffer();
}

} // namespace skgpu::graphite
