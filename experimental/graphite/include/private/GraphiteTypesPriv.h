/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_GraphiteTypesPriv_DEFINED
#define skgpu_GraphiteTypesPriv_DEFINED

#include "experimental/graphite/include/GraphiteTypes.h"

namespace skgpu {

/**
 * Is the Texture renderable or not
 */
enum class Renderable : bool {
    kNo = false,
    kYes = true,
};

enum class DepthStencilType {
    kDepthOnly,
    kStencilOnly,
    kDepthStencil,
};

/**
 * What a GPU buffer will be used for
 */
enum class BufferType {
    kVertex,
    kIndex,
    kXferCpuToGpu,
    kXferGpuToCpu,
    kUniform,
};
static const int kBufferTypeCount = static_cast<int>(BufferType::kUniform) + 1;

/**
 * Will a GPU buffers data be read once (or a couple times) on the GPU before being overwritten. Or will the data be kept around for
 * many GPU reads.
 */
enum class TransientGpuBuffer : bool {
    kNo = false,
    kYes = true,
};

} // namespace skgpu

#endif // skgpu_GraphiteTypesPriv_DEFINED
