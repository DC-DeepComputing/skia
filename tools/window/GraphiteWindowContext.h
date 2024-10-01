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
#include "tools/window/WindowContext.h"

namespace skwindow {

struct GraphiteTestOptions {
    GraphiteTestOptions() { fTestOptions.fContextOptions.fOptionsPriv = &fPriv; }

    GraphiteTestOptions(const GraphiteTestOptions& other)
            : fTestOptions(other.fTestOptions), fPriv(other.fPriv) {
        fTestOptions.fContextOptions.fOptionsPriv = &fPriv;
    }

    GraphiteTestOptions& operator=(const GraphiteTestOptions& other) {
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
    GraphiteWindowContext(const DisplayParams&);

    skgpu::graphite::Context* graphiteContext() const override { return fGraphiteContext.get(); }
    skgpu::graphite::Recorder* graphiteRecorder() const override { return fGraphiteRecorder.get(); }
    void snapRecordingAndSubmit() override;

protected:
    std::unique_ptr<skgpu::graphite::Context> fGraphiteContext;
    std::unique_ptr<skgpu::graphite::Recorder> fGraphiteRecorder;
};

}  // namespace skwindow::internal

#endif