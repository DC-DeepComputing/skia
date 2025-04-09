/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkRasterContext_DEFINED
#define SkRasterContext_DEFINED

#include "include/core/SkContext.h"

class SK_API SkRasterContext : public SkContext {
public:
    struct Options {
        // default: 32Mb
        size_t fResourceCacheSize = 0;
    };

    Type type() const final {
        return SkContext::Type::kRaster;
    }

    static sk_sp<SkRasterContext> Make(const Options&);
    static sk_sp<SkRasterContext> MakeWithDefaults();
protected:
    SkRasterContext();
};

#endif
