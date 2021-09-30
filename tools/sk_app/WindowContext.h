/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef WindowContext_DEFINED
#define WindowContext_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/core/SkSurfaceProps.h"
#include "include/gpu/GrTypes.h"
#include "tools/sk_app/DisplayParams.h"

class GrDirectContext;
class SkSurface;
#ifdef SK_GRAPHITE_ENABLED
namespace skgpu {
class Context;
}
#endif

namespace sk_app {

class WindowContext {
public:
    enum class Type {
#ifdef SK_GRAPHITE_ENABLED
        kGraphite,
#endif
        kGanesh,
    };

    WindowContext(const DisplayParams&,
                  Type contextType = Type::kGanesh);

    virtual ~WindowContext();

    virtual sk_sp<SkSurface> getBackbufferSurface() = 0;

    virtual void swapBuffers() = 0;

    virtual bool isValid() = 0;

    virtual void resize(int w, int h) = 0;

    virtual void activate(bool isActive) {}

    const DisplayParams& getDisplayParams() { return fDisplayParams; }
    virtual void setDisplayParams(const DisplayParams& params) = 0;

    GrDirectContext* directContext() const {
        if (fType == Type::kGanesh) {
            return fContext.get();
        } else {
            return nullptr;
        }
    }
#ifdef SK_GRAPHITE_ENABLED
    skgpu::Context* graphiteContext() const {
        if (fType == Type::kGraphite) {
            return fGraphiteContext.get();
        } else {
            return nullptr;
        }
    }
#endif

    int width() const { return fWidth; }
    int height() const { return fHeight; }
    int sampleCount() const { return fSampleCount; }
    int stencilBits() const { return fStencilBits; }

protected:
    virtual bool isGpuContext() { return true;  }

    union {
        sk_sp<GrDirectContext> fContext;
#if SK_GRAPHITE_ENABLED
        sk_sp<skgpu::Context> fGraphiteContext;
#endif
    };
    Type fType = Type::kGanesh;

    int               fWidth;
    int               fHeight;
    DisplayParams     fDisplayParams;

    // parameters obtained from the native window
    // Note that the platform .cpp file is responsible for
    // initializing fSampleCount and fStencilBits!
    int               fSampleCount = 1;
    int               fStencilBits = 0;
};

}   // namespace sk_app

#endif
