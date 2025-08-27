/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_NewObjects_DEFINED
#define skgpu_graphite_NewObjects_DEFINED

#include "include/core/SkRefCnt.h"
#include "src/base/SkEnumBitMask.h"
#include "src/base/SkSpinlock.h"
#include "src/core/SkTHash.h"

namespace skgpu {
class UniqueKey;
}

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
    PipelineManager();
    ~PipelineManager();

    GraphicsPipelineHandle createHandle(
            ResourceProvider*,
            const GraphicsPipelineDesc&,
            const RenderPassDesc&,
            SkEnumBitMask<PipelineCreationFlags>);

    void kickOffPipelineCreationTask(ResourceProvider*,
                                     sk_sp<const RuntimeEffectDictionary>,
                                     const GraphicsPipelineHandle&);

    sk_sp<GraphicsPipeline> resolveHandle(const GraphicsPipelineHandle&);

private:
    mutable SkSpinlock fSpinLock;

    sk_sp<PipelineCreationTask> findTask(const UniqueKey& pipelineKey) SK_EXCLUDES(fSpinLock);

    sk_sp<PipelineCreationTask> findOrCreateTask(
            const UniqueKey& pipelineKey,
            const GraphicsPipelineDesc&,
            const RenderPassDesc&,
            SkEnumBitMask<PipelineCreationFlags>) SK_EXCLUDES(fSpinLock);

    void removeTask(PipelineCreationTask*) SK_EXCLUDES(fSpinLock);

    struct Traits {
        static const UniqueKey& GetKey(const sk_sp<PipelineCreationTask>&);
        static uint32_t Hash(const UniqueKey& pipelineKey);
    };
    using TaskMap = skia_private::THashTable<sk_sp<PipelineCreationTask>, UniqueKey, Traits>;

    TaskMap fActiveTasks SK_GUARDED_BY(fSpinLock);
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_NewObjects_DEFINED
