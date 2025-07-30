/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_NewObjects_DEFINED
#define skgpu_graphite_NewObjects_DEFINED

#include "src/base/SkSpinlock.h"
#include "src/gpu/graphite/GraphicsPipeline.h"

class SkExecutor;
class SkTaskGroup;

namespace skgpu::graphite {

class ResourceProvider;
class RuntimeEffectDictionary;

class GraphicsPipelineHandle {
public:
    bool isReady() const { return fIsReady; }
    bool autoDelete() const { return fAutoDelete; }

    sk_sp<GraphicsPipeline> wait() {
        SkASSERT(fIsReady);
        return fPipeline;
    }

private:
    friend class PipelineManager;

    GraphicsPipelineHandle(sk_sp<GraphicsPipeline> pipeline, bool autoDelete)
            : fPipeline(std::move(pipeline))
            , fIsReady(fPipeline)
            , fAutoDelete(autoDelete) {}

public:             // TODO: make private
    void update(sk_sp<GraphicsPipeline> pipeline) {
        SkASSERT(!fIsReady);
        fPipeline = std::move(pipeline);
        fIsReady = true;
    }

private:
    sk_sp<GraphicsPipeline> fPipeline;
    bool fIsReady = false;
    bool fAutoDelete;
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

    GraphicsPipelineHandle* createHandle(sk_sp<GraphicsPipeline> pipeline,
                                         bool autoDelete) SK_EXCLUDES(fSpinLock);

private:
    mutable SkSpinlock fSpinLock;

    // TODO: replace this with a block allocated thing
    std::vector<GraphicsPipelineHandle*> fMemoryToDeallocate SK_GUARDED_BY(fSpinLock);
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_NewObjects_DEFINED
