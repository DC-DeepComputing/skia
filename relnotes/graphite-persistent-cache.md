
A new persistent caching feature has been added to Graphite. For now, it is only relevant to Graphite's native Vulkan backend. The API mimics Ganesh's API and consists of:

A new ContextOptions::PersistentCache abstract base class which can be used to implement persistent caching. Additionally, a matching ContextOptions::fPersistentCache has been added which can be used to pass the PersistentCache-derived object to Graphite.

Four new Context methods which can called to control the saving of Pipeline data to the user-supplied PersistentCache.

- canDetectNewVkPipelineCacheData()
  - reports on whether the determination of Pipeline creation is accurate
- hasNewVkPipelineCacheData()
  - reports on whether there is new Pipeline data that could be stored
- storeVkPipelineCacheData()
- storeVkPipelineCacheData(size_t maxSize)
  - stores the available Pipeline data in the user-provided PersistentCache. The second form limits the amount of data written to the cache.

