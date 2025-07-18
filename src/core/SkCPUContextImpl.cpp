/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "src/core/SkCPUContextImpl.h"

namespace skcpu {
ContextImpl::ContextImpl(std::unique_ptr<SkResourceCache> cache, SkResourceCache* cachePtr)
        : fCache(cache), fCachePtr(fCachePtr) {
    // cache can be nullptr while we still have a shared global SkResourceCache
    // for backwards compatibility. cachePtr should not be null (it can point
    // to that shared global, which will outlive this context).
    SkASSERT(fCachePtr);
}
}  // namespace skcpu
