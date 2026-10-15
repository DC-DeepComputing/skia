/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkGraphics.h"

#include "src/core/SkContextPriv.h"
#include "src/core/SkImageFilter_Base.h"
#include "src/core/SkResourceCache.h"
#include "src/core/SkStrikeCache.h"
#include "src/core/SkSynchronizedResourceCache.h"
#include "src/core/SkTypefaceCache.h"

void SkGraphics::Init() {
    // SkGraphics::Init() must be thread-safe and idempotent.
    SkContexts::GetGlobalSkContext();
    SkContexts::GetGlobalSkSharedContext();
}

///////////////////////////////////////////////////////////////////////////////

void SkGraphics::DumpMemoryStatistics(SkTraceMemoryDump* dump) {
    SkResourceCache::DumpMemoryStatistics(dump);
    SkStrikeCache::DumpMemoryStatistics(dump);
}

void SkGraphics::PurgeAllCaches() {
    SkGraphics::PurgeFontCache();
    SkGraphics::PurgeResourceCache();
    SkImageFilter_Base::PurgeCache();
}

///////////////////////////////////////////////////////////////////////////////

size_t SkGraphics::GetFontCacheLimit() {
    return SkContexts::GetGlobalSkSharedContext()->strikeCache()->getCacheSizeLimit();
}

size_t SkGraphics::SetFontCacheLimit(size_t bytes) {
    return SkContexts::GetGlobalSkSharedContext()->strikeCache()->setCacheSizeLimit(bytes);
}

size_t SkGraphics::GetFontCacheUsed() {
    return SkContexts::GetGlobalSkSharedContext()->strikeCache()->getTotalMemoryUsed();
}

int SkGraphics::GetFontCacheCountLimit() {
    return SkContexts::GetGlobalSkSharedContext()->strikeCache()->getCacheCountLimit();
}

int SkGraphics::SetFontCacheCountLimit(int count) {
    return SkContexts::GetGlobalSkSharedContext()->strikeCache()->setCacheCountLimit(count);
}

int SkGraphics::GetFontCacheCountUsed() {
    return SkContexts::GetGlobalSkSharedContext()->strikeCache()->getCacheCountUsed();
}

void SkGraphics::PurgeFontCache() {
    SkContexts::GetGlobalSkSharedContext()->strikeCache()->purgeAll();
    SkTypefaceCache::PurgeAll();
}

void SkGraphics::PurgePinnedFontCache() {
    SkContexts::GetGlobalSkSharedContext()->strikeCache()->purgePinned();
}

size_t SkGraphics::GetResourceCacheTotalBytesUsed() {
    return SkContexts::GetGlobalSkSharedContext()->synchronizedResourceCache()->getTotalBytesUsed();
}

size_t SkGraphics::GetResourceCacheTotalByteLimit() {
    return SkContexts::GetGlobalSkSharedContext()->synchronizedResourceCache()->getTotalByteLimit();
}

size_t SkGraphics::SetResourceCacheTotalByteLimit(size_t newLimit) {
    return SkContexts::GetGlobalSkSharedContext()->synchronizedResourceCache()->setTotalByteLimit(
            newLimit);
}

size_t SkGraphics::GetResourceCacheSingleAllocationByteLimit() {
    return SkContexts::GetGlobalSkSharedContext()
            ->synchronizedResourceCache()
            ->getSingleAllocationByteLimit();
}

size_t SkGraphics::SetResourceCacheSingleAllocationByteLimit(size_t newLimit) {
    return SkContexts::GetGlobalSkSharedContext()
            ->synchronizedResourceCache()
            ->setSingleAllocationByteLimit(newLimit);
}

void SkGraphics::PurgeResourceCache() {
    SkImageFilter_Base::PurgeCache();
    return SkContexts::GetGlobalSkSharedContext()->synchronizedResourceCache()->purgeAll();
}


int SkGraphics::GetTypefaceCacheCountLimit() {
    return SkContexts::GetGlobalSkSharedContext()->typefaceCacheCountLimit();
}

int SkGraphics::SetTypefaceCacheCountLimit(int count) {
    const int prev = SkContexts::GetGlobalSkSharedContext()->typefaceCacheCountLimit();
    SkContexts::GetGlobalSkSharedContext()->setTypefaceCacheCountLimit(count);
    return prev;
}

SkGraphics::OpenTypeSVGDecoderFactory SkGraphics::SetOpenTypeSVGDecoderFactory(
        OpenTypeSVGDecoderFactory svgDecoderFactory) {
    return SkContexts::GetGlobalSkSharedContext()->setSVGDecoderFactory(svgDecoderFactory);
}

SkGraphics::OpenTypeSVGDecoderFactory SkGraphics::GetOpenTypeSVGDecoderFactory() {
    return SkContexts::GetGlobalSkSharedContext()->SVGDecoderFactory();
}
