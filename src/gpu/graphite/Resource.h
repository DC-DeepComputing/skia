/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_Resource_DEFINED
#define skgpu_graphite_Resource_DEFINED

#include "include/gpu/GpuTypes.h"
#include "include/private/base/SkMutex.h"
#include "src/gpu/GpuTypesPriv.h"
#include "src/gpu/graphite/GraphiteResourceKey.h"
#include "src/gpu/graphite/ResourceTypes.h"

#include <atomic>
#include <functional>
#include <string>
#include <string_view>

class SkMutex;
class SkTraceMemoryDump;

namespace skgpu::graphite {

class ResourceCache;
class SharedContext;

#if defined(GRAPHITE_TEST_UTILS)
class Texture;
#endif

/**
 * Base class for objects that can be kept in the ResourceCache and created or fetched by a
 * ResourceProvider. A ResourceProvider controls the creation of a Resource and determines how
 * its lifecycle is managed (e.g. is it cached, shareable, etc.). Once created, however, the
 * Resource's lifecycle is controlled by the provider's ResourceCache so that GPU resources can
 * be reused when ref counts reach 0; a Resource that has no more refs and does not belong to a
 * cache is simply deleted.
 *
 * Resource Threading Model
 * ========================
 *
 * A Resource is allowed to be used in multiple threads and be deleted from any thread safely.
 * It can also have its non-cache ref counts reach zero from any thread and be safely returned to
 * the ResourceCache. Unlike Resource, ResourceCache and ResourceProvider are not thread safe and
 * are only used by a single thread.
 *
 * Active Threads:
 *   1. Context thread: this is the primary thread where Recordings are inserted and GPU work is
 *      scheduled.
 *   2. Cache thread: this is the thread that the ResourceCache is associated with. This can be the
 *      same as the context thread if the ResourceCache and ResourceProvider belong to the Context.
 *      But a Recorder's ResourceCache/Provider can be an arbitrary single thread the client is
 *      using for that specific Recorder.
 *   3. Usage threads: once created on the cache thread, a Resource is not restricted to just the
 *      cache thread and context thread. The Resource can have references held on any other
 *      Recorder's thread if multiple work streams use the resource, or may end up held in other
 *      client threads naturally as part of their lifecycle management of Skia's API objects.
 *      * A Resource could be accessed by an arbitrary number of usage threads.
 *
 * Reference Counting:
 *   1. Usage refs: a Resource starts with a single usage ref when created by the ResourceProvider,
 *      which is then given to the caller as an `sk_sp<R>`. Once it escapes the ResourceProvider,
 *      operations on the `sk_sp` can add more usage refs. Usage refs track the liveness of the
 *      Resource from C++ code external to the ResourceCache (either higher-level Skia/Graphite
 *      objects or client objects keeping it alive).
 *   2. Cache ref: A ResourceProvider can choose to register a Resource with its ResourceCache at
 *      creation time before returning the Resource. The ResourceCache holds a single ref on every
 *      Resource registered with it until the cache is shutdown.
 *   3. Command buffer refs: every Resource used in a command buffer has command buffer refs added
 *      when referencing work (e.g. a Recording) is inserted on the context thread. These refs are
 *      released when the command buffer's finish procs are called and its known that the GPU-side
 *      operations are completely done with the Resources.
 *
 * A Resource cannot be deleted until all of its ref counts reach zero. However, if the Resource
 * has a cache ref and another ref type reaches zero, the resource may be returned to the cache for
 * later reuse (i.e. the ResourceProvider can find it via a GraphiteResourceKey instead of creating
 * an entirely new GPU allocation). Depending on how the resources are used, the ResourceProvider or
 * Resource subclass choose which type of non-cache ref triggers returning to the cache:
 *
 *    - If all work on the GPU representation occurs within a command buffer, a Resource can be
 *      returned to the cache when its usage ref count reaches zero. Since the usage count reached
 *      zero, all previous uses have been converted to command buffer refs (and been scheduled) or
 *      discarded entirely. Any new usage ref increase from reuse after being returned can only be
 *      scheduled in a later command buffer so GPU state remains consistent.
 *    - If the GPU representation can be modified directly by any holder of a usage ref, the
 *      Resource cannot be returned to the cache until its command buffer ref count reaches zero.
 *      Otherwise, even if the prior usage refs had been dropped, a reused Resource could be mapped
 *      and have its contents modified while it was being used actively on the GPU for the work
 *      associated with the originally-held usage refs.
 *
 * Lifecycle:
 *   1. The ResourceProvider searches for an available Resource for a particular key.
 *      - If the Resource is marked shareable, it may have outstanding usage and CB refs.
 *      - Otherwise if available, the purgeable Resource adds a new "initial" usage ref.
 *   2. If there was no available Resource from the cache, the ResourceProvider creates a new one.
 *      - The Resource may be registered with the cache before being returned.
 *      - The Resource may have a key assigned to it before being returned.
 *      - Once returned, the Resource's cache and key can never change.
 *   3. Under normal use, usage and command buffer refs are added and removed. When the ref count
 *      reaches zero *and* the resource belongs to a cache, then the resource is moved into the
 *      cache's return queue in a thread-safe manner.
 *   4. On the cache thread at various points, the cache's return queue will be drained and returned
 *      Resources will either go into the purgeable set (available for reuse) or be removed from the
 *      cache (becoming eligible for full deletion).
 *   5. When the Recorder or Context are deleted, the ResourceCache is shutdown:
 *      - The return queue is locked so future Resource returns become no-ops.
 *      - All registered resources (from the old return queue, the purgeable set, and Resources that
 *        still have other held refs) have their cache ref removed.
 *   6. Each Resource holds a ref on the ResourceCache, and that is not modified when the cache is
 *      shutdown. This assures that the ResourceCache pointer is never modified on the Resource
 *      and keeps the cache alive until all of its reosurces have themselves been deleted.
 *      - After shutdown, the cache is no longer keeping the Resources alive so overtime as their
 *        usage and CB refs drop, they will stop keeping the cache alive.
 *   7. Once all resources ever associated with the cache have been deleted, and the cache has been
 *      shutdown (afterwhich the ResourceProvider has dropped its own ref), the cache can be deleted
 *
 * Ref Counting Properties:
 *   - The ResourceProvider provides resources from the cache thread, and this is the only way a
 *     Resource's usage ref count can change from zero to a positive value (either due to its
 *     construction, or from reuse within the cache).
 *   - The single cache ref (if added) is always added and removed from the cache thread, since it
 *     can only be added once at creation time, and is only removed when the cache has decided to
 *     purge a resource, or the cache is shutdown (only possible on the cache thread).
 *   - The usage ref count can increase above 1 from any thread and can decrease from any thread,
 *     meaning it will reach 0 on an arbitrary thread.
 *   - Command buffer refs are always incremented and decremented on the context thread, although
 *     that may not be the same as the cache thread.
 *   - A Recording has usage refs for all of its resources and holds those refs until *after* the
 *     command buffer refs have been added, meaning the command buffer ref count can only increase
 *     if the usage ref count is already non-zero.
 *   - A Resource will have a usage count of zero when it's added to the return queue. Resources
 *     that aren't returned until the CB count is zero implicitly require the usage ref count to be
 *     zero. If the usage ref count isn't yet zero, they won't be returned until the usage ref
 *     count hits zero and no new CB refs have been added.
 *   - FIXME: how does this work with shareable resources that could still be in the cache and be
 *     handed out regardless of usage ref and CB ref counts? Maybe they require the cache to hold
 *     them via usage ref so they never actually get returned?
 */
class Resource {
public:
    Resource(const Resource&) = delete;
    Resource(Resource&&) = delete;
    Resource& operator=(const Resource&) = delete;
    Resource& operator=(Resource&&) = delete;

    // Adds a usage ref to the resource. Named ref so we can easily manage usage refs with sk_sp.
    void ref() const {
        // Only the cache should be able to add the first usage ref to a resource.
        this->addRef<LastRemovedRef::kUsage>();
    }

    // Removes a usage ref from the resource
    void unref() const {
        this->removeRef<LastRemovedRef::kUsage>();
    }

    // Adds a command buffer ref to the resource
    void refCommandBuffer() const {
        if (fCommandBufferRefsAsUsageRefs) {
            return this->ref();
        } else {
            this->addRef<LastRemovedRef::kCommandBuffer>();
        }
    }

    // Removes a command buffer ref from the resource
    void unrefCommandBuffer() const {
        this->removeRef<LastRemovedRef::kCommandBuffer>();
    }

    Ownership ownership() const { return fOwnership; }

    skgpu::Budgeted budgeted() const { return fBudgeted; }

    // Retrieves the amount of GPU memory used by this resource in bytes. It is approximate since we
    // aren't aware of additional padding or copies made by the driver.
    size_t gpuMemorySize() const { return fGpuMemorySize; }

    class UniqueID {
    public:
        UniqueID() = default;

        explicit UniqueID(uint32_t id) : fID(id) {}

        uint32_t asUInt() const { return fID; }

        bool operator==(const UniqueID& other) const { return fID == other.fID; }
        bool operator!=(const UniqueID& other) const { return !(*this == other); }

    private:
        uint32_t fID = SK_InvalidUniqueID;
    };

    // Gets an id that is unique for this Resource object. It is static in that it does not change
    // when the content of the Resource object changes. This will never return 0.
    UniqueID uniqueID() const { return fUniqueID; }

    // Describes the type of gpu resource that is represented by the implementing
    // class (e.g. texture, buffer, etc).  This data is used for diagnostic
    // purposes by dumpMemoryStatistics().
    //
    // The value returned is expected to be long lived and will not be copied by the caller.
    virtual const char* getResourceType() const = 0;

    std::string getLabel() const { return fLabel; }

    // We allow the label on a Resource to change when used for a different function. For example
    // when reusing a scratch Texture we can change the label to match callers current use.
    void setLabel(std::string_view label) {
        fLabel = label;

        if (!fLabel.empty()) {
            const std::string fullLabel = "Skia_" + fLabel;
            this->setBackendLabel(fullLabel.c_str());
        }
    }

    // Tests whether a object has been abandoned or released. All objects will be in this state
    // after their creating Context is destroyed or abandoned.
    //
    // @return true if the object has been released or abandoned,
    //         false otherwise.
    // TODO: As of now this function isn't really needed because in freeGpuData we are always
    // deleting this object. However, I want to implement all the purging logic first to make sure
    // we don't have a use case for calling internalDispose but not wanting to delete the actual
    // object yet.
    bool wasDestroyed() const { return fSharedContext == nullptr; }

    const GraphiteResourceKey& key() const { return fKey; }

    // This should only ever be called by the ResourceProvider at most once before returning the
    // resource to the caller.
    void setKey(const GraphiteResourceKey& key) {
        SkASSERT(key.shareable() == Shareable::kNo || this->budgeted() == skgpu::Budgeted::kYes);
        SkASSERT(this->isUniquelyHeld());
        fKey = key;
    }

    // Dumps memory usage information for this Resource to traceMemoryDump.
    void dumpMemoryStatistics(SkTraceMemoryDump* traceMemoryDump) const;

    /**
     * If the resource has a non-shareable key then this gives the resource subclass an opportunity
     * to prepare itself to re-enter the cache. The ResourceCache extends its privilege to take the
     * first UsageRef to this function via takeRef. If takeRef is called this resource will not
     * immediately enter the cache but will be re-reprocessed when the Usage Ref count again reaches
     * zero.
     */
    virtual void prepareForReturnToCache(const std::function<void()>& takeRef) {}

#if defined(GRAPHITE_TEST_UTILS)
    bool testingShouldDeleteASAP() const { return fDeleteASAP == DeleteASAP::kYes; }

    virtual const Texture* asTexture() const { return nullptr; }
#endif

protected:
    Resource(const SharedContext*,
             Ownership,
             skgpu::Budgeted,
             size_t gpuMemorySize,
             bool commandBufferRefsAsUsageRefs = false);
    virtual ~Resource();

    const SharedContext* sharedContext() const { return fSharedContext; }

    // Overridden to add extra information to the memory dump.
    virtual void onDumpMemoryStatistics(SkTraceMemoryDump* traceMemoryDump,
                                        const char* dumpName) const {}

#ifdef SK_DEBUG
    bool debugHasCommandBufferRef() const {
        return this->hasRefs<LastRemovedRef::kCommandBuffer>();
    }

    bool isUsableAsScratch() const {
        // This is only called by the ResourceCache, so the state of the Resource's usage refs won't
        // be changed by another thread when this isUsableAsScratch() returns 'true'.
        bool hasUsages = this->hasRefs<LastRemovedRef::kUsage>();
        return fKey.shareable() == Shareable::kNo && !hasUsages && fNonShareableInCache;
    }

    template <bool IgnoreCacheRef = true >
    bool isUniquelyHeld() const {
        static constexpr uint64_t kRefMask = IgnoreCacheRef ? kCacheRefMask : 0;
        return (fRefCnts.load(std::memory_order_acquire) & ~kRefMask) == kUsageRefIncrement;
    }
#endif

    // Needed to be protected for DawnBuffer emscripten prepareForReturnToCache
    void setDeleteASAP() { fDeleteASAP = DeleteASAP::kYes; }

private:
    friend class ProxyCache; // for setDeleteASAP and updateAccessTime

    // Overridden to free GPU resources in the backend API.
    virtual void freeGpuData() = 0;

    // Overridden to call any release callbacks, if necessary
    virtual void invokeReleaseProc() {}

    enum class DeleteASAP : bool {
        kNo = false,
        kYes = true,
    };

    DeleteASAP shouldDeleteASAP() const { return fDeleteASAP; }

    // In the ResourceCache this is called whenever a Resource is moved into the purgeableQueue. It
    // may also be called by the ProxyCache to track the time on Resources it is holding on to.
    void updateAccessTime() {
        fLastAccess = skgpu::StdSteadyClock::now();
    }
    skgpu::StdSteadyClock::time_point lastAccessTime() const {
        return fLastAccess;
    }

    virtual void setBackendLabel(char const* label) {}

    ////////////////////////////////////////////////////////////////////////////
    // The following set of functions are only meant to be called by the ResourceCache. We don't
    // want them public general users of a Resource, but they also aren't purely internal calls.
    ////////////////////////////////////////////////////////////////////////////
    friend ResourceCache;

    void makeBudgeted() { fBudgeted = skgpu::Budgeted::kYes; }
    void makeUnbudgeted() { fBudgeted = skgpu::Budgeted::kNo; }

    int* accessReturnIndex()  const { return &fReturnIndex; }
    int* accessCacheIndex()  const { return &fCacheArrayIndex; }

    uint32_t timestamp() const { return fTimestamp; }
    void setTimestamp(uint32_t ts) { fTimestamp = ts; }

    // Adds a cache ref to the resource. This is only called by ResourceCache. A Resource will only
    // ever add a ref when the Resource is part of the cache (i.e. when insertResource is called)
    // and while the Resource is in the ResourceCache::ReturnQueue.
    // FIXME fix docs
    void registerWithCache(sk_sp<ResourceCache>);

    // This version of ref allows adding a ref when the usage count is 0. This should only be called
    // from the ResourceCache.
    void initialUsageRef() const {
        this->addRef<LastRemovedRef::kUsage, /*MustHaveUsageRefs=*/false>();
    }

    // Removes a cache ref from the resource. The unref here should only ever be called from the
    // ResourceCache and only in the Recorder thread the ResourceCache is part of.
    void unrefCache() const {
        SkASSERT(fReturnCache);
        this->removeRef<LastRemovedRef::kCache>();
    }

    bool isPurgeable() const {
        // This is only called by the ResourceCache on its thread; if the usage and CB ref counts
        // are 0, the ResourceCache is the only way in which they can become non-zero again.
        return (fRefCnts.load(std::memory_order_acquire) & (kUsageRefMask | kCBRefMask)) == 0;
    }

    template <LastRemovedRef RefType>
    bool hasRefs() const {
        // Because a single ref type is checked per load, this cannot atomically determine if all
        // refs are zero.
        static constexpr uint64_t kRefMask = RefMask(RefType);
        return (fRefCnts.load(std::memory_order_acquire) & kRefMask) != 0;
    }

    bool hasAnyRefs() const {
        // Because all ref counts are packed into the same atomic, when this load is actually 0
        // there are no other threads that can reach the object and add new refs (assuming a raw
        // pointer has never leaked).
        return fRefCnts.load(std::memory_order_acquire) != 0;
    }

    ////////////////////////////////////////////////////////////////////////////
    // The remaining calls are meant to be truely private
    ////////////////////////////////////////////////////////////////////////////

    static constexpr uint64_t RefIncrement(LastRemovedRef refType) {
        return refType == LastRemovedRef::kCommandBuffer ? kCBRefIncrement :
               refType == LastRemovedRef::kUsage         ? kUsageRefIncrement :
             /*refType == LastRemovedRef::kCache*/         1;
    }
    static constexpr uint64_t RefMask(LastRemovedRef refType) {
        return refType == LastRemovedRef::kCommandBuffer ? kCBRefMask :
               refType == LastRemovedRef::kUsage         ? kUsageRefMask :
             /*refType == LastRemovedRef::kCache*/         kCacheRefMask;
    }

    template <LastRemovedRef RefType, bool MustHaveUsageRefs=true>
    void addRef() const {
        static constexpr uint64_t kRefIncrement = RefIncrement(RefType);
        // No barrier required
        [[maybe_unused]] uint64_t origCnt =
                fRefCnts.fetch_add(+kRefIncrement, std::memory_order_relaxed);
        // Require that there was an already held usage ref in order to add this new ref,
        // e.g. to add a command buffer ref, a usage ref must already be held; calling code can't
        // add usage refs if it wasn't explicitly handed out by the cache.
        SkASSERT(!MustHaveUsageRefs || (origCnt & kUsageRefMask) > 0);
        // And make sure that the specific type of ref did not overflow into another field
        SkASSERT((RefMask(RefType) - (origCnt & RefMask(RefType))) >= kRefIncrement);
    }

    template <LastRemovedRef RefType>
    void removeRef() const {
        static constexpr uint64_t kRefIncrement = RefIncrement(RefType);

        if (RefType == LastRemovedRef::kCache || !fReturnCache) {
            // FIXME swap this sentence around to match if condition orderGR_GL_FUNCTION_TYPE
            // Either there's no cache, or the cache is in sole control of this ref type (single
            // threaded), with no follow-up non-atomic work *unless* the total ref count reaches 0.
            // However, in these cases, the ref count cannot become non-zero from any other thread
            // so it's safe to immediately delete the object.
            uint64_t origCnt = fRefCnts.fetch_add(-kRefIncrement, std::memory_order_acq_rel);
            if (origCnt == kRefIncrement) {
                SkASSERT(!this->hasAnyRefs());
                Resource* mutableThis = const_cast<Resource*>(this);
                mutableThis->internalDispose();
            }
        } else {
            // When removing a usage or CB ref and that specific count reaches 0, the resouce is
            // returned to the cache. In order to keep the resource alive during this non-atomic
            // operation, a compare-and-exchange loop is used with one extra bit set to prevent
            // the total ref count from reaching zero until notifyARefIsZero() returns.
            static constexpr uint64_t kRefMask = RefMask(RefType);
            uint64_t origCnt, lockedUnrefCnt;
            do {
                // Mask off any set unref mutex bit from another thread so that the CAS only passes
                // when the unref mutex bit is 0.
                origCnt = fRefCnts.load(std::memory_order_acquire) & ~kUnrefMutexMask;
                SkASSERT((origCnt & kRefMask) >= kRefIncrement);
                // But include the unref bit in the updated ref count value so that if the CAS
                // succeeds, this thread will keep the object alive.
                lockedUnrefCnt = (origCnt - kRefIncrement) | kUnrefMutexMask;
            } while (!fRefCnts.compare_exchange_weak(origCnt, lockedUnrefCnt,
                                                     std::memory_order::memory_order_release,
                                                     std::memory_order::memory_order_relaxed));
            // At this point, the current thread has exclusive control over returning this resource.
            // Other threads will be unable to remove CB or usage refs, although the cache ref can
            // still be dropped. In that case, the other thread will skip deleting the object and
            // we have to re-check for 0 total refs on unlock.
            if ((lockedUnrefCnt & kRefMask) == 0) {
                this->notifyARefIsZero(RefType);
            }
            // Remove the unref bit; if that was the last bit set in the field then this unref()
            // was the last (or another thread's cache unref was the "last" but was skipped), and
            // notifyARefIsZero() had not added any other refs to keep it alive longer.
            if (fRefCnts.fetch_and(~kUnrefMutexMask) == kUnrefMutexMask) {
                SkASSERT(!this->hasAnyRefs());
                Resource* mutableThis = const_cast<Resource*>(this);
                mutableThis->internalDispose();
            }
        }
    }

    void notifyARefIsZero(LastRemovedRef removedRef) const;

    // Frees the object in the underlying 3D API.
    void internalDispose();

    // Resource has three separate types of refs that all keep the object alive.
    // - cache ref: at most 2 ref that is owned by fReturnCache (while in the cache, and in the
    //     return queue)
    // - usage refs: conventional refs held by Skia or client-owned objects
    // - command-buffer refs: refs held for the duration of a command buffer's execution on the GPU
    //     that relies on the GPU-side representation of the Resource.
    //
    // The single atomic's bits are split into subfields:
    // commandBufferRefs:31
    // usageRefs:31
    // unrefMutex: 1
    // cacheRefs:1

    static constexpr uint64_t kCBRefMask    = (((uint64_t)1 << 31)-1) << 34;
    static constexpr uint64_t kUsageRefMask = (((uint64_t)1 << 31)-1) << 2;
    static constexpr uint64_t kUnrefMutexMask = 0b10;
    static constexpr uint64_t kCacheRefMask = 0b01;

    static constexpr uint64_t kUsageRefIncrement = (uint64_t) 1 << 2;
    static constexpr uint64_t kCBRefIncrement    = (uint64_t) 1 << 34;

    static constexpr size_t kInvalidGpuMemorySize = ~static_cast<size_t>(0);

    mutable std::atomic<uint64_t> fRefCnts;

    // This is not ref'ed but internalDispose() will be called before the Gpu object is destroyed.
    // That call will set this to nullptr.
    const SharedContext* fSharedContext;

    const UniqueID fUniqueID;
    const Ownership fOwnership;
    const size_t fGpuMemorySize; // = kInvalidGpuMemorySize;

    // Indicates that CommandBufferRefs should be rerouted to UsageRefs.
    const bool fCommandBufferRefsAsUsageRefs;

    // The resource key and return cache are both set at most once, during initialization by the
    // ResourceCache and/or ResourceProvider. Once returned to the caller they are immutable.

    /*const*/ GraphiteResourceKey fKey;
    /*const*/ sk_sp<ResourceCache> fReturnCache;

    // An index into the return cache so we know whether or not the resource is already waiting to
    // be returned or not. Guarded by `fReturnCache::fReturnMutex`.
    mutable int fReturnIndex = -1;

    // The remaining fields are mutable state that is only modified by the ResourceCache on the
    // cache's thread.

    // All resource created internally by Graphite and held in the ResourceCache as a shared
    // resource or available scratch resource are considered budgeted. Resources that back client
    // owned objects (e.g. SkSurface or SkImage) are not budgeted and do not count against cache
    // limits.
    skgpu::Budgeted fBudgeted;

    // This is only used by ProxyCache::purgeProxiesNotUsedSince which is called from
    // ResourceCache::purgeResourcesNotUsedSince. When kYes, this signals that the Resource
    // should've been purged based on its timestamp at some point regardless of what its
    // current timestamp may indicate (since the timestamp will be updated when the Resource
    // is returned to the ResourceCache).
    DeleteASAP fDeleteASAP = DeleteASAP::kNo;

    // An index into a heap when this resource is purgeable or an array when not. This is maintained
    // by the cache. Guarded by `fReturnCache::fSingleOwner`.
    mutable int fCacheArrayIndex = -1;
    // This value reflects how recently this resource was accessed in the cache. This is maintained
    // by the cache. Guarded by `fReturnCache::fSingleOwner`.
    uint32_t fTimestamp;
    skgpu::StdSteadyClock::time_point fLastAccess;

    // String used to describe the current use of this Resource.
    std::string fLabel;

    // This is only used during validation checking. Lots of the validation code depends on a
    // resource being purgeable or not. However, purgeable itself just means having no refs. The
    // refs can be removed before a Resource is returned to the cache (or even added to the
    // ReturnQueue).
    SkDEBUGCODE(mutable bool fNonShareableInCache = false;)
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_Resource_DEFINED
