/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSLBlend_DEFINED
#define SkSLBlend_DEFINED

#include "include/core/SkBlendMode.h"

namespace SkSLBlend {

/**
 * Returns the name of the built-in blend function for a SkBlendMode.
 */
const char* BlendFuncName(SkBlendMode mode);

}  // namespace SkSLBlend

#endif
