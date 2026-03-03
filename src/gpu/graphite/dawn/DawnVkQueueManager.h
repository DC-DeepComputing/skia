/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_DawnVkQueueManager_DEFINED
#define skgpu_graphite_DawnVkQueueManager_DEFINED

#include "src/gpu/graphite/QueueManager.h"

#include "webgpu/webgpu_cpp.h"  // NO_G3_REWRITE

#include "src/gpu/graphite/vk/VulkanQueueManager.h"

namespace skgpu::graphite {

class DawnVkSharedContext;
class SharedContext;

class DawnVkQueueManager : public VulkanQueueManager {
public:
    DawnVkQueueManager(VkQueue, const SharedContext*);
    ~DawnVkQueueManager() override {}

private:
    OutstandingSubmission onSubmitToGpu(const SubmitInfo&) override;

    const DawnVkSharedContext* dawnVkSharedContext() const;
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_DawnMtlQueueManager_DEFINED

