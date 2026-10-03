/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/ComputeScheduler.h"

#include "include/gpu/graphite/Recorder.h"
#include "include/private/SkLog.h"
#include "src/gpu/Swizzle.h"
#include "src/gpu/graphite/DrawContext.h"
#include "src/gpu/graphite/Image_Base_Graphite.h"
#include "src/gpu/graphite/Image_Graphite.h"
#include "src/gpu/graphite/RecorderPriv.h"
#include "src/gpu/graphite/ScratchResourceManager.h"
#include "src/gpu/graphite/TextureProxy.h"
#include "src/gpu/graphite/compute/DispatchGroup.h"
#include "src/gpu/graphite/task/ComputeTask.h"
#include "src/gpu/graphite/task/Task.h"

#include <functional>
#include <utility>

namespace skgpu::graphite {

/**
 * What a reader of a ComputeOutput records into its DependencySink, like the second reference to a
 * scratch DrawTask. It marks the read in the reader's scratch scope, so the output's texture can
 * be returned for reuse once the reader has executed.
 */
class ComputeOutputUseTask final : public Task {
public:
    explicit ComputeOutputUseTask(sk_sp<ComputeOutput> output) : fOutput(std::move(output)) {}

    Status prepareResources(ResourceProvider*,
                            ScratchResourceManager* scratchManager,
                            sk_sp<const RuntimeEffectDictionary>) override {
        // A batch is drained before any reader of its outputs is added to the task graph (T1, T4),
        // so the ComputeTask is ahead of this task in the graph. It owns the listener that's marked
        // here, which is why this task can be discarded right away.
        if (!fOutput->fTask) {
            SkDEBUGFAIL("A compute output was read before the batch that writes it was drained");
            SKIA_LOG_E("Compute output is read before it's written. Dropping Recording!");
            return Status::kFail;
        }
        fOutput->fTask->markOutputInUse(scratchManager, fOutput->fOutputIndex);
        return Status::kDiscard;
    }

    Status addCommands(Context*, CommandBuffer*, ReplayTargetData) override {
        return Status::kDiscard;
    }

    bool visitProxies(const std::function<bool(const TextureProxy*)>& visitor,
                      bool /*readsOnly*/) override {
        return visitor(fOutput->fProxy.get());
    }

    SK_DUMP_TASKS_CODE(const char* getTaskName() const override { return "Compute Output Use"; })

private:
    sk_sp<ComputeOutput> fOutput;
};

/**************************************************************************************************/

ComputeOutput::ComputeOutput(uint32_t recorderID, sk_sp<TextureProxy> proxy)
        : fRecorderID(recorderID), fProxy(std::move(proxy)) {
    SkASSERT(fProxy);
}

ComputeOutput::~ComputeOutput() = default;

void ComputeOutput::recordRead(Recorder* recorder, DependencySink* sink) {
    SkASSERT(sink);
    recorder->priv().addPendingRead(fProxy.get());
    sink->recordDependency(sk_make_sp<ComputeOutputUseTask>(sk_ref_sp(this)));
}

bool ComputeOutput::notifyInUse(Recorder* recorder, DependencySink* sink) {
    // Like reads of a non-scratch Device from another Recorder, reads from another Recorder aren't
    // ordered with the dispatch. They see its result if its Recording is inserted first.
    if (recorder->priv().uniqueID() != fRecorderID) {
        return false;
    }
    ComputeScheduler* scheduler = recorder->priv().computeScheduler();

    if (!fTask) {
        // The dispatch is still queued.
        if (sink == scheduler) {
            // Read by a dispatch that's queued after this one, and so is in the same batch. The
            // dispatches in a batch execute in order, with a barrier between each of them.
            return false;
        }
        if (sink) {
            // T1: the batch is drained before `sink`'s work is added to the task graph.
            this->recordRead(recorder, sink);
            scheduler->fWaitingSinks.add(sink);
            return false;
        }
        // T4: a read without a sink is recorded directly to the root task list, so the batch has
        // to be added to it first.
        scheduler->drain();
        SkASSERT(fTask);
    }

    if (fRecordingID != scheduler->fRecordingID || fProxy->isInstantiated()) {
        // Either the batch was drained into an earlier Recording, which gave the output a regular
        // texture (or failed), or Image_Base::makeNonBudgeted() gave it a regular texture. The
        // ComputeTask is ahead of any later read in the task graph, and the texture is never
        // returned to the scratch pool, so later reads don't need to be tracked.
        return true;
    }
    if (!sink) {
        // Only RenderPassTask consumes the reads that are marked in a scratch scope, so reads that
        // are recorded directly to the root task list (e.g. copies) can't mark theirs. Instead the
        // output is never returned to the scratch pool, which means later reads don't need to be
        // tracked either.
        fTask->disableScratchOutput(fOutputIndex);
        return true;
    }
    // Read by a DrawContext, or by a dispatch in a later batch.
    this->recordRead(recorder, sink);
    return false;
}

/**************************************************************************************************/

ComputeScheduler::ComputeScheduler(Recorder* recorder, std::optional<size_t> batchBudget)
        : fRecorder(recorder), fBatchBudget(batchBudget) {
    SkASSERT(fRecorder);
}

// Dispatches that are still queued are dropped, like the root tasks of a Recorder that's deleted
// before it's snapped. Their outputs stay linked to their Images, but they can't be read by any
// other Recorder (see ComputeOutput::notifyInUse()).
ComputeScheduler::~ComputeScheduler() = default;

size_t ComputeScheduler::batchBudget() const {
    // A fraction of the resource budget leaves most of it for the rest of the Recording, and
    // keeps the memory that batching adds proportional to what the client allows.
    return fBatchBudget.has_value()
                   ? *fBatchBudget
                   : fRecorder->priv().getResourceCacheLimit() / kDefaultBudgetDivisor;
}

size_t ComputeScheduler::sourceCost(const TextureProxy* source) const {
    // Batching only extends the lifetime of scratch sources (the same test as
    // Device::isScratchDevice()). Other sources are instantiated for at least the whole Recording.
    if (source->isInstantiated() || source->isLazy() || fQueuedSources.contains(source)) {
        return 0;
    }
    return source->uninstantiatedGpuMemorySize();
}

sk_sp<Image> ComputeScheduler::enqueue(const Dispatch& dispatch) {
    SkASSERT(dispatch.fStep && dispatch.fSource && dispatch.fParams);
    SkASSERT(dispatch.fSource->textureProxyViews().size() == 1);
    SkASSERT(dispatch.fSource->textureProxyViews()[0].swizzle() == Swizzle::RGBA());
    SkASSERT(dispatch.fOutput.proxy());
    SkASSERT(!dispatch.fOutput.proxy()->isInstantiated() && !dispatch.fOutput.proxy()->isLazy());
    SkASSERT(!fIsEnqueuing && fIncomingDependencies.empty());

    TextureProxy* source = dispatch.fSource->textureProxyViews()[0].proxy();

    // T5: drain the batch if reading another scratch source would put it over budget. A dispatch
    // whose source alone is over budget becomes a batch of one.
    const size_t budget = this->batchBudget();
    const size_t cost = this->sourceCost(source);
    if (!fQueue.empty() && (fQueuedSourceBytes > budget || cost > budget - fQueuedSourceBytes)) {
        this->drain();
    }

    // The batch reads the source like a DrawContext does: a scratch source records the DrawTask
    // that renders it as a dependency, and a non-scratch source flushes its pending work to the
    // root task list. That flush can drain the batch (T1, T2), but this dispatch is only queued
    // afterwards, which is fine since the dispatch has no readers yet.
    SkDEBUGCODE(fIsEnqueuing = true;)
    dispatch.fSource->notifyInUse(fRecorder, this);
    SkDEBUGCODE(fIsEnqueuing = false;)

    fQueuedSourceBytes += this->sourceCost(source);
    fQueuedSources.add(source);
    fDependencies.move_back(fIncomingDependencies);

    sk_sp<ComputeOutput> output(
            new ComputeOutput(fRecorder->priv().uniqueID(), dispatch.fOutput.refProxy()));
    fQueue.push_back({dispatch.fStep, sk_ref_sp(source), dispatch.fParams, dispatch.fGlobalSize,
                      output});
    sk_sp<Image> image = Image::WrapComputeOutput(
            std::move(output), dispatch.fOutput, dispatch.fOutputColorInfo);

    if (budget == 0) {
        this->drain();
    }
    return image;
}

void ComputeScheduler::flushIfNeeded(const DrawContext* drawContext) {
    SkASSERT(drawContext);
    if (fQueue.empty()) {
        return;
    }
    // T1 covers every DrawContext that reads a queued output. T2 only applies to pending work that
    // writes the target: draws, and uploads such as writePixels() (which don't set
    // modifiesTarget()). Reads of queued outputs are pending work too, but those are covered by T1.
    if (fWaitingSinks.contains(drawContext) ||
        (drawContext->hasPendingWork() && fQueuedSources.contains(drawContext->target().proxy()))) {
        this->drain();
    }
}

void ComputeScheduler::drain() {
    if (fQueue.empty()) {
        SkASSERT(fDependencies.empty());
        return;
    }
    // A drain only adds a task to the root task list. If it could flush a Device, it could
    // re-enter itself through Device::flushPendingWork().
    SkASSERT(!fIsDraining);
    SkDEBUGCODE(fIsDraining = true;)

    // Each dispatch rebinds all three shared slots, so one DispatchGroup holds the whole batch and
    // consecutive dispatches of the same step share a pipeline.
    DispatchGroup::Builder builder(fRecorder);
    skia_private::TArray<sk_sp<TextureProxy>> outputs;
    outputs.reserve_exact(fQueue.size());
    for (const QueuedDispatch& dispatch : fQueue) {
        builder.assignSharedTexture(dispatch.fSource, kSourceSlot);
        builder.assignSharedTexture(dispatch.fOutput->fProxy, kOutputSlot);
        builder.assignSharedBuffer(dispatch.fParams, kParamsSlot);
        if (!builder.appendStep(dispatch.fStep, dispatch.fGlobalSize)) {
            // Only possible if the step exceeds the limits that Dispatch::fStep requires. The output
            // is still instantiated below, so its reads remain valid, but its contents are
            // undefined.
            SkDEBUGFAIL("Failed to append a queued ComputeStep");
        }
        outputs.push_back(dispatch.fOutput->fProxy);
    }

    ComputeTask::DispatchGroupList groups;
    groups.push_back(builder.finalize());
    sk_sp<ComputeTask> task =
            ComputeTask::Make(std::move(groups), std::move(fDependencies), std::move(outputs));
    for (int i = 0; i < fQueue.size(); ++i) {
        ComputeOutput* output = fQueue[i].fOutput.get();
        output->fTask = task;
        output->fOutputIndex = i;
        output->fRecordingID = fRecordingID;
    }
    fRecorder->priv().add(std::move(task));

    fQueue.clear();
    fDependencies.clear();
    fWaitingSinks.reset();
    fQueuedSources.reset();
    fQueuedSourceBytes = 0;
#if defined(GPU_TEST_UTILS)
    fDrainedBatchCount++;
#endif
    SkDEBUGCODE(fIsDraining = false;)
}

void ComputeScheduler::snap() {
    this->drain();
    fRecordingID++;
}

void ComputeScheduler::recordDependency(sk_sp<Task> task) {
    SkASSERT(task);
    // Only sources notify the scheduler, and only while their dispatch is queued.
    SkASSERT(fIsEnqueuing);
    fIncomingDependencies.push_back(std::move(task));
}

}  // namespace skgpu::graphite
