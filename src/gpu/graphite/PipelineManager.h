/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_PipelineManager_DEFINED
#define skgpu_graphite_PipelineManager_DEFINED

#include "include/core/SkRefCnt.h"
#include "src/base/SkEnumBitMask.h"
#include "src/base/SkSpinlock.h"
#include "src/core/SkTHash.h"

namespace skgpu {
class UniqueKey;
}

class SkExecutor;
class SkTaskGroup;

namespace skgpu::graphite {

class GraphicsPipeline;
class GraphicsPipelineDesc;
class GraphicsPipelineHandle;
enum class PipelineCreationFlags : uint8_t;
class PipelineCreationTask;
struct RenderPassDesc;
class ResourceProvider;
class RuntimeEffectDictionary;

class PipelineManager {
public:
    PipelineManager(SkExecutor* executor);

    ~PipelineManager();

    GraphicsPipelineHandle createHandle(
            ResourceProvider*,
            const GraphicsPipelineDesc&,
            const RenderPassDesc&,
            SkEnumBitMask<PipelineCreationFlags>);

    void startPipelineCreationTask(ResourceProvider*,
                                   sk_sp<const RuntimeEffectDictionary>,
                                   const GraphicsPipelineHandle&);

    sk_sp<GraphicsPipeline> resolveHandle(const GraphicsPipelineHandle&);

#if defined(GPU_TEST_UTILS)
    struct Stats {
        // The number of times we find a pre-existing task for a Pipeline
        int fNumPreemptivelyFoundTasks = 0;
        int fNumTasksCreated = 0;
        int fNumTaskCreationRaces = 0;
    };

    Stats getStats() const SK_EXCLUDES(fSpinLock);
#endif

private:
    mutable SkSpinlock fSpinLock;

    sk_sp<PipelineCreationTask> findTask(const UniqueKey& pipelineKey) SK_EXCLUDES(fSpinLock);

    sk_sp<PipelineCreationTask> findOrCreateTask(
            const UniqueKey& pipelineKey,
            const GraphicsPipelineDesc&,
            const RenderPassDesc&,
            SkEnumBitMask<PipelineCreationFlags>) SK_EXCLUDES(fSpinLock);

    void kickOffPipelineCreationTask(ResourceProvider*,
                                     sk_sp<const RuntimeEffectDictionary>,
                                     const GraphicsPipelineHandle&);

    void removeTask(PipelineCreationTask*) SK_EXCLUDES(fSpinLock);

    struct Traits {
        static const UniqueKey& GetKey(const sk_sp<PipelineCreationTask>&);
        static uint32_t Hash(const UniqueKey& pipelineKey);
    };
    using TaskMap = skia_private::THashTable<sk_sp<PipelineCreationTask>, UniqueKey, Traits>;

    TaskMap fActiveTasks SK_GUARDED_BY(fSpinLock);

#if defined(GPU_TEST_UTILS)
    Stats fStats SK_GUARDED_BY(fSpinLock);
#endif
    // fTaskGroup must appear before anything that uses it (e.g. fGpu), so that it is destroyed
    // after all of its users. Clients of fTaskGroup will generally want to ensure that they call
    // wait() on it as they are being destroyed, to avoid the possibility of pending tasks being
    // invoked after objects they depend upon have already been destroyed.
    std::unique_ptr<SkTaskGroup> fTaskGroup;
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_PipelineManager_DEFINED
