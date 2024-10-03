/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef GraphiteWindowContext_DEFINED
#define GraphiteWindowContext_DEFINED

#include "include/gpu/graphite/Context.h"
#include "include/gpu/graphite/ContextOptions.h"
#include "include/gpu/graphite/Recorder.h"
#include "src/gpu/graphite/ContextOptionsPriv.h"
#include "tools/graphite/TestOptions.h"
#include "tools/window/DisplayParams.h"
#include "tools/window/WindowContext.h"

namespace skwindow {

struct GraphiteDisplayParams : public DisplayParams {
    GraphiteDisplayParams() { fTestOptions.fContextOptions.fOptionsPriv = &fPriv; }

    GraphiteDisplayParams(const GraphiteDisplayParams& other)
            : fTestOptions(other.fTestOptions), fPriv(other.fPriv) {
        fTestOptions.fContextOptions.fOptionsPriv = &fPriv;
    }

    GraphiteDisplayParams& operator=(const GraphiteDisplayParams& other) {
        fTestOptions = other.fTestOptions;
        fPriv = other.fPriv;
        fTestOptions.fContextOptions.fOptionsPriv = &fPriv;
        return *this;
    }

    skiatest::graphite::TestOptions fTestOptions;
    skgpu::graphite::ContextOptionsPriv fPriv;
};

}  // namespace skwindow

namespace skwindow::internal {

class GraphiteWindowContext : public WindowContext {
public:
    GraphiteWindowContext(sk_sp<DisplayParams>);

    skgpu::graphite::Context* graphiteContext() const override { return fGraphiteContext.get(); }
    skgpu::graphite::Recorder* graphiteRecorder() const override { return fGraphiteRecorder.get(); }
    void snapRecordingAndSubmit() override;

protected:
    std::unique_ptr<skgpu::graphite::Context> fGraphiteContext;
    std::unique_ptr<skgpu::graphite::Recorder> fGraphiteRecorder;
};

}  // namespace skwindow::internal

#endif
