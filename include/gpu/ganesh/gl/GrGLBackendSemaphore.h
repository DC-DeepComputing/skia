/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGLBackendSemaphore_DEFINED
#define GrGLBackendSemaphore_DEFINED

#include "include/gpu/GrBackendSemaphore.h"
#include "include/gpu/gl/GrGLTypes.h"

namespace GrBackendSemaphores {
SK_API GrBackendSemaphore MakeGL(GrGLsync);
SK_API GrGLsync getGLSync(const GrBackendSemaphore&);
}

#endif