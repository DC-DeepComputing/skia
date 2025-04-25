/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skcpu_Recorder_DEFINED
#define skcpu_Recorder_DEFINED

#include "include/core/SkRecorder.h"
#include "include/private/base/SkAPI.h"

namespace skcpu {

class SK_API Recorder : public SkRecorder {
public:
    /** Returns a non-null global context. Can be used as a means of transitioning onto
     * new APIs when a skcpu::Context/Recorder has not been piped down into the code paths
     */
    static Recorder* TODO();

    SkRecorder::Type type() const final { return SkRecorder::Type::kRaster; }
};

}  // namespace skcpu

#endif
