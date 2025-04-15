/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkRasterRecorderPriv_DEFINED
#define SkRasterRecorderPriv_DEFINED

#include "include/core/SkRasterRecorder.h"
#include "src/core/SkRasterContextPriv.h"

class SkRasterRecorderPriv final : public SkRasterRecorder {
public:
    SkRasterRecorderPriv(const SkRasterContextPriv* ctx) : fCtx(ctx) {}

    const SkRasterContextPriv* ctx() const { return fCtx; }

    SkRasterRecorderPriv* rasterRecorder() override {
        return this;
    }

private:
    const SkRasterContextPriv* const fCtx;
};

#endif
