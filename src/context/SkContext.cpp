/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/context/SkContext.h"

#include "include/core/SkImage.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSize.h"
#include "include/core/SkSpan.h"
#include "include/core/SkTraceMemoryDump.h"
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

static sk_sp<SkContext> MakeRaster(const SkContextOptions& options) {}

static sk_sp<SkContext> MakeGanesh(const skgpu::VulkanBackendContext& vkContext,
                                   const SkContextOptions&            options) {}

static sk_sp<SkContext> MakeGanesh(const skgpu::graphite::MtlBackendContext& mtlContext,
                                   const SkContextOptions&                   options) {}

static sk_sp<SkContext> MakeGraphite(const skgpu::graphite::MtlBackendContext& mtlContext,
                                     const SkContextOptions&                   options) {}

static sk_sp<SkContext> MakeGraphite(const skgpu::VulkanBackendContext& vkContext,
                                     const SkContextOptions&            options) {}

SkSynchronizedResourceCache* get_resource_cache() {
#if defined(SK_USE_DISCARDABLE_SCALEDIMAGECACHE)
    static SkSynchronizedResourceCache* gResourceCache =
            new SkSynchronizedResourceCache(SkDiscardableMemory::Create);
#else
    static SkSynchronizedResourceCache* gResourceCache =
            new SkSynchronizedResourceCache(SK_DEFAULT_IMAGE_CACHE_LIMIT);
#endif
    return gResourceCache;
}

SkStrikeCache* get_strike_cache() {
    if (gSkUseThreadLocalStrikeCaches_IAcknowledgeThisIsIncrediblyExperimental) {
        static thread_local auto* cache = new SkStrikeCache;
        return cache;
    }
    static auto* cache = new SkStrikeCache;
    return cache;
}

SkTypefaceCache* get_typeface_cache() {
    static SkTypefaceCache* gCache = new SkTypefaceCache;
    return gCache;
}

static void sk_trace_dump_visitor(const SkResourceCache::Rec& rec, void* context) {
    SkTraceMemoryDump* dump = static_cast<SkTraceMemoryDump*>(context);
    SkString dumpName = SkStringPrintf("skia/sk_resource_cache/%s_%p", rec.getCategory(), &rec);
    SkDiscardableMemory* discardable = rec.diagnostic_only_getDiscardable();
    if (discardable) {
        dump->setDiscardableMemoryBacking(dumpName.c_str(), *discardable);

        // The discardable memory size will be calculated by dumper, but we also dump what we think
        // the size of object in memory is irrespective of whether object is live or dead.
        dump->dumpNumericValue(dumpName.c_str(), "discardable_size", "bytes", rec.bytesUsed());
    } else {
        dump->dumpNumericValue(dumpName.c_str(), "size", "bytes", rec.bytesUsed());
        dump->setMemoryBacking(dumpName.c_str(), "malloc", nullptr);
    }
}

void purgeFontCache() {
    get_strike_cache()->purgeAll();
    get_typeface_cache()->purgeAll();
}

void purgePinnedFontCache() { get_strike_cache()->purgePinned(); }

void dumpMemoryStatistics(SkTraceMemoryDump* dump) {
    get_resource_cache()->visitAll(sk_trace_dump_visitor, dump);
    SkStrikeCache::DumpMemoryStatistics(dump);
}
