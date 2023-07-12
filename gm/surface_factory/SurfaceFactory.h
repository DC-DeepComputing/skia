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

// Abstract class to create and manage surfaces.
class SurfaceFactory {
public:
    // Constructs a SurfaceFactory for the given config name (e.g. "8888", "565", "gles").
    static std::unique_ptr<SurfaceFactory> FromConfig(std::string config, int width, int height);

    // Returns a surface created from the given config
    virtual sk_sp<SkSurface> getSurface() = 0;
    virtual void flush() = 0;

    virtual ~SurfaceFactory() = default;
};

#endif  // SurfaceFactory_DEFINED
