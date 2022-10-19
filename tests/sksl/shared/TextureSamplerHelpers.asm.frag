OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %sk_Clockwise %sk_FragColor %c
OpExecutionMode %main OriginUpperLeft
OpName %sk_Clockwise "sk_Clockwise"
OpName %sk_FragColor "sk_FragColor"
OpName %s "s"
OpName %t "t"
OpName %c "c"
OpName %doTextureStuff_h4ssT "doTextureStuff_h4ssT"
OpName %main "main"
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %s Binding 0
OpDecorate %s DescriptorSet 0
OpDecorate %t Binding 1
OpDecorate %t DescriptorSet 0
OpDecorate %c Location 1
OpDecorate %28 RelaxedPrecision
OpDecorate %32 RelaxedPrecision
OpDecorate %33 RelaxedPrecision
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%12 = OpTypeSampler
%_ptr_UniformConstant_12 = OpTypePointer UniformConstant %12
%s = OpVariable %_ptr_UniformConstant_12 UniformConstant
%15 = OpTypeImage %float 2D 0 0 0 1 Unknown
%_ptr_UniformConstant_15 = OpTypePointer UniformConstant %15
%t = OpVariable %_ptr_UniformConstant_15 UniformConstant
%v2float = OpTypeVector %float 2
%_ptr_Input_v2float = OpTypePointer Input %v2float
%c = OpVariable %_ptr_Input_v2float Input
%20 = OpTypeFunction %v4float %12 %15
%26 = OpTypeSampledImage %15
%void = OpTypeVoid
%30 = OpTypeFunction %void
%doTextureStuff_h4ssT = OpFunction %v4float None %20
%21 = OpFunctionParameter %12
%22 = OpFunctionParameter %15
%23 = OpLabel
%25 = OpSampledImage %26 %22 %21
%27 = OpLoad %v2float %c
%24 = OpImageSampleImplicitLod %v4float %25 %27
%28 = OpVectorShuffle %v4float %24 %24 2 1 0 3
OpReturnValue %28
OpFunctionEnd
%main = OpFunction %void None %30
%31 = OpLabel
%32 = OpLoad %12 %s
%33 = OpLoad %15 %t
%34 = OpFunctionCall %v4float %doTextureStuff_h4ssT %32 %33
OpStore %sk_FragColor %34
OpReturn
OpFunctionEnd
