/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSharedContext_DEFINED
#define SkSharedContext_DEFINED

#include "include/core/SkRefCnt.h"

#include <memory>

class SkOpenTypeSVGDecoder;
class SkStrikeCache;
class SkSynchronizedResourceCache;
class SkTypefaceCache;

using OpenTypeSVGDecoderFactory = std::unique_ptr<SkOpenTypeSVGDecoder> (*)(const uint8_t* svg,
                                                                            size_t length);

class SkSharedContext : public SkRefCnt {
public:
    SkSharedContext();
    ~SkSharedContext() override;

    const SkSynchronizedResourceCache* synchronizedResourceCache() const {
        return fResourceCache.get();
    }
    SkSynchronizedResourceCache* synchronizedResourceCache() { return fResourceCache.get(); }

    const SkStrikeCache* fontCache() const { return fFontCache.get(); }
    SkStrikeCache* fontCache() { return fFontCache.get(); }

    const SkTypefaceCache* typefaceCache() const { return fTypefaceCache.get(); }
    SkTypefaceCache* typefaceCache() { return fTypefaceCache.get(); }

    int fTypefaceCacheCountLimit = 1024;  // historical default value
    OpenTypeSVGDecoderFactory fSVGDecoderFactory;

private:
    std::unique_ptr<SkSynchronizedResourceCache> fResourceCache;
    std::unique_ptr<SkStrikeCache> fFontCache;
    std::unique_ptr<SkTypefaceCache> fTypefaceCache;
};

#endif  // SkSharedContext_DEFINED
