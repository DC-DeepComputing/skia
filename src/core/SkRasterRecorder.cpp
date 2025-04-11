/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "include/core/SkRasterRecorder.h"

#include "include/core/SkTypes.h"
#include "include/private/base/SkOnce.h"
#include "src/core/SkRasterContextPriv.h"
#include <memory>

SkRasterRecorder* SkRasterRecorder::TODO() {
    static std::unique_ptr<SkRasterRecorder> gRecorder;
    static SkOnce once;
    once([]() { gRecorder = SkRasterContextPriv::TODO()->makeRecorder(); });
    return gRecorder.get();
}
