/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 * This is private because clients cannot meaningfully make one of these
 * ahead of time (unlike Vulkan). If clients needed to access the sync object,
 * we could move this to include/gpu/ganesh/gl.
 */
#ifndef GrGLBackendSemaphore_DEFINED
#define GrGLBackendSemaphore_DEFINED

#include "include/gpu/GrBackendSemaphore.h"
#include "include/gpu/gl/GrGLTypes.h"
#include "include/private/base/SkAPI.h"

namespace GrBackendSemaphores {
SK_API GrBackendSemaphore MakeGL(GrGLsync);
SK_API GrGLsync GetGLSync(const GrBackendSemaphore&);
}  // namespace GrBackendSemaphores

#endif
