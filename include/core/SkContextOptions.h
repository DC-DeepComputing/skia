/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkContextOptions_DEFINED
#define SkContextOptions_DEFINED

#include "include/private/SkAPI.h"

class SkData;
class SkImageGenerator;
class SkOpenTypeSVGDecoder;
class SkTraceMemoryDump;
class SkExecutor;

struct SK_API SkContextOptions {

    SkExecutor* fExecutor = nullptr;

    /**
     *  Specify the max number of bytes that should be used by the font cache.
     *  If the cache needs to allocate more, it will purge previous entries.
     *
     *  This function returns the previous setting, as if GetFontCacheLimit()
     *  had be called before the new limit was set.
     */
    size_t fFontCacheLimit = 0;
    /**
     *  Set the limit to the number of entries in the font cache, and return
     *  the previous value. If this new value is lower than the previous,
     *  it will automatically try to purge entries to meet the new limit.
     */
    int fFontCacheCountLimit = 0;

    /**
     *  Set the limit to the number of entries in the typeface cache, and return
     *  the previous value. Changes to this only take effect the next time
     *  each cache object is modified.
     */
    int fTypefaceCacheCountLimit = 0;

    /**
     *  The memory usage limit for the resource cache, used for temporary
     *  bitmaps and other resources. Entries are purged from the cache when the memory useage
     *  exceeds this limit.
     */
    size_t fResourceCacheTotalByteLimit = 0;

    /**
     *  When the cachable entry is very lage (e.g. a large scaled bitmap), adding it to the cache
     *  can cause most/all of the existing entries to be purged. To avoid the, the client can set
     *  a limit for a single allocation. If a cacheable entry would have been cached, but its size
     *  exceeds this limit, then we do not attempt to cache it at all.
     *
     *  Zero is the default value, meaning we always attempt to cache entries.
     */
    size_t fResourceCacheSingleAllocationByteLimit = 0;

    using ImageGeneratorFromEncodedDataFactory =
            std::unique_ptr<SkImageGenerator> (*)(sk_sp<const SkData>);
    /**
     *  To instantiate images from encoded data, first looks at this runtime function-ptr. If it
     *  exists, it is called to create an SkImageGenerator from SkData. If there is no function-ptr
     *  or there is, but it returns NULL, then skia will call its internal default implementation.
     *
     *  Returns the previous factory (which could be NULL).
     */
    ImageGeneratorFromEncodedDataFactory fImageGeneratorFromEncodedDataFactory = nullptr;

    /**
     *  To draw OpenType SVG data, Skia will look at this runtime function pointer. If this function
     *  pointer is set, the SkTypeface implementations which support OpenType SVG will call this
     *  function to create an SkOpenTypeSVGDecoder to decode the OpenType SVG and draw it as needed.
     *  If this function is not set, the SkTypeface implementations will generally not support
     *  OpenType SVG and attempt to use other glyph representations if available.
     */
    using OpenTypeSVGDecoderFactory = std::unique_ptr<SkOpenTypeSVGDecoder> (*)(const uint8_t* svg,
                                                                                size_t length);
    OpenTypeSVGDecoderFactory fOpenTypeSVGDecoderFactory;
};

#endif  // SkContextOptions_DEFINED
