/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkRasterRecorder_Base_DEFINED
#define SkRasterRecorder_Base_DEFINED

#include "include/core/SkRasterRecorder.h"
#include "src/core/SkRasterContext_Base.h"

class SkRasterRecorder_Base final : public SkRasterRecorder {
public:
    SkRasterRecorder_Base(const SkRasterContext_Base* ctx) : fCtx(ctx) {}

    const SkRasterContext_Base* ctx() const { return fCtx; }

private:
    const SkRasterContext_Base* const fCtx;
};

inline SkRasterRecorder_Base* asRRB(SkRasterRecorder* rr) {
    return static_cast<SkRasterRecorder_Base*>(rr);
}

#endif
