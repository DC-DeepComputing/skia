/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_ResourceCache_DEFINED
#define skgpu_ResourceCache_DEFINED

#include "experimental/graphite/src/ResourceTypes.h"
#include "include/core/SkRefCnt.h"
#include "include/private/SkMutex.h"
#include "include/private/SkTArray.h"
#include "include/private/SkTHash.h"
#include "src/core/SkTDPQueue.h"
#include "src/core/SkTMultiMap.h"

#include <vector>

namespace skgpu {

class GraphiteResourceKey;
class Resource;
class SingleOwner;

class ResourceCache {
public:
    ResourceCache(SingleOwner*);
    ~ResourceCache();

    ResourceCache(const ResourceCache&) = delete;
    ResourceCache(ResourceCache&&) = delete;
    ResourceCache& operator=(const ResourceCache&) = delete;
    ResourceCache& operator=(ResourceCache&&) = delete;

    /**
     * Returns the number of resources.
     */
    int getResourceCount() const {
        return fPurgeableQueue.count() + fNonpurgeableResources.count();
    }

    void insertResource(Resource*);

    /**
     * Find a resource that matches a key.
     */
    Resource* findAndRefResource(const GraphiteResourceKey& key);

    /**
     * This helper class is used to help with Resources returning themself to their ResourceCache.
     * This class is thread safe and every Resource that is inserted into the cache is registered
     * with a ref to the cache's Return.
     *
     * Resource will try to return themselves to the ResourceCache when a ref goes to zero via their
     * CacheAccess. The returned Resource's will be queue up on the Return. The ResourceCache
     * will occationally move all Resources from the CacheAccess's ReturnQueue back into the
     * ResourceCache itself.
     */
    class Return : public SkRefCnt {
    public:
        // This is a thread safe call. If it fails the ResourceCache is no longer valid and the
        // Resource should clean itself up if it is the last ref.
        bool returnResource(Resource*, LastRemovedRef);

    private:
        friend class ResourceCache;

        Return(ResourceCache* cache) : fCache(cache) {}

        // Called by the ResourceCache to move Resources off of the ReturnQueue and back into the
        // cache. We have to pass in the cache so this can be called after we a call to
        // invalidateCache.
        void processReturnedResources(ResourceCache* cache);
        // Called by the ResourceCache in its destructor to disallow future Resources to be add to
        // the ReturnQueue. After this is called returnResources will return false.
        void invalidateCache();

        ResourceCache* fCache;
        SkMutex fReturnMutex;
        typedef std::vector<std::pair<Resource*, LastRemovedRef>> ReturnQueue;
        ReturnQueue fReturnQueue SK_GUARDED_BY(fReturnMutex);
    };

private:
    void refAndMakeResourceMRU(Resource*);
    void addToNonpurgeableArray(Resource* resource);
    void removeFromNonpurgeableArray(Resource* resource);

    void returnResource(Resource*, LastRemovedRef);

    uint32_t getNextTimestamp();

#ifdef SK_DEBUG
    bool isInCache(const Resource* r) const;
    void validate() const;
#else
    void validate() const {}
#endif

    struct MapTraits {
        static const GraphiteResourceKey& GetKey(const Resource& r);

        static uint32_t Hash(const GraphiteResourceKey& key);
        static void OnFree(Resource*) {}
    };
    typedef SkTMultiMap<Resource, GraphiteResourceKey, MapTraits> ResourceMap;

    static bool CompareTimestamp(Resource* const& a, Resource* const& b);
    static int* AccessResourceIndex(Resource* const& res);

    using PurgeableQueue = SkTDPQueue<Resource*, CompareTimestamp, AccessResourceIndex>;
    using ResourceArray = SkTDArray<Resource*>;

    // Whenever a resource is added to the cache or the result of a cache lookup, fTimestamp is
    // assigned as the resource's timestamp and then incremented. fPurgeableQueue orders the
    // purgeable resources by this value, and thus is used to purge resources in LRU order.
    uint32_t fTimestamp = 0;
    PurgeableQueue fPurgeableQueue;
    ResourceArray fNonpurgeableResources;

    ResourceMap fResourceMap;

    sk_sp<Return> fCacheReturn;

    SingleOwner* fSingleOwner = nullptr;
};

} // namespace skgpu

#endif // skgpu_ResourceCache_DEFINED
