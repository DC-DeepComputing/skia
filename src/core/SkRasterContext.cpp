/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "include/core/SkRasterContext.h"

#include "include/core/SkTypes.h"
#include "src/core/SkRasterContextImpl.h"
#include "src/core/SkRasterRecorderImpl.h"

std::unique_ptr<const SkRasterContext> SkRasterContext::Make(const SkRasterContext::Options& opts) {
    return std::make_unique<SkRasterContextImpl>();
}

std::unique_ptr<const SkRasterContext> SkRasterContext::MakeWithDefaults() {
    return SkRasterContext::Make({});
}

std::unique_ptr<SkRasterRecorder> SkRasterContext::makeRecorder() const {
    return std::make_unique<SkRasterRecorderImpl>(static_cast<const SkRasterContextImpl*>(this));
}

const SkRasterContextImpl* SkRasterContextImpl::TODO() {
    static auto gContext = SkRasterContext::MakeWithDefaults();
    return static_cast<const SkRasterContextImpl*>(gContext.get());
}
