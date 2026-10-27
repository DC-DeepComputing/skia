/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkGraphics.h"

#include "include/private/SkOnce.h"
#include "src/core/SkBitmapProcState.h"
#include "src/core/SkBlitMask.h"
#include "src/core/SkBlitRow.h"
#include "src/core/SkContextPriv.h"
#include "src/core/SkCpu.h"
#include "src/core/SkImageFilter_Base.h"
#include "src/core/SkMemset.h"
#include "src/core/SkOpts.h"
#include "src/core/SkResourceCache.h"
#include "src/core/SkStrikeCache.h"
#include "src/core/SkSwizzlePriv.h"
#include "src/core/SkTypefaceCache.h"

namespace {
void init_globals_if_needed() {
    static SkOnce once;
    once([] {
        SkCpu::CacheRuntimeFeatures();
        SkOpts::Init();
        SkOpts::Init_BitmapProcState();
        SkOpts::Init_BlitMask();
        SkOpts::Init_BlitRow();
        SkOpts::Init_Memset();
        SkOpts::Init_Swizzler();
    });
}

SkGraphics::TextRenderingParams gTextRenderingParams = {
        SK_GAMMA_EXPONENT, SK_GAMMA_CONTRAST, SkGraphics::A8GammaCorrection::kDefault};
}  // namespace

void SkGraphics::SetTextRenderingParams(TextRenderingParams params) {
    SkASSERT(params.fGamma >= 0 && params.fGamma < 4);
    SkASSERT(params.fContrast >= 0 && params.fContrast <= 1);
    gTextRenderingParams = params;
}

SkGraphics::TextRenderingParams SkGraphics::GetTextRenderingParams() {
    return gTextRenderingParams;
}

bool SkGraphics::ApplyGammaToA8(bool backendDefault) {
    switch (gTextRenderingParams.fApplyToA8) {
        case A8GammaCorrection::kDefault:  return backendDefault;
        case A8GammaCorrection::kDisabled: return false;
        case A8GammaCorrection::kEnabled:  return true;
    }
    SkUNREACHABLE;
}

bool SkGraphics::ApplyGammaToA8() {
#ifdef SK_GAMMA_APPLY_TO_A8
    return ApplyGammaToA8(true);
#else
    return ApplyGammaToA8(false);
#endif
}

void SkGraphics::Init() {
    // SkGraphics::Init() must be thread-safe and idempotent.
    init_globals_if_needed();
    SkContexts::GetGlobalSkContext();
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
#if defined(SK_ENABLE_THREADLOCAL_STRIKECACHE)
    return SkStrikeCache::GlobalStrikeCache()->getCacheSizeLimit();
#else
    return SkContexts::GetGlobalSkContext()->fontCacheLimit();
#endif
}

size_t SkGraphics::SetFontCacheLimit(size_t bytes) {
#if defined(SK_ENABLE_THREADLOCAL_STRIKECACHE)
    return SkStrikeCache::GlobalStrikeCache()->setCacheSizeLimit(bytes);
#else
    return SkContexts::GetGlobalSkContext()->setFontCacheLimit(bytes);
#endif
}

size_t SkGraphics::GetFontCacheUsed() {
#if defined(SK_ENABLE_THREADLOCAL_STRIKECACHE)
    return SkStrikeCache::GlobalStrikeCache()->getTotalMemoryUsed();
#else
    return SkContexts::GetGlobalSkContext()->fontCacheUsed();
#endif
}

int SkGraphics::GetFontCacheCountLimit() {
#if defined(SK_ENABLE_THREADLOCAL_STRIKECACHE)
    return SkStrikeCache::GlobalStrikeCache()->getCacheCountLimit();
#else
    return SkContexts::GetGlobalSkContext()->fontCacheCountLimit();
#endif
}

int SkGraphics::SetFontCacheCountLimit(int count) {
#if defined(SK_ENABLE_THREADLOCAL_STRIKECACHE)
    return SkStrikeCache::GlobalStrikeCache()->setCacheCountLimit(count);
#else
    return SkContexts::GetGlobalSkContext()->setFontCacheCountLimit(count);
#endif
}

int SkGraphics::GetFontCacheCountUsed() {
#if defined(SK_ENABLE_THREADLOCAL_STRIKECACHE)
    return SkStrikeCache::GlobalStrikeCache()->getCacheCountUsed();
#else
    return SkContexts::GetGlobalSkContext()->fontCacheCountUsed();
#endif
}

void SkGraphics::PurgeFontCache() {
#if defined(SK_ENABLE_THREADLOCAL_STRIKECACHE)
    SkStrikeCache::GlobalStrikeCache()->purgeAll();
#else
    SkContexts::GetGlobalSkContext()->purgeFontCache();
#endif
    SkTypefaceCache::PurgeAll();
}

void SkGraphics::PurgePinnedFontCache() {
#if defined(SK_ENABLE_THREADLOCAL_STRIKECACHE)
    SkStrikeCache::GlobalStrikeCache()->purgePinned();
#else
    SkContexts::GetGlobalSkContext()->purgePinnedFontCache();
#endif
}

size_t SkGraphics::GetResourceCacheTotalBytesUsed() {
    return SkContexts::GetGlobalSkContext()->resourceCacheTotalBytesUsed();
}

size_t SkGraphics::GetResourceCacheTotalByteLimit() {
    return SkContexts::GetGlobalSkContext()->resourceCacheTotalByteLimit();
}

size_t SkGraphics::SetResourceCacheTotalByteLimit(size_t newLimit) {
    return SkContexts::GetGlobalSkContext()->setResourceCacheTotalByteLimit(newLimit);
}

size_t SkGraphics::GetResourceCacheSingleAllocationByteLimit() {
    return SkContexts::GetGlobalSkContext()->resourceCacheSingleAllocationByteLimit();
}

size_t SkGraphics::SetResourceCacheSingleAllocationByteLimit(size_t newLimit) {
    return SkContexts::GetGlobalSkContext()->setResourceCacheSingleAllocationByteLimit(newLimit);
}

void SkGraphics::PurgeResourceCache() {
    SkImageFilter_Base::PurgeCache();
    return SkContexts::GetGlobalSkContext()->purgeResourceCache();
}

int SkGraphics::GetTypefaceCacheCountLimit() {
    return SkContexts::GetGlobalSkContext()->typefaceCacheCountLimit();
}

int SkGraphics::SetTypefaceCacheCountLimit(int count) {
    return SkContexts::GetGlobalSkContext()->setTypefaceCacheCountLimit(count);
}

SkGraphics::OpenTypeSVGDecoderFactory SkGraphics::SetOpenTypeSVGDecoderFactory(
        OpenTypeSVGDecoderFactory svgDecoderFactory) {
    SkSharedContext* shrdCtx = SkContexts::GetGlobalSkContext()->priv().sharedContext();
    return shrdCtx->setSVGDecoderFactory(svgDecoderFactory);
}

SkGraphics::OpenTypeSVGDecoderFactory SkGraphics::GetOpenTypeSVGDecoderFactory() {
    SkSharedContext* shrdCtx = SkContexts::GetGlobalSkContext()->priv().sharedContext();
    return shrdCtx->svgDecoderFactory();
}
