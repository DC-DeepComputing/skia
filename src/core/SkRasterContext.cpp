/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "include/core/SkRasterContext.h"

#include "include/core/SkSurfaceProps.h"
#include "include/core/SkTypes.h"
#include "src/core/SkRasterContextPriv.h"
#include "src/core/SkRasterRecorderPriv.h"
#include "src/core/SkResourceCache.h"

#include <utility>

std::unique_ptr<const SkRasterContext> SkRasterContext::Make(const SkRasterContext::Options& opts) {
    return std::make_unique<SkRasterContextPriv>();
}

std::unique_ptr<const SkRasterContext> SkRasterContext::MakeWithDefaults() {
    return SkRasterContext::Make({});
}

std::unique_ptr<SkRasterRecorder> SkRasterContext::makeRecorder() const {
    return std::make_unique<SkRasterRecorderPriv>(static_cast<const SkRasterContextPriv*>(this));
}

const SkRasterContextPriv* SkRasterContextPriv::TODO() {
    static auto gContext = SkRasterContext::MakeWithDefaults();
    return static_cast<const SkRasterContextPriv*>(gContext.get());
}
