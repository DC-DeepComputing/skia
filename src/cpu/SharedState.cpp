/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/cpu/SharedState.h"

#include "include/core/SkContextOptions.h"
#include "src/core/SkStrikeCache.h"
#include "src/core/SkSynchronizedResourceCache.h"
#include "src/core/SkTypefaceCache.h"

#if defined(SK_USE_DISCARDABLE_SCALEDIMAGECACHE)
#include "include/private/chromium/SkDiscardableMemory.h"
#endif

namespace skcpu {

SharedState::SharedState(const SkContextOptions& options) {
#if defined(SK_USE_DISCARDABLE_SCALEDIMAGECACHE)
    fResourceCache = std::make_unique<SkSynchronizedResourceCache>(SkDiscardableMemory::Create);
#else
    fResourceCache =
            std::make_unique<SkSynchronizedResourceCache>(options.fResourceCacheTotalByteLimit);
#endif

    fStrikeCache = std::make_unique<SkStrikeCache>();

    fTypefaceCache = std::make_unique<SkTypefaceCache>();
    fTypefaceCacheCountLimit = options.fTypefaceCacheCountLimit;

    fResourceCache->setSingleAllocationByteLimit(options.fResourceCacheSingleAllocationByteLimit);
    fStrikeCache->setCacheCountLimit(options.fFontCacheCountLimit);
    fStrikeCache->setCacheSizeLimit(options.fFontCacheLimit);
}

SharedState::~SharedState() = default;

size_t SharedState::fontCacheLimit() const { return fStrikeCache->getCacheSizeLimit(); }

size_t SharedState::setFontCacheLimit(size_t bytes) {
    return fStrikeCache->setCacheSizeLimit(bytes);
}

size_t SharedState::fontCacheUsed() const { return fStrikeCache->getTotalMemoryUsed(); }

int SharedState::fontCacheCountLimit() const { return fStrikeCache->getCacheCountLimit(); }

int SharedState::setFontCacheCountLimit(int count) {
    return fStrikeCache->setCacheCountLimit(count);
}

int SharedState::fontCacheCountUsed() const { return fStrikeCache->getCacheCountUsed(); }

void SharedState::purgeFontCache() { fStrikeCache->purgeAll(); }

void SharedState::purgePinnedFontCache() { fStrikeCache->purgePinned(); }

size_t SharedState::resourceCacheTotalBytesUsed() const {
    return fResourceCache->getTotalBytesUsed();
}

size_t SharedState::resourceCacheTotalByteLimit() const {
    return fResourceCache->getTotalByteLimit();
}

size_t SharedState::setResourceCacheTotalByteLimit(size_t newLimit) {
    return fResourceCache->setTotalByteLimit(newLimit);
}

size_t SharedState::resourceCacheSingleAllocationByteLimit() const {
    return fResourceCache->getSingleAllocationByteLimit();
}

size_t SharedState::setResourceCacheSingleAllocationByteLimit(size_t newLimit) {
    return fResourceCache->setSingleAllocationByteLimit(newLimit);
}

void SharedState::purgeResourceCache() { fResourceCache->purgeAll(); }

}  // namespace skcpu
