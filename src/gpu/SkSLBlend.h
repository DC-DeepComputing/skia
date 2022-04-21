/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSLBlend_DEFINED
#define SkSLBlend_DEFINED

#include "include/core/SkBlendMode.h"
#include "include/core/SkSpan.h"

namespace SkSLBlend {

/**
 * Returns the name of the built-in blend function for a SkBlendMode.
 */
const char* BlendFuncName(SkBlendMode mode);

/**
 * Returns a pair of "blend function + uniform data" for a particular SkBlendMode.
 * This allows us to use fewer unique functions when generating shaders, e.g. every Porter-Duff
 * blend can use the same function.
 */
struct ReducedBlendModeInfo {
    const char*         fFunction;
    SkSpan<const float> fUniformData;
};

ReducedBlendModeInfo GetReducedBlendModeInfo(SkBlendMode mode);

}  // namespace SkSLBlend

#endif
