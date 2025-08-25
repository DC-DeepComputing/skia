/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_NewObjects_DEFINED
#define skgpu_graphite_NewObjects_DEFINED

#include "include/core/SkRefCnt.h"
#include "src/base/SkSpinlock.h"
#include "src/core/SkTHash.h"

class SkExecutor;
class SkTaskGroup;

namespace skgpu::graphite {

class GraphicsPipelineHandle;
class PipelineCreationTask;
class ResourceProvider;
class RuntimeEffectDictionary;


class PipelineManager {
public:
    PipelineManager(SkExecutor* executor);
    ~PipelineManager();

    GraphicsPipelineHandle createHandle(
            ResourceProvider*,
            const GraphicsPipelineDesc& pipelineDesc,
            const RenderPassDesc& renderPassDesc,
            SkEnumBitMask<PipelineCreationFlags>);

    void kickOffTask(ResourceProvider*,
                     const RuntimeEffectDictionary*,
                     const GraphicsPipelineHandle&);

    sk_sp<GraphicsPipeline> tradeIn(const GraphicsPipelineHandle&);

private:
    mutable SkSpinlock fSpinLock;

    // Get a compilation going
    sk_sp<PipelineCreationTask> findOrCreateTask1(
            const UniqueKey& pipelineKey,
            const GraphicsPipelineDesc&,
            const RenderPassDesc&,
            SkEnumBitMask<PipelineCreationFlags>) SK_EXCLUDES(fSpinLock);

    void removeTask1(PipelineCreationTask*) SK_EXCLUDES(fSpinLock);

    sk_sp<PipelineCreationTask> findTask1(const UniqueKey& pipelineKey) SK_EXCLUDES(fSpinLock);

    struct Traits {
        static const UniqueKey& GetKey(const sk_sp<PipelineCreationTask>&);
        static uint32_t Hash(const UniqueKey& pipelineKey) {
            return pipelineKey.hash();
        }
    };
    using TaskMap = skia_private::THashTable<sk_sp<PipelineCreationTask>, UniqueKey, Traits>;

    TaskMap fActiveTasks SK_GUARDED_BY(fSpinLock);

    // fTaskGroup must appear before anything that uses it (e.g. fGpu), so that it is destroyed
    // after all of its users. Clients of fTaskGroup will generally want to ensure that they call
    // wait() on it as they are being destroyed, to avoid the possibility of pending tasks being
    // invoked after objects they depend upon have already been destroyed.
    std::unique_ptr<SkTaskGroup> fTaskGroup1;
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_NewObjects_DEFINED
