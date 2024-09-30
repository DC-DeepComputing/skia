/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef WindowContext_DEFINED
#define WindowContext_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/core/SkSurfaceProps.h"
#include "include/gpu/ganesh/GrTypes.h"
#include "tools/window/DisplayParams.h"

class GrDirectContext;
class SkSurface;
namespace skgpu::graphite {
class Context;
class Recorder;
}  // namespace skgpu::graphite

namespace skwindow {

class WindowContext {
public:
    WindowContext(const DisplayParams&);

    virtual ~WindowContext();

    virtual sk_sp<SkSurface> getBackbufferSurface() = 0;

    void swapBuffers();

    virtual bool isValid() = 0;

    virtual void resize(int w, int h) = 0;

    virtual void activate(bool isActive) {}

    const DisplayParams& getDisplayParams() { return fDisplayParams; }
    virtual void setDisplayParams(const DisplayParams& params) = 0;

    GrDirectContext* directContext() const { return fContext.get(); }

    // Implemented by Graphite backends
    virtual skgpu::graphite::Context* graphiteContext() const { return nullptr; }
    virtual skgpu::graphite::Recorder* graphiteRecorder() const { return nullptr; }
    virtual void snapRecordingAndSubmit() {}

    int width() const { return fWidth; }
    int height() const { return fHeight; }
    SkISize dimensions() const { return {fWidth, fHeight}; }
    int sampleCount() const { return fSampleCount; }
    int stencilBits() const { return fStencilBits; }

protected:
    virtual bool isGpuContext() { return true;  }

    virtual void onSwapBuffers() = 0;

    sk_sp<GrDirectContext> fContext;

    int               fWidth;
    int               fHeight;
    DisplayParams     fDisplayParams;

    // parameters obtained from the native window
    // Note that the platform .cpp file is responsible for
    // initializing fSampleCount and fStencilBits!
    int               fSampleCount = 1;
    int               fStencilBits = 0;
};

}  // namespace skwindow

#endif
