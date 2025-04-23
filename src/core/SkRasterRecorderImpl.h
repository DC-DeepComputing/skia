/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkRasterRecorderImpl_DEFINED
#define SkRasterRecorderImpl_DEFINED

#include "include/core/SkRasterRecorder.h"
#include "src/core/SkRasterContextImpl.h"

class SkRasterRecorderImpl final : public SkRasterRecorder {
public:
    SkRasterRecorderImpl(const SkRasterContextImpl* ctx) : fCtx(ctx) {}

    const SkRasterContextImpl* ctx() const { return fCtx; }

private:
    const SkRasterContextImpl* const fCtx;
};

inline SkRasterRecorderImpl* asRRI(SkRasterRecorder* rr) {
    return static_cast<SkRasterRecorderImpl*>(rr);
}

#endif
