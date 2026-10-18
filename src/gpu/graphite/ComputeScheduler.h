/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_ComputeScheduler_DEFINED
#define skgpu_graphite_ComputeScheduler_DEFINED

#include "include/core/SkImageInfo.h"
#include "include/core/SkRefCnt.h"
#include "include/private/SkTArray.h"
#include "src/core/SkTHash.h"
#include "src/gpu/graphite/ComputeTypes.h"
#include "src/gpu/graphite/ResourceTypes.h"
#include "src/gpu/graphite/TextureProxyView.h"
#include "src/gpu/graphite/task/DependencySink.h"

#include <cstddef>
#include <cstdint>
#include <optional>

namespace skgpu::graphite {

class ComputeStep;
class ComputeTask;
class DrawContext;
class Image;
class Image_Base;
class Recorder;
class Task;
class TextureProxy;

/**
 * ComputeOutput links an Image to the queued dispatch that writes its texture, the same way a
 * scratch Device links an Image to the DrawTask that renders it. Every read of the Image notifies
 * the output (see Image_Base::notifyInUse()), which orders the read after the dispatch and tracks
 * the read so the texture can be reused as a scratch texture once every read has executed.
 */
class ComputeOutput final : public SkRefCnt {
public:
    ~ComputeOutput() override;

    // Called when an Image linked to this output is read by `recorder`, and specifically by the
    // work recorded into `sink`. `sink` is null for reads recorded directly to the root task list,
    // e.g. copies. Returns true if the Image no longer needs to be linked to this output.
    bool notifyInUse(Recorder* recorder, DependencySink* sink);

    const TextureProxy* proxy() const { return fProxy.get(); }

private:
    friend class ComputeOutputUseTask;
    friend class ComputeScheduler;

    ComputeOutput(uint32_t recorderID, sk_sp<TextureProxy>);

    // Records `sink` as a reader of this output, like the second reference to a scratch DrawTask.
    void recordRead(Recorder*, DependencySink* sink);

    // Only these are read before checking that the Recorder is the one that queued the dispatch,
    // since an Image can be read by a Recorder on another thread.
    const uint32_t fRecorderID;
    const sk_sp<TextureProxy> fProxy;

    // Set when the batch that writes the output is drained. They're only accessed on the thread of
    // the Recorder that queued the dispatch.
    sk_sp<ComputeTask> fTask;
    int fOutputIndex = -1;
    uint32_t fRecordingID = 0;
};

/**
 * ComputeScheduler batches compute dispatches that aren't tied to the draws of a RenderStep, such
 * as an image filter that's implemented as a ComputeStep (dispatches that must follow the batching
 * of their draws go through the pass-scoped ComputeManager instead). Each Recorder owns one.
 *
 * Queued dispatches are added to the Recorder's root task list as a single ComputeTask when the
 * batch is drained. A batch waits for as long as it can, which is until:
 *   T1: a DrawContext that reads one of its outputs is flushed (read-after-write);
 *   T2: a DrawContext that writes one of its sources is flushed (write-after-read);
 *   T3: the Recorder snaps, since batches never span Recordings;
 *   T4: one of its outputs is read without a DependencySink, e.g. by a copy;
 *   T5: the scratch sources it holds would exceed the batch budget.
 * T1 and T2 are checked by Device::flushPendingWork() before it snaps any tasks, so a batch is in
 * the root task list before every task that depends on it.
 *
 * Batching keeps every scratch source of a batch alive until the batch executes, rather than
 * letting the next source reuse its texture (outputs are unaffected, since they live until their
 * readers execute anyway). The batch budget caps that extra memory.
 *
 * This relies on the following assumptions, which are also noted where they're used:
 *  - When a dispatch is queued, its sources are produced by tasks that are already in the root task
 *    list, or by earlier dispatches in the queue. This holds while Device::snapSpecial() flushes
 *    scratch devices to the root task list (b/323886870). Once scratch DrawTasks are only nested in
 *    their readers, a batch must execute the DrawTasks it holds as dependencies instead.
 *  - Dispatches aren't indirect, so their global size is known when they're queued.
 *  - Draining never flushes a Device, so it's safe to drain inside Device::flushPendingWork().
 *  - Sources are bound without their read swizzle (see Dispatch::fSource).
 */
class ComputeScheduler final : public DependencySink {
public:
    // The shared slots that a dispatch's resources are assigned to. A step that's queued must
    // declare these resources (all with DataFlow::kShared), and may also declare private ones. The
    // source may be sampled, in which case DispatchGroup allocates its sampler from the step's
    // calculateSamplerParameters().
    static constexpr int kSourceSlot = 0;  // ResourceType::kReadOnlyTexture or kSampledTexture
    static constexpr int kOutputSlot = 1;  // ResourceType::kWriteOnlyStorageTexture
    static constexpr int kParamsSlot = 2;  // ResourceType::kUniformBuffer

    // The default batch budget is this fraction of the Recorder's resource budget.
    static constexpr size_t kDefaultBudgetDivisor = 16;

    struct Dispatch {
        // Must fit within the minimum compute limits in Caps, including static `workgroup`
        // variables, since those aren't checked by DispatchGroup::Builder.
        const ComputeStep* fStep = nullptr;
        // The single-plane image that's read through kSourceSlot. The batch reads it the way a
        // DrawContext does, which flushes or references the work that renders it. Its texture is
        // bound directly and the view's read swizzle is ignored, so the swizzle must be RGBA.
        const Image_Base* fSource = nullptr;
        // Written through kOutputSlot. It must be an uninstantiated, non-lazy proxy, which is
        // instantiated by the ComputeTask that executes the batch.
        TextureProxyView fOutput;
        SkColorInfo fOutputColorInfo;
        // Bound to kParamsSlot. Must be allocated from the Recorder's DrawBufferManager.
        BindBufferInfo fParams;
        WorkgroupSize fGlobalSize;
    };

    // `batchBudget` caps the bytes of the scratch sources that a batch reads. If it isn't set, it's
    // the Recorder's resource budget divided by kDefaultBudgetDivisor. 0 drains every dispatch as
    // soon as it's queued, and SIZE_MAX only drains when a dependency requires it.
    ComputeScheduler(Recorder*, std::optional<size_t> batchBudget);
    ~ComputeScheduler() override;

    // Queues `dispatch` and returns an Image of its output. Reads of the Image are ordered after
    // the dispatch.
    sk_sp<Image> enqueue(const Dispatch& dispatch);

    // Drains the batch if `drawContext` reads one of its outputs (T1) or writes one of its sources
    // (T2). Must be called before `drawContext`'s pending work is snapped.
    void flushIfNeeded(const DrawContext* drawContext);

    // Adds every queued dispatch to the root task list as one ComputeTask.
    void drain();

    // Drains the batch (T3). Called by Recorder::snap() before it takes the root task list.
    void snap();

    // A batch reads its sources like a DrawContext does, so it collects the tasks they depend on.
    // This is only called while a dispatch is being queued.
    void recordDependency(sk_sp<Task>) override;

#if defined(GPU_TEST_UTILS)
    int queuedDispatchCount() const { return fQueue.size(); }
    int drainedBatchCount() const { return fDrainedBatchCount; }
#endif

private:
    friend class ComputeOutput;

    struct QueuedDispatch {
        const ComputeStep* fStep;
        sk_sp<TextureProxy> fSource;
        BindBufferInfo fParams;
        WorkgroupSize fGlobalSize;
        sk_sp<ComputeOutput> fOutput;
    };

    size_t batchBudget() const;
    // The bytes that a batch keeps alive by reading `source`, if it doesn't already read it.
    size_t sourceCost(const TextureProxy* source) const;

    Recorder* const fRecorder;
    const std::optional<size_t> fBatchBudget;

    skia_private::TArray<QueuedDispatch> fQueue;
    // Tasks that execute before the queued dispatches, i.e. second references to the DrawTasks of
    // scratch sources and the reads of outputs from earlier batches.
    skia_private::TArray<sk_sp<Task>> fDependencies;
    // Dependencies recorded while the source of the dispatch being queued is notified. Notifying a
    // source can drain the batch, so these are moved to `fDependencies` once the dispatch is queued.
    skia_private::TArray<sk_sp<Task>> fIncomingDependencies;
    // The DependencySinks that read queued outputs (T1).
    skia_private::THashSet<const DependencySink*> fWaitingSinks;
    // The sources of the queued dispatches (T2), and the bytes of the scratch ones (T5).
    skia_private::THashSet<const TextureProxy*> fQueuedSources;
    size_t fQueuedSourceBytes = 0;

    // Incremented by snap(), so outputs know whether they were drained into the Recording that's
    // currently being recorded.
    uint32_t fRecordingID = 1;

    SkDEBUGCODE(bool fIsEnqueuing = false;)
    SkDEBUGCODE(bool fIsDraining = false;)
#if defined(GPU_TEST_UTILS)
    int fDrainedBatchCount = 0;
#endif
};

}  // namespace skgpu::graphite

#endif  // skgpu_graphite_ComputeScheduler_DEFINED
