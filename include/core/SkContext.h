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

class SkContextPriv;
class SkContextPrivConst;
class SkResourceCache;
class SkSharedContext;
class SkStrikeCache;
class SkTraceMemoryDump;
class SkTypefaceCache;
struct SkContextOptions;
class SkRecorder;

namespace SkContexts {

enum SkBackendSelection {
    kAutomatic,  // Default
    kRaster,
    kGPU,
};

}

/**
 * @brief SkContext is a central context object that serves as the mandatory first point of
 * initialization for all Skia clients. SkContext consolidates global state, management of shared
 * resources, and internal caches.
 */
class SK_API SkContext final {
public:
    SkContext(const SkContext&) = delete;
    SkContext(SkContext&&) = delete;
    SkContext& operator=(const SkContext&) = delete;
    SkContext& operator=(SkContext&&) = delete;

    ~SkContext();

    SkContextPriv priv();
    SkContextPrivConst priv() const;

    // Using PassKey pattern to allow only restricted access to SkContextAccessor
    // See //base/types/pass_key.h for Chromium's implementation of the pattern
    class FactoryPassKey {
    private:
        friend class SkContextCtorAccessor;
        FactoryPassKey() = default;
    };

    SkContext(FactoryPassKey, sk_sp<SkSharedContext>, const SkContextOptions&);

    // Must be called in Make() to handle one-time GPU setup operations that can possibly fail and
    // require Context::Make() to return a nullptr.
    bool finishInitialization(FactoryPassKey);

    std::unique_ptr<SkRecorder> makeRecorder(SkContexts::SkBackendSelection selection) const;

private:
    friend class SkContextPrivConst;
    friend class SkContextPriv;

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

    void dumpMemoryStatistics(SkTraceMemoryDump* dump);

    sk_sp<SkSharedContext> fSharedContext;
#if defined(SK_GRAPHITE)
    std::unique_ptr<skgpu::graphite::Context> fGraphiteContext;
#endif
};

#endif  // SkContext_DEFINED
