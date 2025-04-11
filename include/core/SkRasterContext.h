/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkRasterContext_DEFINED
#define SkRasterContext_DEFINED

#include "include/core/SkContext.h"
#include "include/core/SkRefCnt.h"
#include "include/private/base/SkAPI.h"

#include <cstddef>
#include <memory>

class SkRasterRecorder;
class SkSurfaceProps;

class SK_API SkRasterContext : public SkContext {
public:
    struct Options {
        // default: 32Mb
        size_t fResourceCacheSize = 0;

        SkSurfaceProps* fSurfaceProps = nullptr;
    };

    Type type() const final { return SkContext::Type::kRaster; }

    std::unique_ptr<SkRasterRecorder> makeRecorder() const;

    static sk_sp<const SkRasterContext> Make(const Options&);
    static sk_sp<const SkRasterContext> MakeWithDefaults();

protected:
    SkRasterContext() = default;
};

#endif
