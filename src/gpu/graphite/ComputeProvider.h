/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_ComputeProvider_DEFINED
#define skgpu_graphite_ComputeProvider_DEFINED

#include "src/gpu/graphite/ResourceTypes.h"

#include <memory>

namespace skgpu::graphite {

class Caps;
class GlobalCache;
class StaticBufferManager;
enum class PathRendererStrategy;

#if defined(SK_ENABLE_SPARSE_STRIPS)
class SparseStripsCoverageStep;
#endif

class ComputeProvider {
public:
    ~ComputeProvider();

#if defined(SK_ENABLE_SPARSE_STRIPS)
    // The GlobalCache's sparse strips MSAA mask LUT in a static storage buffer, which the GPU sparse
    // strips coverage pass reads. Only uploaded for PathRendererStrategy::kGPUSparseStripsMSAA8,
    // and empty otherwise.
    BindBufferInfo sparseStripsMaskLUT() const { return fSparseStripsMaskLUT; }

    // The GPU sparse strips coverage pass. Only created for
    // PathRendererStrategy::kGPUSparseStripsMSAA8, and null otherwise.
    const SparseStripsCoverageStep* sparseStripsCoverageStep() const {
        return fSparseStripsCoverageStep.get();
    }
#endif

private:
    friend class Context; // for constructor access

    ComputeProvider(const Caps* caps,
                    StaticBufferManager* bufferManager,
                    const GlobalCache* globalCache,
                    PathRendererStrategy pathRendererStrategy);

    ComputeProvider(const ComputeProvider&) = delete;
    ComputeProvider(ComputeProvider&&) = delete;

#if defined(SK_ENABLE_SPARSE_STRIPS)
    // Assigned by StaticBufferManager::finalize().
    BindBufferInfo fSparseStripsMaskLUT;
    std::unique_ptr<SparseStripsCoverageStep> fSparseStripsCoverageStep;
#endif
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_ComputeProvider_DEFINED
