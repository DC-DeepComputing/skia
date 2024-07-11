### Compilation failed:

error: SPIR-V validation error: OpFunctionParameter Result Type <id> '14[%_ptr_UniformConstant_13]' does not match the OpTypeFunction parameter type of the same index.
  %22 = OpFunctionParameter %_ptr_UniformConstant_13

               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %sk_FragColor %c
               OpExecutionMode %main OriginUpperLeft
               OpName %sk_FragColor "sk_FragColor"
               OpName %aTexture "aTexture"
               OpName %aSampledTexture_texture "aSampledTexture_texture"
               OpName %aSampledTexture_sampler "aSampledTexture_sampler"
               OpName %c "c"
               OpName %helpers_helper_h4ZT_aSampledTexture "helpers_helper_h4ZT_aSampledTexture"
               OpName %helper_h4TZ_aSampledTexture "helper_h4TZ_aSampledTexture"
               OpName %main "main"
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpDecorate %aTexture Binding 1
               OpDecorate %aTexture DescriptorSet 0
               OpDecorate %aSampledTexture_texture Binding 2
               OpDecorate %aSampledTexture_texture DescriptorSet 0
               OpDecorate %aSampledTexture_sampler Binding 3
               OpDecorate %aSampledTexture_sampler DescriptorSet 0
               OpDecorate %c Location 1
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
         %10 = OpTypeImage %float 2D 0 0 0 2 R32f
%_ptr_UniformConstant_10 = OpTypePointer UniformConstant %10
   %aTexture = OpVariable %_ptr_UniformConstant_10 UniformConstant
         %13 = OpTypeImage %float 2D 0 0 0 1 Unknown
%_ptr_UniformConstant_13 = OpTypePointer UniformConstant %13
%aSampledTexture_texture = OpVariable %_ptr_UniformConstant_13 UniformConstant
         %16 = OpTypeSampler
%_ptr_UniformConstant_16 = OpTypePointer UniformConstant %16
%aSampledTexture_sampler = OpVariable %_ptr_UniformConstant_16 UniformConstant
    %v2float = OpTypeVector %float 2
%_ptr_Input_v2float = OpTypePointer Input %v2float
          %c = OpVariable %_ptr_Input_v2float Input
         %21 = OpTypeFunction %v4float %_ptr_UniformConstant_10
         %30 = OpTypeSampledImage %13
       %void = OpTypeVoid
         %38 = OpTypeFunction %void
%helpers_helper_h4ZT_aSampledTexture = OpFunction %v4float None %21
         %22 = OpFunctionParameter %_ptr_UniformConstant_13
         %23 = OpFunctionParameter %_ptr_UniformConstant_16
         %24 = OpFunctionParameter %_ptr_UniformConstant_10
         %25 = OpLabel
         %27 = OpLoad %13 %22
         %28 = OpLoad %16 %23
         %29 = OpSampledImage %30 %27 %28
         %31 = OpLoad %v2float %c
         %26 = OpImageSampleImplicitLod %v4float %29 %31
               OpReturnValue %26
               OpFunctionEnd
%helper_h4TZ_aSampledTexture = OpFunction %v4float None %21
         %32 = OpFunctionParameter %_ptr_UniformConstant_10
         %33 = OpFunctionParameter %_ptr_UniformConstant_13
         %34 = OpFunctionParameter %_ptr_UniformConstant_16
         %35 = OpLabel
         %36 = OpFunctionCall %v4float %helpers_helper_h4ZT_aSampledTexture %32
               OpReturnValue %36
               OpFunctionEnd
       %main = OpFunction %void None %38
         %39 = OpLabel
         %40 = OpFunctionCall %v4float %helper_h4TZ_aSampledTexture %aTexture
               OpStore %sk_FragColor %40
               OpReturn
               OpFunctionEnd

1 error
