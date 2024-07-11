### Compilation failed:

error: SPIR-V validation error: Too many OpFunctionParameters for 2: expected 0 based on the function's type
  %21 = OpFunctionParameter %_ptr_UniformConstant_13

               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %sk_FragColor
               OpExecutionMode %main OriginUpperLeft
               OpName %sk_FragColor "sk_FragColor"
               OpName %aSampler_texture "aSampler_texture"
               OpName %aSampler_sampler "aSampler_sampler"
               OpName %anotherSampler_texture "anotherSampler_texture"
               OpName %anotherSampler_sampler "anotherSampler_sampler"
               OpName %helpers_helper_h4Z_aSampler "helpers_helper_h4Z_aSampler"
               OpName %helpers_helper_h4Z_anotherSampler "helpers_helper_h4Z_anotherSampler"
               OpName %helper_h4Z_aSampler "helper_h4Z_aSampler"
               OpName %helper_h4Z_anotherSampler "helper_h4Z_anotherSampler"
               OpName %helper2_h4ZZ_anotherSampler_aSampler "helper2_h4ZZ_anotherSampler_aSampler"
               OpName %main "main"
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpDecorate %aSampler_texture Binding 2
               OpDecorate %aSampler_texture DescriptorSet 1
               OpDecorate %aSampler_sampler Binding 3
               OpDecorate %aSampler_sampler DescriptorSet 1
               OpDecorate %anotherSampler_texture Binding 4
               OpDecorate %anotherSampler_texture DescriptorSet 1
               OpDecorate %anotherSampler_sampler Binding 5
               OpDecorate %anotherSampler_sampler DescriptorSet 1
               OpDecorate %57 RelaxedPrecision
               OpDecorate %68 RelaxedPrecision
               OpDecorate %70 RelaxedPrecision
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
         %13 = OpTypeImage %float 2D 0 0 0 1 Unknown
%_ptr_UniformConstant_13 = OpTypePointer UniformConstant %13
%aSampler_texture = OpVariable %_ptr_UniformConstant_13 UniformConstant
         %16 = OpTypeSampler
%_ptr_UniformConstant_16 = OpTypePointer UniformConstant %16
%aSampler_sampler = OpVariable %_ptr_UniformConstant_16 UniformConstant
%anotherSampler_texture = OpVariable %_ptr_UniformConstant_13 UniformConstant
%anotherSampler_sampler = OpVariable %_ptr_UniformConstant_16 UniformConstant
         %20 = OpTypeFunction %v4float
         %28 = OpTypeSampledImage %13
    %float_1 = OpConstant %float 1
    %v2float = OpTypeVector %float 2
         %31 = OpConstantComposite %v2float %float_1 %float_1
       %void = OpTypeVoid
         %59 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
         %66 = OpConstantComposite %v2float %float_0 %float_0
%helpers_helper_h4Z_aSampler = OpFunction %v4float None %20
         %21 = OpFunctionParameter %_ptr_UniformConstant_13
         %22 = OpFunctionParameter %_ptr_UniformConstant_16
         %23 = OpLabel
         %25 = OpLoad %13 %21
         %26 = OpLoad %16 %22
         %27 = OpSampledImage %28 %25 %26
         %24 = OpImageSampleImplicitLod %v4float %27 %31
               OpReturnValue %24
               OpFunctionEnd
%helpers_helper_h4Z_anotherSampler = OpFunction %v4float None %20
         %32 = OpFunctionParameter %_ptr_UniformConstant_13
         %33 = OpFunctionParameter %_ptr_UniformConstant_16
         %34 = OpLabel
         %36 = OpLoad %13 %32
         %37 = OpLoad %16 %33
         %38 = OpSampledImage %28 %36 %37
         %35 = OpImageSampleImplicitLod %v4float %38 %31
               OpReturnValue %35
               OpFunctionEnd
%helper_h4Z_aSampler = OpFunction %v4float None %20
         %39 = OpFunctionParameter %_ptr_UniformConstant_13
         %40 = OpFunctionParameter %_ptr_UniformConstant_16
         %41 = OpLabel
         %42 = OpFunctionCall %v4float %helpers_helper_h4Z_aSampler
               OpReturnValue %42
               OpFunctionEnd
%helper_h4Z_anotherSampler = OpFunction %v4float None %20
         %43 = OpFunctionParameter %_ptr_UniformConstant_13
         %44 = OpFunctionParameter %_ptr_UniformConstant_16
         %45 = OpLabel
         %46 = OpFunctionCall %v4float %helpers_helper_h4Z_anotherSampler
               OpReturnValue %46
               OpFunctionEnd
%helper2_h4ZZ_anotherSampler_aSampler = OpFunction %v4float None %20
         %47 = OpFunctionParameter %_ptr_UniformConstant_13
         %48 = OpFunctionParameter %_ptr_UniformConstant_16
         %49 = OpFunctionParameter %_ptr_UniformConstant_13
         %50 = OpFunctionParameter %_ptr_UniformConstant_16
         %51 = OpLabel
         %53 = OpLoad %13 %47
         %54 = OpLoad %16 %48
         %55 = OpSampledImage %28 %53 %54
         %52 = OpImageSampleImplicitLod %v4float %55 %31
         %56 = OpFunctionCall %v4float %helper_h4Z_anotherSampler
         %57 = OpFAdd %v4float %52 %56
               OpReturnValue %57
               OpFunctionEnd
       %main = OpFunction %void None %59
         %60 = OpLabel
         %62 = OpLoad %13 %aSampler_texture
         %63 = OpLoad %16 %aSampler_sampler
         %64 = OpSampledImage %28 %62 %63
         %61 = OpImageSampleImplicitLod %v4float %64 %66
         %67 = OpFunctionCall %v4float %helper_h4Z_aSampler
         %68 = OpFAdd %v4float %61 %67
         %69 = OpFunctionCall %v4float %helper2_h4ZZ_anotherSampler_aSampler
         %70 = OpFAdd %v4float %68 %69
               OpStore %sk_FragColor %70
               OpReturn
               OpFunctionEnd

1 error
