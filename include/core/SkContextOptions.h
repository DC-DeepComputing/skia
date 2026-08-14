/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkContextOptions_DEFINED
#define SkContextOptions_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/private/SkAPI.h"

struct SK_API SkContextOptions {
    /**
     *  Specify the max number of bytes that should be used by the font cache.
     *  If the cache needs to allocate more, it will purge previous entries.
     */
    size_t fFontCacheLimit = 0;
    /**
     *  Set the limit to the number of entries in the font cache. If the cache needs
     *  to allocate more, it will automatically try to purge entries to meet the limit.
     */
    int fFontCacheCountLimit = 0;

    /**
     *  Set the limit to the number of entries in the typeface cache. Changes to this
     *  only take effect the next time each cache object is modified.
     */
    int fTypefaceCacheCountLimit = 0;

    /**
     *  The memory usage limit for the resource cache, used for temporary
     *  bitmaps and other resources. Entries are purged from the cache when the memory usage
     *  exceeds this limit.
     */
    size_t fResourceCacheTotalByteLimit = 0;

    /**
     *  When the cacheable entry is very large (e.g. a large scaled bitmap), adding it to the cache
     *  can cause most/all of the existing entries to be purged. To avoid this, the client can set
     *  a limit for a single allocation. If a cacheable entry would have been cached, but its size
     *  exceeds this limit, then we do not attempt to cache it at all.
     *
     *  Zero is the default value, meaning we always attempt to cache entries.
     */
    size_t fResourceCacheSingleAllocationByteLimit = 0;
};

#endif  // SkContextOptions_DEFINED
