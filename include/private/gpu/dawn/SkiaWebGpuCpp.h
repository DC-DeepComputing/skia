/*
* Copyright 2023 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#ifndef SkiaWebGpu_DEFINED
#define SkiaWebGpu_DEFINED

#include "include/core/SkTypes.h"

// IWYU pragma: begin_exports

#if SKIA_IMPLEMENTATION
#include "include/third_party/dawn/webgpu/webgpu_cpp.h"
#else
#include <webgpu/webgpu_cpp.h>  // NO_G3_REWRITE
#endif

// IWYU pragma: end_exports

#endif
