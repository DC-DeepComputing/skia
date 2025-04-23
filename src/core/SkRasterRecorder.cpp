/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "include/core/SkRasterRecorder.h"

#include "include/core/SkTypes.h"
#include "src/core/SkRasterContextImpl.h"

#include <memory>

SkRasterRecorder* SkRasterRecorder::TODO() {
    static std::unique_ptr<SkRasterRecorder> gRecorder =
            SkRasterContextImpl::TODO()->makeRecorder();
    return gRecorder.get();
}
