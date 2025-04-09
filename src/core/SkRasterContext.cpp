/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "include/core/SkRasterContext.h"

#include "src/core/SkRasterContextPriv.h"

#if !defined(SK_DEFAULT_RESOURCE_CACHE_LIMIT)
    #if defined(SK_DEFAULT_IMAGE_CACHE_LIMIT)
        #define SK_DEFAULT_RESOURCE_CACHE_LIMIT SK_DEFAULT_IMAGE_CACHE_LIMIT
    #else
        #define SK_DEFAULT_RESOURCE_CACHE_LIMIT (32 * 1024 * 1024)
    #endif
#endif

sk_sp<SkRasterContext> SkRasterContext::Make(const SkRasterContext::Options& opts) {
    auto cache = SkResourceCache::Make(opts.fResourceCacheSize ? opts.fResourceCacheSize : SK_DEFAULT_RESOURCE_CACHE_LIMIT);

    return sk_make_sp<SkRasterContextPriv>(std::move(cache));
}

sk_sp<SkRasterContext> SkRasterContext::MakeWithDefaults() {
    return SkRasterContext::Make({});
}
