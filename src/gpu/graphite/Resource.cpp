/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/Resource.h"

#include "include/core/SkTraceMemoryDump.h"
#include "src/gpu/graphite/ResourceCache.h"

namespace skgpu::graphite {

namespace {
uint32_t create_unique_id() {
    static std::atomic<uint32_t> nextID{1};
    uint32_t id;
    do {
        id = nextID.fetch_add(1, std::memory_order_relaxed);
    } while (id == SK_InvalidUniqueID);
    return id;
}
} // namespace anonymous

Resource::Resource(const SharedContext* sharedContext,
                   Ownership ownership,
                   skgpu::Budgeted budgeted,
                   size_t gpuMemorySize,
                   bool commandBufferRefsAsUsageRefs)
        : fSharedContext(sharedContext)
        , fRefCnts(kUsageRefIncrement) // Start with 1 usage ref and no others
        , fCommandBufferRefsAsUsageRefs(commandBufferRefsAsUsageRefs)
        , fOwnership(ownership)
        , fGpuMemorySize(gpuMemorySize)
        , fBudgeted(budgeted)
        , fUniqueID(create_unique_id()) {
    // If we don't own the resource that must mean its wrapped in a client object. Thus we should
    // not be budgeted
    SkASSERT(fOwnership == Ownership::kOwned || fBudgeted == skgpu::Budgeted::kNo);
}

Resource::~Resource() {
    // The cache should have released or destroyed this resource.
    SkASSERT(this->wasDestroyed());
}

void Resource::registerWithCache(sk_sp<ResourceCache> returnCache) {
    // ResourceCache should be registered before the Resource escapes the ResourceProvider, e.g. it
    // has a single usage ref and no others.
    SkASSERT(this->isUniquelyHeld</*IgnoreCacheRef=*/false>());
    SkASSERT(!fReturnCache);
    SkASSERT(returnCache);

    fReturnCache = std::move(returnCache);
    this->addRef<LastRemovedRef::kCache>();
}

void Resource::notifyARefIsZero(LastRemovedRef removedRef) const {
    // No resource should have been destroyed if there was still any sort of ref on it.
    SkASSERT(!this->wasDestroyed());

    Resource* mutableThis = const_cast<Resource*>(this);

    // Not all resources are registered with the cache, but notifyARefIsZero() should only be
    // called when they have been registered.
    SkASSERT(fReturnCache && removedRef != LastRemovedRef::kCache);
    (void) fReturnCache->returnResource(mutableThis, removedRef);
//     if (removedRef != LastRemovedRef::kCache &&
//         fReturnCache &&
//         fReturnCache->returnResource(mutableThis, removedRef)) {
//         return false;
//     }

//     // For unref'ing cache refs, we won't be taking any of the returnResource() paths so the
//     // ref count wouldn't be changing in this function -> so we can just check the atomic prior value.

//     // but if we are a CB or usage ref that hit 0, which we can determine from the fetch_add return value
//     // there might be new refs that were added
//     //   -> Do we really need to be adding cache refs for the return queue?
//     //   -> a cache ref can be promoted to a new usage ref on another thread
//     //       but only if it were already returned to the cache to be handed out
//     //   -> a usage ref can allow a CB ref to be created on another thread even if we were on a thread
//     //      that was processing the fact that the CB ref count had just dropped to 0.

//     // these adding of refs would then violate the hasAnyRefs test based on the original value.
//     // but if we were going to delete the resourcer, it means we a) didn't have any cache refs
//     // and no usage/CB ref to cause other unrefs with.
//     //
//     // So we could perhaps check for 0 -> dispose, else -> have returnCache returnREsource...

//     // Does anything ever set the return cache to null? or do resources keep the cache alive until
//     // they are all unrefed entirely?

//     if (!this->hasAnyRefs()) {
//         return true;
//     }
//     return false;
}

void Resource::internalDispose() {
    SkASSERT(fSharedContext);
    this->invokeReleaseProc();
    this->freeGpuData();
    fSharedContext = nullptr;
    // TODO: If we ever support freeing all the backend objects without deleting the object, we'll
    // need to add a hasAnyRefs() check here.
    delete this;
}

void Resource::dumpMemoryStatistics(SkTraceMemoryDump* traceMemoryDump) const {
    if (this->ownership() == Ownership::kWrapped && !traceMemoryDump->shouldDumpWrappedObjects()) {
        return;
    }

    if (this->budgeted() == skgpu::Budgeted::kNo &&
        !traceMemoryDump->shouldDumpUnbudgetedObjects()) {
        return;
    }

    size_t size = this->gpuMemorySize();

    // Avoid dumping objects without a size (e.g. Samplers, pipelines, etc).
    // TODO: Would a client ever actually want to see all of this? Wouldn't be hard to add it as an
    // option.
    if (size == 0) {
        return;
    }

    SkString resourceName("skia/gpu_resources/resource_");
    resourceName.appendU32(this->uniqueID().asUInt());

    traceMemoryDump->dumpNumericValue(resourceName.c_str(), "size", "bytes", size);
    traceMemoryDump->dumpStringValue(resourceName.c_str(), "type", this->getResourceType());
    traceMemoryDump->dumpStringValue(resourceName.c_str(), "label", this->getLabel().c_str());
    if (this->isPurgeable()) {
        traceMemoryDump->dumpNumericValue(resourceName.c_str(), "purgeable_size", "bytes", size);
    }
    if (traceMemoryDump->shouldDumpWrappedObjects()) {
        traceMemoryDump->dumpWrappedState(resourceName.c_str(),
                                          this->ownership() == Ownership::kWrapped);
    }
    if (traceMemoryDump->shouldDumpUnbudgetedObjects()) {
        traceMemoryDump->dumpBudgetedState(resourceName.c_str(),
                                           this->budgeted() == skgpu::Budgeted::kYes);
    }

    this->onDumpMemoryStatistics(traceMemoryDump, resourceName.c_str());

    // TODO: implement this to report real gpu id backing the resource. Will be virtual implemented
    // by backend specific resource subclasses.
    //this->setMemoryBacking(traceMemoryDump, resourceName);
}

} // namespace skgpu::graphite
