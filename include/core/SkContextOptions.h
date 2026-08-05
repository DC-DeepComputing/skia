/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkContextOptions_DEFINED
#define SkContextOptions_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/private/SkAPI.h"

#include <memory>
#include <optional>

class SkData;
class SkImageGenerator;
class SkOpenTypeSVGDecoder;
class SkTraceMemoryDump;
class SkExecutor;

struct SK_API SkContextOptions {
    /**
     * Executor to handle threaded work within GPU contexts. If this is nullptr, then all work will
     * be done serially on the main thread. To have worker threads assist with various tasks, set
     * this to a valid SkExecutor instance.
     *
     * Within Ganesh: Currently, used for software path rendering, but may be used
     * for other tasks.
     * Within Graphite: Currently, used for Pipeline compilation, but may be used
     * for other tasks. It is up to the client to ensure the SkExecutor remains valid throughout
     * the lifetime of the Context.
     */
    SkExecutor* fExecutor = nullptr;

    /**
     *  Specify the max number of bytes that should be used by the font cache.
     *  If the cache needs to allocate more, it will purge previous entries.
     */
    std::optional<size_t> fFontCacheLimit;

    /**
     *  Set the limit on the number of entries in the font cache. If a new limit is
     *  lower than the previous limit, entries will automatically be purged to meet the new limit.
     */
    std::optional<int> fFontCacheCountLimit;

    /**
     *  Set the limit on the number of entries in the typeface cache.
     */
    std::optional<int> fTypefaceCacheCountLimit;

    /**
     *  The memory usage limit for the resource cache, used for temporary
     *  bitmaps and other resources. Entries are purged from the cache when the memory usage
     *  exceeds this limit.
     */
    std::optional<size_t> fResourceCacheTotalByteLimit;

    /**
     *  When a cacheable entry is very large (e.g. a large scaled bitmap), adding it to the cache
     *  can cause most/all of the existing entries to be purged. To avoid this, the client can set
     *  a limit for a single allocation. If a cacheable entry would have been cached, but its size
     *  exceeds this limit, then we do not attempt to cache it at all.
     */
    std::optional<size_t> fResourceCacheSingleAllocationByteLimit;

    /**
     *  When enabled, internal synchronization (e.g., mutexes) is used for shared caches,
     *  allowing multiple thread workers to safely access resources concurrently.
     */
    bool fThreadSafeSharedResources = false;

    using ImageGeneratorFromEncodedDataFactory =
            std::unique_ptr<SkImageGenerator> (*)(sk_sp<const SkData>);
    /**
     *  To instantiate images from encoded data, first looks at this runtime function-ptr. If it
     *  exists, it is called to create an SkImageGenerator from SkData. If there is no function-ptr
     *  or if it returns nullptr, Skia will call its internal default implementation.
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
    OpenTypeSVGDecoderFactory fOpenTypeSVGDecoderFactory = nullptr;
};

#endif  // SkContextOptions_DEFINED
