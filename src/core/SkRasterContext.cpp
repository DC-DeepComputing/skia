/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "include/core/SkRasterContext.h"

#include "include/core/SkTypes.h"
#include "src/core/SkRasterContext_Base.h"
#include "src/core/SkRasterRecorder_Base.h"

std::unique_ptr<const SkRasterContext> SkRasterContext::Make(const SkRasterContext::Options& opts) {
    return std::make_unique<SkRasterContext_Base>();
}

std::unique_ptr<const SkRasterContext> SkRasterContext::MakeWithDefaults() {
    return SkRasterContext::Make({});
}

std::unique_ptr<SkRasterRecorder> SkRasterContext::makeRecorder() const {
    return std::make_unique<SkRasterRecorder_Base>(static_cast<const SkRasterContext_Base*>(this));
}

const SkRasterContext_Base* SkRasterContext_Base::TODO() {
    static auto gContext = SkRasterContext::MakeWithDefaults();
    return static_cast<const SkRasterContext_Base*>(gContext.get());
}
