/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_PrecompileContext_DEFINED
#define skgpu_graphite_PrecompileContext_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/private/base/SingleOwner.h"

#include <chrono>
#include <memory>

namespace skgpu::graphite {

class SharedContext;
class PrecompileContextPriv;
class ResourceProvider;

class SK_API PrecompileContext {
public:
    ~PrecompileContext();

    /**
     * Purge GPU resources that haven't been used in the past 'msNotUsed' milliseconds or are
     * otherwise marked for deletion, regardless of whether the context is under budget.
     *
     * @param msNotUsed   Only unlocked resources not used in these last milliseconds will be
     *                    cleaned up.
     */

// From GrDirectContext.h --------------------------------------------------------------------------
//    enum class GrPurgeResourceOptions1 {
//        kAllResources,
//        kScratchResourcesOnly,
//    };

//    void performDeferredCleanup(
//            std::chrono::milliseconds msNotUsed,
//            GrPurgeResourceOptions1 opts = GrPurgeResourceOptions1::kAllResources);

    // Temporary compatibility API for Android.
//    void purgeResourcesNotUsedInMs(std::chrono::milliseconds msNotUsed) {
//        this->performDeferredCleanup(msNotUsed);
//    }

// From Context.h ----------------------------------------------------------------------------------
//    void performDeferredCleanup(std::chrono::milliseconds msNotUsed);

    // Proposed
    void purgeUnusedPipelinesSince(std::chrono::milliseconds msNotUsed);

    // Provides access to functions that aren't part of the public API.
    PrecompileContextPriv priv();
    const PrecompileContextPriv priv() const;  // NOLINT(readability-const-return-type)

private:
    friend class PrecompileContextPriv;
    friend class Context; // for ctor

    PrecompileContext(sk_sp<SharedContext>);

    mutable SingleOwner fSingleOwner;
    sk_sp<SharedContext> fSharedContext;
    std::unique_ptr<ResourceProvider> fResourceProvider;
};

}  // namespace skgpu::graphite

#endif // skgpu_graphite_PrecompileContext_DEFINED
