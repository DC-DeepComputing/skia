/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkImage.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSize.h"
#include "include/core/SkSpan.h"
#include "include/core/SkTypes.h"
#include "include/gpu/GpuTypes.h"
#include "include/gpu/ganesh/GrDirectContext.h"
#include "include/gpu/ganesh/mtl/GrMtlBackendContext.h"
#include "include/gpu/ganesh/vk/GrVkDirectContext.h"
#include "include/gpu/graphite/GraphiteTypes.h"
#include "include/gpu/graphite/Recorder.h"  // IWYU pragma: keep
#include "include/gpu/graphite/mtl/MtlBackendContext.h"
#include "include/gpu/vk/VulkanBackendContext.h"
#include "include/private/SingleOwner.h"
#include "include/private/SkThreadAnnotations.h"
#include "src/context/SkContextOptions.h"
#include "src/core/SkStrikeCache.h"
#include "src/core/SkSynchronizedResourceCache.h"
#include "src/core/SkTypefaceCache.h"

class SkOpenTypeSVGDecoder;
class SkContextOptions;

bool gSkUseThreadLocalStrikeCaches_IAcknowledgeThisIsIncrediblyExperimental = false;

// This can be defined by the caller's build system
// #define SK_USE_DISCARDABLE_SCALEDIMAGECACHE

#ifndef SK_DISCARDABLEMEMORY_SCALEDIMAGECACHE_COUNT_LIMIT
#define SK_DISCARDABLEMEMORY_SCALEDIMAGECACHE_COUNT_LIMIT 1024
#endif

#ifndef SK_DEFAULT_IMAGE_CACHE_LIMIT
#define SK_DEFAULT_IMAGE_CACHE_LIMIT (32 * 1024 * 1024)
#endif

using OpenTypeSVGDecoderFactory = std::unique_ptr<SkOpenTypeSVGDecoder> (*)(const uint8_t* svg,
                                                                            size_t         length);

class SK_API SkContext final {
public:
    SkContext(const SkContext&)            = delete;
    SkContext(SkContext&&)                 = delete;
    SkContext& operator=(const SkContext&) = delete;
    SkContext& operator=(SkContext&&)      = delete;

    ~SkContext();

    // Creates a context restricted to software rasterization
    static sk_sp<SkContext> MakeRaster(const SkContextOptions& options);

    // Creates a context wrapping a Ganesh GPU backend with Vulkan
    static sk_sp<SkContext> MakeGanesh(const skgpu::VulkanBackendContext& vkContext,
                                       const SkContextOptions&            options);

    // Creates a context wrapping a Ganesh GPU backend with Metal
    static sk_sp<SkContext> MakeGanesh(const skgpu::graphite::MtlBackendContext& mtlContext,
                                       const SkContextOptions&                   options);

    //  Creates a context wrapping a Graphite GPU backend using Metal
    static sk_sp<SkContext> MakeGraphite(const skgpu::graphite::MtlBackendContext& mtlContext,
                                         const SkContextOptions&                   options);

    //  Creates a context wrapping a Graphite GPU backend using Metal
    static sk_sp<SkContext> MakeGraphite(const skgpu::VulkanBackendContext& vkContext,
                                         const SkContextOptions&            options);

    // TODO should this be defined in this way?
    // as in, should the cache be a local field member defined like
    // SkSynchronizedResourceCache* gResourceCache
    // or live as a static field member within this function?
    // ideally it won't be shared amongt objects of SkContext right?
    SkSynchronizedResourceCache* get_resource_cache();

    SkStrikeCache* get_strike_cache();

    SkTypefaceCache* get_typeface_cache();

    size_t getFontCacheLimit() { return get_strike_cache()->getCacheSizeLimit(); }

    size_t setFontCacheLimit(size_t bytes) { return get_strike_cache()->setCacheSizeLimit(bytes); }

    size_t getFontCacheUsed() { return get_strike_cache()->getTotalMemoryUsed(); }

    int getFontCacheCountLimit() { return get_strike_cache()->getCacheCountLimit(); }

    int setFontCacheCountLimit(int count) { return get_strike_cache()->setCacheCountLimit(count); }

    int getFontCacheCountUsed() { return get_strike_cache()->getCacheCountUsed(); }

    void purgeFontCache();

    void purgePinnedFontCache();

    int getTypefaceCacheCountLimit() { return gTypefaceCacheCountLimit; }

    int setTypefaceCacheCountLimit(int count) {
        const int prev           = gTypefaceCacheCountLimit;
        gTypefaceCacheCountLimit = count;
        return prev;
    }

    void dumpMemoryStatistics(SkTraceMemoryDump* dump);

private:
    int                       gTypefaceCacheCountLimit = 1024;  // historical default value
    OpenTypeSVGDecoderFactory gSVGDecoderFactory       = nullptr;
};
