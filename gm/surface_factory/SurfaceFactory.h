/*
 * Copyright 2023 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SurfaceFactory_DEFINED
#define SurfaceFactory_DEFINED

#include "include/core/SkSurface.h"

#include <string>

// Returns a surface according to the given config name (e.g. "8888", "565", etc.).
sk_sp<SkSurface> make_surface(std::string config, int width, int height);

#endif  // SurfaceFactory_DEFINED
