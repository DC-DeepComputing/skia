OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %sk_Clockwise %sk_FragColor
OpExecutionMode %main OriginUpperLeft
OpName %sk_Clockwise "sk_Clockwise"
OpName %sk_FragColor "sk_FragColor"
OpName %aSampler_texture "aSampler_texture"
OpName %aSampler_sampler "aSampler_sampler"
OpName %helpers_helper_h4Z "helpers_helper_h4Z"
OpName %helper_h4Z "helper_h4Z"
OpName %main "main"
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %aSampler_texture Binding 2
OpDecorate %aSampler_texture DescriptorSet 1
OpDecorate %aSampler_sampler Binding 3
OpDecorate %aSampler_sampler DescriptorSet 1
OpDecorate %44 RelaxedPrecision
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
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
%18 = OpTypeFunction %v4float %_ptr_UniformConstant_13 %_ptr_UniformConstant_16
%26 = OpTypeSampledImage %13
%float_1 = OpConstant %float 1
%v2float = OpTypeVector %float 2
%29 = OpConstantComposite %v2float %float_1 %float_1
%void = OpTypeVoid
%35 = OpTypeFunction %void
%float_0 = OpConstant %float 0
%42 = OpConstantComposite %v2float %float_0 %float_0
%helpers_helper_h4Z = OpFunction %v4float None %18
%19 = OpFunctionParameter %_ptr_UniformConstant_13
%20 = OpFunctionParameter %_ptr_UniformConstant_16
%21 = OpLabel
%23 = OpLoad %13 %19
%24 = OpLoad %16 %20
%25 = OpSampledImage %26 %23 %24
%22 = OpImageSampleImplicitLod %v4float %25 %29
OpReturnValue %22
OpFunctionEnd
%helper_h4Z = OpFunction %v4float None %18
%30 = OpFunctionParameter %_ptr_UniformConstant_13
%31 = OpFunctionParameter %_ptr_UniformConstant_16
%32 = OpLabel
%33 = OpFunctionCall %v4float %helpers_helper_h4Z %30 %31
OpReturnValue %33
OpFunctionEnd
%main = OpFunction %void None %35
%36 = OpLabel
%38 = OpLoad %13 %aSampler_texture
%39 = OpLoad %16 %aSampler_sampler
%40 = OpSampledImage %26 %38 %39
%37 = OpImageSampleImplicitLod %v4float %40 %42
%43 = OpFunctionCall %v4float %helper_h4Z %aSampler_texture %aSampler_sampler
%44 = OpFAdd %v4float %37 %43
OpStore %sk_FragColor %44
OpReturn
OpFunctionEnd
