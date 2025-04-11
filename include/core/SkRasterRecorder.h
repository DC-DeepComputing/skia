/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkRasterRecorder_DEFINED
#define SkRasterRecorder_DEFINED

#include "include/private/base/SkAPI.h"
#include "include/core/SkRecorder.h"

class SK_API SkRasterRecorder : public SkRecorder {
public:
    /** Returns a non-null global context. Can be used as a means of transitioning onto
     * new APIs when a SkRasterContext has not been piped down into the code paths
     */
    static SkRasterRecorder* TODO();

    Type type() const final { return SkRecorder::Type::kRaster; }
};

#endif
