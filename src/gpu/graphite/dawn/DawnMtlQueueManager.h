/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_DawnMtlQueueManager_DEFINED
#define skgpu_graphite_DawnMtlQueueManager_DEFINED

#include "src/gpu/graphite/QueueManager.h"

#include "webgpu/webgpu_cpp.h"  // NO_G3_REWRITE

#include "src/gpu/graphite/mtl/MtlQueueManager.h"

namespace skgpu::graphite {

class DawnMtlSharedContext;
class SharedContext;

class DawnMtlQueueManager : public MtlQueueManager {
public:
    DawnMtlQueueManager(sk_cfp<id<MTLCommandQueue>> queue, const SharedContext*);
    ~DawnMtlQueueManager() override {}

private:
    OutstandingSubmission onSubmitToGpu(const SubmitInfo&) override;

    const DawnMtlSharedContext* dawnMtlSharedContext() const;
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_DawnMtlQueueManager_DEFINED

