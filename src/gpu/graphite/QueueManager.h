/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_QueueManager_DEFINED
#define skgpu_graphite_QueueManager_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/private/SkDeque.h"

namespace skgpu::graphite {

class CommandBuffer;
class Gpu;

class QueueManager {
public:
    bool submit(sk_sp<CommandBuffer>);

    CommandBuffer* currentCommandBuffer();

protected:
    QueueManager(Gpu* gpu) : fGpu(gpu) {}

    using OutstandingSubmission = std::unique_ptr<GpuWorkSubmission>;

    Gpu* fGpu;

private:
    virtual sk_sp<CommandBuffer> getNewCommandBuffer()  = 0;
    virtual OutstandingSubmission onSubmit(sk_sp<CommandBuffer>) = 0;

    sk_sp<CommandBuffer> fCurrentCommandBuffer;
    SkDeque fOutstandingSubmissions;
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_QueueManager_DEFINED
