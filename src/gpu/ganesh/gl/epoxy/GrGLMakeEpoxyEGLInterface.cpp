/*
 * Copyright 2024 Igalia S.L.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/gpu/gl/GrGLAssembleInterface.h"
#include "include/gpu/gl/GrGLInterface.h"
#include "src/gpu/ganesh/gl/GrGLCoreFunctions.h"
#include "src/gpu/ganesh/gl/GrGLUtil.h"

#include <epoxy/egl.h>
#include <epoxy/gl.h>

static GrGLFuncPtr epoxy_get_gl_proc(void* ctx, const char name[])
{
    SkASSERT(nullptr == ctx);
    #define M(X) if (0 == strcmp(#X, name)) { return (GrGLFuncPtr) epoxy_ ## X; }
    GR_GL_CORE_FUNCTIONS_EACH(M)
    #undef M
    return epoxy_eglGetProcAddress(name);
}

sk_sp<const GrGLInterface> GrGLMakeEpoxyEGLInterface() {
    return GrGLMakeAssembledInterface(nullptr, epoxy_get_gl_proc);
}
