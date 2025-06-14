/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_VulkanSpirvTransforms_DEFINED
#define skgpu_graphite_VulkanSpirvTransforms_DEFINED

#include "src/sksl/codegen/SkSLNativeShader.h"

namespace skgpu::graphite {

struct SPIRVTransformOptions {
    // Adjust the SPIR-V to support loading from input attachments when multisampled.
    bool fMultisampleInputLoad = false;
};

SkSL::NativeShader TransformSPIRV(const SkSL::NativeShader& spirv,
                                  const SPIRVTransformOptions& options);

}  // namespace skgpu::graphite

#endif  // skgpu_graphite_VulkanSpirvTransforms_DEFINED
