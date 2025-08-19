/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_NewObjects_DEFINED
#define skgpu_graphite_NewObjects_DEFINED

#include "src/base/SkSpinlock.h"
#include "src/core/SkTHash.h"
#include "src/gpu/graphite/GraphicsPipeline.h"
#include "src/gpu/graphite/GraphicsPipelineDesc.h"
#include "src/gpu/graphite/RenderPassDesc.h"


class SkExecutor;
class SkTaskGroup;

namespace skgpu::graphite {

class ResourceProvider;
class RuntimeEffectDictionary;

/*
 * Here is the plan:
 *    The handle will be ref counted
 *    While the PipelineMgr is actively tracking a Pipeline's compilation it will return the
 *        same handle (from its active list)
 *    When the Pipeline's compilation finishes, the Mgr will be notified and it will update
 *        the single Handle and remove it from its active list (i.e., drop its ref)
 *    The Handles that have been distributed already will lock the Pipeline in the Pipeline cache
 *    If a new compilation request for the same pipeline arrives it will cause the allocation
 *        of a new handle that just wraps the existing pipeline (since it doesn't need compilation)
 *        and won't be added to the Mgr's active list
 *    This means that while the Pipeline is being compiled there will be a 1-to-1 mapping
 *        from handle to Pipeline but, once compilation is complete, there will be a many-to-one
 *        mapping.
 *    The ref count will be useful during the compilation phase but almost pure overhead once
 *        the pipeline is compiled. Perhaps there is a better system. The problem is that:
 *           we only want to signal completion on one handle (-> refCounting)
 *           we want to hand handles out to lots of folks
 *           we don't want the Mgr to always have a Handle - since that complicates Pipeline purging
 *    Having the handles be block allocated could reduce the (allocation) cost of this plan
 *
 * Alternatively, I have considered just handing out a new Handle for each usage. This doesn't
 * require a ref count but makes signalling completion more complicated.
 *
 * Another alternative would be for the handle to hold all the info needed for compilation (i.e.,
 * a GraphicsPipelineDesc/RenderPassDesc). The DrawPass could then hold them, just as it currently
 * holds GraphicsPipelineDesc/RenderPassDesc's, and then trade them in for Pipelines in ZZZ.
 *
 * Yet another alternative is for the handle to have both a UniqueKey for the Pipeline and an sk_sp
 * of the Pipeline (which could be null). The handle
 * can then be "traded-in" for the real Pipeline when it is actually needed. So, in this plan,
 * the DrawPass would just only ever hold GraphicsPipelineHandles (and not GraphicsPipelineDescs
 * nor GraphicsPipelines). The Mgr would have a map of activeTasks (indexed by the UniqueKey)
 * while the PipelineCache would still have a map of Pipelines (also indexed by the UniqueKey).
 * Upon a request the Mgr would check in the PipelineCache, if it was there it returns a "completed"
 * Handle in order to lock the Pipeline in the cache. If isn't there it creates a task to build
 * it and returns a "partial" Handle. Once compiled, the Pipeline will be entered in the Pipeline
 * cache and wait there to be requested.
 * At "trade-in" time the Mgr:
 *    returns the Pipeline if the handle is "completed"
 *    waits on the Task if there is an extant one (can check there isn't a Pipeline in the cache)
 *    returns the Pipeline if the Pipeline is in the PipelineCache
 *    fires off a new Task if there isn't one already (and marks a bug!)
 *       - I guess this could happen since we aren't locking "partial" Handle Pipelines in the cache
 */
class GraphicsPipelineHandle {
public:
    bool isReady() const { return fIsReady; }
    bool failed() const { return fFailed; }

    // Can this be moved to the Mgr?
    sk_sp<GraphicsPipeline> wait() {
        SkASSERT(fIsReady);
        return fPipeline;
    }

private:
    friend class Compilinator;     // for update1
    friend class PipelineManager;  // for ctor

    GraphicsPipelineHandle(sk_sp<GraphicsPipeline> pipeline)
            : fPipeline(std::move(pipeline))
            , fIsReady(SkToBool(fPipeline)) {}

    GraphicsPipelineHandle(const UniqueKey& pipelineKey) : fPipelineKey(pipelineKey) {}

    void update1(sk_sp<GraphicsPipeline> pipeline) {
        SkASSERT(!fIsReady);
        fPipeline = std::move(pipeline);
        fIsReady = true;
    }

    void markFailed() {
        fFailed = true;
    }

    UniqueKey fPipelineKey;
    sk_sp<GraphicsPipeline> fPipeline;

    bool fIsReady = false;
    bool fFailed = false;
};

class Compilinator {
public:
    Compilinator(SkExecutor*);
    ~Compilinator();

    void compilinate(ResourceProvider*,
                     const RuntimeEffectDictionary*,
                     GraphicsPipelineHandle*,
                     const GraphicsPipelineDesc&,
                     const RenderPassDesc&,
                     SkEnumBitMask<PipelineCreationFlags>);

private:
    static void ResolveHandle(ResourceProvider* resourceProvider,
                              const RuntimeEffectDictionary* runtimeDict,
                              GraphicsPipelineHandle* handle,
                              const GraphicsPipelineDesc& pipelineDesc,
                              const RenderPassDesc& renderPassDesc);

    // fTaskGroup must appear before anything that uses it (e.g. fGpu), so that it is destroyed
    // after all of its users. Clients of fTaskGroup will generally want to ensure that they call
    // wait() on it as they are being destroyed, to avoid the possibility of pending tasks being
    // invoked after objects they depend upon have already been destroyed.
    std::unique_ptr<SkTaskGroup> fTaskGroup;
};

class PipelineManager {
public:
    PipelineManager();
    ~PipelineManager();

    std::unique_ptr<GraphicsPipelineHandle> createHandle(
            ResourceProvider*,
            const GraphicsPipelineDesc& pipelineDesc,
            const RenderPassDesc& renderPassDesc,
            SkEnumBitMask<PipelineCreationFlags>) SK_EXCLUDES(fSpinLock);

    sk_sp<GraphicsPipeline> tradeIn(ResourceProvider*,
                                    GraphicsPipelineHandle*) SK_EXCLUDES(fSpinLock);

private:
    mutable SkSpinlock fSpinLock;

    // Get a compilation going
    void addTask(const UniqueKey& pipelineKey,
                 const GraphicsPipelineDesc&,
                 const RenderPassDesc&,
                 SkEnumBitMask<PipelineCreationFlags>) SK_EXCLUDES(fSpinLock);

    void waitOnTask(const UniqueKey& pipelineKey) SK_EXCLUDES(fSpinLock);

    void taskFinished(const UniqueKey& pipelineKey,
                      sk_sp<GraphicsPipeline>) SK_EXCLUDES(fSpinLock);

    struct Task1 {
        Task1(const GraphicsPipelineDesc& graphicsPipelineDesc,
              const RenderPassDesc& renderPassDesc,
              SkEnumBitMask<PipelineCreationFlags> pipelineCreationFlags)
                  : fGraphicsPipelineDesc(graphicsPipelineDesc)
                  , fRenderPassDesc(renderPassDesc)
                  , fPipelineCreationFlags(pipelineCreationFlags) {}

        void signalDone() {}

        GraphicsPipelineDesc fGraphicsPipelineDesc;
        RenderPassDesc fRenderPassDesc;
        SkEnumBitMask<PipelineCreationFlags> fPipelineCreationFlags;
    };

    struct KeyHash {
        uint32_t operator()(const UniqueKey& k) { return k.hash(); }
    };
    using TaskMap = skia_private::THashMap<UniqueKey, Task1*, KeyHash>;

    TaskMap fActiveTasks SK_GUARDED_BY(fSpinLock);
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_NewObjects_DEFINED
