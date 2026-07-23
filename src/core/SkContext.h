/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkContext_DEFINED
#define SkContext_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/private/SkAPI.h"

#include <memory>

class SkOpenTypeSVGDecoder;
class SkResourceCache;
class SkStrikeCache;
class SkTypefaceCache;
class SkTraceMemoryDump;
class SkContextPriv;
class SkSharedContext;

using OpenTypeSVGDecoderFactory = std::unique_ptr<SkOpenTypeSVGDecoder> (*)(const uint8_t* svg,
                                                                            size_t length);

class SK_API SkContext final {
public:
    SkContext(const SkContext&) = delete;
    SkContext(SkContext&&) = delete;
    SkContext& operator=(const SkContext&) = delete;
    SkContext& operator=(SkContext&&) = delete;

    ~SkContext();

    SkContextPriv priv();
    const SkContextPriv priv() const;

private:
    SkResourceCache* resourceCache() const;
    SkStrikeCache* fontCache() const;
    SkTypefaceCache* typefaceCache() const;

    size_t fontCacheLimit() const;
    size_t setFontCacheLimit(size_t bytes);
    size_t fontCacheUsed() const;

    int fontCacheCountLimit() const;
    int setFontCacheCountLimit(int count);
    int fontCacheCountUsed() const;

    void purgeFontCache();
    void purgePinnedFontCache();

    int typefaceCacheCountLimit() const { return fTypefaceCacheCountLimit; }
    int setTypefaceCacheCountLimit(int count) {
        const int prev = fTypefaceCacheCountLimit;
        fTypefaceCacheCountLimit = count;
        return prev;
    }

    void dumpMemoryStatistics(SkTraceMemoryDump* dump);

    SkContext();

    friend class SkContextPriv;

    sk_sp<SkSharedContext> fSharedContext;

    int fTypefaceCacheCountLimit = 1024;  // historical default value
    OpenTypeSVGDecoderFactory fSVGDecoderFactory = nullptr;
};

#endif  // SkContext_DEFINED
