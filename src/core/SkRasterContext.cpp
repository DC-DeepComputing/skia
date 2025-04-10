/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "include/core/SkRasterContext.h"

#include "include/core/SkSurfaceProps.h"
#include "src/core/SkRasterContextPriv.h"
#include "src/core/SkRasterRecorderPriv.h"

#if !defined(SK_DEFAULT_RESOURCE_CACHE_LIMIT)
    #if defined(SK_DEFAULT_IMAGE_CACHE_LIMIT)
        #define SK_DEFAULT_RESOURCE_CACHE_LIMIT SK_DEFAULT_IMAGE_CACHE_LIMIT
    #else
        #define SK_DEFAULT_RESOURCE_CACHE_LIMIT (32 * 1024 * 1024)
    #endif
#endif

sk_sp<const SkRasterContext> SkRasterContext::Make(const SkRasterContext::Options& opts) {
    auto cache = SkResourceCache::Make(opts.fResourceCacheSize ? opts.fResourceCacheSize : SK_DEFAULT_RESOURCE_CACHE_LIMIT);

    return sk_make_sp<SkRasterContextPriv>(std::move(cache),
        opts.fSurfaceProps ? *opts.fSurfaceProps : SkSurfaceProps{});
}

sk_sp<const SkRasterContext> SkRasterContext::MakeWithDefaults() {
    return SkRasterContext::Make({});
}

std::unique_ptr<SkRasterRecorder> SkRasterContext::makeRecorder() const {
    return std::make_unique<SkRasterRecorderPriv>(sk_ref_sp(static_cast<const SkRasterContextPriv*>(this)));
}
