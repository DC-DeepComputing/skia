/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSharedContext_DEFINED
#define SkSharedContext_DEFINED

#include "include/core/SkRefCnt.h"

#include <memory>

class SkStrikeCache;
class SkSynchronizedResourceCache;
class SkTypefaceCache;

class SkSharedContext : public SkRefCnt {
public:
    SkSharedContext();

    const SkSynchronizedResourceCache* synchronizedResourceCache() const { return fResourceCache.get(); }
    SkSynchronizedResourceCache* synchronizedResourceCache() { return fResourceCache.get(); }

    const SkStrikeCache* fontCache() const { return fFontCache.get(); }
    SkStrikeCache* fontCache() { return fFontCache.get(); }

    const SkTypefaceCache* typefaceCache() const { return fTypefaceCache.get(); }
    SkTypefaceCache* typefaceCache() { return fTypefaceCache.get(); }

protected:
    ~SkSharedContext() override;

private:
    std::unique_ptr<SkSynchronizedResourceCache> fResourceCache;
    std::unique_ptr<SkStrikeCache> fFontCache;
    std::unique_ptr<SkTypefaceCache> fTypefaceCache;
};

#endif  // SkSharedContext_DEFINED
