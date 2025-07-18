/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "include/core/SkCPUContext.h"

#include "include/core/SkTypes.h"
#include "src/core/SkCPUContextImpl.h"
#include "src/core/SkCPURecorderImpl.h"
#include "src/core/SkSynchronizedResourceCache.h"

#include <memory>

namespace skcpu {

std::unique_ptr<const Context> Context::Make(const Context::Options& opts) {
    std::unique_ptr<SkResourceCache> cache = nullptr;
    SkResourceCache* cachePtr = nullptr;
    if (opts.fResourceCacheByteLimit || opts.fResourceCacheFactory) {
        if (opts.fResourceCacheFactory) {
            cache = std::make_unique<SkSynchronizedResourceCache>(opts.fResourceCacheFactory);
        } else {
            cache = std::make_unique<SkSynchronizedResourceCache>(opts.fResourceCacheByteLimit);
        }
        cachePtr = cache.get();
    } else {
        cachePtr = SkSynchronizedResourceCache::Global();
    }
    return std::make_unique<ContextImpl>(std::move(cache), cachePtr);
}

std::unique_ptr<const Context> Context::Make() {
    return Context::Make({});
}

std::unique_ptr<Recorder> Context::makeRecorder() const {
    return std::make_unique<RecorderImpl>(static_cast<const ContextImpl*>(this));
}

const ContextImpl* ContextImpl::TODO() {
    static auto gContext = Context::Make();
    return static_cast<const ContextImpl*>(gContext.get());
}

}  // namespace skcpu
