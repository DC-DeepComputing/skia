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
    SkRasterRecorderPriv(sk_sp<const SkRasterContextPriv> ctx): fCtx(ctx) {}

    sk_sp<const SkRasterContextPriv> ctx() const {
        return fCtx;
    }
private:
    const sk_sp<const SkRasterContextPriv> fCtx;
};


inline SkRasterRecorderPriv* asRecorderPriv(SkRasterRecorder* r) {
    return static_cast<SkRasterRecorderPriv*>(r);
}

#endif
