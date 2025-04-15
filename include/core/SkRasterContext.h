/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkRasterContext_DEFINED
#define SkRasterContext_DEFINED

#include "include/core/SkContext.h"
#include "include/private/base/SkAPI.h"

#include <memory>

class SkRasterRecorder;

class SK_API SkRasterContext : public SkContext {
public:
    struct Options {};

    Type type() const final { return SkContext::Type::kRaster; }

    std::unique_ptr<SkRasterRecorder> makeRecorder() const;

    static std::unique_ptr<const SkRasterContext> Make(const Options&);
    static std::unique_ptr<const SkRasterContext> MakeWithDefaults();

protected:
    SkRasterContext() = default;
};

#endif
