/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skcpu_Recorder_DEFINED
#define skcpu_Recorder_DEFINED

#include "include/core/SkRecorder.h"
#include "include/private/SkAPI.h"

class SkCanvas;
class SkSurface;

namespace skcpu {

class SK_API Recorder : public SkRecorder {
public:
    /** Returns a non-null global context. Can be used as a means of transitioning onto
     * new APIs when a skcpu::Context/Recorder has not been piped down into the code paths
     */
    static Recorder* TODO() {
        return nullptr;
    }

    SkRecorder::Type type() const final { return SkRecorder::Type::kCPU; }

private:
    SkCanvas* makeCaptureCanvas(SkCanvas*) final { return nullptr; }
    void createCaptureBreakpoint(SkSurface*) final {}
};

inline Recorder* AsRecorder(SkRecorder* recorder) {
    if (!recorder) {
        return nullptr;
    }
    if (recorder->type() != SkRecorder::Type::kCPU) {
        return nullptr;
    }
    return static_cast<Recorder*>(recorder);
}

}  // namespace skcpu

#endif
