/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_PipelineCreationTask_DEFINED
#define skgpu_graphite_PipelineCreationTask_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/private/base/SkSemaphore.h"
#include "src/gpu/graphite/GraphicsPipeline.h"
#include "src/gpu/graphite/GraphicsPipelineDesc.h"
#include "src/gpu/graphite/RenderPassDesc.h"

namespace skgpu::graphite {

// Once completed the PipelineTask will lock the created Pipeline in the cache until the
// PipelineTask is deleted.
class PipelineCreationTask : public SkRefCnt {
private:
    friend class PipelineManager; // for entire API and fPipeline1/fWaited

    PipelineCreationTask(const UniqueKey& pipelineKey,
                         const GraphicsPipelineDesc& graphicsPipelineDesc,
                         const RenderPassDesc& renderPassDesc,
                         SkEnumBitMask<PipelineCreationFlags> pipelineCreationFlags)
            : fPipelineKey(pipelineKey)
            , fGraphicsPipelineDesc(graphicsPipelineDesc)
            , fRenderPassDesc(renderPassDesc)
            , fPipelineCreationFlags(pipelineCreationFlags) {}

    void signalComplete() { fTaskComplete.signal(); }

    void wait1() {
        fTaskComplete.wait();
        SkASSERT(fCompleted1);
    }

    const UniqueKey fPipelineKey;  // used to track this task in the PipelineManager
    const GraphicsPipelineDesc fGraphicsPipelineDesc;
    const RenderPassDesc fRenderPassDesc;
    const SkEnumBitMask<PipelineCreationFlags> fPipelineCreationFlags;

    // Once completed, this task will have filled in 'fPipeline' (if compilation succeeded).
    // This also serves to lock the pipeline in the cache.
    sk_sp<GraphicsPipeline> fPipeline1;

    SkSemaphore fTaskComplete;
    std::atomic<bool> fInProgress = false;
    std::atomic<bool> fCompleted1 = false;
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_PipelineCreationTask_DEFINED
