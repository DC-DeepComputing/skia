/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkRefCnt.h"
#include "include/core/SkTypes.h"
#include "include/gpu/ganesh/GrDirectContext.h"
#include "include/gpu/ganesh/vk/GrVkDirectContext.h"
#include "include/gpu/graphite/Recorder.h"  // IWYU pragma: keep
#include "src/core/SkContextOptions.h"
#include "src/core/SkResourceCache.h"
#include "src/core/SkSharedContext.h"
#include "src/core/SkStrikeCache.h"
#include "src/core/SkSynchronizedResourceCache.h"
#include "src/core/SkTypefaceCache.h"

#include <memory>

class SkOpenTypeSVGDecoder;
class SkResourceCache;
class SkStrikeCache;
class SkTypefaceCache;
class SkTraceMemoryDump;
class SkSharedContext;
class SkSynchronizedResourceCache;
// This can be defined by the caller's build system
// #define SK_USE_DISCARDABLE_SCALEDIMAGECACHE

#ifndef SK_DISCARDABLEMEMORY_SCALEDIMAGECACHE_COUNT_LIMIT
#define SK_DISCARDABLEMEMORY_SCALEDIMAGECACHE_COUNT_LIMIT 1024
#endif

#ifndef SK_DEFAULT_IMAGE_CACHE_LIMIT
#define SK_DEFAULT_IMAGE_CACHE_LIMIT (32 * 1024 * 1024)
#endif

using OpenTypeSVGDecoderFactory = std::unique_ptr<SkOpenTypeSVGDecoder> (*)(const uint8_t* svg,
                                                                            size_t length);

class SkContextOptions;

class SK_API SkContext final {
public:
    SkContext(const SkContext&) = delete;
    SkContext(SkContext&&) = delete;
    SkContext& operator=(const SkContext&) = delete;
    SkContext& operator=(SkContext&&) = delete;

    ~SkContext();

private:
    SkResourceCache* getResourceCache() { return fResourceCache.get(); }

    SkStrikeCache* getFontCache() { return fFontCache.get(); }

    SkTypefaceCache* getTypefaceCache() { return fTypefaceCache.get(); }

    size_t getFontCacheLimit() const { return fFontCache->getCacheSizeLimit(); }

    size_t setFontCacheLimit(size_t bytes) { return fFontCache->setCacheSizeLimit(bytes); }

    size_t getFontCacheUsed() const { return fFontCache->getTotalMemoryUsed(); }

    int getFontCacheCountLimit() const { return fFontCache->getCacheCountLimit(); }

    int setFontCacheCountLimit(int count) { return fFontCache->setCacheCountLimit(count); }

    int getFontCacheCountUsed() const { return fFontCache->getCacheCountUsed(); }

    void purgeFontCache();

    void purgePinnedFontCache();

    int getTypefaceCacheCountLimit() const { return fTypefaceCacheCountLimit; }

    int setTypefaceCacheCountLimit(int count) {
        const int prev = fTypefaceCacheCountLimit;
        fTypefaceCacheCountLimit = count;
        return prev;
    }

    void dumpMemoryStatistics(SkTraceMemoryDump* dump);

    SkContext();

    friend class SkContextPriv;

    sk_sp<SkSharedContext> fSharedContext;

    std::unique_ptr<SkSynchronizedResourceCache> fResourceCache;

    std::unique_ptr<SkStrikeCache> fFontCache;

    std::unique_ptr<SkTypefaceCache> fTypefaceCache;

    int fTypefaceCacheCountLimit = 1024;  // historical default value
    OpenTypeSVGDecoderFactory gSVGDecoderFactory = nullptr;
};
