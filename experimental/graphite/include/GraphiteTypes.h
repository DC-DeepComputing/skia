/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_GraphiteTypes_DEFINED
#define skgpu_GraphiteTypes_DEFINED

#include "include/core/SkTypes.h"

namespace skgpu {

/**
 * Possible 3D APIs that may be used by Graphite.
 */
enum class BackendApi : unsigned {
    kMetal,
    kMock,
};

/**
 * Is the data protected on the GPU or not.
 */
enum class Protected : bool {
    kNo = false,
    kYes = true,
};

/**
 * Graphite uses the depth buffer to enforce painter's order. This type represents the painter's
 * order indices that are written and tested again when drawing. They do not represent "depth" as
 * would be used in a regular 3D rendering API.
 */
using ZValue = uint16_t;

/**
 * An ordinal number that allows draw commands to be re-ordered so long as when they are executed,
 * the read/writes to the color|depth|stencil buffers respect the buffer index sequence.
 */
using BufferIndex = uint16_t;

} // namespace skgpu

#endif // skgpu_GraphiteTypes_DEFINED
