/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/codegen/SkSLSPIRVtoHLSL.h"

#include <spirv_hlsl.hpp>

/*
 * This translation unit serves as a bridge between Skia/SkSL and SPIRV-Cross.
 * Each library is built with a separate copy of spirv.h (or spirv.hpp), so we
 * avoid conflicts by never including both in the same cpp.
 */

namespace SkSL {

void SPIRVtoHLSL(SkSpan<const uint32_t> spirv, std::string* hlsl, uint32_t shaderModel) {
    spirv_cross::CompilerHLSL hlslCompiler(spirv.data(), spirv.size());

    spirv_cross::CompilerGLSL::Options optionsGLSL;
    // Force all uninitialized variables to be 0, otherwise they will fail to compile
    // by FXC.
    optionsGLSL.force_zero_initialized_variables = true;

    spirv_cross::CompilerHLSL::Options optionsHLSL;
    optionsHLSL.shader_model = shaderModel;
    // PointCoord and PointSize are not supported in HLSL
    optionsHLSL.point_coord_compat = true;
    optionsHLSL.point_size_compat = true;

    hlslCompiler.set_common_options(optionsGLSL);
    hlslCompiler.set_hlsl_options(optionsHLSL);

    if (shaderModel <= 50) {
        spirv_cross::ShaderResources resources = hlslCompiler.get_shader_resources();
        for (const auto& resource : resources.storage_buffers) {
            hlslCompiler.set_decoration(resource.id, spv::DecorationNonWritable);
        }

        for (spv::ExecutionModel stage : {spv::ExecutionModelVertex, spv::ExecutionModelFragment}) {
            // Set 0, Binding 0: IntrinsicUniforms (CBV b0)
            spirv_cross::HLSLResourceBinding b0;
            b0.stage = stage;
            b0.desc_set = 0;
            b0.binding = 0;
            b0.cbv.register_binding = 0;
            hlslCompiler.add_hlsl_resource_binding(b0);

            // Set 0, Binding 1: CombinedUniforms (CBV b1 if UBO, SRV t16 if SSBO)
            spirv_cross::HLSLResourceBinding b1;
            b1.stage = stage;
            b1.desc_set = 0;
            b1.binding = 1;
            b1.cbv.register_binding = 1;
            b1.srv.register_binding = 16;
            hlslCompiler.add_hlsl_resource_binding(b1);

            // Set 0, Binding 2: StorageBuffer (SRV t17)
            spirv_cross::HLSLResourceBinding b2;
            b2.stage = stage;
            b2.desc_set = 0;
            b2.binding = 2;
            b2.srv.register_binding = 17;
            hlslCompiler.add_hlsl_resource_binding(b2);

            // Set 1, Binding 0..15: Textures and Samplers (SRVs t0..t15, Samplers s0..s15)
            for (uint32_t i = 0; i < 16; ++i) {
                spirv_cross::HLSLResourceBinding tex;
                tex.stage = stage;
                tex.desc_set = 1;
                tex.binding = i;
                tex.srv.register_binding = i;
                tex.sampler.register_binding = i;
                hlslCompiler.add_hlsl_resource_binding(tex);
            }
        }
    }

    hlsl->assign(hlslCompiler.compile());
}

}  // namespace SkSL
